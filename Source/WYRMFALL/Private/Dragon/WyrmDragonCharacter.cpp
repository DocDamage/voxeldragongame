#include "Dragon/WyrmDragonCharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/WyrmAttributeSet.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Player/WyrmCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/DamageEvents.h"
#include "Region/WyrmRegion01Subsystem.h"

AWyrmDragonCharacter::AWyrmDragonCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule
    GetCapsuleComponent()->InitCapsuleSize(45.f, 65.f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Movement settings
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 600.f;

    // GAS Authority
    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    Attributes = CreateDefaultSubobject<UWyrmAttributeSet>(TEXT("Attributes"));
    if (Attributes)
    {
        Attributes->InitMaxHealth(1800.f);
        Attributes->InitHealth(1800.f);
        Attributes->InitMaxFocus(100.f);
        Attributes->InitFocus(100.f);
        Attributes->InitPower(25.f);
        Attributes->InitArmor(10.f);
    }

    // Direct control camera boom & follow camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 650.f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void AWyrmDragonCharacter::EnsureAbilitySystemInitialized()
{
    if (AbilitySystem)
    {
        if (!AbilitySystem->GetAvatarActor())
        {
            AbilitySystem->InitAbilityActorInfo(this, this);
        }
        if (Attributes && !AbilitySystem->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
        {
            AbilitySystem->AddAttributeSetSubobject(Attributes.Get());
        }
    }
}

void AWyrmDragonCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    EnsureAbilitySystemInitialized();
}

void AWyrmDragonCharacter::BeginPlay()
{
    Super::BeginPlay();
    EnsureAbilitySystemInitialized();

    if (AbilitySystem && Attributes)
    {
        AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attributes->GetHealthAttribute())
            .AddUObject(this, &AWyrmDragonCharacter::HandleHealthChanged);
    }

    // Initialize attributes according to role
    if (Attributes)
    {
        if (CurrentRole == EWyrmDragonRole::HostileBoss)
        {
            Attributes->InitMaxHealth(1800.f);
            Attributes->SetMaxHealth(1800.f);
            Attributes->InitHealth(1800.f);
            Attributes->SetHealth(1800.f);
            Attributes->InitMaxFocus(100.f);
            Attributes->SetMaxFocus(100.f);
            Attributes->InitFocus(100.f);
            Attributes->SetFocus(100.f);
            Attributes->InitPower(25.f);
            Attributes->SetPower(25.f);
            Attributes->InitArmor(10.f);
            Attributes->SetArmor(10.f);
        }
        else if (CurrentRole == EWyrmDragonRole::AlliedCompanion)
        {
            Attributes->InitMaxHealth(420.f);
            Attributes->SetMaxHealth(420.f);
            if (!bHasBondReceipt && Attributes->GetHealth() <= 0.f)
            {
                Attributes->InitHealth(210.f);
                Attributes->SetHealth(210.f);
            }
            Attributes->InitMaxFocus(100.f);
            Attributes->SetMaxFocus(100.f);
            Attributes->InitFocus(100.f);
            Attributes->SetFocus(100.f);
            Attributes->InitPower(20.f);
            Attributes->SetPower(20.f);
            Attributes->InitArmor(8.f);
            Attributes->SetArmor(8.f);
        }
    }

    SetupModularMeshes();
    ApplyFormDimensions();
}

void AWyrmDragonCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (AreaAttackCooldownRemaining > 0.f)
    {
        AreaAttackCooldownRemaining = FMath::Max(0.f, AreaAttackCooldownRemaining - DeltaSeconds);
    }

    if (FormTransitionCooldownRemaining > 0.f)
    {
        FormTransitionCooldownRemaining = FMath::Max(0.f, FormTransitionCooldownRemaining - DeltaSeconds);
    }

    if (bIsTransitioningForm)
    {
        FormTransitionTimeRemaining -= DeltaSeconds;
        if (FormTransitionTimeRemaining <= 0.f)
        {
            CompleteFormTransition();
        }
    }

    if (CurrentRole == EWyrmDragonRole::AlliedCompanion)
    {
        if (bIsDirectlyControlled)
        {
            CheckTetherStatus();
        }
        else
        {
            UpdateCompanionAI(DeltaSeconds);
        }
    }
}

void AWyrmDragonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AWyrmDragonCharacter::SetDragonRole(EWyrmDragonRole NewRole)
{
    EnsureAbilitySystemInitialized();
    CurrentRole = NewRole;
    if (CurrentRole == EWyrmDragonRole::HostileBoss)
    {
        bHasBondReceipt = false;
        if (Attributes)
        {
            Attributes->InitMaxHealth(1800.f);
            Attributes->SetMaxHealth(1800.f);
            Attributes->InitHealth(1800.f);
            Attributes->SetHealth(1800.f);
            Attributes->InitMaxFocus(100.f);
            Attributes->SetMaxFocus(100.f);
            Attributes->InitFocus(100.f);
            Attributes->SetFocus(100.f);
            Attributes->InitPower(25.f);
            Attributes->SetPower(25.f);
            Attributes->InitArmor(10.f);
            Attributes->SetArmor(10.f);
        }
        CurrentForm = EWyrmDragonForm::TrueForm;
        ApplyFormDimensions();
    }
    else if (CurrentRole == EWyrmDragonRole::DefeatedAlive)
    {
        PerformBossDefeat();
    }
    else if (CurrentRole == EWyrmDragonRole::AlliedCompanion)
    {
        if (Attributes)
        {
            Attributes->InitMaxHealth(420.f);
            Attributes->SetMaxHealth(420.f);
        }
        CurrentForm = EWyrmDragonForm::CompanionForm;
        ApplyFormDimensions();
    }
}

bool AWyrmDragonCharacter::PerformBossDefeat()
{
    EnsureAbilitySystemInitialized();
    CurrentRole = EWyrmDragonRole::DefeatedAlive;
    if (Attributes)
    {
        Attributes->InitHealth(0.f);
        Attributes->SetHealth(0.f);
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
    }
    CurrentCombatTarget = nullptr;

    OnDragonDefeated.Broadcast(this);
    return true;
}

