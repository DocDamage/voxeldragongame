#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Combat/WyrmAttributeSet.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Combat/Abilities/WyrmRangedAttackAbility.h"
#include "Combat/Abilities/WyrmEvadeAbility.h"
#include "Combat/Abilities/WyrmRelentlessAdvanceAbility.h"
#include "Combat/Abilities/WyrmMoonboundFormAbility.h"
#include "Combat/Abilities/WyrmMirrorStepAbility.h"
#include "Combat/Abilities/WyrmUnseenHandAbility.h"
#include "Combat/Abilities/WyrmHuntersVeilAbility.h"
#include "Combat/Abilities/WyrmSanguineStrikeAbility.h"
#include "Combat/Abilities/WyrmSecondTurnAbility.h"
#include "Combat/Abilities/WyrmBeastAttackAbilities.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "Combat/WyrmUnseenHandTarget.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/OverlapResult.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "Activities/WyrmFishingComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Water/WyrmWaterVolume.h"
#include "GameplayTagsManager.h"
#include "MuCO/CustomizableSkeletalComponent.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableObjectInstance.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Misc/Base64.h"
#include "DrawDebugHelpers.h"

AWyrmCharacter::AWyrmCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    BaseWalkSpeed = 450.f;
    GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed; // development-only tuning
    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    Attributes = CreateDefaultSubobject<UWyrmAttributeSet>(TEXT("Attributes"));
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->bDoCollisionTest = true;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    CustomizableSkeletalComponent = CreateDefaultSubobject<UCustomizableSkeletalComponent>(TEXT("CustomizableSkeletalComponent"));
    CustomizableSkeletalComponent->SetupAttachment(GetMesh());
    BeastMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeastMeshComponent"));
    BeastMeshComponent->SetupAttachment(GetCapsuleComponent());
    BeastMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -45.f));
    BeastMeshComponent->SetVisibility(false);
    BeastMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InventoryComponent = CreateDefaultSubobject<UWyrmInventoryComponent>(TEXT("InventoryComponent"));
    FishingComponent = CreateDefaultSubobject<UWyrmFishingComponent>(TEXT("FishingComponent"));
    ApplyCamera();
}

void AWyrmCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (AbilitySystem)
    {
        AbilitySystem->InitAbilityActorInfo(this, this);
        if (Attributes && !AbilitySystem->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
        {
            AbilitySystem->AddAttributeSetSubobject(Attributes.Get());
        }
    }
}

void AWyrmCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (AbilitySystem)
    {
        AbilitySystem->InitAbilityActorInfo(this, this);
        if (Attributes && !AbilitySystem->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
        {
            AbilitySystem->AddAttributeSetSubobject(Attributes.Get());
        }

        static const FGameplayTag PlayerTeamTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Combat.Team.Player")), false);
        if (PlayerTeamTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(PlayerTeamTag))
        {
            AbilitySystem->AddLooseGameplayTag(PlayerTeamTag);
        }

        AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attributes->GetHealthAttribute())
            .AddUObject(this, &AWyrmCharacter::HandleHealthChanged);
    }
    GrantCombatAbilities();

    // Unpossessing this body must not reset GAS state when dragon control is added.
    ApplyCamera();
    if (CustomizableInstance && CustomizableSkeletalComponent)
    {
        CustomizableSkeletalComponent->SetCustomizableObjectInstance(CustomizableInstance);
        ApplyAppearance(true);
    }

    if (BeastMeshComponent && !BeastMeshComponent->GetSkeletalMeshAsset())
    {
        USkeletalMesh* WolfMesh = Cast<USkeletalMesh>(StaticLoadObject(
            USkeletalMesh::StaticClass(),
            nullptr,
            TEXT("/Game/WYRMFALL/Development/Intake/WP00/Review/Wolf/wolf1.wolf1")));
        if (WolfMesh)
        {
            BeastMeshComponent->SetSkeletalMeshAsset(WolfMesh);
        }
    }
}
UAbilitySystemComponent* AWyrmCharacter::GetAbilitySystemComponent() const { return AbilitySystem; }
void AWyrmCharacter::SetCameraMode(EWyrmCameraMode NewMode)
{
    CameraMode = NewMode;
    ApplyCamera();
}

void AWyrmCharacter::ToggleCamera()
{
    SetCameraMode(CameraMode == EWyrmCameraMode::ThirdPerson ? EWyrmCameraMode::TopDown : EWyrmCameraMode::ThirdPerson);
}

void AWyrmCharacter::SetMovementLocked(bool bLocked)
{
    bMovementLocked = bLocked;
    if (bMovementLocked)
    {
        StopJumping();
        if (AController* C = GetController())
        {
            C->StopMovement();
        }
    }
}

void AWyrmCharacter::CaptureControlState(FWyrmControlState& OutState) const
{
    OutState.CameraMode = CameraMode;
    OutState.bMovementLocked = bMovementLocked;
}

void AWyrmCharacter::RestoreControlState(const FWyrmControlState& InState)
{
    SetCameraMode(InState.CameraMode);
    SetMovementLocked(InState.bMovementLocked);
}

