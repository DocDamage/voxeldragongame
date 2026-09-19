#include "Combat/WyrmJadeDiscipleCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Player/WyrmCharacter.h"
#include "Region/WyrmJadePeaksSubsystem.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"

AWyrmJadeDiscipleCharacter::AWyrmJadeDiscipleCharacter()
{
    bIsBoss = true;
    BaseWalkSpeed = 520.f;
}

void AWyrmJadeDiscipleCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(600.f);
        Attributes->InitHealth(600.f);
        Attributes->InitArmor(10.f);
        Attributes->InitPower(18.f);
    }
    if (GetMesh() && !GetMesh()->GetSkeletalMeshAsset())
    {
        if (USkeletalMesh* DiscipleMesh = Cast<USkeletalMesh>(StaticLoadObject(
            USkeletalMesh::StaticClass(), nullptr,
            TEXT("/Game/WYRMFALL/Development/Intake/WP22/JadePeaks/Palace/Characters/TVS_VoxelPalace_Guard.TVS_VoxelPalace_Guard"))))
        {
            GetMesh()->SetSkeletalMeshAsset(DiscipleMesh);
        }
    }
    if (GetMesh())
    {
        if (UMaterialInterface* DiscipleMaterial = Cast<UMaterialInterface>(StaticLoadObject(
            UMaterialInterface::StaticClass(), nullptr,
            TEXT("/Game/WYRMFALL/Development/Intake/WP22/JadePeaks/Palace/Materials/M_TVS_VoxelPalace_Guard.M_TVS_VoxelPalace_Guard"))))
        {
            GetMesh()->SetMaterial(0, DiscipleMaterial);
        }
    }
}

bool AWyrmJadeDiscipleCharacter::ResolveWithTrust(AWyrmCharacter* PlayerCharacter)
{
    return ResolveInternal(PlayerCharacter, true);
}

bool AWyrmJadeDiscipleCharacter::ResolveAfterLivingDefeat(AWyrmCharacter* PlayerCharacter)
{
    return IsDefeated() && ResolveInternal(PlayerCharacter, false);
}

bool AWyrmJadeDiscipleCharacter::ResolveInternal(AWyrmCharacter* PlayerCharacter, bool bTrust)
{
    if (!PlayerCharacter || bEncounterResolved)
    {
        return false;
    }
    UWyrmJadePeaksSubsystem* Region = UWyrmJadePeaksSubsystem::GetJadePeaksSubsystem(this);
    if (!Region || !Region->RecordDiscipleResolution(bTrust))
    {
        return false;
    }
    PlayerCharacter->LearnEcho(FName(TEXT("MirrorStep")));
    Region->RecordMirrorStepUnlock();
    PlayerCharacter->LearnEcho(FName(TEXT("UnseenHand")));
    Region->RecordUnseenHandUnlock();
    bEncounterResolved = true;
    bTrustedResolution = bTrust;
    return true;
}

void AWyrmJadeDiscipleCharacter::ResetResolution()
{
    bEncounterResolved = false;
    bTrustedResolution = false;
}