bool AWyrmDragonCharacter::BondWithHumanoid(AWyrmCharacter* Humanoid)
{
    if (bHasBondReceipt)
    {
        // One-way conversion receipt already issued: replay does NOT re-heal or re-bond
        return false;
    }

    if (!Humanoid)
    {
        return false;
    }

    // REG-07 Consent Sequence: In Region 01, claim must be broken before voluntary bond is permitted
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
            {
                if (!Region01->HasFact(FName(TEXT("verdance.claim_broken"))))
                {
                    return false;
                }
            }
        }
    }

    EnsureAbilitySystemInitialized();

    // Role conversion
    CurrentRole = EWyrmDragonRole::AlliedCompanion;
    bHasBondReceipt = true;

    if (Attributes)
    {
        Attributes->InitMaxHealth(420.f);
        Attributes->SetMaxHealth(420.f);
        // Explicitly recover to at least 50% allied maximum on first bond (210 HP)
        Attributes->InitHealth(210.f);
        Attributes->SetHealth(210.f);
        Attributes->InitMaxFocus(100.f);
        Attributes->SetMaxFocus(100.f);
        Attributes->InitFocus(100.f);
        Attributes->SetFocus(100.f);
        Attributes->InitPower(20.f);
        Attributes->SetPower(20.f);
        Attributes->InitArmor(8.f);
        Attributes->SetArmor(8.f);
    }

    WaitingHumanoid = Humanoid;
    CurrentOrder = EWyrmCompanionOrder::Follow;
    CurrentForm = EWyrmDragonForm::CompanionForm;
    ApplyFormDimensions();

    return true;
}

bool AWyrmDragonCharacter::CanOfferVoluntaryBond(const AActor* Interactor) const
{
    if (!Interactor || bHasBondReceipt)
    {
        return false;
    }

    if (CurrentRole != EWyrmDragonRole::DefeatedAlive)
    {
        return false;
    }

    const float DistSq = FVector::DistSquared(GetActorLocation(), Interactor->GetActorLocation());
    if (DistSq > FMath::Square(500.f))
    {
        return false;
    }

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
            {
                if (!Region01->HasFact(FName(TEXT("verdance.claim_broken"))))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool AWyrmDragonCharacter::InteractVoluntaryBond(AWyrmCharacter* Humanoid)
{
    if (!CanOfferVoluntaryBond(Humanoid))
    {
        return false;
    }

    return BondWithHumanoid(Humanoid);
}

void AWyrmDragonCharacter::SetDragonForm(EWyrmDragonForm NewForm)
{
    CurrentForm = NewForm;
    bIsTransitioningForm = false;
    FormTransitionTimeRemaining = 0.f;
    PendingForm = CurrentForm;
    ApplyFormDimensions();
}

bool AWyrmDragonCharacter::CanChangeForm(EWyrmDragonForm TargetForm, FString& OutReason) const
{
    if (TargetForm == CurrentForm)
    {
        OutReason = TEXT("Dragon is already in requested form");
        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] CanChangeForm false: %s"), *OutReason);
        return false;
    }

    if (!HasSupportedRigProfile())
    {
        OutReason = FString::Printf(TEXT("Dragon rig '%s' has no validated Heartfold profile"), *DragonId.ToString());
        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] CanChangeForm false: %s"), *OutReason);
        return false;
    }

    if (CurrentRole != EWyrmDragonRole::AlliedCompanion)
    {
        OutReason = TEXT("Only allied companion can change form");
        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] CanChangeForm false: %s"), *OutReason);
        return false;
    }

    if (!Attributes || Attributes->GetCurrentHealth() <= 0.f)
    {
        OutReason = TEXT("Dragon must be living to change form");
        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] CanChangeForm false: %s"), *OutReason);
        return false;
    }

    if (CurrentFlightState != EWyrmDragonFlightState::Grounded)
    {
        OutReason = TEXT("Dragon cannot change form while in flight; must be grounded");
        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] CanChangeForm false: %s"), *OutReason);
        return false;
    }

    if (MountedRider.IsValid())
    {
        OutReason = TEXT("Dragon cannot change form while a rider is mounted");
        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] CanChangeForm false: %s"), *OutReason);
        return false;
    }

    if (FormTransitionCooldownRemaining > 0.f)
    {
        OutReason = TEXT("Form transition is on cooldown");
        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] CanChangeForm false: %s (Remaining=%.2f)"), *OutReason, FormTransitionCooldownRemaining);
        return false;
    }

    if (bIsTransitioningForm)
    {
        OutReason = TEXT("Form transition already in progress");
        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] CanChangeForm false: %s"), *OutReason);
        return false;
    }

    if (TargetForm == EWyrmDragonForm::TrueForm)
    {
        // 3D Volumetric clearance test for True Form capsule (Radius=120, HalfHeight=160)
        // Companion capsule half height is 35. Center of True Form capsule aligns with base:
        // Base Z = CurrentLocation.Z - 35.f. TrueForm Center Z = Base Z + 160.f = CurrentLocation.Z + 125.f
        // Lift test capsule by a floor clearance tolerance so ground contact geometry and minor terrain slopes do not register as an obstacle.
        const float FloorClearanceTolerance = 20.0f;
        FVector TargetCenter = GetActorLocation() + FVector(0.f, 0.f, 125.f + FloorClearanceTolerance);
        FCollisionShape TrueFormCapsule = FCollisionShape::MakeCapsule(120.f, 160.f - FloorClearanceTolerance);
        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WyrmDragonFormClearance), false, this);
        if (WaitingHumanoid.IsValid())
        {
            QueryParams.AddIgnoredActor(WaitingHumanoid.Get());
        }
        if (MountedRider.IsValid())
        {
            QueryParams.AddIgnoredActor(MountedRider.Get());
        }

        FCollisionResponseParams ResponseParams;
        bool bBlocked = false;
        if (UWorld* World = GetWorld())
        {
            FCollisionObjectQueryParams ObjParams;
            ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
            ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
            ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);

            TArray<FOverlapResult> Overlaps;
            if (World->OverlapMultiByObjectType(Overlaps, TargetCenter, FQuat::Identity, ObjParams, TrueFormCapsule, QueryParams))
            {
                for (const FOverlapResult& Overlap : Overlaps)
                {
                    AActor* OverlapActor = Overlap.GetActor();
                    if (OverlapActor && OverlapActor != this && OverlapActor != WaitingHumanoid.Get() && OverlapActor != MountedRider.Get())
                    {
                        if (Overlap.Component.IsValid() && Overlap.Component->GetCollisionResponseToChannel(ECC_Pawn) != ECR_Block)
                        {
                            continue;
                        }
                        bBlocked = true;
                        OutReason = FString::Printf(TEXT("Not enough room for True Form (Blocked by Object: %s)"), *OverlapActor->GetName());
                        FVector CompLoc = Overlap.Component.IsValid() ? Overlap.Component->GetComponentLocation() : FVector::ZeroVector;
                        FBoxSphereBounds CompBounds = Overlap.Component.IsValid() ? Overlap.Component->Bounds : FBoxSphereBounds();
                        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] %s (Actor=%s Class=%s Comp=%s Loc=%s BoxMin=%s BoxMax=%s TargetCenter=%s CapsuleRadius=%.1f CapsuleHalfHeight=%.1f)"),
                            *OutReason, *OverlapActor->GetName(), *OverlapActor->GetClass()->GetName(),
                            Overlap.Component.IsValid() ? *Overlap.Component->GetName() : TEXT("None"),
                            *CompLoc.ToString(), *CompBounds.GetBox().Min.ToString(), *CompBounds.GetBox().Max.ToString(),
                            *TargetCenter.ToString(), TrueFormCapsule.GetCapsuleRadius(), TrueFormCapsule.GetCapsuleHalfHeight());
                        break;
                    }
                }
            }
            if (!bBlocked && World->OverlapMultiByChannel(Overlaps, TargetCenter, FQuat::Identity, ECC_Pawn, TrueFormCapsule, QueryParams, ResponseParams))
            {
                for (const FOverlapResult& Overlap : Overlaps)
                {
                    AActor* OverlapActor = Overlap.GetActor();
                    if (OverlapActor && OverlapActor != this && OverlapActor != WaitingHumanoid.Get() && OverlapActor != MountedRider.Get())
                    {
                        if (Overlap.Component.IsValid() && Overlap.Component->GetCollisionResponseToChannel(ECC_Pawn) != ECR_Block)
                        {
                            continue;
                        }
                        bBlocked = true;
                        OutReason = FString::Printf(TEXT("Not enough room for True Form (Blocked by Pawn: %s)"), *OverlapActor->GetName());
                        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] %s (Actor=%s Class=%s Loc=%s)"), *OutReason, *OverlapActor->GetName(), *OverlapActor->GetClass()->GetName(), *OverlapActor->GetActorLocation().ToString());
                        break;
                    }
                }
            }
            if (!bBlocked && World->OverlapMultiByChannel(Overlaps, TargetCenter, FQuat::Identity, ECC_WorldStatic, TrueFormCapsule, QueryParams, ResponseParams))
            {
                for (const FOverlapResult& Overlap : Overlaps)
                {
                    AActor* OverlapActor = Overlap.GetActor();
                    if (OverlapActor && OverlapActor != this && OverlapActor != WaitingHumanoid.Get() && OverlapActor != MountedRider.Get())
                    {
                        if (Overlap.Component.IsValid() && Overlap.Component->GetCollisionResponseToChannel(ECC_Pawn) != ECR_Block)
                        {
                            continue;
                        }
                        bBlocked = true;
                        OutReason = FString::Printf(TEXT("Not enough room for True Form (Blocked by Static: %s)"), *OverlapActor->GetName());
                        UE_LOG(LogTemp, Warning, TEXT("[WyrmDragon] %s (Actor=%s Class=%s Loc=%s)"), *OutReason, *OverlapActor->GetName(), *OverlapActor->GetClass()->GetName(), *OverlapActor->GetActorLocation().ToString());
                        break;
                    }
                }
            }
        }

        if (bBlocked)
        {
            if (OutReason.IsEmpty())
            {
                OutReason = TEXT("Not enough room for True Form");
            }
            return false;
        }
    }

    OutReason = TEXT("");
    UE_LOG(LogTemp, Log, TEXT("[WyrmDragon] CanChangeForm TRUE at %s"), *GetActorLocation().ToString());
    return true;
}