void AWyrmCharacter::ApplyCamera()
{
    const bool bTop = CameraMode == EWyrmCameraMode::TopDown;
    CameraBoom->TargetArmLength = bTop ? 1100.f : 450.f;
    CameraBoom->bUsePawnControlRotation = !bTop;
    CameraBoom->SetWorldRotation(bTop ? FRotator(-60.f, 0.f, 0.f) : FRotator(-18.f, 0.f, 0.f));
    Camera->FieldOfView = bTop ? 55.f : 80.f;
}
void AWyrmCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Food buff expiration and regen (ACT-04)
    if (ActiveFoodBuff.IsActive())
    {
        ActiveFoodBuff.RemainingDuration -= DeltaSeconds;

        if (ActiveFoodBuff.HealthRegenPerSecond > 0.f && Attributes)
        {
            const float CurrentHP = Attributes->GetCurrentHealth();
            const float MaxHP = Attributes->GetCurrentMaxHealth();
            if (CurrentHP < MaxHP)
            {
                Attributes->SetCurrentHealth(FMath::Min(MaxHP, CurrentHP + (ActiveFoodBuff.HealthRegenPerSecond * DeltaSeconds)));
            }
        }

        if (ActiveFoodBuff.RemainingDuration <= 0.f)
        {
            ClearFoodBuff();
        }
    }

    // Status effect and Echo timers (WP-15)
    bool bNeedsMovementUpdate = false;

    if (SlowRemainingTimer > 0.f)
    {
        SlowRemainingTimer -= DeltaSeconds;
        if (SlowRemainingTimer <= 0.f)
        {
            SlowRemainingTimer = 0.f;
            ActiveSlowMagnitude = 0.f;
            bNeedsMovementUpdate = true;
            if (AbilitySystem)
            {
                static const FGameplayTag SlowTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Slow")), false);
                if (SlowTag.IsValid()) { AbilitySystem->RemoveLooseGameplayTag(SlowTag); }
            }
        }
    }

    if (RootRemainingTimer > 0.f)
    {
        RootRemainingTimer -= DeltaSeconds;
        if (RootRemainingTimer <= 0.f)
        {
            RootRemainingTimer = 0.f;
            bNeedsMovementUpdate = true;
            if (AbilitySystem)
            {
                static const FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Root")), false);
                if (RootTag.IsValid()) { AbilitySystem->RemoveLooseGameplayTag(RootTag); }
            }
        }
    }

    if (StunRemainingTimer > 0.f)
    {
        StunRemainingTimer -= DeltaSeconds;
        if (StunRemainingTimer <= 0.f)
        {
            StunRemainingTimer = 0.f;
            bNeedsMovementUpdate = true;
            if (AbilitySystem)
            {
                static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stun")), false);
                if (StunTag.IsValid()) { AbilitySystem->RemoveLooseGameplayTag(StunTag); }
            }
        }
    }

    if (StaggerRemainingTimer > 0.f)
    {
        StaggerRemainingTimer -= DeltaSeconds;
        if (StaggerRemainingTimer <= 0.f)
        {
            StaggerRemainingTimer = 0.f;
            if (AbilitySystem)
            {
                static const FGameplayTag StaggerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stagger")), false);
                if (StaggerTag.IsValid()) { AbilitySystem->RemoveLooseGameplayTag(StaggerTag); }
            }
        }
    }

    if (bRelentlessAdvanceActive)
    {
        RelentlessAdvanceRemainingTimer -= DeltaSeconds;
        if (RelentlessAdvanceRemainingTimer <= 0.f)
        {
            RelentlessAdvanceRemainingTimer = 0.f;
            bRelentlessAdvanceActive = false;
            bNeedsMovementUpdate = true;
            if (AbilitySystem)
            {
                static const FGameplayTag RelentlessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.RelentlessAdvance")), false);
                if (RelentlessTag.IsValid()) { AbilitySystem->RemoveLooseGameplayTag(RelentlessTag); }
            }
        }
    }

    if (RelentlessAdvanceCooldownTimer > 0.f)
    {
        RelentlessAdvanceCooldownTimer = FMath::Max(0.f, RelentlessAdvanceCooldownTimer - DeltaSeconds);
        if (RelentlessAdvanceCooldownTimer <= 0.f)
        {
            RelentlessAdvanceCooldownTimer = 0.f;
            if (AbilitySystem)
            {
                static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.RelentlessAdvance")), false);
                if (CooldownTag.IsValid())
                {
                    if (AbilitySystem->HasMatchingGameplayTag(CooldownTag))
                    {
                        AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
                    }
                    FGameplayTagContainer CDContainer(CooldownTag);
                    AbilitySystem->RemoveActiveEffectsWithGrantedTags(CDContainer);
                }
            }
        }
    }

    if (bMoonboundActive)
    {
        MoonboundRemainingTimer -= DeltaSeconds;
        if (MoonboundRemainingTimer <= 0.f)
        {
            MoonboundRemainingTimer = 0.f;
            DeactivateMoonboundForm();
        }
    }

    if (MoonboundCooldownTimer > 0.f)
    {
        MoonboundCooldownTimer = FMath::Max(0.f, MoonboundCooldownTimer - DeltaSeconds);
        if (MoonboundCooldownTimer <= 0.f)
        {
            MoonboundCooldownTimer = 0.f;
            if (AbilitySystem)
            {
                static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.MoonboundForm")), false);
                if (CooldownTag.IsValid())
                {
                    if (AbilitySystem->HasMatchingGameplayTag(CooldownTag))
                    {
                        AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
                    }
                    FGameplayTagContainer CDContainer(CooldownTag);
                    AbilitySystem->RemoveActiveEffectsWithGrantedTags(CDContainer);
                }
            }
        }
    }

    if (MirrorStepCooldownTimer > 0.f)
    {
        MirrorStepCooldownTimer = FMath::Max(0.f, MirrorStepCooldownTimer - DeltaSeconds);
        if (MirrorStepCooldownTimer <= 0.f && AbilitySystem)
        {
            static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.MirrorStep")), false);
            if (CooldownTag.IsValid())
            {
                AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
                FGameplayTagContainer CooldownTags(CooldownTag);
                AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
            }
        }
    }

    if (UnseenHandCooldownTimer > 0.f)
    {
        UnseenHandCooldownTimer = FMath::Max(0.f, UnseenHandCooldownTimer - DeltaSeconds);
        if (UnseenHandCooldownTimer <= 0.f && AbilitySystem)
        {
            static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.UnseenHand")), false);
            if (CooldownTag.IsValid())
            {
                AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
                FGameplayTagContainer CooldownTags(CooldownTag);
                AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
            }
        }
    }

    if (bHuntersVeilActive)
    {
        HuntersVeilRemainingTimer = FMath::Max(0.f, HuntersVeilRemainingTimer - DeltaSeconds);
        if (HuntersVeilRemainingTimer <= 0.f)
        {
            BreakHuntersVeil();
        }
    }

    if (HuntersVeilCooldownTimer > 0.f)
    {
        HuntersVeilCooldownTimer = FMath::Max(0.f, HuntersVeilCooldownTimer - DeltaSeconds);
        if (HuntersVeilCooldownTimer <= 0.f && AbilitySystem)
        {
            static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.HuntersVeil")), false);
            if (CooldownTag.IsValid())
            {
                AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
                FGameplayTagContainer CooldownTags(CooldownTag);
                AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
            }
        }
    }

    if (bSanguineStrikePrimed)
    {
        SanguineStrikeWindowTimer = FMath::Max(0.f, SanguineStrikeWindowTimer - DeltaSeconds);
        if (SanguineStrikeWindowTimer <= 0.f)
        {
            bSanguineStrikePrimed = false;
            if (AbilitySystem)
            {
                static const FGameplayTag PrimedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SanguineStrikePrimed")), false);
                if (PrimedTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(PrimedTag);
            }
        }
    }

    if (SanguineStrikeCooldownTimer > 0.f)
    {
        SanguineStrikeCooldownTimer = FMath::Max(0.f, SanguineStrikeCooldownTimer - DeltaSeconds);
        if (SanguineStrikeCooldownTimer <= 0.f && AbilitySystem)
        {
            static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.SanguineStrike")), false);
            if (CooldownTag.IsValid())
            {
                AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
                FGameplayTagContainer CooldownTags(CooldownTag);
                AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
            }
        }
    }

    if (SecondTurnCooldownTimer > 0.f)
    {
        SecondTurnCooldownTimer = FMath::Max(0.f, SecondTurnCooldownTimer - DeltaSeconds);
        if (SecondTurnCooldownTimer <= 0.f && AbilitySystem)
        {
            static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.SecondTurn")), false);
            if (CooldownTag.IsValid())
            {
                AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
                FGameplayTagContainer CooldownTags(CooldownTag);
                AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
            }
        }
    }

    if (SecondTurnRepeatTimer > 0.f)
    {
        SecondTurnRepeatTimer = FMath::Max(0.f, SecondTurnRepeatTimer - DeltaSeconds);
        if (SecondTurnRepeatTimer <= 0.f)
        {
            UAbilitySystemComponent* RepeatTarget = PendingSecondTurnTarget.Get();
            const float RepeatDamage = PendingSecondTurnRawDamage;
            PendingSecondTurnTarget.Reset();
            PendingSecondTurnRawDamage = 0.f;
            if (AbilitySystem)
            {
                static const FGameplayTag PendingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SecondTurnPending")), false);
                if (PendingTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(PendingTag);
            }
            const UWyrmAttributeSet* TargetAttributes = RepeatTarget
                ? Cast<UWyrmAttributeSet>(RepeatTarget->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
                : nullptr;
            static const FGameplayTag PlayerTeamTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Combat.Team.Player")), false);
            const bool bIsNowFriendly = PlayerTeamTag.IsValid() && AbilitySystem && RepeatTarget &&
                AbilitySystem->HasMatchingGameplayTag(PlayerTeamTag) &&
                RepeatTarget->HasMatchingGameplayTag(PlayerTeamTag);
            if (AbilitySystem && Attributes && Attributes->GetHealth() > 0.f &&
                RepeatTarget && TargetAttributes && TargetAttributes->GetHealth() > 0.f && !bIsNowFriendly)
            {
                // The generic GAS path is deliberate: the spectral repeat is
                // noncritical and cannot recurse or trigger any Echo proc.
                UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, RepeatTarget, RepeatDamage);
            }
        }
    }

    if (bNeedsMovementUpdate)
    {
        UpdateMovementForStatus();
    }

#if !UE_BUILD_SHIPPING
    // Wireframe diagnostic, not replacement character artwork.
    if (!GetMesh()->GetSkeletalMeshAsset())
    {
        DrawDebugCapsule(GetWorld(), GetActorLocation(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
            GetCapsuleComponent()->GetScaledCapsuleRadius(), GetActorQuat(), FColor::Cyan, false, -1.f, 0, 2.f);
    }
#endif
}

bool AWyrmCharacter::SetCustomizableObject(UCustomizableObject* InCustomizableObject)
{
    if (!InCustomizableObject)
    {
        return false;
    }
    UCustomizableObjectInstance* NewInstance = InCustomizableObject->CreateInstance();
    if (!NewInstance)
    {
        return false;
    }
    SetCustomizableInstance(NewInstance);

    if (GetMesh() && InCustomizableObject->GetComponentCount() > 0)
    {
        FName CompName = InCustomizableObject->GetComponentName(0);
        if (USkeletalMesh* RefMesh = InCustomizableObject->GetSkeletalMeshComponentReferenceSkeletalMesh(CompName))
        {
            GetMesh()->SetSkeletalMeshAsset(RefMesh);
        }
    }
    return true;
}

void AWyrmCharacter::SetCustomizableInstance(UCustomizableObjectInstance* InInstance)
{
    CustomizableInstance = InInstance;
    if (CustomizableSkeletalComponent)
    {
        CustomizableSkeletalComponent->SetCustomizableObjectInstance(InInstance);
    }
}

void AWyrmCharacter::SetColorParameter(FName ParamName, FLinearColor Color)
{
    if (CustomizableInstance)
    {
        CustomizableInstance->SetColorParameterSelectedOption(ParamName.ToString(), Color);
    }
}

FLinearColor AWyrmCharacter::GetColorParameter(FName ParamName) const
{
    if (CustomizableInstance)
    {
        return CustomizableInstance->GetColorParameterSelectedOption(ParamName.ToString());
    }
    return FLinearColor::White;
}

void AWyrmCharacter::SetOptionParameter(FName ParamName, const FString& OptionName)
{
    if (CustomizableInstance)
    {
        CustomizableInstance->SetIntParameterSelectedOption(ParamName.ToString(), OptionName);
    }
}

FString AWyrmCharacter::GetOptionParameter(FName ParamName) const
{
    if (CustomizableInstance)
    {
        return CustomizableInstance->GetIntParameterSelectedOption(ParamName.ToString());
    }
    return FString();
}

void AWyrmCharacter::SetFloatParameter(FName ParamName, float Value)
{
    if (CustomizableInstance)
    {
        CustomizableInstance->SetFloatParameterSelectedOption(ParamName.ToString(), Value);
    }
}

float AWyrmCharacter::GetFloatParameter(FName ParamName) const
{
    if (CustomizableInstance)
    {
        return CustomizableInstance->GetFloatParameterSelectedOption(ParamName.ToString());
    }
    return 0.f;
}

void AWyrmCharacter::ApplyAppearance(bool bAsync)
{
    if (CustomizableSkeletalComponent)
    {
        CustomizableSkeletalComponent->UpdateSkeletalMeshAsync(true);
    }
    else if (CustomizableInstance)
    {
        CustomizableInstance->UpdateSkeletalMeshAsync(true);
    }
}

FString AWyrmCharacter::CaptureAppearanceDescriptor() const
{
    if (!CustomizableInstance)
    {
        return FString();
    }
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes, true);
    CustomizableInstance->SaveDescriptor(Writer, false);
    return FBase64::Encode(Bytes);
}

bool AWyrmCharacter::RestoreAppearanceDescriptor(const FString& InDescriptor)
{
    if (!CustomizableInstance || InDescriptor.IsEmpty())
    {
        return false;
    }
    TArray<uint8> Bytes;
    if (!FBase64::Decode(InDescriptor, Bytes) || Bytes.Num() == 0)
    {
        return false;
    }
    FMemoryReader Reader(Bytes, true);
    CustomizableInstance->LoadDescriptor(Reader);
    ApplyAppearance(true);
    return true;
}

void AWyrmCharacter::SetCharacterScale(FVector NewScale)
{
    CharacterScale = NewScale;
    if (GetMesh())
    {
        GetMesh()->SetRelativeScale3D(CharacterScale);
    }
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        const float BaseHalfHeight = 96.0f;
        const float BaseRadius = 42.0f;
        const float MaxHoriz = FMath::Max(CharacterScale.X, CharacterScale.Y);
        Capsule->SetCapsuleSize(BaseRadius * MaxHoriz, BaseHalfHeight * CharacterScale.Z);
    }
}

bool AWyrmCharacter::AttachEquipmentMesh(USceneComponent* ItemMesh, FName SocketName)
{
    if (!ItemMesh || !GetMesh())
    {
        return false;
    }
    ItemMesh->SetMobility(EComponentMobility::Movable);
    if (!SocketName.IsNone() && GetMesh()->DoesSocketExist(SocketName))
    {
        ItemMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
        return true;
    }
    ItemMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    return true;
}

bool AWyrmCharacter::IsSocketValid(FName SocketName) const
{
    return GetMesh() && GetMesh()->DoesSocketExist(SocketName);
}

