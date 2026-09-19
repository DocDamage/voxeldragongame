#include "Region/WyrmVerdantRoyalGuardCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"

AWyrmVerdantRoyalGuardCharacter::AWyrmVerdantRoyalGuardCharacter()
{
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    SetActorScale3D(FVector(0.5f));
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
        TEXT("/Game/WYRMFALL/Development/Intake/WP12/NormalizedNPCTrimmed/Tamsin/SK_Tamsin.SK_Tamsin"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(
        TEXT("/Game/WYRMFALL/Development/Intake/WP12/NPC/Materials/M_PaletteDiagnosticEmissive_Tamsin.M_PaletteDiagnosticEmissive_Tamsin"));
    if (MeshAsset.Succeeded()) GetMesh()->SetSkeletalMeshAsset(MeshAsset.Object);
    if (Material.Succeeded()) GetMesh()->SetMaterial(0, Material.Object);
}
