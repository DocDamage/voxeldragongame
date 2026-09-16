#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/WyrmAttributeSet.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Combat/Abilities/WyrmRangedAttackAbility.h"
#include "Combat/Abilities/WyrmEvadeAbility.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "Activities/WyrmFishingComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
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
    GetCharacterMovement()->MaxWalkSpeed = 450.f; // development-only tuning
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