void AWyrmCharacter::GrantCombatAbilities()
{
    if (!AbilitySystem)
    {
        return;
    }

    AbilitySystem->InitAbilityActorInfo(this, this);
    if (Attributes && !AbilitySystem->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
    {
        AbilitySystem->AddAttributeSetSubobject(Attributes.Get());
    }

    static const FGameplayTag PlayerTeamTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Combat.Team.Player")), false);
    if (PlayerTeamTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(PlayerTeamTag))
    {
        AbilitySystem->AddLooseGameplayTag(PlayerTeamTag);
    }

    if (!PrimaryMeleeHandle.IsValid())
    {
        PrimaryMeleeHandle = AbilitySystem->GiveAbility(
            FGameplayAbilitySpec(UWyrmPrimaryMeleeAbility::StaticClass(), 1, INDEX_NONE, this));
    }

    if (!SecondaryMeleeHandle.IsValid())
    {
        SecondaryMeleeHandle = AbilitySystem->GiveAbility(
            FGameplayAbilitySpec(UWyrmSecondaryMeleeAbility::StaticClass(), 1, INDEX_NONE, this));
    }

    if (!PrimaryRangedHandle.IsValid())
    {
        PrimaryRangedHandle = AbilitySystem->GiveAbility(
            FGameplayAbilitySpec(UWyrmPrimaryRangedAbility::StaticClass(), 1, INDEX_NONE, this));
    }

    if (!SecondaryRangedHandle.IsValid())
    {
        SecondaryRangedHandle = AbilitySystem->GiveAbility(
            FGameplayAbilitySpec(UWyrmSecondaryRangedAbility::StaticClass(), 1, INDEX_NONE, this));
    }

    if (!EvadeHandle.IsValid())
    {
        EvadeHandle = AbilitySystem->GiveAbility(
            FGameplayAbilitySpec(UWyrmEvadeAbility::StaticClass(), 1, INDEX_NONE, this));
    }

    UpdateActiveWeaponKit();
}

void AWyrmCharacter::UpdateActiveWeaponKit()
{
    if (!InventoryComponent)
    {
        ActiveWeaponFamily = EWyrmWeaponFamily::Unarmed;
        return;
    }

    FWyrmItemInstance MainHandItem;
    if (InventoryComponent->GetEquippedItem(EWyrmEquipSlot::MainHand, MainHandItem) && MainHandItem.ItemType == EWyrmItemType::Weapon)
    {
        ActiveWeaponFamily = MainHandItem.WeaponFamily;
        if (ActiveWeaponFamily == EWyrmWeaponFamily::Unarmed)
        {
            if (MainHandItem.ItemId.ToString().Contains(TEXT("Bow")) || MainHandItem.ItemId.ToString().Contains(TEXT("Ranger")))
            {
                ActiveWeaponFamily = EWyrmWeaponFamily::RangedBow;
            }
            else
            {
                ActiveWeaponFamily = EWyrmWeaponFamily::Melee1H;
            }
        }
    }
    else
    {
        ActiveWeaponFamily = EWyrmWeaponFamily::Unarmed;
    }
}

bool AWyrmCharacter::PerformPrimaryAttack()
{
    if (bMovementLocked || !AbilitySystem)
    {
        return false;
    }

    if (bMoonboundActive)
    {
        if (bMoonboundReturnPending)
        {
            return false;
        }
        return PrimaryBeastClawHandle.IsValid() && AbilitySystem->TryActivateAbility(PrimaryBeastClawHandle);
    }

    BreakHuntersVeil();
    if (ActiveWeaponFamily == EWyrmWeaponFamily::RangedBow)
    {
        return PrimaryRangedHandle.IsValid() && AbilitySystem->TryActivateAbility(PrimaryRangedHandle);
    }

    return PrimaryMeleeHandle.IsValid() && AbilitySystem->TryActivateAbility(PrimaryMeleeHandle);
}

bool AWyrmCharacter::PerformSecondaryAttack()
{
    if (bMovementLocked || !AbilitySystem)
    {
        return false;
    }

    if (bMoonboundActive)
    {
        if (bMoonboundReturnPending)
        {
            return false;
        }
        return SecondaryBeastPounceHandle.IsValid() && AbilitySystem->TryActivateAbility(SecondaryBeastPounceHandle);
    }

    BreakHuntersVeil();
    if (ActiveWeaponFamily == EWyrmWeaponFamily::RangedBow)
    {
        return SecondaryRangedHandle.IsValid() && AbilitySystem->TryActivateAbility(SecondaryRangedHandle);
    }

    return SecondaryMeleeHandle.IsValid() && AbilitySystem->TryActivateAbility(SecondaryMeleeHandle);
}

bool AWyrmCharacter::AttackTarget(AActor* TargetActor)
{
    if (!TargetActor || !AbilitySystem || !Attributes)
    {
        return false;
    }

    IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
    UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
    if (!TargetASC)
    {
        return false;
    }

    BreakHuntersVeil();
    const float RawDamage = UWyrmAttributeSet::CalculateRawDamage(10.f, Attributes->GetPower(), 0.5f);
    return UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, TargetASC, RawDamage);
}

bool AWyrmCharacter::PerformEvade()
{
    if (bMovementLocked || !AbilitySystem || !EvadeHandle.IsValid())
    {
        return false;
    }
    return AbilitySystem->TryActivateAbility(EvadeHandle);
}

float AWyrmCharacter::CalculateXPForNextLevel(float InLevel)
{
    // XPNeeded(L) = 100 + 50*(L - 1) per GAME_DESIGN.md line 28
    return 100.f + 50.f * FMath::Max(0.f, InLevel - 1.f);
}

float AWyrmCharacter::GetCharacterLevel() const
{
    return Attributes ? Attributes->GetCurrentCharacterLevel() : 1.f;
}

float AWyrmCharacter::GetXPToNextLevel() const
{
    return FMath::Max(0.f, GetXPForNextLevel() - CurrentXP);
}

float AWyrmCharacter::GetXPForNextLevel() const
{
    return CalculateXPForNextLevel(GetCharacterLevel());
}

void AWyrmCharacter::SetCurrentXP(float InXP)
{
    CurrentXP = FMath::Max(0.f, InXP);
}

void AWyrmCharacter::SetCharacterLevel(float NewLevel)
{
    const float ClampedLevel = FMath::Max(1.f, NewLevel);
    if (Attributes)
    {
        Attributes->SetCurrentCharacterLevel(ClampedLevel);

        // Scale base attributes per COMBAT_AND_NIGHTMARE_ECHOES.md Section 2
        const float NewMaxHealth = UWyrmAttributeSet::CalculateMaxHealthForLevel(ClampedLevel);
        const float NewPower = UWyrmAttributeSet::CalculatePowerForLevel(ClampedLevel);

        // Account for any active equipment bonuses
        float EquipHealthBonus = 0.f;
        float EquipPowerBonus = 0.f;
        if (InventoryComponent)
        {
            EquipHealthBonus = InventoryComponent->GetEquippedStatBonus(FName(TEXT("MaxHealth")));
            EquipPowerBonus = InventoryComponent->GetEquippedStatBonus(FName(TEXT("Power")));
        }

        Attributes->SetCurrentMaxHealth(NewMaxHealth + EquipHealthBonus);
        Attributes->SetCurrentPower(NewPower + EquipPowerBonus);
        Attributes->SetCurrentHealth(Attributes->GetCurrentMaxHealth());
    }
}

bool AWyrmCharacter::AddExperience(float Amount)
{
    if (Amount <= 0.f)
    {
        return false;
    }

    const float StartingLevel = GetCharacterLevel();
    CurrentXP += Amount;
    CheckLevelUp();
    return GetCharacterLevel() > StartingLevel;
}

void AWyrmCharacter::CheckLevelUp()
{
    float RequiredXP = GetXPForNextLevel();
    while (RequiredXP > 0.f && CurrentXP >= RequiredXP)
    {
        CurrentXP -= RequiredXP;
        SetCharacterLevel(GetCharacterLevel() + 1.f);
        RequiredXP = GetXPForNextLevel();
    }
}

void AWyrmCharacter::SetWet(bool bInWet)
{
    bIsWet = bInWet;
    if (AbilitySystem)
    {
        static const FGameplayTag WetTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Wet")), false);
        if (WetTag.IsValid())
        {
            if (bIsWet)
            {
                if (!AbilitySystem->HasMatchingGameplayTag(WetTag))
                {
                    AbilitySystem->AddLooseGameplayTag(WetTag);
                }
            }
            else
            {
                if (AbilitySystem->HasMatchingGameplayTag(WetTag))
                {
                    AbilitySystem->RemoveLooseGameplayTag(WetTag);
                }
            }
        }
    }
}

void AWyrmCharacter::ApplyFoodBuff(const FWyrmActiveFoodBuff& InBuff)
{
    if (InBuff.BuffId.IsNone() || (InBuff.RemainingDuration <= 0.f && InBuff.TotalDuration <= 0.f))
    {
        return;
    }

    // Case 1: Consuming identical preparation buff refreshes duration (ACT-04)
    if (ActiveFoodBuff.IsActive() && ActiveFoodBuff.BuffId == InBuff.BuffId)
    {
        ActiveFoodBuff.RemainingDuration = InBuff.RemainingDuration > 0.f
            ? InBuff.RemainingDuration
            : InBuff.TotalDuration;
        return;
    }

    // Case 2: Consuming different preparation buff replaces existing buff (ACT-04)
    if (ActiveFoodBuff.IsActive())
    {
        ClearFoodBuff();
    }

    ActiveFoodBuff = InBuff;
    if (ActiveFoodBuff.RemainingDuration <= 0.f)
    {
        ActiveFoodBuff.RemainingDuration = InBuff.TotalDuration;
    }

    // Apply MaxFocus percent bonus via GAS attributes (ACT-04)
    if (Attributes && ActiveFoodBuff.MaxFocusPercentBonus > 0.f)
    {
        const float CurrentMaxFocus = Attributes->GetCurrentMaxFocus();
        const float NewMaxFocus = CurrentMaxFocus * (1.f + ActiveFoodBuff.MaxFocusPercentBonus);
        Attributes->SetCurrentMaxFocus(NewMaxFocus);
        // Do NOT grant a free Focus refill; keep current focus unchanged (clamped to max)
        Attributes->SetCurrentFocus(FMath::Min(Attributes->GetCurrentFocus(), NewMaxFocus));
    }

    if (Attributes && ActiveFoodBuff.PowerBonus != 0.f)
    {
        Attributes->SetCurrentPower(Attributes->GetCurrentPower() + ActiveFoodBuff.PowerBonus);
    }
}

void AWyrmCharacter::ClearFoodBuff()
{
    // Expired buffs still own their modifiers until this method removes them.
    if (ActiveFoodBuff.BuffId.IsNone())
    {
        return;
    }

    // Revert MaxFocus bonus
    if (Attributes && ActiveFoodBuff.MaxFocusPercentBonus > 0.f)
    {
        const float ScaledMaxFocus = Attributes->GetCurrentMaxFocus();
        const float BaseMaxFocus = ScaledMaxFocus / (1.f + ActiveFoodBuff.MaxFocusPercentBonus);
        Attributes->SetCurrentMaxFocus(BaseMaxFocus);
        Attributes->SetCurrentFocus(FMath::Min(Attributes->GetCurrentFocus(), BaseMaxFocus));
    }


    if (Attributes && ActiveFoodBuff.PowerBonus != 0.f)
    {
        Attributes->SetCurrentPower(FMath::Max(0.f, Attributes->GetCurrentPower() - ActiveFoodBuff.PowerBonus));
    }

    ActiveFoodBuff = FWyrmActiveFoodBuff();
}

