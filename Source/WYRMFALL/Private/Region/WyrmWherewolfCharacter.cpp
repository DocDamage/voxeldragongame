#include "Region/WyrmWherewolfCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmWherewolfAssets
{
    constexpr float TargetHeight = 205.f;
    const TCHAR* Parts[] = {
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_84_L_Foot_2_Werewolf.HorrorMovieCharacter_004_84_L_Foot_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_85_L_Arm_2_Werewolf.HorrorMovieCharacter_004_85_L_Arm_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_86_L_Hand_2_Werewolf.HorrorMovieCharacter_004_86_L_Hand_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_87_L_ForeArm_2_Werewolf.HorrorMovieCharacter_004_87_L_ForeArm_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_88_L_Arm_2_Werewolf.HorrorMovieCharacter_004_88_L_Arm_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_89_L_Hand_2_Werewolf.HorrorMovieCharacter_004_89_L_Hand_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_90_L_ForeArm_2_Werewolf.HorrorMovieCharacter_004_90_L_ForeArm_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_91_Spine2_3_Werewolf.HorrorMovieCharacter_004_91_Spine2_3_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_92_Spine3_9_Werewolf.HorrorMovieCharacter_004_92_Spine3_9_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_93_Head_Werewolf.HorrorMovieCharacter_004_93_Head_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_94_Spine1_2_Werewolf.HorrorMovieCharacter_004_94_Spine1_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_95_R_Calf_2_Werewolf.HorrorMovieCharacter_004_95_R_Calf_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_96_R_Thigh_1_Werewolf.HorrorMovieCharacter_004_96_R_Thigh_1_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_97_L_Foot_2_Werewolf.HorrorMovieCharacter_004_97_L_Foot_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_98_L_Calf_2_Werewolf.HorrorMovieCharacter_004_98_L_Calf_2_Werewolf"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_99_L_Thigh_2_Werewolf.HorrorMovieCharacter_004_99_L_Thigh_2_Werewolf"),
    };
}

AWyrmWherewolfCharacter::AWyrmWherewolfCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 430.f;
    GetCapsuleComponent()->InitCapsuleSize(44.f, 102.5f);
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WherewolfPresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WyrmWherewolfAssets::Parts); ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("WherewolfPart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PresentationParts.Add(Part);
    }
}

void AWyrmWherewolfCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(640.f);
        Attributes->InitHealth(640.f);
        Attributes->InitArmor(15.f);
        Attributes->InitPower(24.f);
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
            UStaticMesh::StaticClass(), nullptr, WyrmWherewolfAssets::Parts[Index]));
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
        PresentationScale = WyrmWherewolfAssets::TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmWherewolfCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmWherewolfResolution::Unresolved)
    {
        const float Breath = FMath::Sin(IdleTime * 1.15f);
        PresentationRoot->SetRelativeRotation(FRotator(Breath * 0.9f, Breath * 1.4f, 0.f));
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale * (1.f + Breath * 0.008f)));
    }
}

bool AWyrmWherewolfCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmWherewolfResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.pleatherface_resolved"))))
    {
        return false;
    }
    bEncounterActive = true;
    PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

bool AWyrmWherewolfCharacter::ResolveAfterCalmedSubmission(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmWherewolfResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordWherewolfResolution()) return false;
    Resolution = EWyrmWherewolfResolution::CalmedSubmission;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmWherewolfCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmWherewolfResolution::Unresolved;
    SetInvulnerable(false);
    PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
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

float AWyrmWherewolfCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? WyrmWherewolfAssets::TargetHeight : 0.f;
}

bool AWyrmWherewolfCharacter::HasCompletePresentation() const
{
    if (PresentationParts.Num() != UE_ARRAY_COUNT(WyrmWherewolfAssets::Parts)) return false;
    for (const UStaticMeshComponent* Part : PresentationParts)
    {
        if (!Part || !Part->GetStaticMesh()) return false;
    }
    return true;
}
