#include "Region/WyrmMichaelMireCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmMichaelMireAssets
{
    constexpr float TargetHeight = 190.f;
    const TCHAR* Parts[] = {
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_33_Knife_MicahelMeyers.HorrorMovieCharacter_004_33_Knife_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_34_Head_MicahelMeyers.HorrorMovieCharacter_004_34_Head_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_35_L_Arm_2_MicahelMeyers.HorrorMovieCharacter_004_35_L_Arm_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_36_L_Hand_2_MicahelMeyers.HorrorMovieCharacter_004_36_L_Hand_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_37_L_ForeArm_2_MicahelMeyers.HorrorMovieCharacter_004_37_L_ForeArm_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_38_L_Arm_2_MicahelMeyers.HorrorMovieCharacter_004_38_L_Arm_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_39_L_Hand_2_MicahelMeyers.HorrorMovieCharacter_004_39_L_Hand_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_40_L_ForeArm_2_MicahelMeyers.HorrorMovieCharacter_004_40_L_ForeArm_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_41_Spine2_3_MicahelMeyers.HorrorMovieCharacter_004_41_Spine2_3_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_42_Spine1_2_MicahelMeyers.HorrorMovieCharacter_004_42_Spine1_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_43_R_Foot_1_MicahelMeyers.HorrorMovieCharacter_004_43_R_Foot_1_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_44_R_Calf_2_MicahelMeyers.HorrorMovieCharacter_004_44_R_Calf_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_45_R_Thigh_1_MicahelMeyers.HorrorMovieCharacter_004_45_R_Thigh_1_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_46_L_Foot_2_MicahelMeyers.HorrorMovieCharacter_004_46_L_Foot_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_47_L_Calf_2_MicahelMeyers.HorrorMovieCharacter_004_47_L_Calf_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_48_L_Thigh_2_MicahelMeyers.HorrorMovieCharacter_004_48_L_Thigh_2_MicahelMeyers"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_49_Spine3_9_MicahelMeyers.HorrorMovieCharacter_004_49_Spine3_9_MicahelMeyers"),
    };
}

AWyrmMichaelMireCharacter::AWyrmMichaelMireCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 385.f;
    GetCapsuleComponent()->InitCapsuleSize(38.f, 95.f);
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MichaelMirePresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WyrmMichaelMireAssets::Parts); ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("MichaelMirePart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PresentationParts.Add(Part);
    }
}

void AWyrmMichaelMireCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(520.f);
        Attributes->InitHealth(520.f);
        Attributes->InitArmor(10.f);
        Attributes->InitPower(18.f);
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
            UStaticMesh::StaticClass(), nullptr, WyrmMichaelMireAssets::Parts[Index]));
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
        PresentationScale = WyrmMichaelMireAssets::TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmMichaelMireCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmMichaelMireResolution::Unresolved)
    {
        PresentationRoot->SetRelativeRotation(FRotator(0.f, FMath::Sin(IdleTime * 0.55f) * 2.5f, 0.f));
    }
}

bool AWyrmMichaelMireCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmMichaelMireResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.hollow_twins_resolved"))))
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

bool AWyrmMichaelMireCharacter::ResolveAfterLivingSubmission(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmMichaelMireResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordMichaelMireResolution())
    {
        return false;
    }
    Resolution = EWyrmMichaelMireResolution::LivingSubmission;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmMichaelMireCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmMichaelMireResolution::Unresolved;
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

float AWyrmMichaelMireCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? WyrmMichaelMireAssets::TargetHeight : 0.f;
}

bool AWyrmMichaelMireCharacter::HasCompletePresentation() const
{
    if (PresentationParts.Num() != UE_ARRAY_COUNT(WyrmMichaelMireAssets::Parts)) return false;
    for (const UStaticMeshComponent* Part : PresentationParts)
    {
        if (!Part || !Part->GetStaticMesh()) return false;
    }
    return true;
}