bool AWyrmCharacter::ConsumeItem(const FGuid& ItemInstanceId)
{
    if (!InventoryComponent)
    {
        return false;
    }

    const TArray<FWyrmItemInstance>& Bag = InventoryComponent->GetBagItems();
    const FWyrmItemInstance* FoundItem = nullptr;
    for (const FWyrmItemInstance& Item : Bag)
    {
        if (Item.InstanceId == ItemInstanceId)
        {
            FoundItem = &Item;
            break;
        }
    }

    if (!FoundItem || FoundItem->ItemType != EWyrmItemType::Consumable)
    {
        return false;
    }

    // Check healing consumable waste rejection (ACTIVITIES_AND_BUILDING.md line 50)
    const float HealthHeal = FoundItem->GetStatValue(TEXT("Buff.HealthRegen"));
    const float MaxFocusBonus = FoundItem->GetStatValue(TEXT("Buff.MaxFocusPercentBonus"));
    const float Duration = FoundItem->GetStatValue(TEXT("Buff.Duration"));

    if (Attributes && HealthHeal > 0.f && MaxFocusBonus <= 0.f && Duration <= 1.f)
    {
        if (Attributes->GetCurrentHealth() >= Attributes->GetCurrentMaxHealth())
        {
            // Already at full health: reject waste
            return false;
        }

        // Apply instant healing
        Attributes->SetCurrentHealth(FMath::Min(Attributes->GetCurrentMaxHealth(), Attributes->GetCurrentHealth() + HealthHeal));
        InventoryComponent->RemoveItem(ItemInstanceId, 1);
        return true;
    }

    // Food preparation buff
    FWyrmActiveFoodBuff NewBuff;
    NewBuff.BuffId = FoundItem->ItemId;
    NewBuff.BuffName = FoundItem->DisplayName;
    NewBuff.TotalDuration = Duration > 0.f ? Duration : 300.f;
    NewBuff.RemainingDuration = NewBuff.TotalDuration;
    NewBuff.MaxFocusPercentBonus = MaxFocusBonus;
    NewBuff.HealthRegenPerSecond = HealthHeal;
    NewBuff.PowerBonus = FoundItem->GetStatValue(TEXT("Buff.Power"));

    ApplyFoodBuff(NewBuff);
    InventoryComponent->RemoveItem(ItemInstanceId, 1);
    return true;
}

void AWyrmCharacter::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue < Data.OldValue)
    {
        BreakHuntersVeil();
        float DamageTaken = Data.OldValue - Data.NewValue;
        OnCharacterDamaged.Broadcast(DamageTaken);
    }
}

// --- Status Reactions & Gating (WP-15) ---

void AWyrmCharacter::ApplyStatusEffect(FGameplayTag StatusTag, float DurationSeconds, float Magnitude)
{
    if (Attributes && Attributes->GetHealth() <= 0.f)
    {
        return;
    }

    static const FGameplayTag SlowTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Slow")), false);
    static const FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Root")), false);
    static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stun")), false);
    static const FGameplayTag StaggerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stagger")), false);

    if (StatusTag == SlowTag)
    {
        ActiveSlowMagnitude = FMath::Max(ActiveSlowMagnitude, Magnitude);
        SlowRemainingTimer = FMath::Max(SlowRemainingTimer, DurationSeconds);
        if (AbilitySystem && SlowTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(SlowTag))
        {
            AbilitySystem->AddLooseGameplayTag(SlowTag);
        }
    }
    else if (StatusTag == RootTag)
    {
        RootRemainingTimer = FMath::Max(RootRemainingTimer, DurationSeconds);
        if (AbilitySystem && RootTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(RootTag))
        {
            AbilitySystem->AddLooseGameplayTag(RootTag);
        }
    }
    else if (StatusTag == StunTag)
    {
        StunRemainingTimer = FMath::Max(StunRemainingTimer, DurationSeconds);
        if (AbilitySystem && StunTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(StunTag))
        {
            AbilitySystem->AddLooseGameplayTag(StunTag);
        }
    }
    else if (StatusTag == StaggerTag)
    {
        // Relentless Advance resists light/medium stagger reactions (Section 7)
        if (!bRelentlessAdvanceActive)
        {
            StaggerRemainingTimer = FMath::Max(StaggerRemainingTimer, DurationSeconds);
            if (AbilitySystem && StaggerTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(StaggerTag))
            {
                AbilitySystem->AddLooseGameplayTag(StaggerTag);
            }
        }
    }
    else
    {
        // Fallback for custom or control tags (e.g. State.Control.Transition)
        if (AbilitySystem && StatusTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(StatusTag))
        {
            AbilitySystem->AddLooseGameplayTag(StatusTag);
        }
    }

    UpdateMovementForStatus();
}

void AWyrmCharacter::ApplyNamedStatusEffect(FName TagName, float DurationSeconds, float Magnitude)
{
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
    if (Tag.IsValid())
    {
        ApplyStatusEffect(Tag, DurationSeconds, Magnitude);
    }
}

void AWyrmCharacter::ClearNamedStatusEffect(FName TagName)
{
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
    if (Tag.IsValid() && AbilitySystem)
    {
        if (AbilitySystem->HasMatchingGameplayTag(Tag))
        {
            AbilitySystem->RemoveLooseGameplayTag(Tag);
        }
        FGameplayTagContainer TagContainer(Tag);
        AbilitySystem->RemoveActiveEffectsWithGrantedTags(TagContainer);
    }

    if (TagName == FName(TEXT("State.Combat.Slow")))
    {
        SlowRemainingTimer = 0.f;
        ActiveSlowMagnitude = 0.f;
    }
    else if (TagName == FName(TEXT("State.Combat.Root")))
    {
        RootRemainingTimer = 0.f;
    }
    else if (TagName == FName(TEXT("State.Combat.Stun")))
    {
        StunRemainingTimer = 0.f;
    }
    else if (TagName == FName(TEXT("State.Combat.Stagger")))
    {
        StaggerRemainingTimer = 0.f;
    }
    else if (TagName == FName(TEXT("State.Combat.RelentlessAdvance")))
    {
        bRelentlessAdvanceActive = false;
        RelentlessAdvanceRemainingTimer = 0.f;
    }
    else if (TagName == FName(TEXT("Cooldown.Echo.RelentlessAdvance")))
    {
        RelentlessAdvanceCooldownTimer = 0.f;
    }
    else if (TagName == FName(TEXT("State.Combat.MoonboundForm")))
    {
        DeactivateMoonboundForm();
    }
    else if (TagName == FName(TEXT("Cooldown.Echo.MoonboundForm")))
    {
        MoonboundCooldownTimer = 0.f;
    }

    UpdateMovementForStatus();
}

void AWyrmCharacter::UpdateMovementForStatus()
{
    if (!GetCharacterMovement())
    {
        return;
    }

    const bool bDead = (Attributes && Attributes->GetHealth() <= 0.f);
    if (bDead || bMovementLocked || StunRemainingTimer > 0.f || RootRemainingTimer > 0.f)
    {
        GetCharacterMovement()->MaxWalkSpeed = 0.f;
    }
    else if (bRelentlessAdvanceActive)
    {
        // Suppress movement slows during active Relentless Advance (Section 7)
        // No speed boost: capped at BaseWalkSpeed.
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
    else if (bMoonboundActive)
    {
        float BeastBase = 700.f;
        if (SlowRemainingTimer > 0.f && ActiveSlowMagnitude > 0.f)
        {
            GetCharacterMovement()->MaxWalkSpeed = BeastBase * FMath::Clamp(1.f - ActiveSlowMagnitude, 0.1f, 1.f);
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = BeastBase;
        }
    }
    else if (SlowRemainingTimer > 0.f && ActiveSlowMagnitude > 0.f)
    {
        const float Multiplier = FMath::Clamp(1.0f - ActiveSlowMagnitude, 0.1f, 1.0f);
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * Multiplier;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
}

float AWyrmCharacter::GetCurrentSpeed() const
{
    return GetCharacterMovement() ? GetCharacterMovement()->MaxWalkSpeed : 0.f;
}

bool AWyrmCharacter::HasMatchingGameplayTag(FName TagName) const
{
    if (!AbilitySystem)
    {
        return false;
    }
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
    return Tag.IsValid() && AbilitySystem->HasMatchingGameplayTag(Tag);
}

// --- Horror Echo Powers (WP-15) ---

bool AWyrmCharacter::LearnEcho(FName EchoId)
{
    if (EchoId.IsNone())
    {
        return false;
    }

    if (!LearnedEchoes.Contains(EchoId))
    {
        LearnedEchoes.Add(EchoId);
    }

    if (EchoId == FName(TEXT("RelentlessAdvance")))
    {
        if (AbilitySystem)
        {
            static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.RelentlessAdvance")), false);
            if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag))
            {
                AbilitySystem->AddLooseGameplayTag(UnlockTag);
            }

            if (!RelentlessAdvanceHandle.IsValid())
            {
                RelentlessAdvanceHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmRelentlessAdvanceAbility::StaticClass(), 1, INDEX_NONE, this));
            }
        }
    }
    else if (EchoId == FName(TEXT("MoonboundForm")))
    {
        if (AbilitySystem)
        {
            static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.MoonboundForm")), false);
            if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag))
            {
                AbilitySystem->AddLooseGameplayTag(UnlockTag);
            }

            if (!MoonboundFormHandle.IsValid())
            {
                MoonboundFormHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmMoonboundFormAbility::StaticClass(), 1, INDEX_NONE, this));
            }
            if (!PrimaryBeastClawHandle.IsValid())
            {
                PrimaryBeastClawHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmBeastClawAbility::StaticClass(), 1, INDEX_NONE, this));
            }
            if (!SecondaryBeastPounceHandle.IsValid())
            {
                SecondaryBeastPounceHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmBeastPounceAbility::StaticClass(), 1, INDEX_NONE, this));
            }
        }
    }
    else if (EchoId == FName(TEXT("MirrorStep")))
    {
        if (AbilitySystem)
        {
            static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.MirrorStep")), false);
            if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag))
            {
                AbilitySystem->AddLooseGameplayTag(UnlockTag);
            }
            if (!MirrorStepHandle.IsValid())
            {
                MirrorStepHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmMirrorStepAbility::StaticClass(), 1, INDEX_NONE, this));
            }
        }
    }
    else if (EchoId == FName(TEXT("UnseenHand")))
    {
        if (AbilitySystem)
        {
            static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.UnseenHand")), false);
            if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag)) AbilitySystem->AddLooseGameplayTag(UnlockTag);
            if (!UnseenHandHandle.IsValid())
            {
                UnseenHandHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmUnseenHandAbility::StaticClass(), 1, INDEX_NONE, this));
            }
        }
    }
    else if (EchoId == FName(TEXT("HuntersVeil")))
    {
        if (AbilitySystem)
        {
            static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.HuntersVeil")), false);
            if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag)) AbilitySystem->AddLooseGameplayTag(UnlockTag);
            if (!HuntersVeilHandle.IsValid())
            {
                HuntersVeilHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmHuntersVeilAbility::StaticClass(), 1, INDEX_NONE, this));
            }
        }
    }
    else if (EchoId == FName(TEXT("SanguineStrike")))
    {
        if (AbilitySystem)
        {
            static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.SanguineStrike")), false);
            if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag)) AbilitySystem->AddLooseGameplayTag(UnlockTag);
            if (!SanguineStrikeHandle.IsValid())
            {
                SanguineStrikeHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmSanguineStrikeAbility::StaticClass(), 1, INDEX_NONE, this));
            }
        }
    }
    else if (EchoId == FName(TEXT("SecondTurn")))
    {
        if (AbilitySystem)
        {
            static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.SecondTurn")), false);
            if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag)) AbilitySystem->AddLooseGameplayTag(UnlockTag);
            if (!SecondTurnHandle.IsValid())
            {
                SecondTurnHandle = AbilitySystem->GiveAbility(
                    FGameplayAbilitySpec(UWyrmSecondTurnAbility::StaticClass(), 1, INDEX_NONE, this));
            }
        }
    }

    // Auto-equip if first echo and none equipped
    if (EquippedEcho.IsNone())
    {
        EquipEcho(EchoId);
    }

    return true;
}

