#include "Region/WyrmFrankNShrineCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmFrankNShrineAssets
{
    constexpr float TargetHeight = 202.f;
    const TCHAR* Parts[] = {
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_282_L_Arm_2_Frankenstein.HorrorMovieCharacter_004_282_L_Arm_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_283_L_Hand_2_Frankenstein.HorrorMovieCharacter_004_283_L_Hand_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_284_L_ForeArm_2_Frankenstein.HorrorMovieCharacter_004_284_L_ForeArm_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_285_L_Arm_2_Frankenstein.HorrorMovieCharacter_004_285_L_Arm_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_286_L_Hand_2_Frankenstein.HorrorMovieCharacter_004_286_L_Hand_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_287_L_ForeArm_2_Frankenstein.HorrorMovieCharacter_004_287_L_ForeArm_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_288_Spine2_3_Frankenstein.HorrorMovieCharacter_004_288_Spine2_3_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_289_Spine3_9_Frankenstein.HorrorMovieCharacter_004_289_Spine3_9_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_290_Head_Frankenstein.HorrorMovieCharacter_004_290_Head_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_291_Spine1_2_Frankenstein.HorrorMovieCharacter_004_291_Spine1_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_292_R_Foot_1_Frankenstein.HorrorMovieCharacter_004_292_R_Foot_1_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_293_R_Calf_2_Frankenstein.HorrorMovieCharacter_004_293_R_Calf_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_294_R_Thigh_1_Frankenstein.HorrorMovieCharacter_004_294_R_Thigh_1_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_295_L_Foot_2_Frankenstein.HorrorMovieCharacter_004_295_L_Foot_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_296_L_Calf_2_Frankenstein.HorrorMovieCharacter_004_296_L_Calf_2_Frankenstein"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_297_L_Thigh_2_Frankenstein.HorrorMovieCharacter_004_297_L_Thigh_2_Frankenstein"),
    };
}

AWyrmFrankNShrineCharacter::AWyrmFrankNShrineCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 315.f;
    GetCapsuleComponent()->InitCapsuleSize(38.f, 101.f);
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FrankNShrinePresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WyrmFrankNShrineAssets::Parts); ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("FrankNShrinePart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PresentationParts.Add(Part);
    }
}

void AWyrmFrankNShrineCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(840.f);
        Attributes->InitHealth(840.f);
        Attributes->InitArmor(23.f);
        Attributes->InitPower(31.f);
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
            UStaticMesh::StaticClass(), nullptr, WyrmFrankNShrineAssets::Parts[Index]));
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
        PresentationScale = WyrmFrankNShrineAssets::TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmFrankNShrineCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmFrankNShrineResolution::Unresolved)
    {
        PresentationRoot->SetRelativeRotation(FRotator(
            FMath::Sin(IdleTime * 0.33f) * 1.0f,
            FMath::Sin(IdleTime * 0.27f) * 0.8f,
            FMath::Sin(IdleTime * 0.41f) * 0.7f));
    }
}

bool AWyrmFrankNShrineCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmFrankNShrineResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.count_dripula_resolved"))))
    {
        return false;
    }
    bEncounterActive = true;
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

bool AWyrmFrankNShrineCharacter::ResolveAfterGroundedSubmission(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmFrankNShrineResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordFrankNShrineResolution()) return false;
    Resolution = EWyrmFrankNShrineResolution::GroundedSubmission;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmFrankNShrineCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmFrankNShrineResolution::Unresolved;
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

float AWyrmFrankNShrineCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? WyrmFrankNShrineAssets::TargetHeight : 0.f;
}

bool AWyrmFrankNShrineCharacter::HasCompletePresentation() const
{
    if (PresentationParts.Num() != UE_ARRAY_COUNT(WyrmFrankNShrineAssets::Parts)) return false;
    for (const UStaticMeshComponent* Part : PresentationParts)
    {
        if (!Part || !Part->GetStaticMesh()) return false;
    }
    return true;
}
