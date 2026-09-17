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

void AWyrmDragonCharacter::SetDragonForm(EWyrmDragonForm NewForm)
{
    CurrentForm = NewForm;
    ApplyFormDimensions();
}

void AWyrmDragonCharacter::ApplyFormDimensions()
{
    float TargetScale = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 0.009f : 0.035f;
    float CapsuleRadius = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 40.f : 120.f;
    float CapsuleHalfHeight = (CurrentForm == EWyrmDragonForm::CompanionForm) ? 55.f : 160.f;

    GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
    if (GetMesh())
    {
        GetMesh()->SetRelativeScale3D(FVector(TargetScale));
    }
}

void AWyrmDragonCharacter::IssueOrder(EWyrmCompanionOrder NewOrder, AActor* TargetActor)
{
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
    if (CurrentRole == EWyrmDragonRole::DefeatedAlive || CurrentRole == EWyrmDragonRole::Recovering)
    {
        return false;
    }

    AActor* Victim = TargetActor ? TargetActor : CurrentCombatTarget.Get();

    // If no explicit victim, trace ahead for a hostile target
    if (!Victim)
    {
        FVector Start = GetActorLocation() + GetActorForwardVector() * 50.f;
        FVector End = Start + GetActorForwardVector() * AttackRange;

        TArray<FHitResult> Hits;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(120.f);
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

    // Damage amount: Allied Companion strike 24 damage
    float DamageAmount = (CurrentRole == EWyrmDragonRole::AlliedCompanion) ? 24.f : 28.f;
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

    // 600 cm area sweep
    TArray<FHitResult> Hits;
    FVector Center = GetActorLocation();
    FCollisionShape Sphere = FCollisionShape::MakeSphere(600.f);
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
            UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, HitASC, 18.f);
        }
        else
        {
            UGameplayStatics::ApplyDamage(HitActor, 18.f, GetController(), this, UDamageType::StaticClass());
        }
    }

    // Commit 6-second cooldown
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
            if (DistToHumanoid > CompanionFollowDistance)
            {
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
    if (Data.NewValue <= 0.f)
    {
        if (CurrentRole == EWyrmDragonRole::HostileBoss)
        {
            PerformBossDefeat();
        }
        else if (CurrentRole == EWyrmDragonRole::AlliedCompanion)
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

void AWyrmDragonCharacter::SetupModularMeshes()
{
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

    SetActorLocationAndRotation(InRecord.WorldLocation, InRecord.WorldRotation);

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

    if (InRecord.bIsDirectlyControlled && HumanoidContext)
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