bool AWyrmCharacter::EquipEcho(FName EchoId)
{
    if (!LearnedEchoes.Contains(EchoId))
    {
        return false;
    }
    EquippedEcho = EchoId;
    return true;
}

void AWyrmCharacter::UnequipEcho()
{
    // Cooldown is NOT cleared on unequip per Section 7 contract.
    EquippedEcho = NAME_None;
}

bool AWyrmCharacter::IsEchoUnlocked(FName EchoId) const
{
    return LearnedEchoes.Contains(EchoId);
}

bool AWyrmCharacter::IsEchoEquipped(FName EchoId) const
{
    return !EquippedEcho.IsNone() && EquippedEcho == EchoId;
}

bool AWyrmCharacter::CanActivateRelentlessAdvance(FString& OutFailureReason) const
{
    if (!LearnedEchoes.Contains(FName(TEXT("RelentlessAdvance"))))
    {
        OutFailureReason = TEXT("NotUnlocked");
        return false;
    }

    if (EquippedEcho != FName(TEXT("RelentlessAdvance")))
    {
        OutFailureReason = TEXT("NotEquipped");
        return false;
    }

    if (Attributes && Attributes->GetHealth() <= 0.f)
    {
        OutFailureReason = TEXT("Dead");
        return false;
    }

    if (StunRemainingTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("State.Combat.Stun"))))
    {
        OutFailureReason = TEXT("HardStunned");
        return false;
    }

    if (HasMatchingGameplayTag(FName(TEXT("State.Control.Transition"))))
    {
        OutFailureReason = TEXT("ControlTransitioning");
        return false;
    }

    if (RelentlessAdvanceCooldownTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("Cooldown.Echo.RelentlessAdvance"))))
    {
        OutFailureReason = TEXT("OnCooldown");
        return false;
    }

    if (Attributes && Attributes->GetFocus() < 30.f)
    {
        OutFailureReason = TEXT("InsufficientFocus");
        return false;
    }

    OutFailureReason = TEXT("");
    return true;
}

bool AWyrmCharacter::ActivateEquippedEcho()
{
    if (EquippedEcho.IsNone())
    {
        return false;
    }

    if (EquippedEcho == FName(TEXT("RelentlessAdvance")))
    {
        FString Reason;
        if (!CanActivateRelentlessAdvance(Reason))
        {
            return false;
        }

        if (AbilitySystem && RelentlessAdvanceHandle.IsValid())
        {
            return AbilitySystem->TryActivateAbility(RelentlessAdvanceHandle);
        }
    }
    else if (EquippedEcho == FName(TEXT("MoonboundForm")))
    {
        FString Reason;
        if (!CanActivateMoonboundForm(Reason))
        {
            return false;
        }

        if (AbilitySystem && MoonboundFormHandle.IsValid())
        {
            return AbilitySystem->TryActivateAbility(MoonboundFormHandle);
        }
    }
    else if (EquippedEcho == FName(TEXT("HuntersVeil")))
    {
        return ActivateHuntersVeil();
    }
    else if (EquippedEcho == FName(TEXT("SanguineStrike")))
    {
        return ActivateSanguineStrike();
    }
    else if (EquippedEcho == FName(TEXT("SecondTurn")))
    {
        return ActivateSecondTurn();
    }

    return false;
}

bool AWyrmCharacter::CanActivateSanguineStrike(FString& OutFailureReason) const
{
    if (!LearnedEchoes.Contains(FName(TEXT("SanguineStrike")))) { OutFailureReason = TEXT("NotUnlocked"); return false; }
    if (EquippedEcho != FName(TEXT("SanguineStrike"))) { OutFailureReason = TEXT("NotEquipped"); return false; }
    if (!Attributes || Attributes->GetHealth() <= 0.f) { OutFailureReason = TEXT("Dead"); return false; }
    if (bSanguineStrikePrimed) { OutFailureReason = TEXT("AlreadyPrimed"); return false; }
    if (HasMatchingGameplayTag(FName(TEXT("State.Combat.Stun"))) || HasMatchingGameplayTag(FName(TEXT("State.Control.Transition"))))
    { OutFailureReason = TEXT("ControlBlocked"); return false; }
    if (SanguineStrikeCooldownTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("Cooldown.Echo.SanguineStrike"))))
    { OutFailureReason = TEXT("OnCooldown"); return false; }
    if (Attributes->GetFocus() < 25.f) { OutFailureReason = TEXT("InsufficientFocus"); return false; }
    OutFailureReason.Reset();
    return true;
}

bool AWyrmCharacter::ActivateSanguineStrike()
{
    FString Reason;
    return CanActivateSanguineStrike(Reason) && AbilitySystem && SanguineStrikeHandle.IsValid() &&
        AbilitySystem->TryActivateAbility(SanguineStrikeHandle);
}

bool AWyrmCharacter::CommitSanguineStrike()
{
    if (bSanguineStrikePrimed)
    {
        return false;
    }
    bSanguineStrikePrimed = true;
    SanguineStrikeWindowTimer = 4.f;
    SanguineStrikeCooldownTimer = 12.f;
    if (AbilitySystem)
    {
        static const FGameplayTag PrimedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SanguineStrikePrimed")), false);
        if (PrimedTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(PrimedTag)) AbilitySystem->AddLooseGameplayTag(PrimedTag);
    }
    return true;
}

float AWyrmCharacter::GetSanguineStrikeBonusDamage() const
{
    return bSanguineStrikePrimed && SanguineStrikeWindowTimer > 0.f && Attributes
        ? FMath::Max(0.f, Attributes->GetPower() * 0.5f)
        : 0.f;
}

bool AWyrmCharacter::ConsumeSanguineStrike(float ActualDamage)
{
    if (!bSanguineStrikePrimed || !AbilitySystem || !Attributes)
    {
        return false;
    }

    bSanguineStrikePrimed = false;
    SanguineStrikeWindowTimer = 0.f;
    static const FGameplayTag PrimedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SanguineStrikePrimed")), false);
    if (PrimedTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(PrimedTag);

    const float HealAmount = FMath::Min(ActualDamage * 0.25f, Attributes->GetMaxHealth() * 0.12f);
    if (HealAmount > 0.f)
    {
        UGameplayEffect* HealingGE = NewObject<UGameplayEffect>();
        HealingGE->DurationPolicy = EGameplayEffectDurationType::Instant;
        FGameplayModifierInfo ModInfo;
        ModInfo.Attribute = UWyrmAttributeSet::GetIncomingHealingAttribute();
        ModInfo.ModifierOp = EGameplayModOp::Additive;
        ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(HealAmount));
        HealingGE->Modifiers.Add(ModInfo);
        AbilitySystem->ApplyGameplayEffectToSelf(HealingGE, 1.f, AbilitySystem->MakeEffectContext());
    }
    return true;
}

void AWyrmCharacter::RestoreSanguineStrikeState(float RemainingCooldown)
{
    bSanguineStrikePrimed = false;
    SanguineStrikeWindowTimer = 0.f;
    SanguineStrikeCooldownTimer = FMath::Max(0.f, RemainingCooldown);
    if (!AbilitySystem) return;

    static const FGameplayTag PrimedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SanguineStrikePrimed")), false);
    static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.SanguineStrike")), false);
    if (PrimedTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(PrimedTag);
    if (CooldownTag.IsValid())
    {
        AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
        FGameplayTagContainer CooldownTags(CooldownTag);
        AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
        if (SanguineStrikeCooldownTimer > 0.f) AbilitySystem->AddLooseGameplayTag(CooldownTag);
    }
}

bool AWyrmCharacter::CanActivateSecondTurn(FString& OutFailureReason) const
{
    if (!LearnedEchoes.Contains(FName(TEXT("SecondTurn")))) { OutFailureReason = TEXT("NotUnlocked"); return false; }
    if (EquippedEcho != FName(TEXT("SecondTurn"))) { OutFailureReason = TEXT("NotEquipped"); return false; }
    if (!Attributes || Attributes->GetHealth() <= 0.f) { OutFailureReason = TEXT("Dead"); return false; }
    if (bSecondTurnPrimed || PendingSecondTurnTarget.IsValid()) { OutFailureReason = TEXT("AlreadyPending"); return false; }
    if (HasMatchingGameplayTag(FName(TEXT("State.Combat.Stun"))) || HasMatchingGameplayTag(FName(TEXT("State.Control.Transition"))))
    { OutFailureReason = TEXT("ControlBlocked"); return false; }
    if (SecondTurnCooldownTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("Cooldown.Echo.SecondTurn"))))
    { OutFailureReason = TEXT("OnCooldown"); return false; }
    if (Attributes->GetFocus() < 25.f) { OutFailureReason = TEXT("InsufficientFocus"); return false; }
    OutFailureReason.Reset();
    return true;
}

