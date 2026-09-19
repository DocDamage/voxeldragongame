#include "Combat/WyrmJadeEmperorCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Region/WyrmJadePeaksSubsystem.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Materials/MaterialInterface.h"

AWyrmJadeEmperorCharacter::AWyrmJadeEmperorCharacter()
{
    bIsBoss = true;
    BaseWalkSpeed = 460.f;
    GetCharacterMovement()->GravityScale = 0.f;
}

void AWyrmJadeEmperorCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(850.f); Attributes->InitHealth(850.f);
        Attributes->InitArmor(14.f); Attributes->InitPower(20.f);
    }
    if (GetMesh() && !GetMesh()->GetSkeletalMeshAsset())
    {
        if (USkeletalMesh* KingMesh = LoadObject<USkeletalMesh>(nullptr,
            TEXT("/Game/WYRMFALL/Development/Intake/WP23/JadePeaks/Palace/Characters/TVS_VoxelPalace_King.TVS_VoxelPalace_King")))
            GetMesh()->SetSkeletalMeshAsset(KingMesh);
    }
    if (GetMesh())
    {
        if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Game/WYRMFALL/Development/Intake/WP23/JadePeaks/Palace/Materials/M_TVS_VoxelPalace_King.M_TVS_VoxelPalace_King")))
            GetMesh()->SetMaterial(0, Material);

        // Let the newly assigned skeletal pose update before using its bounds.
        GetWorldTimerManager().SetTimerForNextTick(this, &AWyrmJadeEmperorCharacter::GroundPresentationMesh);
    }
}

void AWyrmJadeEmperorCharacter::GroundPresentationMesh()
{
    if (!GetMesh() || !GetCapsuleComponent()) return;
    GetMesh()->UpdateBounds();
    const float CapsuleBottom = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float MeshBottom = GetMesh()->Bounds.Origin.Z - GetMesh()->Bounds.BoxExtent.Z;
    FVector RelativeLocation = GetMesh()->GetRelativeLocation();
    RelativeLocation.Z += CapsuleBottom - MeshBottom;
    GetMesh()->SetRelativeLocation(RelativeLocation);
}

bool AWyrmJadeEmperorCharacter::ResolveByDiplomacy() { return Resolve(true); }
bool AWyrmJadeEmperorCharacter::ResolveAfterLivingDefeat() { return IsDefeated() && Resolve(false); }

bool AWyrmJadeEmperorCharacter::Resolve(bool bDiplomacy)
{
    if (bPactResolved) return false;
    UWyrmJadePeaksSubsystem* Region = UWyrmJadePeaksSubsystem::GetJadePeaksSubsystem(this);
    if (!Region || !Region->RecordImperialPactResolution(bDiplomacy)) return false;
    bPactResolved = true;
    return true;
}
