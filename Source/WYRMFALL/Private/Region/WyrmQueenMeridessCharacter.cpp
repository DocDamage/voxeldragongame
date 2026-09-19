#include "Region/WyrmQueenMeridessCharacter.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"

AWyrmQueenMeridessCharacter::AWyrmQueenMeridessCharacter()
{
    SetActorScale3D(FVector(0.5f));
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_1/VerdantReach/Characters/SK_Meridess.SK_Meridess"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_1/VerdantReach/Materials/M_Meridess.M_Meridess"));
    if (MeshAsset.Succeeded()) GetMesh()->SetSkeletalMeshAsset(MeshAsset.Object);
    if (Material.Succeeded()) GetMesh()->SetMaterial(0, Material.Object);
}

bool AWyrmQueenMeridessCharacter::ResolveWithEvidence()
{
    if (UWyrmRegion01Subsystem* Region = UWyrmRegion01Subsystem::GetRegion01Subsystem(this))
    {
        return Region->ResolveVerdantCrownClaim(true);
    }
    return false;
}

bool AWyrmQueenMeridessCharacter::ResolveAfterGuardDefeat(AWyrmEnemyCharacter* DefeatedGuard)
{
    if (!DefeatedGuard || !DefeatedGuard->IsDefeated())
    {
        return false;
    }
    if (UWyrmRegion01Subsystem* Region = UWyrmRegion01Subsystem::GetRegion01Subsystem(this))
    {
        return Region->ResolveVerdantCrownClaim(false);
    }
    return false;
}