bool AWyrmCharacter::ActivateSecondTurn()
{
    FString Reason;
    return CanActivateSecondTurn(Reason) && AbilitySystem && SecondTurnHandle.IsValid() &&
        AbilitySystem->TryActivateAbility(SecondTurnHandle);
}

bool AWyrmCharacter::CommitSecondTurn()
{
    if (bSecondTurnPrimed || PendingSecondTurnTarget.IsValid())
    {
        return false;
    }
    bSecondTurnPrimed = true;
    SecondTurnCooldownTimer = 14.f;
    if (AbilitySystem)
    {
        static const FGameplayTag PrimedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SecondTurnPrimed")), false);
        if (PrimedTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(PrimedTag)) AbilitySystem->AddLooseGameplayTag(PrimedTag);
    }
    return true;
}

bool AWyrmCharacter::QueueSecondTurnRepeat(UAbilitySystemComponent* TargetASC, float SnapshottedBaseRawDamage)
{
    if (!bSecondTurnPrimed || PendingSecondTurnTarget.IsValid() || !TargetASC || SnapshottedBaseRawDamage <= 0.f)
    {
        return false;
    }
    bSecondTurnPrimed = false;
    PendingSecondTurnTarget = TargetASC;
    PendingSecondTurnRawDamage = SnapshottedBaseRawDamage * 0.5f;
    SecondTurnRepeatTimer = 0.6f;
    if (AbilitySystem)
    {
        static const FGameplayTag PrimedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SecondTurnPrimed")), false);
        static const FGameplayTag PendingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SecondTurnPending")), false);
        if (PrimedTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(PrimedTag);
        if (PendingTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(PendingTag)) AbilitySystem->AddLooseGameplayTag(PendingTag);
    }
    return true;
}

void AWyrmCharacter::RestoreSecondTurnState(float RemainingCooldown)
{
    bSecondTurnPrimed = false;
    PendingSecondTurnTarget.Reset();
    PendingSecondTurnRawDamage = 0.f;
    SecondTurnRepeatTimer = 0.f;
    SecondTurnCooldownTimer = FMath::Max(0.f, RemainingCooldown);
    if (!AbilitySystem) return;

    static const FGameplayTag PrimedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SecondTurnPrimed")), false);
    static const FGameplayTag PendingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.SecondTurnPending")), false);
    static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.SecondTurn")), false);
    if (PrimedTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(PrimedTag);
    if (PendingTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(PendingTag);
    if (CooldownTag.IsValid())
    {
        AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
        FGameplayTagContainer CooldownTags(CooldownTag);
        AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
        if (SecondTurnCooldownTimer > 0.f) AbilitySystem->AddLooseGameplayTag(CooldownTag);
    }
}

bool AWyrmCharacter::CanActivateMirrorStep(const FVector& Destination, FString& OutFailureReason) const
{
    if (!LearnedEchoes.Contains(FName(TEXT("MirrorStep"))))
    {
        OutFailureReason = TEXT("NotUnlocked");
        return false;
    }
    if (EquippedEcho != FName(TEXT("MirrorStep")))
    {
        OutFailureReason = TEXT("NotEquipped");
        return false;
    }
    if (!Attributes || Attributes->GetHealth() <= 0.f)
    {
        OutFailureReason = TEXT("Dead");
        return false;
    }
    if (HasMatchingGameplayTag(FName(TEXT("State.Combat.Stun"))) ||
        HasMatchingGameplayTag(FName(TEXT("State.Control.Transition"))))
    {
        OutFailureReason = TEXT("ControlBlocked");
        return false;
    }
    if (MirrorStepCooldownTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("Cooldown.Echo.MirrorStep"))))
    {
        OutFailureReason = TEXT("OnCooldown");
        return false;
    }
    if (Attributes->GetFocus() < 20.f)
    {
        OutFailureReason = TEXT("InsufficientFocus");
        return false;
    }
    const FVector Start = GetActorLocation();
    if (FVector::Dist(Start, Destination) > 400.f + KINDA_SMALL_NUMBER)
    {
        OutFailureReason = TEXT("OutOfRange");
        return false;
    }
    UWorld* World = GetWorld();
    if (!World || !GetCapsuleComponent())
    {
        OutFailureReason = TEXT("NoWorld");
        return false;
    }
    for (TActorIterator<AWyrmWaterVolume> It(World); It; ++It)
    {
        if (It->IsPointInWater(Destination))
        {
            OutFailureReason = TEXT("WaterDestination");
            return false;
        }
    }
    FNavLocation Projected;
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!Nav || !Nav->ProjectPointToNavigation(Destination, Projected, FVector(50.f, 50.f, 150.f)))
    {
        OutFailureReason = TEXT("UnreachableOrVoid");
        return false;
    }
    if (FVector::DistSquared2D(Projected.Location, Destination) > FMath::Square(60.f))
    {
        OutFailureReason = TEXT("UnreachableOrVoid");
        return false;
    }

    const FVector LandingLocation = Projected.Location + FVector(
        0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MirrorStepValidation), false, this);
    FHitResult BarrierHit;
    const FVector EyeOffset(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.5f);
    if (World->LineTraceSingleByChannel(BarrierHit, Start + EyeOffset, LandingLocation + EyeOffset, ECC_Visibility, Params))
    {
        OutFailureReason = TEXT("SealedBoundary");
        return false;
    }

    const FCollisionShape Capsule = FCollisionShape::MakeCapsule(
        GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    if (World->OverlapBlockingTestByChannel(LandingLocation, GetActorQuat(), ECC_Pawn, Capsule, Params))
    {
        OutFailureReason = TEXT("DestinationOccupied");
        return false;
    }

    FHitResult FloorHit;
    if (!World->LineTraceSingleByChannel(FloorHit, LandingLocation + FVector(0.f, 0.f, 100.f),
        LandingLocation - FVector(0.f, 0.f, 250.f), ECC_Visibility, Params))
    {
        OutFailureReason = TEXT("NoSafeFloor");
        return false;
    }

    OutFailureReason.Reset();
    return true;
}

FString AWyrmCharacter::GetMirrorStepFailureReason(const FVector& Destination) const
{
    FString FailureReason;
    return CanActivateMirrorStep(Destination, FailureReason) ? FString() : FailureReason;
}

bool AWyrmCharacter::ActivateMirrorStep(const FVector& Destination)
{
    FString Reason;
    if (!CanActivateMirrorStep(Destination, Reason) || !AbilitySystem || !MirrorStepHandle.IsValid())
    {
        return false;
    }
    PendingMirrorStepDestination = Destination;
    bHasPendingMirrorStepDestination = true;
    const bool bActivated = AbilitySystem->TryActivateAbility(MirrorStepHandle);
    if (!bActivated)
    {
        bHasPendingMirrorStepDestination = false;
    }
    return bActivated;
}

bool AWyrmCharacter::CommitMirrorStep()
{
    if (!bHasPendingMirrorStepDestination)
    {
        return false;
    }
    FNavLocation Projected;
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!Nav || !Nav->ProjectPointToNavigation(PendingMirrorStepDestination, Projected, FVector(50.f, 50.f, 150.f)))
    {
        bHasPendingMirrorStepDestination = false;
        return false;
    }
    const FVector LandingLocation = Projected.Location + FVector(
        0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    const bool bMoved = SetActorLocation(LandingLocation, false, nullptr, ETeleportType::TeleportPhysics);
    bHasPendingMirrorStepDestination = false;
    if (bMoved)
    {
        MirrorStepCooldownTimer = 10.f;
    }
    return bMoved;
}

void AWyrmCharacter::RestoreMirrorStepState(float RemainingCooldown)
{
    MirrorStepCooldownTimer = FMath::Max(0.f, RemainingCooldown);
    if (!AbilitySystem)
    {
        return;
    }
    static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.MirrorStep")), false);
    if (CooldownTag.IsValid())
    {
        if (MirrorStepCooldownTimer > 0.f)
        {
            if (!AbilitySystem->HasMatchingGameplayTag(CooldownTag))
            {
                AbilitySystem->AddLooseGameplayTag(CooldownTag);
            }
        }
        else
        {
            AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
            FGameplayTagContainer CooldownTags(CooldownTag);
            AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
        }
    }
}

bool AWyrmCharacter::CanActivateUnseenHand(AActor* Target, FString& OutFailureReason) const
{
    if (!LearnedEchoes.Contains(FName(TEXT("UnseenHand")))) { OutFailureReason = TEXT("NotUnlocked"); return false; }
    if (EquippedEcho != FName(TEXT("UnseenHand"))) { OutFailureReason = TEXT("NotEquipped"); return false; }
    if (!Target || Target == this) { OutFailureReason = TEXT("InvalidTarget"); return false; }
    if (!Attributes || Attributes->GetHealth() <= 0.f) { OutFailureReason = TEXT("Dead"); return false; }
    if (HasMatchingGameplayTag(FName(TEXT("State.Combat.Stun"))) || HasMatchingGameplayTag(FName(TEXT("State.Control.Transition"))))
    { OutFailureReason = TEXT("ControlBlocked"); return false; }
    if (UnseenHandCooldownTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("Cooldown.Echo.UnseenHand"))))
    { OutFailureReason = TEXT("OnCooldown"); return false; }
    if (Attributes->GetFocus() < 25.f) { OutFailureReason = TEXT("InsufficientFocus"); return false; }
    if (FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) > FMath::Square(900.f))
    { OutFailureReason = TEXT("OutOfRange"); return false; }

    if (const AWyrmEnemyCharacter* Enemy = Cast<AWyrmEnemyCharacter>(Target))
    {
        if (Enemy->bIsBoss || Enemy->IsDefeated()) { OutFailureReason = TEXT("HeavyOrBossTarget"); return false; }
        OutFailureReason.Reset(); return true;
    }
    if (const AWyrmUnseenHandTarget* Prop = Cast<AWyrmUnseenHandTarget>(Target))
    {
        if (!Prop->IsEligibleForUnseenHand()) { OutFailureReason = TEXT("HeavyOrUnauthoredTarget"); return false; }
        OutFailureReason.Reset(); return true;
    }
    OutFailureReason = TEXT("HeavyOrUnauthoredTarget");
    return false;
}

bool AWyrmCharacter::ActivateUnseenHand(AActor* Target)
{
    FString Reason;
    if (!CanActivateUnseenHand(Target, Reason) || !AbilitySystem || !UnseenHandHandle.IsValid()) return false;
    PendingUnseenHandTarget = Target;
    const bool bActivated = AbilitySystem->TryActivateAbility(UnseenHandHandle);
    if (!bActivated) PendingUnseenHandTarget.Reset();
    return bActivated;
}

