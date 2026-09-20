#include "Region/WyrmMacheteMasonCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmMacheteMasonAssets
{
    constexpr float TargetHeight = 188.f;
    const TCHAR* Parts[] = {
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_314_Sword_Jason.HorrorMovieCharacter_004_314_Sword_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_315_L_Arm_2_Jason.HorrorMovieCharacter_004_315_L_Arm_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_316_L_Hand_2_Jason.HorrorMovieCharacter_004_316_L_Hand_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_317_L_ForeArm_2_Jason.HorrorMovieCharacter_004_317_L_ForeArm_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_318_Head_Jason.HorrorMovieCharacter_004_318_Head_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_319_L_Arm_2_Jason.HorrorMovieCharacter_004_319_L_Arm_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_320_L_Hand_2_Jason.HorrorMovieCharacter_004_320_L_Hand_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_321_L_ForeArm_2_Jason.HorrorMovieCharacter_004_321_L_ForeArm_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_322_Spine2_3_Jason.HorrorMovieCharacter_004_322_Spine2_3_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_323_Spine1_2_Jason.HorrorMovieCharacter_004_323_Spine1_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_324_R_Foot_1_Jason.HorrorMovieCharacter_004_324_R_Foot_1_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_325_R_Calf_2_Jason.HorrorMovieCharacter_004_325_R_Calf_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_326_R_Thigh_1_Jason.HorrorMovieCharacter_004_326_R_Thigh_1_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_327_L_Foot_2_Jason.HorrorMovieCharacter_004_327_L_Foot_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_328_L_Calf_2_Jason.HorrorMovieCharacter_004_328_L_Calf_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_329_L_Thigh_2_Jason.HorrorMovieCharacter_004_329_L_Thigh_2_Jason"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_330_Spine3_9_Jason.HorrorMovieCharacter_004_330_Spine3_9_Jason"),
    };
}

AWyrmMacheteMasonCharacter::AWyrmMacheteMasonCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 370.f;
    GetCapsuleComponent()->InitCapsuleSize(38.f, 94.f);
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MacheteMasonPresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WyrmMacheteMasonAssets::Parts); ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("MacheteMasonPart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PresentationParts.Add(Part);
    }
}

void AWyrmMacheteMasonCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(560.f);
        Attributes->InitHealth(560.f);
        Attributes->InitArmor(12.f);
        Attributes->InitPower(20.f);
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
            UStaticMesh::StaticClass(), nullptr, WyrmMacheteMasonAssets::Parts[Index]));
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
        PresentationScale = WyrmMacheteMasonAssets::TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmMacheteMasonCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmMacheteMasonResolution::Unresolved)
    {
        PresentationRoot->SetRelativeRotation(FRotator(
            FMath::Sin(IdleTime * 0.4f) * 0.8f,
            FMath::Sin(IdleTime * 0.45f) * 1.8f,
            0.f));
    }
}

bool AWyrmMacheteMasonCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmMacheteMasonResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.michael_mire_resolved"))))
    {
        return false;
    }
    bEncounterActive = true;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->GravityScale = 1.f;
    }
    return true;
}

bool AWyrmMacheteMasonCharacter::ResolveAfterDisarmedSubmission(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmMacheteMasonResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordMacheteMasonResolution())
    {
        return false;
    }
    Resolution = EWyrmMacheteMasonResolution::DisarmedSubmission;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmMacheteMasonCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmMacheteMasonResolution::Unresolved;
    SetInvulnerable(false);
    if (Attributes)
    {
        Attributes->SetCurrentHealth(Attributes->GetMaxHealth());
    }
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

float AWyrmMacheteMasonCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? WyrmMacheteMasonAssets::TargetHeight : 0.f;
}

bool AWyrmMacheteMasonCharacter::HasCompletePresentation() const
{
    if (PresentationParts.Num() != UE_ARRAY_COUNT(WyrmMacheteMasonAssets::Parts)) return false;
    for (const UStaticMeshComponent* Part : PresentationParts)
    {
        if (!Part || !Part->GetStaticMesh()) return false;
    }
    return true;
}
