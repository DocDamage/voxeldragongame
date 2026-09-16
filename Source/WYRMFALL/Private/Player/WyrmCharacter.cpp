#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/WyrmAttributeSet.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Inventory/WyrmInventoryComponent.h"
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
    ApplyCamera();
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

    if (!PrimaryAttackHandle.IsValid())
    {
        PrimaryAttackHandle = AbilitySystem->GiveAbility(
            FGameplayAbilitySpec(UWyrmPrimaryMeleeAbility::StaticClass(), 1, INDEX_NONE, this));
    }

    if (!SecondaryAttackHandle.IsValid())
    {
        SecondaryAttackHandle = AbilitySystem->GiveAbility(
            FGameplayAbilitySpec(UWyrmSecondaryMeleeAbility::StaticClass(), 1, INDEX_NONE, this));
    }
}

bool AWyrmCharacter::PerformPrimaryAttack()
{
    if (bMovementLocked || !AbilitySystem || !PrimaryAttackHandle.IsValid())
    {
        return false;
    }
    return AbilitySystem->TryActivateAbility(PrimaryAttackHandle);
}

bool AWyrmCharacter::PerformSecondaryAttack()
{
    if (bMovementLocked || !AbilitySystem || !SecondaryAttackHandle.IsValid())
    {
        return false;
    }
    return AbilitySystem->TryActivateAbility(SecondaryAttackHandle);
}