bool AWyrmDragonCharacter::RequestFormChange(EWyrmDragonForm TargetForm)
{
    if (TargetForm == CurrentForm)
    {
        return false;
    }

    FString Reason;
    if (!CanChangeForm(TargetForm, Reason))
    {
        return false;
    }

    bIsTransitioningForm = true;
    PendingForm = TargetForm;
    FormTransitionTimeRemaining = FormTransitionDuration; // 1.0s
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
    }
    return true;
}

void AWyrmDragonCharacter::InterruptFormTransition()
{
    if (!bIsTransitioningForm)
    {
        return;
    }

    bIsTransitioningForm = false;
    FormTransitionTimeRemaining = 0.f;
    // Commit 4.0s recovery cooldown without stat duplication or healing
    FormTransitionCooldownRemaining = FormTransitionCooldownDuration;
}

void AWyrmDragonCharacter::CompleteFormTransition()
{
    bIsTransitioningForm = false;
    FormTransitionTimeRemaining = 0.f;

    // The world may have changed during the one-second presentation. Revalidate
    // before changing collision, mesh scale, or navigation dimensions.
    FString Reason;
    if (!CanChangeForm(PendingForm, Reason))
    {
        FormTransitionCooldownRemaining = FormTransitionCooldownDuration;
        UE_LOG(LogTemp, Warning, TEXT("Form transition canceled before commit: %s"), *Reason);
        PendingForm = CurrentForm;
        return;
    }

    CurrentForm = PendingForm;
    FormTransitionCooldownRemaining = FormTransitionCooldownDuration; // 4.0s
    ApplyFormDimensions();
}

float AWyrmDragonCharacter::GetFormTransitionProgress() const
{
    if (!bIsTransitioningForm || FormTransitionDuration <= 0.f)
    {
        return 0.f;
    }

    return FMath::Clamp(1.0f - (FormTransitionTimeRemaining / FormTransitionDuration), 0.f, 1.0f);
}

void AWyrmDragonCharacter::SetTownModeEnabled(bool bEnabled)
{
    bTownModeEnabled = bEnabled;
    if (bTownModeEnabled)
    {
        CurrentCombatTarget = nullptr;
        if (CurrentOrder == EWyrmCompanionOrder::AttackTarget)
        {
            CurrentOrder = EWyrmCompanionOrder::Follow;
        }
    }
}

float AWyrmDragonCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = (DamageAmount > 0.f) ? DamageAmount : Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (ActualDamage > 0.f)
    {
        if (Attributes)
        {
            float CurrentHP = Attributes->GetCurrentHealth();
            Attributes->SetCurrentHealth(FMath::Max(0.f, CurrentHP - ActualDamage));
        }
        if (bIsTransitioningForm)
        {
            InterruptFormTransition();
        }
    }
    return ActualDamage;
}

