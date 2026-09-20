#include "Region/WyrmChefAurelioCharacter.h"
#include "Region/WyrmCogspireSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

namespace WyrmChefAssets
{
    const TCHAR* Mesh = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/Roles/ChefAurelioVane_Chef/TVS_VoxelVillage_Chef.TVS_VoxelVillage_Chef");
    const TCHAR* Material = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/CastMaterials/M_ChefAurelioVane_SuppliedPaletteQA.M_ChefAurelioVane_SuppliedPaletteQA");
    const TCHAR* Walk = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/CastAnimations/A_ChefAurelioVane_Walk.A_ChefAurelioVane_Walk");
}

AWyrmChefAurelioCharacter::AWyrmChefAurelioCharacter()
{
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    GetCapsuleComponent()->InitCapsuleSize(40.f, 88.f);
    GetMesh()->SetVisibility(false, true);
    PresentationMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChefAurelioPresentation"));
    PresentationMesh->SetupAttachment(GetCapsuleComponent());
    PresentationMesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
    PresentationMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    PresentationMesh->SetRelativeScale3D(FVector(40.f));
    PresentationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWyrmChefAurelioCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(480.f); Attributes->InitHealth(480.f);
        Attributes->InitArmor(10.f); Attributes->InitPower(20.f);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 460.f;
        GetCharacterMovement()->GravityScale = 0.f;
    }
    if (USkeletalMesh* Asset = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, WyrmChefAssets::Mesh)))
    {
        PresentationMesh->SetSkeletalMeshAsset(Asset); PresentationMesh->SetVisibility(true, true);
    }
    if (UMaterialInterface* Material = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, WyrmChefAssets::Material)))
    {
        PresentationMesh->SetMaterial(0, Material);
    }
    if (UAnimSequence* Walk = Cast<UAnimSequence>(StaticLoadObject(UAnimSequence::StaticClass(), nullptr, WyrmChefAssets::Walk)))
    {
        PresentationMesh->PlayAnimation(Walk, true);
    }
    OnEnemyDied.AddDynamic(this, &AWyrmChefAurelioCharacter::HandleChefDefeated);
}

bool AWyrmChefAurelioCharacter::RecordPatronTestimony(AWyrmCharacter* Player)
{
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    return Player && Region && Region->RecordChefPatronTestimony();
}

bool AWyrmChefAurelioCharacter::ExamineKitchenEvidence(AWyrmCharacter* Player)
{
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    return Player && Region && Region->RecordChefKitchenEvidence();
}

bool AWyrmChefAurelioCharacter::TraceIngredientSource(AWyrmCharacter* Player)
{
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    return Player && Region && Region->RecordChefIngredientSource();
}

bool AWyrmChefAurelioCharacter::BeginConfrontation(AWyrmCharacter* Player)
{
    if (!Player || bConfrontationActive || IsDefeated()) return false;
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    if (!Region || !Region->RecordChefConfrontation()) return false;
    bConfrontationActive = true;
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

void AWyrmChefAurelioCharacter::HandleChefDefeated(AWyrmEnemyCharacter* DefeatedEnemy)
{
    if (bConfrontationActive && DefeatedEnemy == this)
    {
        if (UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this)) Region->RecordChefDefeat();
        bConfrontationActive = false;
    }
}

bool AWyrmChefAurelioCharacter::ManifestCarversPrecision(AWyrmCharacter* Player)
{
    if (!Player || !IsDefeated()) return false;
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    return Region && Region->RecordCarversPrecisionUnlock() && Player->LearnEcho(FName(TEXT("CarversPrecision")));
}