FString AWyrmCharacter::GetUnseenHandFailureReason(AActor* Target) const
{
    FString Reason;
    return CanActivateUnseenHand(Target, Reason) ? FString() : Reason;
}

bool AWyrmCharacter::CommitUnseenHand()
{
    AActor* Target = PendingUnseenHandTarget.Get();
    const AWyrmEnemyCharacter* EnemyTarget = Cast<AWyrmEnemyCharacter>(Target);
    const AWyrmUnseenHandTarget* PropTarget = Cast<AWyrmUnseenHandTarget>(Target);
    const bool bEligibleEnemy = EnemyTarget && !EnemyTarget->bIsBoss && !EnemyTarget->IsDefeated();
    const bool bEligibleProp = PropTarget && PropTarget->IsEligibleForUnseenHand();
    if (!Target || (!bEligibleEnemy && !bEligibleProp) ||
        FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) > FMath::Square(900.f))
    {
        PendingUnseenHandTarget.Reset();
        return false;
    }
    FVector Direction = Target->GetActorLocation() - GetActorLocation();
    Direction.Z = FMath::Max(Direction.Z, 100.f);
    Direction = Direction.GetSafeNormal();
    bool bApplied = false;
    if (AWyrmEnemyCharacter* Enemy = Cast<AWyrmEnemyCharacter>(Target))
    {
        Enemy->LaunchCharacter(Direction * 900.f + FVector(0.f, 0.f, 250.f), true, true);
        bApplied = true;
    }
    else if (AWyrmUnseenHandTarget* Prop = Cast<AWyrmUnseenHandTarget>(Target))
    {
        bApplied = Prop->ApplyUnseenHandImpulse(Direction * 1100.f);
    }
    PendingUnseenHandTarget.Reset();
    if (bApplied) UnseenHandCooldownTimer = 8.f;
    return bApplied;
}

void AWyrmCharacter::RestoreUnseenHandState(float RemainingCooldown)
{
    UnseenHandCooldownTimer = FMath::Max(0.f, RemainingCooldown);
    if (!AbilitySystem) return;
    static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.UnseenHand")), false);
    if (!CooldownTag.IsValid()) return;
    if (UnseenHandCooldownTimer > 0.f)
    {
        if (!AbilitySystem->HasMatchingGameplayTag(CooldownTag)) AbilitySystem->AddLooseGameplayTag(CooldownTag);
    }
    else
    {
        AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
        FGameplayTagContainer CooldownTags(CooldownTag);
        AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
    }
}

bool AWyrmCharacter::CanActivateHuntersVeil(FString& OutFailureReason) const
{
    if (!LearnedEchoes.Contains(FName(TEXT("HuntersVeil")))) { OutFailureReason = TEXT("NotUnlocked"); return false; }
    if (EquippedEcho != FName(TEXT("HuntersVeil"))) { OutFailureReason = TEXT("NotEquipped"); return false; }
    if (!Attributes || Attributes->GetHealth() <= 0.f) { OutFailureReason = TEXT("Dead"); return false; }
    if (bHuntersVeilActive) { OutFailureReason = TEXT("AlreadyActive"); return false; }
    if (HasMatchingGameplayTag(FName(TEXT("State.Combat.Stun"))) || HasMatchingGameplayTag(FName(TEXT("State.Control.Transition"))))
    { OutFailureReason = TEXT("ControlBlocked"); return false; }
    if (HuntersVeilCooldownTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("Cooldown.Echo.HuntersVeil"))))
    { OutFailureReason = TEXT("OnCooldown"); return false; }
    if (Attributes->GetFocus() < 25.f) { OutFailureReason = TEXT("InsufficientFocus"); return false; }
    OutFailureReason.Reset();
    return true;
}

bool AWyrmCharacter::ActivateHuntersVeil()
{
    FString Reason;
    return CanActivateHuntersVeil(Reason) && AbilitySystem && HuntersVeilHandle.IsValid() &&
        AbilitySystem->TryActivateAbility(HuntersVeilHandle);
}

bool AWyrmCharacter::CommitHuntersVeil()
{
    if (bHuntersVeilActive)
    {
        return false;
    }
    bHuntersVeilActive = true;
    HuntersVeilRemainingTimer = 5.f;
    HuntersVeilCooldownTimer = 16.f;
    if (AbilitySystem)
    {
        static const FGameplayTag StateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.HuntersVeil")), false);
        if (StateTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(StateTag)) AbilitySystem->AddLooseGameplayTag(StateTag);
    }
    return true;
}

void AWyrmCharacter::BreakHuntersVeil()
{
    bHuntersVeilActive = false;
    HuntersVeilRemainingTimer = 0.f;
    if (AbilitySystem)
    {
        static const FGameplayTag StateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.HuntersVeil")), false);
        if (StateTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(StateTag);
    }
}

void AWyrmCharacter::RestoreHuntersVeilState(bool bActive, float RemainingDuration, float RemainingCooldown)
{
    bHuntersVeilActive = bActive && RemainingDuration > 0.f;
    HuntersVeilRemainingTimer = bHuntersVeilActive ? FMath::Min(5.f, RemainingDuration) : 0.f;
    HuntersVeilCooldownTimer = FMath::Max(0.f, RemainingCooldown);
    if (!AbilitySystem) return;
    static const FGameplayTag StateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.HuntersVeil")), false);
    static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.HuntersVeil")), false);
    if (StateTag.IsValid())
    {
        if (bHuntersVeilActive) AbilitySystem->AddLooseGameplayTag(StateTag); else AbilitySystem->RemoveLooseGameplayTag(StateTag);
    }
    if (CooldownTag.IsValid())
    {
        if (HuntersVeilCooldownTimer > 0.f) AbilitySystem->AddLooseGameplayTag(CooldownTag);
        else
        {
            AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
            FGameplayTagContainer CooldownTags(CooldownTag);
            AbilitySystem->RemoveActiveEffectsWithGrantedTags(CooldownTags);
        }
    }
}

void AWyrmCharacter::ActivateRelentlessAdvanceStance(float Duration)
{
    bRelentlessAdvanceActive = true;
    RelentlessAdvanceRemainingTimer = Duration > 0.f ? Duration : 6.0f;
    RelentlessAdvanceCooldownTimer = 18.0f;

    if (AbilitySystem)
    {
        static const FGameplayTag RelentlessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.RelentlessAdvance")), false);
        if (RelentlessTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(RelentlessTag))
        {
            AbilitySystem->AddLooseGameplayTag(RelentlessTag);
        }

        static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.RelentlessAdvance")), false);
        if (CooldownTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(CooldownTag))
        {
            AbilitySystem->AddLooseGameplayTag(CooldownTag);
        }
    }

    // Suppress any existing slow immediately
    UpdateMovementForStatus();
}

void AWyrmCharacter::RestoreEchoState(const TArray<FName>& InLearnedEchoes, FName InEquippedEcho, bool bActive, float RemainingDuration, float RemainingCooldown)
{
    LearnedEchoes = InLearnedEchoes;
    EquippedEcho = InEquippedEcho;

    for (const FName& EchoId : LearnedEchoes)
    {
        if (EchoId == FName(TEXT("RelentlessAdvance")))
        {
            if (AbilitySystem)
            {
                static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.RelentlessAdvance")), false);
                if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag))
                {
                    AbilitySystem->AddLooseGameplayTag(UnlockTag);
                }

                if (!RelentlessAdvanceHandle.IsValid())
                {
                    RelentlessAdvanceHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmRelentlessAdvanceAbility::StaticClass(), 1, INDEX_NONE, this));
                }
            }
        }
        else if (EchoId == FName(TEXT("MirrorStep")))
        {
            if (AbilitySystem)
            {
                static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.MirrorStep")), false);
                if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag))
                {
                    AbilitySystem->AddLooseGameplayTag(UnlockTag);
                }
                if (!MirrorStepHandle.IsValid())
                {
                    MirrorStepHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmMirrorStepAbility::StaticClass(), 1, INDEX_NONE, this));
                }
            }
        }
        else if (EchoId == FName(TEXT("UnseenHand")))
        {
            if (AbilitySystem)
            {
                static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.UnseenHand")), false);
                if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag)) AbilitySystem->AddLooseGameplayTag(UnlockTag);
                if (!UnseenHandHandle.IsValid())
                {
                    UnseenHandHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmUnseenHandAbility::StaticClass(), 1, INDEX_NONE, this));
                }
            }
        }
        else if (EchoId == FName(TEXT("HuntersVeil")))
        {
            if (AbilitySystem)
            {
                static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.HuntersVeil")), false);
                if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag)) AbilitySystem->AddLooseGameplayTag(UnlockTag);
                if (!HuntersVeilHandle.IsValid())
                {
                    HuntersVeilHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmHuntersVeilAbility::StaticClass(), 1, INDEX_NONE, this));
                }
            }
        }
        else if (EchoId == FName(TEXT("SanguineStrike")))
        {
            if (AbilitySystem)
            {
                static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.SanguineStrike")), false);
                if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag)) AbilitySystem->AddLooseGameplayTag(UnlockTag);
                if (!SanguineStrikeHandle.IsValid())
                {
                    SanguineStrikeHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmSanguineStrikeAbility::StaticClass(), 1, INDEX_NONE, this));
                }
            }
        }
        else if (EchoId == FName(TEXT("SecondTurn")))
        {
            if (AbilitySystem)
            {
                static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.SecondTurn")), false);
                if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag)) AbilitySystem->AddLooseGameplayTag(UnlockTag);
                if (!SecondTurnHandle.IsValid())
                {
                    SecondTurnHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmSecondTurnAbility::StaticClass(), 1, INDEX_NONE, this));
                }
            }
        }
    }

    bRelentlessAdvanceActive = bActive;
    RelentlessAdvanceRemainingTimer = FMath::Max(0.f, RemainingDuration);
    RelentlessAdvanceCooldownTimer = FMath::Max(0.f, RemainingCooldown);

    if (AbilitySystem)
    {
        static const FGameplayTag RelentlessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.RelentlessAdvance")), false);
        if (RelentlessTag.IsValid())
        {
            if (bRelentlessAdvanceActive)
            {
                if (!AbilitySystem->HasMatchingGameplayTag(RelentlessTag))
                {
                    AbilitySystem->AddLooseGameplayTag(RelentlessTag);
                }
            }
            else
            {
                if (AbilitySystem->HasMatchingGameplayTag(RelentlessTag))
                {
                    AbilitySystem->RemoveLooseGameplayTag(RelentlessTag);
                }
            }
        }

        static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.RelentlessAdvance")), false);
        if (CooldownTag.IsValid())
        {
            if (RelentlessAdvanceCooldownTimer > 0.f)
            {
                if (!AbilitySystem->HasMatchingGameplayTag(CooldownTag))
                {
                    AbilitySystem->AddLooseGameplayTag(CooldownTag);
                }
            }
            else
            {
                if (AbilitySystem->HasMatchingGameplayTag(CooldownTag))
                {
                    AbilitySystem->RemoveLooseGameplayTag(CooldownTag);
                }
                FGameplayTagContainer CDContainer(CooldownTag);
                AbilitySystem->RemoveActiveEffectsWithGrantedTags(CDContainer);
            }
        }
    }

    UpdateMovementForStatus();
}