void AWyrmDragonCharacter::ApplyFormDimensions()
{
    // Baseline Rig is Green Dragon (/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local.Hip-Local)
    // Scale and envelope rules are rig-specific (DRG-15)
    float TargetScale = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 0.009f : 0.035f;
    float CapsuleRadius = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 30.f : 120.f;
    float CapsuleHalfHeight = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 35.f : 160.f;

    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
    }
    if (GetMesh())
    {
        GetMesh()->SetRelativeScale3D(FVector(TargetScale));
    }

    if (GetCharacterMovement())
    {
        if (CurrentForm == EWyrmDragonForm::CompanionForm)
        {
            GroundSpeed = CompanionGroundSpeed;
            GetCharacterMovement()->MaxWalkSpeed = CompanionGroundSpeed;
        }
        else
        {
            GroundSpeed = TrueFormGroundSpeed;
            if (IsInFlight())
            {
                GetCharacterMovement()->MaxFlySpeed = FlightSpeed;
            }
            else
            {
                GetCharacterMovement()->MaxWalkSpeed = TrueFormGroundSpeed;
            }
        }
    }
}

void AWyrmDragonCharacter::IssueOrder(EWyrmCompanionOrder NewOrder, AActor* TargetActor)
{
    if (bTownModeEnabled && NewOrder == EWyrmCompanionOrder::AttackTarget)
    {
        CurrentOrder = EWyrmCompanionOrder::Follow;
        CurrentCombatTarget = nullptr;
        return;
    }

    CurrentOrder = NewOrder;
    if (TargetActor)
    {
        CurrentCombatTarget = TargetActor;
    }

    if (CurrentOrder == EWyrmCompanionOrder::Hold && GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
    }
}

bool AWyrmDragonCharacter::PerformPrimaryAttack(AActor* TargetActor)
{
    if (bIsTransitioningForm || CurrentRole == EWyrmDragonRole::DefeatedAlive || CurrentRole == EWyrmDragonRole::Recovering)
    {
        return false;
    }

    AActor* Victim = TargetActor ? TargetActor : CurrentCombatTarget.Get();

    float SweepRadius = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 45.f : 120.f;
    float ForwardReach = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 120.f : 250.f;

    // If no explicit victim, trace ahead for a hostile target
    if (!Victim)
    {
        FVector Start = GetActorLocation() + GetActorForwardVector() * 50.f;
        FVector End = Start + GetActorForwardVector() * ForwardReach;

        TArray<FHitResult> Hits;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(SweepRadius);
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        if (WaitingHumanoid.IsValid())
        {
            QueryParams.AddIgnoredActor(WaitingHumanoid.Get());
        }

        if (GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, QueryParams))
        {
            for (const FHitResult& Hit : Hits)
            {
                if (Hit.GetActor() && Hit.GetActor() != this && Hit.GetActor() != WaitingHumanoid.Get())
                {
                    Victim = Hit.GetActor();
                    break;
                }
            }
        }
    }

    if (!Victim)
    {
        return false;
    }

    // Damage amount: CompanionForm deals 9 raw damage; TrueForm deals 24 (allied) or 28 (boss)
    float DamageAmount;
    if (CurrentRole == EWyrmDragonRole::AlliedCompanion)
    {
        DamageAmount = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 9.f : 24.f;
    }
    else
    {
        DamageAmount = 28.f;
    }

    IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Victim);
    UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
    if (TargetASC && AbilitySystem)
    {
        UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, TargetASC, DamageAmount);
    }
    else
    {
        UGameplayStatics::ApplyDamage(Victim, DamageAmount, GetController(), this, UDamageType::StaticClass());
    }

    return true;
}

bool AWyrmDragonCharacter::PerformSecondaryAttack(AActor* TargetActor)
{
    if (bIsTransitioningForm)
    {
        return false;
    }

    if (AreaAttackCooldownRemaining > 0.f)
    {
        return false;
    }

    if (CurrentRole == EWyrmDragonRole::DefeatedAlive || CurrentRole == EWyrmDragonRole::Recovering)
    {
        return false;
    }

    TSet<AActor*> AffectedActors;
    if (TargetActor && TargetActor != this && TargetActor != WaitingHumanoid.Get())
    {
        AffectedActors.Add(TargetActor);
    }

    // Area sweep: CompanionForm deals 6 damage with 250cm radius; TrueForm deals 18 damage with 600cm radius
    const float AreaRadius = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 250.f : 600.f;
    const float AreaDamage = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 6.f : 18.f;

    TArray<FHitResult> Hits;
    FVector Center = GetActorLocation();
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AreaRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    if (WaitingHumanoid.IsValid())
    {
        QueryParams.AddIgnoredActor(WaitingHumanoid.Get());
    }

    GetWorld()->SweepMultiByChannel(Hits, Center, Center, FQuat::Identity, ECC_Pawn, Sphere, QueryParams);

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor && HitActor != this && HitActor != WaitingHumanoid.Get())
        {
            AffectedActors.Add(HitActor);
        }
    }

    for (AActor* HitActor : AffectedActors)
    {
        IAbilitySystemInterface* HitASI = Cast<IAbilitySystemInterface>(HitActor);
        UAbilitySystemComponent* HitASC = HitASI ? HitASI->GetAbilitySystemComponent() : nullptr;
        if (HitASC && AbilitySystem)
        {
            UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, HitASC, AreaDamage);
        }
        else
        {
            UGameplayStatics::ApplyDamage(HitActor, AreaDamage, GetController(), this, UDamageType::StaticClass());
        }
    }

    // Commit 6-second cooldown (shared across forms)
    AreaAttackCooldownRemaining = AreaAttackCooldownDuration;
    return true;
}

bool AWyrmDragonCharacter::StartDirectControl(APlayerController* InPlayerController, AWyrmCharacter* HumanoidBody)
{
    if (!InPlayerController || !HumanoidBody)
    {
        return false;
    }

    if (CurrentRole != EWyrmDragonRole::AlliedCompanion)
    {
        return false;
    }

    if (bIsTransitioningForm)
    {
        return false;
    }

    WaitingHumanoid = HumanoidBody;
    bIsDirectlyControlled = true;

    // Lock humanoid body in place while preserving visible mesh, inventory, and collision
    HumanoidBody->SetMovementLocked(true);

    // Transfer possession
    InPlayerController->UnPossess();
    InPlayerController->Possess(this);

    CurrentTetherStatus = EWyrmTetherStatus::WithinTether;
    return true;
}

bool AWyrmDragonCharacter::EndDirectControl(APlayerController* InPlayerController)
{
    if (!bIsDirectlyControlled)
    {
        return false;
    }

    bIsDirectlyControlled = false;

    if (InPlayerController)
    {
        InPlayerController->UnPossess();
    }

    if (WaitingHumanoid.IsValid())
    {
        WaitingHumanoid->SetMovementLocked(false);
        if (InPlayerController)
        {
            InPlayerController->Possess(WaitingHumanoid.Get());
        }
    }

    CurrentOrder = EWyrmCompanionOrder::Follow;
    const float Dist = GetDistanceToHumanoid();
    if (Dist >= TetherLimitDistance)
    {
        CurrentTetherStatus = EWyrmTetherStatus::LimitReached;
    }
    else if (Dist >= TetherWarningDistance)
    {
        CurrentTetherStatus = EWyrmTetherStatus::Warning;
    }
    else
    {
        CurrentTetherStatus = EWyrmTetherStatus::WithinTether;
    }
    return true;
}

