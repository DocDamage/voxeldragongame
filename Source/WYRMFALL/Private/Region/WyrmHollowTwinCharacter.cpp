#include "Region/WyrmHollowTwinCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInterface.h"

namespace WyrmHollowTwinAssets
{
    const TCHAR* Mesh = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/Roles/HollowTwins/TVS_VoxelCathedral_Nun.TVS_VoxelCathedral_Nun");
    const TCHAR* MorrowMaterial = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureMaterials/M_HollowTwinMorrow_ProductionPalette.M_HollowTwinMorrow_ProductionPalette");
    const TCHAR* MournMaterial = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureMaterials/M_HollowTwinMourn_ProductionPalette.M_HollowTwinMourn_ProductionPalette");
    const TCHAR* MorrowPrayer = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/RoleAnimations/A_HollowTwinMorrow_Praying.A_HollowTwinMorrow_Praying");
    const TCHAR* MournPrayer = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/RoleAnimations/A_HollowTwinMourn_Praying.A_HollowTwinMourn_Praying");
}

AWyrmHollowTwinCharacter::AWyrmHollowTwinCharacter()
{
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 440.f;
    GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);
    GetMesh()->SetVisibility(false, true);
    PresentationMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HollowTwinPresentation"));
    PresentationMesh->SetupAttachment(GetCapsuleComponent());
    PresentationMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    PresentationMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    // The supplied Nun has shorter source bounds than the Priest; this measured
    // scale preserves the fixture-approved 180 cm Hollow Twin presentation.
    PresentationMesh->SetRelativeScale3D(FVector(67.37f));
    PresentationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWyrmHollowTwinCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(400.f);
        Attributes->InitHealth(400.f);
        Attributes->InitArmor(8.f);
        Attributes->InitPower(14.f);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        GetCharacterMovement()->GravityScale = 0.f;
    }
    if (USkeletalMesh* MeshAsset = Cast<USkeletalMesh>(StaticLoadObject(
        USkeletalMesh::StaticClass(), nullptr, WyrmHollowTwinAssets::Mesh)))
    {
        PresentationMesh->SetSkeletalMeshAsset(MeshAsset);
        PresentationMesh->SetVisibility(true, true);
        PresentationMesh->SetHiddenInGame(false, true);
        PresentationMesh->SetRenderInMainPass(true);
    }
    const bool bMorrow = TwinIdentity == EWyrmHollowTwinIdentity::Morrow;
    if (UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(StaticLoadObject(
        UMaterialInterface::StaticClass(), nullptr,
        bMorrow ? WyrmHollowTwinAssets::MorrowMaterial : WyrmHollowTwinAssets::MournMaterial)))
    {
        PresentationMesh->SetMaterial(0, MaterialAsset);
    }
    if (UAnimSequence* Prayer = Cast<UAnimSequence>(StaticLoadObject(
        UAnimSequence::StaticClass(), nullptr,
        bMorrow ? WyrmHollowTwinAssets::MorrowPrayer : WyrmHollowTwinAssets::MournPrayer)))
    {
        PresentationMesh->PlayAnimation(Prayer, true);
        if (!bMorrow)
        {
            PresentationMesh->SetPosition(Prayer->GetPlayLength() * 0.65f, false);
        }
    }
}

bool AWyrmHollowTwinCharacter::HasDistinctValidPartner() const
{
    return Partner && Partner != this && Partner->Partner == this &&
        Partner->TwinIdentity != TwinIdentity;
}

bool AWyrmHollowTwinCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !HasDistinctValidPartner() ||
        Resolution != EWyrmHollowTwinsResolution::Unresolved ||
        Partner->Resolution != EWyrmHollowTwinsResolution::Unresolved)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.malvaine_encounter_resolved"))))
    {
        return false;
    }
    SetPairEncounterActive(true);
    return true;
}

bool AWyrmHollowTwinCharacter::ResolveByRelease(AWyrmCharacter* PlayerCharacter)
{
    return ResolvePair(PlayerCharacter, true);
}

bool AWyrmHollowTwinCharacter::ResolveAfterLivingSubmission(AWyrmCharacter* PlayerCharacter)
{
    if (!IsAtLivingSubmissionThreshold() || !Partner || !Partner->IsAtLivingSubmissionThreshold())
    {
        return false;
    }
    return ResolvePair(PlayerCharacter, false);
}

bool AWyrmHollowTwinCharacter::ManifestSecondTurn(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution == EWyrmHollowTwinsResolution::Unresolved ||
        !HasDistinctValidPartner() || Partner->Resolution != Resolution)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordSecondTurnUnlock())
    {
        return false;
    }
    return PlayerCharacter->LearnEcho(FName(TEXT("SecondTurn")));
}

bool AWyrmHollowTwinCharacter::ResolvePair(AWyrmCharacter* PlayerCharacter, bool bReleased)
{
    if (!PlayerCharacter || !bEncounterActive || !HasDistinctValidPartner() ||
        !Partner->bEncounterActive || Resolution != EWyrmHollowTwinsResolution::Unresolved ||
        Partner->Resolution != EWyrmHollowTwinsResolution::Unresolved)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordHollowTwinsResolution(bReleased))
    {
        return false;
    }
    const EWyrmHollowTwinsResolution NewResolution = bReleased
        ? EWyrmHollowTwinsResolution::Released
        : EWyrmHollowTwinsResolution::LivingSubmission;
    ApplyResolution(NewResolution);
    Partner->ApplyResolution(NewResolution);
    return true;
}

bool AWyrmHollowTwinCharacter::IsAtLivingSubmissionThreshold() const
{
    return Attributes && Attributes->GetHealth() > 0.f &&
        Attributes->GetHealth() <= Attributes->GetMaxHealth() * 0.25f;
}

void AWyrmHollowTwinCharacter::SetPairEncounterActive(bool bActive)
{
    bEncounterActive = bActive;
    Partner->bEncounterActive = bActive;
    for (AWyrmHollowTwinCharacter* Twin : {this, Partner.Get()})
    {
        if (Twin && Twin->GetCharacterMovement())
        {
            Twin->GetCharacterMovement()->GravityScale = bActive ? 1.f : 0.f;
        }
    }
}

void AWyrmHollowTwinCharacter::ApplyResolution(EWyrmHollowTwinsResolution NewResolution)
{
    Resolution = NewResolution;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
}

void AWyrmHollowTwinCharacter::ResetSelf()
{
    bEncounterActive = false;
    Resolution = EWyrmHollowTwinsResolution::Unresolved;
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
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        GetCharacterMovement()->GravityScale = 1.f;
    }
}

void AWyrmHollowTwinCharacter::ResetEncounterPair()
{
    ResetSelf();
    if (HasDistinctValidPartner())
    {
        Partner->ResetSelf();
    }
}

float AWyrmHollowTwinCharacter::GetPresentationHeight() const
{
    return PresentationMesh ? PresentationMesh->Bounds.BoxExtent.Z * 2.f : 0.f;
}