bool AWyrmCharacter::CanActivateMoonboundForm(FString& OutFailureReason) const
{
    if (!LearnedEchoes.Contains(FName(TEXT("MoonboundForm"))))
    {
        OutFailureReason = TEXT("NotUnlocked");
        return false;
    }

    if (EquippedEcho != FName(TEXT("MoonboundForm")))
    {
        OutFailureReason = TEXT("NotEquipped");
        return false;
    }

    if (Attributes && Attributes->GetHealth() <= 0.f)
    {
        OutFailureReason = TEXT("Dead");
        return false;
    }

    if (StunRemainingTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("State.Combat.Stun"))))
    {
        OutFailureReason = TEXT("HardStunned");
        return false;
    }

    if (HasMatchingGameplayTag(FName(TEXT("State.Control.Transition"))))
    {
        OutFailureReason = TEXT("ControlTransitioning");
        return false;
    }

    if (MoonboundCooldownTimer > 0.f || HasMatchingGameplayTag(FName(TEXT("Cooldown.Echo.MoonboundForm"))))
    {
        OutFailureReason = TEXT("OnCooldown");
        return false;
    }

    if (Attributes && Attributes->GetFocus() < 40.f)
    {
        OutFailureReason = TEXT("InsufficientFocus");
        return false;
    }

    if (bMoonboundActive)
    {
        OutFailureReason = TEXT("AlreadyActive");
        return false;
    }

    if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
    {
        OutFailureReason = TEXT("Airborne");
        return false;
    }

    OutFailureReason = TEXT("");
    return true;
}

void AWyrmCharacter::ActivateMoonboundForm(float Duration)
{
    SavedHumanoidCapsuleRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
    SavedHumanoidCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    LastSafeHumanoidLocation = GetActorLocation();

    bMoonboundActive = true;
    bMoonboundReturnPending = false;
    MoonboundRemainingTimer = Duration > 0.f ? Duration : 12.0f;
    MoonboundCooldownTimer = 35.0f;

    if (AbilitySystem)
    {
        static const FGameplayTag MoonboundTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.MoonboundForm")), false);
        if (MoonboundTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(MoonboundTag))
        {
            AbilitySystem->AddLooseGameplayTag(MoonboundTag);
        }

        static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.MoonboundForm")), false);
        if (CooldownTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(CooldownTag))
        {
            AbilitySystem->AddLooseGameplayTag(CooldownTag);
        }
    }

    if (GetMesh())
    {
        GetMesh()->SetVisibility(false);
    }

    if (BeastMeshComponent)
    {
        BeastMeshComponent->SetVisibility(true);
    }

    // Adjust capsule to beast profile
    GetCapsuleComponent()->SetCapsuleSize(45.f, 45.f);

    UpdateMovementForStatus();
}

void AWyrmCharacter::DeactivateMoonboundForm()
{
    // Test return clearance for humanoid envelope
    UWorld* World = GetWorld();
    if (World)
    {
        const float BeastHalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() : 45.f;
        const float HumanoidHalfHeight = FMath::Max(88.f, SavedHumanoidCapsuleHalfHeight);
        const float HumanoidRadius = FMath::Max(34.f, SavedHumanoidCapsuleRadius);
        const float FloorClearanceTolerance = 15.0f;
        const float ZOffset = (HumanoidHalfHeight - BeastHalfHeight) + FloorClearanceTolerance;
        const FVector TargetCenter = GetActorLocation() + FVector(0.f, 0.f, ZOffset);
        const FCollisionShape HumanoidCapsule = FCollisionShape::MakeCapsule(HumanoidRadius, HumanoidHalfHeight - FloorClearanceTolerance);

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        FCollisionResponseParams ResponseParams;

        FCollisionObjectQueryParams ObjParams;
        ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
        ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
        ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);

        TArray<FOverlapResult> Overlaps;
        bool bBlocked = false;

        if (World->OverlapMultiByObjectType(Overlaps, TargetCenter, FQuat::Identity, ObjParams, HumanoidCapsule, QueryParams))
        {
            for (const FOverlapResult& Overlap : Overlaps)
            {
                AActor* OverlapActor = Overlap.GetActor();
                if (OverlapActor && OverlapActor != this)
                {
                    if (Overlap.Component.IsValid() && Overlap.Component->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block)
                    {
                        bBlocked = true;
                        UE_LOG(LogTemp, Warning, TEXT("[WyrmCharacter] DeactivateMoonboundForm blocked by Object: %s (Comp: %s)"),
                            *OverlapActor->GetName(), Overlap.Component.IsValid() ? *Overlap.Component->GetName() : TEXT("None"));
                        break;
                    }
                }
            }
        }

        if (!bBlocked && World->OverlapMultiByChannel(Overlaps, TargetCenter, FQuat::Identity, ECC_Pawn, HumanoidCapsule, QueryParams, ResponseParams))
        {
            for (const FOverlapResult& Overlap : Overlaps)
            {
                AActor* OverlapActor = Overlap.GetActor();
                if (OverlapActor && OverlapActor != this)
                {
                    if (Overlap.Component.IsValid() && Overlap.Component->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block)
                    {
                        bBlocked = true;
                        UE_LOG(LogTemp, Warning, TEXT("[WyrmCharacter] DeactivateMoonboundForm blocked by Channel: %s (Comp: %s)"),
                            *OverlapActor->GetName(), Overlap.Component.IsValid() ? *Overlap.Component->GetName() : TEXT("None"));
                        break;
                    }
                }
            }
        }

        UE_LOG(LogTemp, Display, TEXT("[WyrmCharacter] DeactivateMoonboundForm clearance check: TargetCenter=%s Radius=%.1f HalfHeight=%.1f bBlocked=%d"),
            *TargetCenter.ToString(), HumanoidCapsule.GetCapsuleRadius(), HumanoidCapsule.GetCapsuleHalfHeight(), bBlocked ? 1 : 0);

        if (bBlocked)
        {
            // Blocked by low ceiling or obstacle! Enter return-pending state (ECHO-09)
            bMoonboundReturnPending = true;
            return;
        }
    }

    // Clear return-pending and deactivate beast presentation
    bMoonboundActive = false;
    bMoonboundReturnPending = false;

    if (AbilitySystem)
    {
        static const FGameplayTag MoonboundTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.MoonboundForm")), false);
        if (MoonboundTag.IsValid() && AbilitySystem->HasMatchingGameplayTag(MoonboundTag))
        {
            AbilitySystem->RemoveLooseGameplayTag(MoonboundTag);
        }
    }

    if (BeastMeshComponent)
    {
        BeastMeshComponent->SetVisibility(false);
    }

    if (GetMesh())
    {
        GetMesh()->SetVisibility(true);
    }

    GetCapsuleComponent()->SetCapsuleSize(SavedHumanoidCapsuleRadius, SavedHumanoidCapsuleHalfHeight);
    UpdateMovementForStatus();
}

bool AWyrmCharacter::ResolveMoonboundReturnBlockage()
{
    if (!bMoonboundReturnPending)
    {
        return false;
    }

    // Teleport safely to the last validated same-side return location
    SetActorLocation(LastSafeHumanoidLocation, false, nullptr, ETeleportType::TeleportPhysics);

    // Now restore humanoid form safely
    bMoonboundActive = false;
    bMoonboundReturnPending = false;

    if (AbilitySystem)
    {
        static const FGameplayTag MoonboundTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.MoonboundForm")), false);
        if (MoonboundTag.IsValid() && AbilitySystem->HasMatchingGameplayTag(MoonboundTag))
        {
            AbilitySystem->RemoveLooseGameplayTag(MoonboundTag);
        }
    }

    if (BeastMeshComponent)
    {
        BeastMeshComponent->SetVisibility(false);
    }

    if (GetMesh())
    {
        GetMesh()->SetVisibility(true);
    }

    GetCapsuleComponent()->SetCapsuleSize(SavedHumanoidCapsuleRadius, SavedHumanoidCapsuleHalfHeight);
    UpdateMovementForStatus();
    return true;
}

void AWyrmCharacter::RestoreMoonboundState(bool bInActive, float RemainingDuration, float RemainingCooldown, bool bInPending, const FVector& InLastSafeLoc)
{
    bMoonboundActive = bInActive;
    MoonboundRemainingTimer = FMath::Max(0.f, RemainingDuration);
    MoonboundCooldownTimer = FMath::Max(0.f, RemainingCooldown);
    bMoonboundReturnPending = bInPending;
    LastSafeHumanoidLocation = InLastSafeLoc;

    for (const FName& EchoId : LearnedEchoes)
    {
        if (EchoId == FName(TEXT("MoonboundForm")))
        {
            if (AbilitySystem)
            {
                static const FGameplayTag UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.MoonboundForm")), false);
                if (UnlockTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(UnlockTag))
                {
                    AbilitySystem->AddLooseGameplayTag(UnlockTag);
                }

                if (!MoonboundFormHandle.IsValid())
                {
                    MoonboundFormHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmMoonboundFormAbility::StaticClass(), 1, INDEX_NONE, this));
                }
                if (!PrimaryBeastClawHandle.IsValid())
                {
                    PrimaryBeastClawHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmBeastClawAbility::StaticClass(), 1, INDEX_NONE, this));
                }
                if (!SecondaryBeastPounceHandle.IsValid())
                {
                    SecondaryBeastPounceHandle = AbilitySystem->GiveAbility(
                        FGameplayAbilitySpec(UWyrmBeastPounceAbility::StaticClass(), 1, INDEX_NONE, this));
                }
            }
        }
    }

    if (bMoonboundActive)
    {
        if (AbilitySystem)
        {
            static const FGameplayTag MoonboundTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.MoonboundForm")), false);
            if (MoonboundTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(MoonboundTag))
            {
                AbilitySystem->AddLooseGameplayTag(MoonboundTag);
            }
        }

        if (GetMesh())
        {
            GetMesh()->SetVisibility(false);
        }

        if (BeastMeshComponent)
        {
            BeastMeshComponent->SetVisibility(true);
        }

        GetCapsuleComponent()->SetCapsuleSize(45.f, 45.f);
        UpdateMovementForStatus();
    }
}