float AWyrmDragonCharacter::GetDistanceToHumanoid() const
{
    if (!WaitingHumanoid.IsValid())
    {
        return 0.f;
    }
    return FVector::Dist(GetActorLocation(), WaitingHumanoid->GetActorLocation());
}

void AWyrmDragonCharacter::HandleWaitingBodyDamaged(float DamageAmount)
{
    // Remote risk: any accepted damaging hit on the waiting body requests immediate safe return of player control
    if (bIsDirectlyControlled)
    {
        APlayerController* PC = Cast<APlayerController>(GetController());
        EndDirectControl(PC);
    }
}

bool AWyrmDragonCharacter::CanMount(AWyrmCharacter* Humanoid, FString& OutReason) const
{
    if (!Humanoid)
    {
        OutReason = TEXT("Invalid humanoid");
        return false;
    }

    if (CurrentRole != EWyrmDragonRole::AlliedCompanion)
    {
        OutReason = TEXT("Dragon is not an allied companion");
        return false;
    }

    if (!HasSupportedRigProfile())
    {
        OutReason = FString::Printf(TEXT("Dragon rig '%s' has no validated mount profile"), *DragonId.ToString());
        return false;
    }

    if (CurrentForm == EWyrmDragonForm::CompanionForm)
    {
        OutReason = TEXT("Compact dragon cannot be mounted; True Form required");
        return false;
    }

    if (CurrentFlightState != EWyrmDragonFlightState::Grounded)
    {
        OutReason = TEXT("Dragon is in flight; must be grounded to mount");
        return false;
    }

    if (MountedRider.IsValid())
    {
        OutReason = TEXT("Dragon already has a mounted rider");
        return false;
    }

    OutReason = TEXT("");
    return true;
}

bool AWyrmDragonCharacter::MountHumanoid(AWyrmCharacter* Humanoid)
{
    FString RejectionReason;
    if (!CanMount(Humanoid, RejectionReason))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mount rejected: %s"), *RejectionReason);
        return false;
    }

    MountedRider = Humanoid;

    // Ignore collisions between humanoid capsule and dragon capsule
    if (Humanoid->GetCapsuleComponent() && GetCapsuleComponent())
    {
        Humanoid->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
        GetCapsuleComponent()->IgnoreActorWhenMoving(Humanoid, true);
    }

    // Lock humanoid locomotion
    Humanoid->SetMovementLocked(true);

    // Attach humanoid directly to dragon at mount offset
    Humanoid->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    Humanoid->SetActorRelativeLocation(MountSocketOffset);
    Humanoid->SetActorRelativeRotation(FRotator::ZeroRotator);

    // Transfer player controller possession to dragon
    if (APlayerController* PC = Cast<APlayerController>(Humanoid->GetController()))
    {
        PC->UnPossess();
        PC->Possess(this);
    }

    // Configure camera boom for riding
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = 1100.f;
    }

    return true;
}

bool AWyrmDragonCharacter::CanDismount(FVector& OutGroundLocation, FString& OutReason) const
{
    if (!MountedRider.IsValid())
    {
        OutReason = TEXT("No rider mounted");
        return false;
    }

    if (CurrentFlightState != EWyrmDragonFlightState::Grounded)
    {
        OutReason = TEXT("Cannot dismount in flight; land first");
        return false;
    }

    // Search adjacent ground for dismount placement
    const FVector DismountOffset = (GetActorRightVector() * 200.f) - (GetActorForwardVector() * 80.f);
    const FVector TraceStart = GetActorLocation() + DismountOffset + FVector(0.f, 0.f, 150.f);
    const FVector TraceEnd = TraceStart - FVector(0.f, 0.f, 400.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (MountedRider.IsValid())
    {
        Params.AddIgnoredActor(MountedRider.Get());
    }

    if (GetWorld() && (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params) ||
                       GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params)))
    {
        OutGroundLocation = Hit.Location + FVector(0.f, 0.f, 90.f);
    }
    else
    {
        OutGroundLocation = GetActorLocation() + DismountOffset;
    }

    OutReason = TEXT("");
    return true;
}

bool AWyrmDragonCharacter::DismountHumanoid(FVector& OutDismountLocation)
{
    FString Reason;
    if (!CanDismount(OutDismountLocation, Reason))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dismount rejected: %s"), *Reason);
        return false;
    }

    AWyrmCharacter* Humanoid = MountedRider.Get();
    if (!Humanoid)
    {
        return false;
    }

    // Detach humanoid and position at ground location
    Humanoid->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Humanoid->SetActorLocation(OutDismountLocation);
    Humanoid->SetActorRotation(GetActorRotation());

    // Restore collision and locomotion
    if (Humanoid->GetCapsuleComponent() && GetCapsuleComponent())
    {
        Humanoid->GetCapsuleComponent()->IgnoreActorWhenMoving(this, false);
        GetCapsuleComponent()->IgnoreActorWhenMoving(Humanoid, false);
    }
    Humanoid->SetMovementLocked(false);

    // Transfer controller back to humanoid
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->UnPossess();
        PC->Possess(Humanoid);
    }

    MountedRider.Reset();

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = 650.f;
    }

    return true;
}

bool AWyrmDragonCharacter::CanTakeOff(FString& OutReason) const
{
    if (CurrentRole == EWyrmDragonRole::DefeatedAlive || CurrentRole == EWyrmDragonRole::Recovering)
    {
        OutReason = TEXT("Dragon is incapacitated");
        return false;
    }

    if (!HasSupportedRigProfile())
    {
        OutReason = FString::Printf(TEXT("Dragon rig '%s' has no validated flight profile"), *DragonId.ToString());
        return false;
    }

    if (CurrentForm == EWyrmDragonForm::CompanionForm)
    {
        OutReason = TEXT("Companion form cannot fly; True Form required for flight");
        return false;
    }

    if (CurrentFlightState != EWyrmDragonFlightState::Grounded)
    {
        OutReason = TEXT("Already airborne");
        return false;
    }

    // Sweep upward to verify overhead clearance
    const float HalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 65.f;
    const FVector Start = GetActorLocation() + FVector(0.f, 0.f, HalfHeight + 50.f);
    const FVector End = Start + FVector(0.f, 0.f, TakeoffClearanceHeight);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (MountedRider.IsValid())
    {
        Params.AddIgnoredActor(MountedRider.Get());
    }

    const FCollisionShape SweepBox = FCollisionShape::MakeBox(FVector(WingSpanSweepRadius, WingSpanSweepRadius, 40.f));
    if (GetWorld() && GetWorld()->SweepSingleByChannel(Hit, Start, End, GetActorQuat(), ECC_WorldStatic, SweepBox, Params))
    {
        if (Hit.bBlockingHit && !Hit.bStartPenetrating)
        {
            OutReason = TEXT("Overhead clearance blocked");
            return false;
        }
    }

    OutReason = TEXT("");
    return true;
}

