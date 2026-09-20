#include "Region/WyrmCountDripulaCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmCountDripulaAssets
{
    constexpr float TargetHeight = 184.f;
    const TCHAR* Parts[] = {
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_266_L_Arm_2_Dracula.HorrorMovieCharacter_004_266_L_Arm_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_267_L_Hand_2_Dracula.HorrorMovieCharacter_004_267_L_Hand_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_268_L_ForeArm_2_Dracula.HorrorMovieCharacter_004_268_L_ForeArm_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_269_Head_Dracula.HorrorMovieCharacter_004_269_Head_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_270_L_Arm_2_Dracula.HorrorMovieCharacter_004_270_L_Arm_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_271_L_Hand_2_Dracula.HorrorMovieCharacter_004_271_L_Hand_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_272_L_ForeArm_2_Dracula.HorrorMovieCharacter_004_272_L_ForeArm_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_273_Spine2_3_Dracula.HorrorMovieCharacter_004_273_Spine2_3_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_274_Spine1_2_Dracula.HorrorMovieCharacter_004_274_Spine1_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_275_R_Foot_1_Dracula.HorrorMovieCharacter_004_275_R_Foot_1_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_276_R_Calf_2_Dracula.HorrorMovieCharacter_004_276_R_Calf_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_277_R_Thigh_1_Dracula.HorrorMovieCharacter_004_277_R_Thigh_1_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_278_L_Foot_2_Dracula.HorrorMovieCharacter_004_278_L_Foot_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_279_L_Calf_2_Dracula.HorrorMovieCharacter_004_279_L_Calf_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_280_L_Thigh_2_Dracula.HorrorMovieCharacter_004_280_L_Thigh_2_Dracula"),
        TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/HorrorMovieCharacter_004_281_Spine3_9_Dracula.HorrorMovieCharacter_004_281_Spine3_9_Dracula"),
    };
}

AWyrmCountDripulaCharacter::AWyrmCountDripulaCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    BaseWalkSpeed = 345.f;
    GetCapsuleComponent()->InitCapsuleSize(34.f, 92.f);
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CountDripulaPresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WyrmCountDripulaAssets::Parts); ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("CountDripulaPart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PresentationParts.Add(Part);
    }
}

void AWyrmCountDripulaCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(800.f);
        Attributes->InitHealth(800.f);
        Attributes->InitArmor(21.f);
        Attributes->InitPower(29.f);
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
            UStaticMesh::StaticClass(), nullptr, WyrmCountDripulaAssets::Parts[Index]));
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
        PresentationScale = WyrmCountDripulaAssets::TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmCountDripulaCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmCountDripulaResolution::Unresolved)
    {
        PresentationRoot->SetRelativeRotation(FRotator(
            FMath::Sin(IdleTime * 0.39f) * 0.8f,
            FMath::Sin(IdleTime * 0.31f) * 1.2f,
            FMath::Sin(IdleTime * 0.47f) * 0.6f));
    }
}

bool AWyrmCountDripulaCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmCountDripulaResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->HasFact(FName(TEXT("gloaming.chuckles_resolved"))))
    {
        return false;
    }
    bEncounterActive = true;
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

bool AWyrmCountDripulaCharacter::ResolveAfterBloodlessSurrender(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmCountDripulaResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f ||
        Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordCountDripulaResolution()) return false;
    Resolution = EWyrmCountDripulaResolution::BloodlessSurrender;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmCountDripulaCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmCountDripulaResolution::Unresolved;
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

float AWyrmCountDripulaCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? WyrmCountDripulaAssets::TargetHeight : 0.f;
}

bool AWyrmCountDripulaCharacter::HasCompletePresentation() const
{
    if (PresentationParts.Num() != UE_ARRAY_COUNT(WyrmCountDripulaAssets::Parts)) return false;
    for (const UStaticMeshComponent* Part : PresentationParts)
    {
        if (!Part || !Part->GetStaticMesh()) return false;
    }
    return true;
}
