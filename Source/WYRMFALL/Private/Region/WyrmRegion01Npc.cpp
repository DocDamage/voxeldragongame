#include "Region/WyrmRegion01Npc.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Materials/MaterialInterface.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "Player/WyrmCharacter.h"

AWyrmRegion01Npc::AWyrmRegion01Npc()
{
    PrimaryActorTick.bCanEverTick = false;

    // Standard humanoid capsule dimensions (matching player: CapsuleHalfHeight = 88.f, Radius = 34.f)
    GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Scale 0.5 brings normalized NPC mesh height into the authentic 1.87m - 2.28m range
    SetActorScale3D(FVector(0.5f, 0.5f, 0.5f));

    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void AWyrmRegion01Npc::BeginPlay()
{
    Super::BeginPlay();

    if (!RoleName.IsNone())
    {
        ConfigureRole(RoleName);
    }
}

FString AWyrmRegion01Npc::GetNormalizedMeshPathForRole(FName InRole)
{
    const FString RoleStr = InRole.ToString();
    return FString::Printf(
        TEXT("/Game/WYRMFALL/Development/Intake/WP12/NormalizedNPCTrimmed/%s/SK_%s.SK_%s"),
        *RoleStr, *RoleStr, *RoleStr);
}

FString AWyrmRegion01Npc::GetNormalizedIdleAnimPathForRole(FName InRole)
{
    const FString RoleStr = InRole.ToString();
    return FString::Printf(
        TEXT("/Game/WYRMFALL/Development/Intake/WP12/NormalizedNPCTrimmed/%s/A_%s_Idle.A_%s_Idle"),
        *RoleStr, *RoleStr, *RoleStr);
}

FString AWyrmRegion01Npc::GetPaletteMaterialPathForRole(FName InRole)
{
    const FString RoleStr = InRole.ToString();
    return FString::Printf(
        TEXT("/Game/WYRMFALL/Development/Intake/WP12/NPC/Materials/M_PaletteDiagnosticEmissive_%s.M_PaletteDiagnosticEmissive_%s"),
        *RoleStr, *RoleStr);
}

bool AWyrmRegion01Npc::ConfigureRole(FName InRoleName)
{
    RoleName = InRoleName;
    if (RoleName.IsNone())
    {
        return false;
    }

    SetActorScale3D(FVector(0.5f, 0.5f, 0.5f));

    // Load and assign normalized skeletal mesh
    const FString MeshPath = GetNormalizedMeshPathForRole(RoleName);
    USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(
        StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *MeshPath));
    if (SkeletalMesh)
    {
        GetMesh()->SetSkeletalMesh(SkeletalMesh);
    }

    // Load and assign point-sampled palette material
    const FString MatPath = GetPaletteMaterialPathForRole(RoleName);
    UMaterialInterface* Material = Cast<UMaterialInterface>(
        StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MatPath));
    if (Material)
    {
        GetMesh()->SetMaterial(0, Material);
    }

    // Load and loop idle animation
    const FString AnimPath = GetNormalizedIdleAnimPathForRole(RoleName);
    UAnimSequence* IdleAnim = Cast<UAnimSequence>(
        StaticLoadObject(UAnimSequence::StaticClass(), nullptr, *AnimPath));
    if (IdleAnim)
    {
        GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        GetMesh()->PlayAnimation(IdleAnim, true);
    }

    return SkeletalMesh != nullptr;
}

bool AWyrmRegion01Npc::CanInteract(const AActor* Interactor) const
{
    if (!Interactor)
    {
        return false;
    }
    const float DistSq = FVector::DistSquared(GetActorLocation(), Interactor->GetActorLocation());
    return DistSq <= (InteractionRadius * InteractionRadius);
}

bool AWyrmRegion01Npc::Interact(AActor* Interactor)
{
    if (!CanInteract(Interactor))
    {
        return false;
    }

    UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(this);
    if (!Region01)
    {
        return false;
    }

    const FString RoleStr = RoleName.ToString();

    if (RoleStr == TEXT("Tamsin"))
    {
        if (Region01->IsHomecomingReady())
        {
            Region01->CompleteHomecoming();
            LastDialogue = FText::FromString(TEXT("Three. You already counted. I know."));
        }
        else
        {
            LastDialogue = FText::FromString(TEXT("That path was buried yesterday. We need to find Sella, Pell, and Iven."));
        }
        return true;
    }
    else if (RoleStr == TEXT("Mara"))
    {
        LastDialogue = FText::FromString(TEXT("Presumed lost is what they write when they want the wages to stop before the searching does. Quiet Water is safe for fishing."));
        return true;
    }
    else if (RoleStr == TEXT("Sella"))
    {
        if (bRescued)
        {
            return false;
        }
        bRescued = Region01->SecureWorker(EWyrmRegion01Worker::Sella);
        LastDialogue = FText::FromString(TEXT("Stopped, yes. Dumped through this passage, no. The auxiliary restraint can be shut down now."));
        return bRescued;
    }
    else if (RoleStr == TEXT("Pell"))
    {
        if (bRescued)
        {
            return false;
        }
        bRescued = Region01->SecureWorker(EWyrmRegion01Worker::Pell);
        LastDialogue = FText::FromString(TEXT("The rock came down fast... thank you for clearing the way out."));
        return bRescued;
    }
    else if (RoleStr == TEXT("Iven"))
    {
        if (bRescued)
        {
            return false;
        }
        bRescued = Region01->SecureWorker(EWyrmRegion01Worker::Iven);
        LastDialogue = FText::FromString(TEXT("I thought we were done for behind that collapse. Tell Tamsin we're safe."));
        return bRescued;
    }
    else if (RoleStr == TEXT("Rusk"))
    {
        if (Region01->HasFact(FName(TEXT("verdance.bond_accepted"))) || Region01->HasFact(FName(TEXT("verdance.defeated_alive"))))
        {
            Region01->ResolveRusk(EWyrmRegion01RuskOutcome::SurrenderedCustody);
            LastDialogue = FText::FromString(TEXT("The beast is free... I surrender into custody. I kept it from reaching three hundred."));
        }
        else
        {
            Region01->ResolveRusk(EWyrmRegion01RuskOutcome::DefeatedCustody);
            LastDialogue = FText::FromString(TEXT("I kept it from reaching three hundred. You won't stop the Crown's claim."));
        }
        return true;
    }

    return false;
}

void AWyrmRegion01Npc::RelocateToSafeStaging(const FVector& StagingLocation)
{
    SetActorLocation(StagingLocation, false, nullptr, ETeleportType::TeleportPhysics);
}