bool AWyrmDragonCharacter::TakeOff()
{
    FString Reason;
    if (!CanTakeOff(Reason))
    {
        UE_LOG(LogTemp, Warning, TEXT("Takeoff rejected: %s"), *Reason);
        return false;
    }

    CurrentFlightState = EWyrmDragonFlightState::Flying;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Flying);
        GetCharacterMovement()->MaxFlySpeed = FlightSpeed;
        GetCharacterMovement()->BrakingDecelerationFlying = 2000.f;
    }

    LaunchCharacter(FVector(0.f, 0.f, 350.f), false, true);
    return true;
}

bool AWyrmDragonCharacter::CanLand(FVector& OutLandingLocation, FString& OutReason) const
{
    if (CurrentFlightState != EWyrmDragonFlightState::Flying && CurrentFlightState != EWyrmDragonFlightState::TakingOff)
    {
        OutReason = TEXT("Not in flight");
        return false;
    }

    const FVector Start = GetActorLocation();
    const FVector End = Start - FVector(0.f, 0.f, LandingSearchDistance);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (MountedRider.IsValid())
    {
        Params.AddIgnoredActor(MountedRider.Get());
    }

    bool bHitGround = false;
    if (GetWorld())
    {
        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) ||
            GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            bHitGround = true;
        }
    }

    if (bHitGround)
    {
        const float SlopeDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Hit.Normal, FVector::UpVector)));
        if (SlopeDeg > MaxLandingSlopeAngle)
        {
            OutReason = TEXT("Ground slope too steep for landing");
            return false;
        }

        const float HalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 65.f;
        OutLandingLocation = Hit.Location + FVector(0.f, 0.f, HalfHeight + 5.f);
        OutReason = TEXT("");
        return true;
    }

    // In minimal automation environments without collision geometry below, support safe landing if near ground
    if (GetActorLocation().Z <= 600.f && (!GetWorld() || !GetWorld()->IsGameWorld()))
    {
        const float HalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 65.f;
        OutLandingLocation = FVector(GetActorLocation().X, GetActorLocation().Y, HalfHeight + 5.f);
        OutReason = TEXT("");
        return true;
    }

    OutReason = TEXT("No supported ground beneath landing distance");
    return false;
}

bool AWyrmDragonCharacter::Land()
{
    FVector LandingLoc;
    FString Reason;
    if (!CanLand(LandingLoc, Reason))
    {
        UE_LOG(LogTemp, Warning, TEXT("Landing rejected: %s"), *Reason);
        return false;
    }

    CurrentFlightState = EWyrmDragonFlightState::Grounded;
    SetActorLocation(LandingLoc);

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->MaxWalkSpeed = GroundSpeed;
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
    }

    return true;
}

FVector AWyrmDragonCharacter::GetSafeGroundAnchor() const
{
    const FVector Start = GetActorLocation();
    const FVector End = Start - FVector(0.f, 0.f, 20000.f);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (MountedRider.IsValid())
    {
        Params.AddIgnoredActor(MountedRider.Get());
    }

    if (GetWorld() && (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) ||
                       GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)))
    {
        return Hit.Location + FVector(0.f, 0.f, 90.f);
    }
    return FVector(GetActorLocation().X, GetActorLocation().Y, 90.f);
}

void AWyrmDragonCharacter::HandleMountedDefeat()
{
    const FVector SafeGround = GetSafeGroundAnchor();
    AWyrmCharacter* Rider = MountedRider.Get();
    if (Rider)
    {
        Rider->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        Rider->SetActorLocation(SafeGround);
        Rider->SetActorRotation(GetActorRotation());

        if (Rider->GetCapsuleComponent() && GetCapsuleComponent())
        {
            Rider->GetCapsuleComponent()->IgnoreActorWhenMoving(this, false);
            GetCapsuleComponent()->IgnoreActorWhenMoving(Rider, false);
        }
        Rider->SetMovementLocked(false);

        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            PC->UnPossess();
            PC->Possess(Rider);
        }
        MountedRider.Reset();
    }

    CurrentFlightState = EWyrmDragonFlightState::Grounded;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
    }

    CurrentRole = EWyrmDragonRole::Recovering;
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = 650.f;
    }

    OnDragonDefeated.Broadcast(this);
}

bool AWyrmDragonCharacter::RecoverCompanion()
{
    if (CurrentRole == EWyrmDragonRole::Recovering || CurrentRole == EWyrmDragonRole::DefeatedAlive)
    {
        CurrentRole = EWyrmDragonRole::AlliedCompanion;
        CurrentFlightState = EWyrmDragonFlightState::Grounded;
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            GetCharacterMovement()->MaxWalkSpeed = GroundSpeed;
        }

        if (Attributes)
        {
            Attributes->InitHealth(420.f);
            Attributes->SetHealth(420.f);
            Attributes->InitFocus(100.f);
            Attributes->SetFocus(100.f);
        }

        AreaAttackCooldownRemaining = 0.f;
        return true;
    }
    return false;
}

void AWyrmDragonCharacter::SetFlightCameraMode(bool bTopDown)
{
    if (!CameraBoom)
    {
        return;
    }

    if (bTopDown)
    {
        CameraBoom->TargetArmLength = 1800.f;
        CameraBoom->SetRelativeRotation(FRotator(-65.f, 0.f, 0.f));
        CameraBoom->bUsePawnControlRotation = false;
    }
    else
    {
        CameraBoom->TargetArmLength = IsRiderMounted() ? 1100.f : 650.f;
        CameraBoom->bUsePawnControlRotation = true;
    }
}

