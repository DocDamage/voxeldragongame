#include "Region/WyrmCountMalvaineCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

namespace WyrmMalvaineAssets
{
    const TCHAR* Mesh = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/Roles/CountMalvaine/TVS_VoxelCathedral_Priest.TVS_VoxelCathedral_Priest");
    const TCHAR* Material = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureMaterials/M_CountMalvaine_SuppliedPaletteV2.M_CountMalvaine_SuppliedPaletteV2");
    const TCHAR* Preach = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/RoleAnimations/A_CountMalvaine_Preach.A_CountMalvaine_Preach");
}

AWyrmCountMalvaineCharacter::AWyrmCountMalvaineCharacter()
{
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 475.f;
    GetCapsuleComponent()->InitCapsuleSize(42.f, 90.f);
    GetMesh()->SetVisibility(false, true);
    PresentationMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MalvainePresentation"));
    PresentationMesh->SetupAttachment(GetCapsuleComponent());
    PresentationMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    PresentationMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    // Character-attached skeletal components retain the FBX's 0.01 scene conversion;
    // this measured scale produces the validated 180 cm presentation used by the fixture.
    PresentationMesh->SetRelativeScale3D(FVector(47.326f));
    PresentationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWyrmCountMalvaineCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(700.f);
        Attributes->InitHealth(700.f);
        Attributes->InitArmor(12.f);
        Attributes->InitPower(20.f);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        // The finite GeoForge surface finishes collision generation just after BeginPlay.
        // Hold authored encounter actors in place until the player reaches this gated beat.
        GetCharacterMovement()->GravityScale = 0.f;
    }
    if (USkeletalMesh* MeshAsset = Cast<USkeletalMesh>(StaticLoadObject(
        USkeletalMesh::StaticClass(), nullptr, WyrmMalvaineAssets::Mesh)))
    {
        PresentationMesh->SetSkeletalMeshAsset(MeshAsset);
        PresentationMesh->SetVisibility(true, true);
        PresentationMesh->SetHiddenInGame(false, true);
        PresentationMesh->SetRenderInMainPass(true);
    }
    if (UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(StaticLoadObject(
        UMaterialInterface::StaticClass(), nullptr, WyrmMalvaineAssets::Material)))
    {
        PresentationMesh->SetMaterial(0, MaterialAsset);
    }
    if (UAnimSequence* PreachAnimation = Cast<UAnimSequence>(StaticLoadObject(
        UAnimSequence::StaticClass(), nullptr, WyrmMalvaineAssets::Preach)))
    {
        PresentationMesh->PlayAnimation(PreachAnimation, true);
    }
}

bool AWyrmCountMalvaineCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmMalvaineResolution::Unresolved)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.ashgrave_extraction_seal_resolved"))))
    {
        return false;
    }
    bEncounterActive = true;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->GravityScale = 1.f;
    }
    return true;
}

bool AWyrmCountMalvaineCharacter::ResolveByParley(AWyrmCharacter* PlayerCharacter)
{
    return ResolveInternal(PlayerCharacter, true);
}

bool AWyrmCountMalvaineCharacter::ResolveAfterLivingDefeat(AWyrmCharacter* PlayerCharacter)
{
    if (!Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    return ResolveInternal(PlayerCharacter, false);
}

bool AWyrmCountMalvaineCharacter::ResolveInternal(AWyrmCharacter* PlayerCharacter, bool bParley)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmMalvaineResolution::Unresolved)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordMalvaineResolution(bParley))
    {
        return false;
    }
    Resolution = bParley ? EWyrmMalvaineResolution::Parley : EWyrmMalvaineResolution::LivingDefeat;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmCountMalvaineCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmMalvaineResolution::Unresolved;
    SetInvulnerable(false);
    if (Attributes)
    {
        Attributes->SetCurrentHealth(Attributes->GetMaxHealth());
    }
    if (AbilitySystem)
    {
        const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")), false);
        if (DeadTag.IsValid())
        {
            AbilitySystem->RemoveLooseGameplayTag(DeadTag);
        }
    }
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        GetCharacterMovement()->GravityScale = 1.f;
    }
}

float AWyrmCountMalvaineCharacter::GetPresentationHeight() const
{
    return PresentationMesh ? PresentationMesh->Bounds.BoxExtent.Z * 2.f : 0.f;
}
