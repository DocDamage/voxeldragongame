#include "Player/WyrmPlayerController.h"
#include "Player/WyrmCharacter.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "WYRMFALL.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputCoreTypes.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"

AWyrmPlayerController::AWyrmPlayerController()
{
    bShowMouseCursor = false;
    bClickMoveEnabled = true;
    bMovementLocked = false;
}

AWyrmPlayerController* AWyrmPlayerController::SpawnWyrmPlayerController(UObject* WorldContextObject, const FTransform& SpawnTransform)
{
    if (!WorldContextObject) { return nullptr; }
    UWorld* World = WorldContextObject->GetWorld();
    if (!World && GEngine)
    {
        World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    }
    if (!World) { return nullptr; }
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return World->SpawnActor<AWyrmPlayerController>(AWyrmPlayerController::StaticClass(), SpawnTransform, SpawnParams);
}

void AWyrmPlayerController::SetClickMoveEnabled(bool bEnabled)
{
    bClickMoveEnabled = bEnabled;
    if (!bClickMoveEnabled)
    {
        StopMovement();
    }
}

void AWyrmPlayerController::SetMovementLocked(bool bLocked)
{
    bMovementLocked = bLocked;
    if (bMovementLocked)
    {
        StopMovement();
        JumpReleased();
    }
    if (auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        Body->SetMovementLocked(bLocked);
    }
}

EWyrmCameraMode AWyrmPlayerController::GetActiveCameraMode() const
{
    if (const auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        return Body->GetCameraMode();
    }
    return EWyrmCameraMode::ThirdPerson;
}

void AWyrmPlayerController::SetActiveCameraMode(EWyrmCameraMode NewMode)
{
    if (auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        StopMovement();
        Body->SetCameraMode(NewMode);
        RefreshCursor();
    }
}

void AWyrmPlayerController::CaptureControlState(FWyrmControlState& OutState) const
{
    OutState.CameraMode = GetActiveCameraMode();
    OutState.bClickMoveEnabled = bClickMoveEnabled;
    OutState.bMovementLocked = bMovementLocked;
}

void AWyrmPlayerController::RestoreControlState(const FWyrmControlState& InState)
{
    SetClickMoveEnabled(InState.bClickMoveEnabled);
    SetMovementLocked(InState.bMovementLocked);
    SetActiveCameraMode(InState.CameraMode);
}