void AWyrmDragonCharacter::CheckTetherStatus()
{
    if (!WaitingHumanoid.IsValid())
    {
        return;
    }

    float Dist = GetDistanceToHumanoid();
    if (Dist >= TetherLimitDistance)
    {
        CurrentTetherStatus = EWyrmTetherStatus::LimitReached;
        OnTetherWarningChanged.Broadcast(CurrentTetherStatus, Dist);

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->StopMovementImmediately();
        }

        // Return control safely to the waiting humanoid body
        APlayerController* PC = Cast<APlayerController>(GetController());
        EndDirectControl(PC);
    }
    else if (Dist >= TetherWarningDistance)
    {
        if (CurrentTetherStatus != EWyrmTetherStatus::Warning)
        {
            CurrentTetherStatus = EWyrmTetherStatus::Warning;
            OnTetherWarningChanged.Broadcast(CurrentTetherStatus, Dist);
        }
    }
    else
    {
        if (CurrentTetherStatus != EWyrmTetherStatus::WithinTether)
        {
            CurrentTetherStatus = EWyrmTetherStatus::WithinTether;
            OnTetherWarningChanged.Broadcast(CurrentTetherStatus, Dist);
        }
    }
}

void AWyrmDragonCharacter::UpdateCompanionAI(float DeltaSeconds)
{
    if (!WaitingHumanoid.IsValid() && CurrentRole == EWyrmDragonRole::AlliedCompanion)
    {
        // Try finding player character
        if (AWyrmCharacter* PlayerChar = Cast<AWyrmCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
        {
            WaitingHumanoid = PlayerChar;
        }
    }

    if (bTownModeEnabled)
    {
        // Town mode: suppress combat targeting and maintain comfortable follow (DRG-13)
        CurrentCombatTarget = nullptr;
        if (CurrentOrder == EWyrmCompanionOrder::AttackTarget)
        {
            CurrentOrder = EWyrmCompanionOrder::Follow;
        }
    }

    if (bIsTransitioningForm)
    {
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->StopMovementImmediately();
        }
        return;
    }

    if (CurrentOrder == EWyrmCompanionOrder::Hold)
    {
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->StopMovementImmediately();
        }
        return;
    }

    if (CurrentOrder == EWyrmCompanionOrder::AttackTarget)
    {
        if (CurrentCombatTarget.IsValid())
        {
            float DistToTarget = FVector::Dist(GetActorLocation(), CurrentCombatTarget->GetActorLocation());
            if (DistToTarget <= AttackRange)
            {
                PerformPrimaryAttack(CurrentCombatTarget.Get());
            }
            else
            {
                UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), CurrentCombatTarget.Get());
            }
        }
        else
        {
            // Target lost or defeated, return to follow
            CurrentOrder = EWyrmCompanionOrder::Follow;
        }
        return;
    }

    if (CurrentOrder == EWyrmCompanionOrder::Return || CurrentOrder == EWyrmCompanionOrder::Follow)
    {
        if (WaitingHumanoid.IsValid())
        {
            float DistToHumanoid = GetDistanceToHumanoid();
            float FollowTargetDistance = bTownModeEnabled ? 280.f : CompanionFollowDistance;
            if (DistToHumanoid > FollowTargetDistance)
            {
                if (GetCharacterMovement())
                {
                    const float NormalSpeed = (CurrentForm == EWyrmDragonForm::CompanionForm) ? CompanionGroundSpeed : TrueFormGroundSpeed;
                    const float CatchUpSpeed = (CurrentForm == EWyrmDragonForm::CompanionForm) ? CompanionCatchUpSpeed : TrueFormGroundSpeed;
                    GetCharacterMovement()->MaxWalkSpeed = (DistToHumanoid > 600.f) ? CatchUpSpeed : NormalSpeed;
                }
                UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), WaitingHumanoid.Get());
            }
            else
            {
                if (CurrentOrder == EWyrmCompanionOrder::Return)
                {
                    CurrentOrder = EWyrmCompanionOrder::Follow;
                }
                if (GetCharacterMovement())
                {
                    GetCharacterMovement()->StopMovementImmediately();
                }
            }
        }
    }
}

void AWyrmDragonCharacter::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue < Data.OldValue && bIsTransitioningForm)
    {
        InterruptFormTransition();
    }

    if (Data.NewValue <= 0.f)
    {
        if (CurrentRole == EWyrmDragonRole::HostileBoss)
        {
            PerformBossDefeat();
        }
        else if (CurrentRole == EWyrmDragonRole::AlliedCompanion)
        {
            if (IsRiderMounted())
            {
                HandleMountedDefeat();
            }
            else
            {
                CurrentRole = EWyrmDragonRole::Recovering;
                if (bIsDirectlyControlled)
                {
                    APlayerController* PC = Cast<APlayerController>(GetController());
                    EndDirectControl(PC);
                }
                OnDragonDefeated.Broadcast(this);
            }
        }
    }
}

void AWyrmDragonCharacter::SetupModularMeshes()
{
    if (!HasSupportedRigProfile())
    {
        UE_LOG(LogTemp, Warning, TEXT("Dragon rig '%s' is not mapped to the Green Dragon modular mesh profile"), *DragonId.ToString());
        return;
    }

    static const TCHAR* LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local.Hip-Local");
    static const TCHAR* FollowerNames[] = {
        TEXT("Chest-Local"), TEXT("Claw-Local"), TEXT("Detail3-1-Local"), TEXT("Detail4-1-Local"),
        TEXT("Detail5-1-Local"), TEXT("Detail6-1-Local"), TEXT("ear_L-Local"), TEXT("ear_R-Local"),
        TEXT("Head-Local"), TEXT("horn1_ear1-2-Local"), TEXT("horn1_ear1-3-Local"), TEXT("horn2_ear1-2-Local"),
        TEXT("horn2_ear1-3-Local"), TEXT("Jaw-Local"), TEXT("L_Wing1-Local"), TEXT("L_Wing2-Local"),
        TEXT("L_Wing3-Local"), TEXT("LB_Claw-Local"), TEXT("LB_Foot-Local"), TEXT("LB_Leg1-Local"),
        TEXT("LB_Leg2-Local"), TEXT("LB_Thigh-Local"), TEXT("LF_Claw-Local"), TEXT("LF_Foot-Local"),
        TEXT("LF_Leg-Local"), TEXT("LF_Thigh-Local"), TEXT("Mini2-1-Local"), TEXT("Neck-Local"),
        TEXT("R_Wing1-Local"), TEXT("R_Wing2-Local"), TEXT("R_Wing3-Local"), TEXT("RB_Foot-Local"),
        TEXT("RB_Leg1-Local"), TEXT("RB_Leg2-Local"), TEXT("RB_Thigh-Local"), TEXT("RF_Claw-Local"),
        TEXT("RF_Foot-Local"), TEXT("RF_Leg-Local"), TEXT("RF_Thigh-Local"), TEXT("Tail_1-Local"),
        TEXT("Tail_2-Local"), TEXT("Tail_3-Local"), TEXT("Tail_4-Local")
    };

    USkeletalMesh* LeaderMesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, LeaderMeshPath));
    if (LeaderMesh && GetMesh())
    {
        GetMesh()->SetSkeletalMeshAsset(LeaderMesh);
    }

    if (GetMesh() && FollowerMeshComponents.Num() == 0)
    {
        for (const TCHAR* FollowerName : FollowerNames)
        {
            FString MeshPath = FString::Printf(TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/%s.%s"), FollowerName, FollowerName);
            USkeletalMesh* PartMesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *MeshPath));
            if (PartMesh)
            {
                USkeletalMeshComponent* PartComp = NewObject<USkeletalMeshComponent>(this, FName(FollowerName));
                PartComp->RegisterComponent();
                PartComp->SetSkeletalMeshAsset(PartMesh);
                PartComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
                PartComp->SetLeaderPoseComponent(GetMesh());
                FollowerMeshComponents.Add(PartComp);
            }
        }
    }
}

