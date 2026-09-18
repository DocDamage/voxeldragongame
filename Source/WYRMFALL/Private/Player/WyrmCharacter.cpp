#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/WyrmAttributeSet.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Combat/Abilities/WyrmRangedAttackAbility.h"
#include "Combat/Abilities/WyrmEvadeAbility.h"
#include "Combat/Abilities/WyrmRelentlessAdvanceAbility.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "Activities/WyrmFishingComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
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

    return false;
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