bool AWyrmPlayerController::RequestClickMoveToDestination(const FVector& DestinationLocation)
{
    const auto* Body = Cast<AWyrmCharacter>(GetPawn());
    if (IsMoveInputIgnored() || UGameplayStatics::IsGamePaused(this) || bMovementLocked || (Body && Body->IsMovementLocked()))
    {
        StopMovement();
        return false;
    }
    if (!Body || !bClickMoveEnabled || Body->GetCameraMode() != EWyrmCameraMode::TopDown)
    {
        return false;
    }
    StopMovement();
    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!Nav) { return false; }
    FNavLocation ProjectedNav;
    if (!Nav->ProjectPointToNavigation(DestinationLocation, ProjectedNav, FVector(100.f, 100.f, 250.f)))
    {
        UE_LOG(LogWyrmfall, Warning, TEXT("Click blocked: no usable navigation at destination."));
        return false;
    }
    UNavigationPath* Path = Nav->FindPathToLocationSynchronously(GetWorld(), Body->GetActorLocation(), ProjectedNav.Location, const_cast<AWyrmCharacter*>(Body));
    if (!Path || !Path->IsValid() || Path->IsPartial())
    {
        UE_LOG(LogWyrmfall, Warning, TEXT("Click blocked: no complete path."));
        return false;
    }
    UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, ProjectedNav.Location);
    return true;
}
void AWyrmPlayerController::BuildInputContext()
{
    if (Context) { return; }
    Context = NewObject<UInputMappingContext>(this, TEXT("IMC_DevelopmentHumanoid"));
    const auto MakeAction = [this](const TCHAR* Name, EInputActionValueType Type)
    {
        UInputAction* Action = NewObject<UInputAction>(this, FName(Name));
        Action->ValueType = Type;
        Action->AccumulationBehavior = EInputActionAccumulationBehavior::Cumulative;
        return Action;
    };
    MoveAction = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D);
    LookAction = MakeAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
    PadLookAction = MakeAction(TEXT("IA_PadLook"), EInputActionValueType::Axis2D);
    CameraAction = MakeAction(TEXT("IA_Camera"), EInputActionValueType::Boolean);
    ClickToggleAction = MakeAction(TEXT("IA_DevClickToggle"), EInputActionValueType::Boolean);
    ClickAction = MakeAction(TEXT("IA_DevClick"), EInputActionValueType::Boolean);
    JumpAction = MakeAction(TEXT("IA_Jump"), EInputActionValueType::Boolean);
    const auto MapMove = [this](FKey Key, bool bNegate, bool bSwizzle)
    {
        FEnhancedActionKeyMapping& Mapping = Context->MapKey(MoveAction, Key);
        if (bNegate) { Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(Context)); }
        if (bSwizzle)
        {
            UInputModifierSwizzleAxis* Modifier = NewObject<UInputModifierSwizzleAxis>(Context);
            Modifier->Order = EInputAxisSwizzle::YXZ;
            Mapping.Modifiers.Add(Modifier);
        }
    };
    MapMove(EKeys::W, false, true); MapMove(EKeys::S, true, true);
    MapMove(EKeys::D, false, false); MapMove(EKeys::A, true, false);
    Context->MapKey(MoveAction, EKeys::Gamepad_Left2D).Modifiers.Add(NewObject<UInputModifierDeadZone>(Context));
    Context->MapKey(LookAction, EKeys::Mouse2D);
    Context->MapKey(PadLookAction, EKeys::Gamepad_Right2D).Modifiers.Add(NewObject<UInputModifierDeadZone>(Context));
    Context->MapKey(CameraAction, EKeys::C);
    Context->MapKey(CameraAction, EKeys::Gamepad_RightThumbstick);
    Context->MapKey(ClickToggleAction, EKeys::F6); // diagnostic toggle, not final UX
    Context->MapKey(ClickAction, EKeys::LeftMouseButton);
    Context->MapKey(JumpAction, EKeys::SpaceBar);
    Context->MapKey(JumpAction, EKeys::Gamepad_FaceButton_Bottom);
}
void AWyrmPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    BuildInputContext();
    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
    if (!ensureMsgf(Input, TEXT("WYRMFALL requires Enhanced Input; inspect DefaultInput.ini"))) { return; }
    Input->ClearBindingsForObject(this); // Idempotent re-setup; preserve other owners' bindings.
    Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWyrmPlayerController::Move);
    Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWyrmPlayerController::Look);
    Input->BindAction(PadLookAction, ETriggerEvent::Triggered, this, &AWyrmPlayerController::LookGamepad);
    Input->BindAction(CameraAction, ETriggerEvent::Started, this, &AWyrmPlayerController::SwitchCamera);
    Input->BindAction(ClickToggleAction, ETriggerEvent::Started, this, &AWyrmPlayerController::ToggleClickMove);
    Input->BindAction(ClickAction, ETriggerEvent::Started, this, &AWyrmPlayerController::ClickMove);
    Input->BindAction(JumpAction, ETriggerEvent::Started, this, &AWyrmPlayerController::JumpPressed);
    Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &AWyrmPlayerController::JumpReleased);
    Input->BindAction(JumpAction, ETriggerEvent::Canceled, this, &AWyrmPlayerController::JumpReleased);
}
#if WITH_EDITOR
void AWyrmPlayerController::WyrmDevRebindInput()
{
    const UEnhancedInputComponent* BeforeInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (!BeforeInput || !GetPawn())
    {
        UE_LOG(LogWyrmfall, Warning, TEXT("BOOT-01 rebind NOT_RUN: requires an initialized possessed controller."));
        return;
    }
    const APawn* BeforePawn = GetPawn();
    const int32 BeforeCount = BeforeInput->GetActionEventBindings().Num();
    SetupInputComponent();
    SetupInputComponent();
    const UEnhancedInputComponent* AfterInput = Cast<UEnhancedInputComponent>(InputComponent);
    UE_LOG(LogWyrmfall, Display,
        TEXT("BOOT-01 rebind: pawn=%s same_pawn=%d same_component=%d bindings_before=%d bindings_after=%d. Exercise C/F6/jump to check event behavior."),
        *GetNameSafe(GetPawn()), GetPawn() == BeforePawn, AfterInput == BeforeInput,
        BeforeCount, AfterInput ? AfterInput->GetActionEventBindings().Num() : -1);
}
#endif
void AWyrmPlayerController::BeginPlay()
{
    Super::BeginPlay();
    BuildInputContext();
    if (ULocalPlayer* Local = GetLocalPlayer())
    {
        if (auto* Input = Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        { Input->AddMappingContext(Context, 0); }
    }
    SetControlRotation(FRotator(-18.f, 0.f, 0.f));
    RefreshCursor();
}
void AWyrmPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
    if (ULocalPlayer* Local = GetLocalPlayer())
    {
        if (auto* Input = Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        { Input->RemoveMappingContext(Context); }
    }
    Super::EndPlay(Reason);
}
void AWyrmPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    RefreshCursor();
}
void AWyrmPlayerController::OnUnPossess()
{
    StopMovement();
    JumpReleased();
    Super::OnUnPossess();
    RefreshCursor();
}
void AWyrmPlayerController::RefreshCursor()
{
    const auto* Body = Cast<AWyrmCharacter>(GetPawn());
    bShowMouseCursor = Body && Body->GetCameraMode() == EWyrmCameraMode::TopDown;
    // Diagnostic world only: final menu focus and UI consumption are WP-03.
    SetInputMode(FInputModeGameOnly());
}
void AWyrmPlayerController::Move(const FInputActionValue& Value)
{
    if (IsMoveInputIgnored() || UGameplayStatics::IsGamePaused(this) || bMovementLocked) { return; }
    const FVector2D Axis = Value.Get<FVector2D>().GetClampedToMaxSize(1.f);
    if (Axis.IsNearlyZero()) { return; }

    if (auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        if (Body->IsMovementLocked()) { return; }
        StopMovement();
        const float Yaw = Body->GetCameraMode() == EWyrmCameraMode::TopDown ? 0.f : GetControlRotation().Yaw;
        const FRotationMatrix Basis(FRotator(0.f, Yaw, 0.f));
        Body->AddMovementInput(Basis.GetUnitAxis(EAxis::X), Axis.Y);
        Body->AddMovementInput(Basis.GetUnitAxis(EAxis::Y), Axis.X);
    }
    else if (auto* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        if (Dragon->IsTransitioningForm()) { return; }
        StopMovement();
        const float Yaw = GetControlRotation().Yaw;
        const FRotationMatrix Basis(FRotator(0.f, Yaw, 0.f));
        Dragon->AddMovementInput(Basis.GetUnitAxis(EAxis::X), Axis.Y);
        Dragon->AddMovementInput(Basis.GetUnitAxis(EAxis::Y), Axis.X);
    }
}
void AWyrmPlayerController::Look(const FInputActionValue& Value)
{
    if (IsLookInputIgnored() || UGameplayStatics::IsGamePaused(this) || bMovementLocked) { return; }
    if (const auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        if (Body->GetCameraMode() == EWyrmCameraMode::TopDown || Body->IsMovementLocked()) { return; }
    }
    const FVector2D Axis = Value.Get<FVector2D>();
    AddYawInput(Axis.X);
    AddPitchInput(-Axis.Y);
}
void AWyrmPlayerController::LookGamepad(const FInputActionValue& Value)
{
    Look(FInputActionValue(Value.Get<FVector2D>() * GetWorld()->GetDeltaSeconds() * 90.f));
}
void AWyrmPlayerController::SwitchCamera()
{
    if (auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    { StopMovement(); Body->ToggleCamera(); RefreshCursor(); }
    else if (auto* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        if (Dragon->IsTransitioningForm()) { return; }
        StopMovement();
        static bool bDragonTopDown = false;
        bDragonTopDown = !bDragonTopDown;
        Dragon->SetFlightCameraMode(bDragonTopDown);
    }
}
void AWyrmPlayerController::ToggleClickMove() { SetClickMoveEnabled(!bClickMoveEnabled); }
void AWyrmPlayerController::ClickMove()
{
    const auto* Body = Cast<AWyrmCharacter>(GetPawn());
    if (IsMoveInputIgnored() || UGameplayStatics::IsGamePaused(this) || bMovementLocked || (Body && Body->IsMovementLocked()))
    {
        StopMovement();
        return;
    }
    if (!Body || !bClickMoveEnabled || Body->GetCameraMode() != EWyrmCameraMode::TopDown) { return; }
    // Cancel the old destination even when the replacement click hits nothing.
    StopMovement();
    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit) || !Hit.bBlockingHit) { return; }
    RequestClickMoveToDestination(Hit.ImpactPoint);
}
void AWyrmPlayerController::JumpPressed()
{
    if (IsMoveInputIgnored() || UGameplayStatics::IsGamePaused(this) || bMovementLocked) { return; }
    if (auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        if (Body->IsMovementLocked()) { return; }
        StopMovement();
        Body->Jump();
    }
    else if (auto* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        if (Dragon->IsTransitioningForm()) { return; }
        StopMovement();
        if (Dragon->GetFlightState() == EWyrmDragonFlightState::Grounded)
        {
            Dragon->TakeOff();
        }
        else if (Dragon->IsInFlight())
        {
            FVector LandingLoc;
            FString Reason;
            if (Dragon->CanLand(LandingLoc, Reason))
            {
                Dragon->Land();
            }
            else
            {
                Dragon->AddMovementInput(FVector::UpVector, 1.0f);
            }
        }
        else
        {
            Dragon->Jump();
        }
    }
}
void AWyrmPlayerController::JumpReleased()
{
    if (auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        Body->StopJumping();
    }
    else if (auto* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        Dragon->StopJumping();
    }
}