void AWyrmDragonCharacter::BuildSaveRecord(FWyrmDragonSaveRecord& OutRecord) const
{
    OutRecord.DragonId = DragonId;
    OutRecord.Role = CurrentRole;
    OutRecord.Form = CurrentForm;
    OutRecord.Order = CurrentOrder;
    OutRecord.bHasBondReceipt = bHasBondReceipt;
    OutRecord.Health = Attributes ? Attributes->GetHealth() : 420.f;
    OutRecord.MaxHealth = Attributes ? Attributes->GetMaxHealth() : 420.f;
    OutRecord.Focus = Attributes ? Attributes->GetFocus() : 100.f;
    OutRecord.MaxFocus = Attributes ? Attributes->GetMaxFocus() : 100.f;
    OutRecord.AreaAttackCooldownRemaining = AreaAttackCooldownRemaining;
    OutRecord.WorldLocation = GetActorLocation();
    OutRecord.WorldRotation = GetActorRotation();
    OutRecord.bIsDirectlyControlled = bIsDirectlyControlled;

    if (WaitingHumanoid.IsValid())
    {
        OutRecord.HumanoidWaitingLocation = WaitingHumanoid->GetActorLocation();
        OutRecord.HumanoidWaitingRotation = WaitingHumanoid->GetActorRotation();
    }

    OutRecord.bIsRiderMounted = MountedRider.IsValid();
    OutRecord.FlightState = CurrentFlightState;
    OutRecord.SafeGroundAnchor = GetSafeGroundAnchor();
    OutRecord.FormTransitionCooldownRemaining = FormTransitionCooldownRemaining;
    OutRecord.bTownModeEnabled = bTownModeEnabled;
}

void AWyrmDragonCharacter::RestoreFromSaveRecord(const FWyrmDragonSaveRecord& InRecord, AWyrmCharacter* HumanoidContext)
{
    EnsureAbilitySystemInitialized();
    DragonId = InRecord.DragonId;
    CurrentRole = InRecord.Role;
    CurrentForm = InRecord.Form;
    CurrentOrder = InRecord.Order;
    bHasBondReceipt = InRecord.bHasBondReceipt;
    AreaAttackCooldownRemaining = InRecord.AreaAttackCooldownRemaining;
    CurrentFlightState = InRecord.FlightState;
    FormTransitionCooldownRemaining = InRecord.FormTransitionCooldownRemaining;
    bTownModeEnabled = InRecord.bTownModeEnabled;
    bIsTransitioningForm = false;
    FormTransitionTimeRemaining = 0.f;

    // Check if saved position is obstructed
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (HumanoidContext)
    {
        Params.AddIgnoredActor(HumanoidContext);
    }
    FHitResult ObstructionHit;
    const float Radius = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.8f : 35.f;
    const bool bObstructed = GetWorld() && GetWorld()->SweepSingleByChannel(
        ObstructionHit, InRecord.WorldLocation, InRecord.WorldLocation,
        InRecord.WorldRotation.Quaternion(), ECC_WorldStatic, FCollisionShape::MakeSphere(Radius), Params);

    if (bObstructed && !InRecord.SafeGroundAnchor.IsZero())
    {
        // Disclosed recovery to safe ground anchor
        SetActorLocationAndRotation(InRecord.SafeGroundAnchor, InRecord.WorldRotation);
        CurrentFlightState = EWyrmDragonFlightState::Grounded;
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            GetCharacterMovement()->MaxWalkSpeed = GroundSpeed;
            GetCharacterMovement()->Velocity = FVector::ZeroVector;
        }
    }
    else
    {
        SetActorLocationAndRotation(InRecord.WorldLocation, InRecord.WorldRotation);
        if (GetCharacterMovement())
        {
            if (CurrentFlightState == EWyrmDragonFlightState::Flying || CurrentFlightState == EWyrmDragonFlightState::TakingOff)
            {
                GetCharacterMovement()->SetMovementMode(MOVE_Flying);
                GetCharacterMovement()->MaxFlySpeed = FlightSpeed;
            }
            else
            {
                GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                GetCharacterMovement()->MaxWalkSpeed = GroundSpeed;
            }
        }
    }

    if (Attributes)
    {
        Attributes->InitMaxHealth(InRecord.MaxHealth);
        Attributes->SetMaxHealth(InRecord.MaxHealth);
        Attributes->InitHealth(InRecord.Health);
        Attributes->SetHealth(InRecord.Health);
        Attributes->InitMaxFocus(InRecord.MaxFocus);
        Attributes->SetMaxFocus(InRecord.MaxFocus);
        Attributes->InitFocus(InRecord.Focus);
        Attributes->SetFocus(InRecord.Focus);
    }

    if (HumanoidContext)
    {
        WaitingHumanoid = HumanoidContext;
    }

    ApplyFormDimensions();

    if (InRecord.bIsRiderMounted && HumanoidContext)
    {
        MountHumanoid(HumanoidContext);
    }
    else if (InRecord.bIsDirectlyControlled && HumanoidContext)
    {
        if (APlayerController* PC = Cast<APlayerController>(HumanoidContext->GetController()))
        {
            StartDirectControl(PC, HumanoidContext);
        }
    }
}

AWyrmDragonCharacter* AWyrmDragonCharacter::SpawnWyrmDragon(UObject* WorldContextObject, EWyrmDragonRole InRole, const FTransform& SpawnTransform)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(AWyrmDragonCharacter::StaticClass(), SpawnTransform, Params);
    if (Dragon)
    {
        Dragon->SetDragonRole(InRole);
    }
    return Dragon;
}
