#include "Region/WyrmCanopyHunterCharacter.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"

AWyrmCanopyHunterCharacter::AWyrmCanopyHunterCharacter()
{
    EnemyRole = EWyrmEnemyRole::RangedSkirmisher;
    SetActorScale3D(FVector(0.5f));
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_1/VerdantReach/Characters/SK_CanopyHunter.SK_CanopyHunter"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_1/VerdantReach/Materials/M_CanopyHunter.M_CanopyHunter"));
    if (MeshAsset.Succeeded()) GetMesh()->SetSkeletalMeshAsset(MeshAsset.Object);
    if (Material.Succeeded()) GetMesh()->SetMaterial(0, Material.Object);
}

bool AWyrmCanopyHunterCharacter::ResolveByTrust(AWyrmCharacter* Player)
{
    return GrantHuntersVeil(Player, true);
}

bool AWyrmCanopyHunterCharacter::ResolveByLivingDefeat(AWyrmCharacter* Player)
{
    UWyrmAttributeSet* HunterAttributes = GetAttributes();
    if (!HunterAttributes || HunterAttributes->GetHealth() <= 0.f ||
        HunterAttributes->GetHealth() > HunterAttributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    return GrantHuntersVeil(Player, false);
}

bool AWyrmCanopyHunterCharacter::GrantHuntersVeil(AWyrmCharacter* Player, bool bTrustRoute)
{
    if (!Player)
    {
        return false;
    }
    UWyrmRegion01Subsystem* Region = UWyrmRegion01Subsystem::GetRegion01Subsystem(this);
    if (!Region || !Region->ResolveCanopyHunter(bTrustRoute))
    {
        return false;
    }
    SetInvulnerable(true);
    return Player->LearnEcho(FName(TEXT("HuntersVeil")));
}