void AWyrmPlayerController::PrimaryAttack()
{
    if (IsMoveInputIgnored() || UGameplayStatics::IsGamePaused(this) || bMovementLocked)
    {
        return;
    }
    if (auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        if (Body->IsMovementLocked()) { return; }
        Body->PerformPrimaryAttack();
    }
    else if (auto* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        if (Dragon->IsTransitioningForm()) { return; }
        Dragon->PerformPrimaryAttack();
    }
}

void AWyrmPlayerController::SecondaryAttack()
{
    if (IsMoveInputIgnored() || UGameplayStatics::IsGamePaused(this) || bMovementLocked)
    {
        return;
    }
    if (auto* Body = Cast<AWyrmCharacter>(GetPawn()))
    {
        if (Body->IsMovementLocked()) { return; }
        Body->PerformSecondaryAttack();
    }
    else if (auto* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        if (Dragon->IsTransitioningForm()) { return; }
        Dragon->PerformSecondaryAttack();
    }
}

bool AWyrmPlayerController::TransferControlToDragon(AWyrmDragonCharacter* Dragon)
{
    if (!Dragon)
    {
        return false;
    }
    AWyrmCharacter* Body = Cast<AWyrmCharacter>(GetPawn());
    if (!Body)
    {
        return false;
    }
    return Dragon->StartDirectControl(this, Body);
}

bool AWyrmPlayerController::ReturnControlToHumanoid()
{
    if (AWyrmDragonCharacter* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        return Dragon->EndDirectControl(this);
    }
    return false;
}

bool AWyrmPlayerController::MountDragon(AWyrmDragonCharacter* Dragon)
{
    if (!Dragon)
    {
        return false;
    }
    if (AWyrmCharacter* Humanoid = Cast<AWyrmCharacter>(GetPawn()))
    {
        return Dragon->MountHumanoid(Humanoid);
    }
    return false;
}

bool AWyrmPlayerController::DismountDragon()
{
    if (AWyrmDragonCharacter* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        FVector DismountLoc;
        return Dragon->DismountHumanoid(DismountLoc);
    }
    return false;
}

bool AWyrmPlayerController::TakeOffDragon()
{
    if (AWyrmDragonCharacter* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        return Dragon->TakeOff();
    }
    return false;
}

bool AWyrmPlayerController::LandDragon()
{
    if (AWyrmDragonCharacter* Dragon = Cast<AWyrmDragonCharacter>(GetPawn()))
    {
        return Dragon->Land();
    }
    return false;
}
