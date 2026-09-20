#include "Region/WyrmChucklesCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmChucklesAssets
{
    constexpr float TargetHeight = 110.f;
    const TCHAR* Parts[] = {
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_100_L_Arm_2_Chucky.HorrorMovieCharacter_004_100_L_Arm_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_101_L_Hand_2_Chucky.HorrorMovieCharacter_004_101_L_Hand_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_102_L_ForeArm_2_Chucky.HorrorMovieCharacter_004_102_L_ForeArm_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_103_L_Calf_2_Chucky.HorrorMovieCharacter_004_103_L_Calf_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_104_L_Thigh_2_Chucky.HorrorMovieCharacter_004_104_L_Thigh_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_105_L_Arm_2_Chucky.HorrorMovieCharacter_004_105_L_Arm_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_106_L_Hand_2_Chucky.HorrorMovieCharacter_004_106_L_Hand_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_107_L_ForeArm_2_Chucky.HorrorMovieCharacter_004_107_L_ForeArm_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_108_L_Calf_2_Chucky.HorrorMovieCharacter_004_108_L_Calf_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_109_L_Thigh_2_Chucky.HorrorMovieCharacter_004_109_L_Thigh_2_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_110_R_Foot_1_Chucky.HorrorMovieCharacter_004_110_R_Foot_1_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_111_R_Foot_1_Chucky.HorrorMovieCharacter_004_111_R_Foot_1_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_112_Spine3_Chucky.HorrorMovieCharacter_004_112_Spine3_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_113_Head_Chucky.HorrorMovieCharacter_004_113_Head_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_114_Spine2_3_Chucky.HorrorMovieCharacter_004_114_Spine2_3_Chucky"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_115_Spine1_2_Chucky.HorrorMovieCharacter_004_115_Spine1_2_Chucky"),
    };
}

AWyrmChucklesCharacter::AWyrmChucklesCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 390.f;
    GetCapsuleComponent()->InitCapsuleSize(24.f, 55.f);
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ChucklesPresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WyrmChucklesAssets::Parts); ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("ChucklesPart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PresentationParts.Add(Part);
    }
}

void AWyrmChucklesCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(760.f);
        Attributes->InitHealth(760.f);
        Attributes->InitArmor(19.f);
        Attributes->InitPower(27.f);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        GetCharacterMovement()->GravityScale = 0.f;
    }

    FBox CombinedBounds(ForceInit);
    for (int32 Index = 0; Index < PresentationParts.Num(); ++Index)
    {
        UStaticMesh* MeshAsset = Cast<UStaticMesh>(StaticLoadObject(
            UStaticMesh::StaticClass(), nullptr, WyrmChucklesAssets::Parts[Index]));
        if (MeshAsset)
        {
            PresentationParts[Index]->SetStaticMesh(MeshAsset);
            PresentationParts[Index]->SetVisibility(true, true);
            PresentationParts[Index]->SetHiddenInGame(false, true);
            CombinedBounds += MeshAsset->GetBoundingBox();
        }
    }
    if (CombinedBounds.IsValid && CombinedBounds.GetSize().Z > KINDA_SMALL_NUMBER)
    {
        PresentationScale = WyrmChucklesAssets::TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmChucklesCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmChucklesResolution::Unresolved)
    {
        PresentationRoot->SetRelativeRotation(FRotator(
            FMath::Sin(IdleTime * 1.37f) * 1.8f,
            FMath::Sin(IdleTime * 0.83f) * 2.4f,
            FMath::Sin(IdleTime * 1.11f) * 1.5f));
    }
}

bool AWyrmChucklesCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmChucklesResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.scarrie_resolved"))))
    {
        return false;
    }
    bEncounterActive = true;
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

bool AWyrmChucklesCharacter::ResolveAfterContainedSubmission(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmChucklesResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordChucklesResolution()) return false;
    Resolution = EWyrmChucklesResolution::ContainedSubmission;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmChucklesCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmChucklesResolution::Unresolved;
    SetInvulnerable(false);
    if (Attributes) Attributes->SetCurrentHealth(Attributes->GetMaxHealth());
    if (AbilitySystem)
    {
        const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")), false);
        if (DeadTag.IsValid()) AbilitySystem->RemoveLooseGameplayTag(DeadTag);
    }
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        GetCharacterMovement()->GravityScale = 0.f;
    }
}

float AWyrmChucklesCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? WyrmChucklesAssets::TargetHeight : 0.f;
}

bool AWyrmChucklesCharacter::HasCompletePresentation() const
{
    if (PresentationParts.Num() != UE_ARRAY_COUNT(WyrmChucklesAssets::Parts)) return false;
    for (const UStaticMeshComponent* Part : PresentationParts)
    {
        if (!Part || !Part->GetStaticMesh()) return false;
    }
    return true;
}
