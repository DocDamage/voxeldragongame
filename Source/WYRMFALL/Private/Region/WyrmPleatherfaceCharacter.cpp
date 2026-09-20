#include "Region/WyrmPleatherfaceCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmPleatherfaceAssets
{
    constexpr float TargetHeight = 192.f;
    const TCHAR* Parts[] = {
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_67_Sword_Leatherface.HorrorMovieCharacter_004_67_Sword_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_68_L_Arm_2_Leatherface.HorrorMovieCharacter_004_68_L_Arm_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_69_L_Hand_2_Leatherface.HorrorMovieCharacter_004_69_L_Hand_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_70_L_ForeArm_2_Leatherface.HorrorMovieCharacter_004_70_L_ForeArm_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_71_L_Arm_2_Leatherface.HorrorMovieCharacter_004_71_L_Arm_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_72_L_Hand_2_Leatherface.HorrorMovieCharacter_004_72_L_Hand_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_73_L_ForeArm_2_Leatherface.HorrorMovieCharacter_004_73_L_ForeArm_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_74_Spine2_3_Leatherface.HorrorMovieCharacter_004_74_Spine2_3_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_75_Spine3_9_Leatherface.HorrorMovieCharacter_004_75_Spine3_9_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_76_Head_Leatherface.HorrorMovieCharacter_004_76_Head_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_77_Spine1_2_Leatherface.HorrorMovieCharacter_004_77_Spine1_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_78_R_Foot_1_Leatherface.HorrorMovieCharacter_004_78_R_Foot_1_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_79_R_Calf_2_Leatherface.HorrorMovieCharacter_004_79_R_Calf_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_80_R_Thigh_1_Leatherface.HorrorMovieCharacter_004_80_R_Thigh_1_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_81_L_Foot_2_Leatherface.HorrorMovieCharacter_004_81_L_Foot_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_82_L_Calf_2_Leatherface.HorrorMovieCharacter_004_82_L_Calf_2_Leatherface"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_83_L_Thigh_2_Leatherface.HorrorMovieCharacter_004_83_L_Thigh_2_Leatherface"),
    };
}

AWyrmPleatherfaceCharacter::AWyrmPleatherfaceCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 360.f;
    GetCapsuleComponent()->InitCapsuleSize(39.f, 96.f);
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PleatherfacePresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WyrmPleatherfaceAssets::Parts); ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("PleatherfacePart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PresentationParts.Add(Part);
    }
}

void AWyrmPleatherfaceCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(600.f);
        Attributes->InitHealth(600.f);
        Attributes->InitArmor(14.f);
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
            UStaticMesh::StaticClass(), nullptr, WyrmPleatherfaceAssets::Parts[Index]));
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
        PresentationScale = WyrmPleatherfaceAssets::TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmPleatherfaceCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmPleatherfaceResolution::Unresolved)
    {
        PresentationRoot->SetRelativeRotation(FRotator(
            0.f, FMath::Sin(IdleTime * 0.38f) * 2.2f,
            FMath::Sin(IdleTime * 0.31f) * 0.7f));
    }
}

bool AWyrmPleatherfaceCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmPleatherfaceResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.machete_mason_resolved"))))
    {
        return false;
    }
    bEncounterActive = true;
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

bool AWyrmPleatherfaceCharacter::ResolveAfterDisarmedSubmission(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmPleatherfaceResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordPleatherfaceResolution()) return false;
    Resolution = EWyrmPleatherfaceResolution::DisarmedSubmission;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmPleatherfaceCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmPleatherfaceResolution::Unresolved;
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

float AWyrmPleatherfaceCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? WyrmPleatherfaceAssets::TargetHeight : 0.f;
}

bool AWyrmPleatherfaceCharacter::HasCompletePresentation() const
{
    if (PresentationParts.Num() != UE_ARRAY_COUNT(WyrmPleatherfaceAssets::Parts)) return false;
    for (const UStaticMeshComponent* Part : PresentationParts)
    {
        if (!Part || !Part->GetStaticMesh()) return false;
    }
    return true;
}
