#include "Region/WyrmHouseMarkCharacter.h"
#include "Region/WyrmCogspireSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

namespace WyrmHouseMarkAssets
{
    const TCHAR* Mesh = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/Roles/HouseMark_Champion/TVS_VoxelKnights_Champion.TVS_VoxelKnights_Champion");
    const TCHAR* Material = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/CastMaterials/M_HouseMarkChampion_SuppliedPaletteQA.M_HouseMarkChampion_SuppliedPaletteQA");
    const TCHAR* Slash = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/CastAnimations/A_HouseMarkChampion_Slash.A_HouseMarkChampion_Slash");
}

AWyrmHouseMarkCharacter::AWyrmHouseMarkCharacter()
{
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    GetCapsuleComponent()->InitCapsuleSize(42.f, 95.f);
    GetMesh()->SetVisibility(false, true);
    PresentationMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HouseMarkPresentation"));
    PresentationMesh->SetupAttachment(GetCapsuleComponent());
    PresentationMesh->SetRelativeLocation(FVector(0.f, 0.f, -95.f));
    PresentationMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    PresentationMesh->SetRelativeScale3D(FVector(42.f));
    PresentationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWyrmHouseMarkCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(520.f);
        Attributes->InitHealth(520.f);
        Attributes->InitArmor(14.f);
        Attributes->InitPower(22.f);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 500.f;
        GetCharacterMovement()->GravityScale = 0.f;
    }
    if (USkeletalMesh* Asset = Cast<USkeletalMesh>(StaticLoadObject(
        USkeletalMesh::StaticClass(), nullptr, WyrmHouseMarkAssets::Mesh)))
    {
        PresentationMesh->SetSkeletalMeshAsset(Asset);
        PresentationMesh->SetVisibility(true, true);
    }
    if (UAnimSequence* Slash = Cast<UAnimSequence>(StaticLoadObject(
        UAnimSequence::StaticClass(), nullptr, WyrmHouseMarkAssets::Slash)))
    {
        PresentationMesh->PlayAnimation(Slash, true);
    }
    if (UMaterialInterface* Material = Cast<UMaterialInterface>(StaticLoadObject(
        UMaterialInterface::StaticClass(), nullptr, WyrmHouseMarkAssets::Material)))
    {
        PresentationMesh->SetMaterial(0, Material);
    }
    OnEnemyDied.AddDynamic(this, &AWyrmHouseMarkCharacter::HandleHouseMarkDefeated);
}

bool AWyrmHouseMarkCharacter::ExamineVictimEvidence(AWyrmCharacter* Player)
{
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    return Player && Region && Region->RecordHouseMarkVictimEvidence();
}

bool AWyrmHouseMarkCharacter::ExamineRelicTradeEvidence(AWyrmCharacter* Player)
{
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    return Player && Region && Region->RecordHouseMarkRelicEvidence();
}

bool AWyrmHouseMarkCharacter::IdentifyHouseMark(AWyrmCharacter* Player)
{
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    return Player && Region && Region->RecordHouseMarkIdentity();
}

bool AWyrmHouseMarkCharacter::BeginConfrontation(AWyrmCharacter* Player)
{
    if (!Player || bConfrontationActive || IsDefeated()) return false;
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    if (!Region || !Region->RecordHouseMarkConfrontation()) return false;
    bConfrontationActive = true;
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

void AWyrmHouseMarkCharacter::HandleHouseMarkDefeated(AWyrmEnemyCharacter* DefeatedEnemy)
{
    if (bConfrontationActive && DefeatedEnemy == this)
    {
        if (UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this))
        {
            Region->RecordHouseMarkDefeat();
        }
        bConfrontationActive = false;
    }
}

bool AWyrmHouseMarkCharacter::ManifestDeathmark(AWyrmCharacter* Player)
{
    if (!Player || !IsDefeated()) return false;
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    return Region && Region->RecordDeathmarkUnlock() && Player->LearnEcho(FName(TEXT("Deathmark")));
}
