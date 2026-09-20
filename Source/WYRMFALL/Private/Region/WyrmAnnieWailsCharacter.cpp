#include "Region/WyrmAnnieWailsCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmAnnieWailsAssets
{
    constexpr float TargetHeight = 176.f;
    const TCHAR* Parts[] = {
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_50_Sword_AnnieWilkes.HorrorMovieCharacter_004_50_Sword_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_51_R_Calf_2_AnnieWilkes.HorrorMovieCharacter_004_51_R_Calf_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_52_Head_AnnieWilkes.HorrorMovieCharacter_004_52_Head_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_53_R_Thigh_1_AnnieWilkes.HorrorMovieCharacter_004_53_R_Thigh_1_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_54_L_Arm_2_AnnieWilkes.HorrorMovieCharacter_004_54_L_Arm_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_55_L_Hand_2_AnnieWilkes.HorrorMovieCharacter_004_55_L_Hand_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_56_L_ForeArm_2_AnnieWilkes.HorrorMovieCharacter_004_56_L_ForeArm_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_57_L_Arm_2_AnnieWilkes.HorrorMovieCharacter_004_57_L_Arm_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_58_L_Hand_2_AnnieWilkes.HorrorMovieCharacter_004_58_L_Hand_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_59_L_ForeArm_2_AnnieWilkes.HorrorMovieCharacter_004_59_L_ForeArm_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_60_Spine2_3_AnnieWilkes.HorrorMovieCharacter_004_60_Spine2_3_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_61_Spine1_2_AnnieWilkes.HorrorMovieCharacter_004_61_Spine1_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_62_R_Foot_1_AnnieWilkes.HorrorMovieCharacter_004_62_R_Foot_1_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_63_R_Calf_2_AnnieWilkes.HorrorMovieCharacter_004_63_R_Calf_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_64_R_Thigh_1_AnnieWilkes.HorrorMovieCharacter_004_64_R_Thigh_1_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_65_L_Foot_2_AnnieWilkes.HorrorMovieCharacter_004_65_L_Foot_2_AnnieWilkes"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_66_Spine3_9_AnnieWilkes.HorrorMovieCharacter_004_66_Spine3_9_AnnieWilkes"),
    };
}

AWyrmAnnieWailsCharacter::AWyrmAnnieWailsCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 350.f;
    GetCapsuleComponent()->InitCapsuleSize(36.f, 88.f);
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("AnnieWailsPresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WyrmAnnieWailsAssets::Parts); ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("AnnieWailsPart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PresentationParts.Add(Part);
    }
}

void AWyrmAnnieWailsCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(680.f);
        Attributes->InitHealth(680.f);
        Attributes->InitArmor(16.f);
        Attributes->InitPower(22.f);
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
            UStaticMesh::StaticClass(), nullptr, WyrmAnnieWailsAssets::Parts[Index]));
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
        PresentationScale = WyrmAnnieWailsAssets::TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmAnnieWailsCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmAnnieWailsResolution::Unresolved)
    {
        PresentationRoot->SetRelativeRotation(FRotator(
            FMath::Sin(IdleTime * 0.46f) * 0.6f,
            FMath::Sin(IdleTime * 0.62f) * 1.8f,
            FMath::Sin(IdleTime * 0.39f) * 0.5f));
    }
}

bool AWyrmAnnieWailsCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmAnnieWailsResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.wherewolf_resolved"))))
    {
        return false;
    }
    bEncounterActive = true;
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

bool AWyrmAnnieWailsCharacter::ResolveAfterDisarmedSurrender(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmAnnieWailsResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordAnnieWailsResolution()) return false;
    Resolution = EWyrmAnnieWailsResolution::DisarmedSurrender;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmAnnieWailsCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmAnnieWailsResolution::Unresolved;
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

float AWyrmAnnieWailsCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? WyrmAnnieWailsAssets::TargetHeight : 0.f;
}

bool AWyrmAnnieWailsCharacter::HasCompletePresentation() const
{
    if (PresentationParts.Num() != UE_ARRAY_COUNT(WyrmAnnieWailsAssets::Parts)) return false;
    for (const UStaticMeshComponent* Part : PresentationParts)
    {
        if (!Part || !Part->GetStaticMesh()) return false;
    }
    return true;
}
