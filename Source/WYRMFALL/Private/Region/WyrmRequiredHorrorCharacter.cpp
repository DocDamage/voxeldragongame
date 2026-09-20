#include "Region/WyrmRequiredHorrorCharacter.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace WyrmRequiredHorrorAssets
{
    const TCHAR* Root = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Horror/");
}

AWyrmRequiredHorrorCharacter::AWyrmRequiredHorrorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;
    EnemyRole = EWyrmEnemyRole::MeleeChaser;
    GetMesh()->SetVisibility(false, true);
    PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RequiredHorrorPresentationRoot"));
    PresentationRoot->SetupAttachment(GetCapsuleComponent());
    for (int32 Index = 0; Index < 21; ++Index)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("RequiredHorrorPart_%02d"), Index));
        Part->SetupAttachment(PresentationRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetVisibility(false, true);
        PresentationParts.Add(Part);
    }
}

void AWyrmRequiredHorrorCharacter::Configure(EWyrmRequiredHorrorIdentity InIdentity,
    const TCHAR* InSourceToken, float InHeight, float InHealth, float InArmor,
    float InPower, float InWalkSpeed, const TArray<FString>& InAssetNames)
{
    HorrorIdentity = InIdentity;
    SourceToken = InSourceToken;
    TargetHeight = InHeight;
    EncounterHealth = InHealth;
    EncounterArmor = InArmor;
    EncounterPower = InPower;
    BaseWalkSpeed = InWalkSpeed;
    GetCapsuleComponent()->InitCapsuleSize(FMath::Clamp(TargetHeight * 0.20f, 32.f, 44.f), TargetHeight * 0.5f);
    PartAssetPaths.Reset(InAssetNames.Num());
    for (const FString& AssetName : InAssetNames)
    {
        if (!AssetName.EndsWith(SourceToken))
        {
            continue;
        }
        PartAssetPaths.Add(FString::Printf(TEXT("%s%s.%s"), WyrmRequiredHorrorAssets::Root, *AssetName, *AssetName));
    }
}

void AWyrmRequiredHorrorCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (Attributes)
    {
        Attributes->InitMaxHealth(EncounterHealth);
        Attributes->InitHealth(EncounterHealth);
        Attributes->InitArmor(EncounterArmor);
        Attributes->InitPower(EncounterPower);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        GetCharacterMovement()->GravityScale = 0.f;
    }

    FBox CombinedBounds(ForceInit);
    for (int32 Index = 0; Index < PartAssetPaths.Num() && Index < PresentationParts.Num(); ++Index)
    {
        UStaticMesh* MeshAsset = Cast<UStaticMesh>(StaticLoadObject(
            UStaticMesh::StaticClass(), nullptr, *PartAssetPaths[Index]));
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
        PresentationScale = TargetHeight / CombinedBounds.GetSize().Z;
        const FVector SourceCenter = CombinedBounds.GetCenter();
        PresentationRoot->SetRelativeScale3D(FVector(PresentationScale));
        PresentationRoot->SetRelativeLocation(FVector(
            -SourceCenter.X * PresentationScale,
            -SourceCenter.Y * PresentationScale,
            -GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - CombinedBounds.Min.Z * PresentationScale));
    }
}

void AWyrmRequiredHorrorCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    IdleTime += DeltaSeconds;
    if (PresentationRoot && !bEncounterActive && Resolution == EWyrmRequiredHorrorResolution::Unresolved)
    {
        const float Variant = 0.19f + static_cast<uint8>(HorrorIdentity) * 0.025f;
        PresentationRoot->SetRelativeRotation(FRotator(
            FMath::Sin(IdleTime * Variant) * (0.6f + static_cast<uint8>(HorrorIdentity) * 0.04f),
            FMath::Sin(IdleTime * (Variant + 0.07f)) * 0.8f,
            FMath::Sin(IdleTime * (Variant + 0.13f)) * 0.7f));
    }
}

bool AWyrmRequiredHorrorCharacter::BeginEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || Resolution != EWyrmRequiredHorrorResolution::Unresolved || !HasCompletePresentation())
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->CanResolveRequiredHorror(HorrorIdentity))
    {
        return false;
    }
    bEncounterActive = true;
    if (GetCharacterMovement()) GetCharacterMovement()->GravityScale = 1.f;
    return true;
}

bool AWyrmRequiredHorrorCharacter::ResolveAfterLivingSubmission(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bEncounterActive || Resolution != EWyrmRequiredHorrorResolution::Unresolved ||
        !Attributes || Attributes->GetHealth() <= 0.f || Attributes->GetHealth() > Attributes->GetMaxHealth() * 0.25f)
    {
        return false;
    }
    UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this);
    if (!Region || !Region->RecordRequiredHorrorResolution(HorrorIdentity)) return false;
    Resolution = EWyrmRequiredHorrorResolution::LivingSubmission;
    bEncounterActive = false;
    SetInvulnerable(true);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    return true;
}

void AWyrmRequiredHorrorCharacter::ResetEncounter()
{
    bEncounterActive = false;
    Resolution = EWyrmRequiredHorrorResolution::Unresolved;
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

float AWyrmRequiredHorrorCharacter::GetPresentationHeight() const
{
    return HasCompletePresentation() ? TargetHeight : 0.f;
}

bool AWyrmRequiredHorrorCharacter::HasCompletePresentation() const
{
    if (PartAssetPaths.IsEmpty() || PartAssetPaths.Num() > PresentationParts.Num()) return false;
    for (int32 Index = 0; Index < PartAssetPaths.Num(); ++Index)
    {
        if (!PresentationParts[Index] || !PresentationParts[Index]->GetStaticMesh()) return false;
    }
    return true;
}

AWyrmAilYenCharacter::AWyrmAilYenCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::AilYen, TEXT("Alien"), 214.f, 880.f, 25.f, 33.f, 340.f, {
        TEXT("HorrorMovieCharacter_004_116_Spine1_2_Alien"), TEXT("HorrorMovieCharacter_004_117_Tail3_Alien"),
        TEXT("HorrorMovieCharacter_004_118_Tail2_Alien"), TEXT("HorrorMovieCharacter_004_119_Tail1_Alien"),
        TEXT("HorrorMovieCharacter_004_120_Tail4_Alien"), TEXT("HorrorMovieCharacter_004_121_L_Arm_2_Alien"),
        TEXT("HorrorMovieCharacter_004_122_L_Hand_2_Alien"), TEXT("HorrorMovieCharacter_004_123_L_ForeArm_2_Alien"),
        TEXT("HorrorMovieCharacter_004_124_R_Foot_1_Alien"), TEXT("HorrorMovieCharacter_004_125_R_Calf_2_Alien"),
        TEXT("HorrorMovieCharacter_004_126_L_Arm_2_Alien"), TEXT("HorrorMovieCharacter_004_127_L_Hand_2_Alien"),
        TEXT("HorrorMovieCharacter_004_128_L_ForeArm_2_Alien"), TEXT("HorrorMovieCharacter_004_129_Spine2_3_Alien"),
        TEXT("HorrorMovieCharacter_004_130_Spine3_9_Alien"), TEXT("HorrorMovieCharacter_004_131_Head_Alien"),
        TEXT("HorrorMovieCharacter_004_132_Spine1_2_Alien"), TEXT("HorrorMovieCharacter_004_133_R_Foot_1_Alien"),
        TEXT("HorrorMovieCharacter_004_134_R_Calf_2_Alien"), TEXT("HorrorMovieCharacter_004_135_R_Thigh_1_Alien"),
        TEXT("HorrorMovieCharacter_004_136_L_Thigh_2_Alien")});
}

AWyrmBellraiserCharacter::AWyrmBellraiserCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::Bellraiser, TEXT("Hellraiser"), 186.f, 920.f, 27.f, 34.f, 300.f, {
        TEXT("HorrorMovieCharacter_004_137_Head_Hellraiser"), TEXT("HorrorMovieCharacter_004_138_R_Thigh_1_Hellraiser"),
        TEXT("HorrorMovieCharacter_004_139_L_Arm_2_Hellraiser"), TEXT("HorrorMovieCharacter_004_140_L_Hand_2_Hellraiser"),
        TEXT("HorrorMovieCharacter_004_141_L_ForeArm_2_Hellraiser"), TEXT("HorrorMovieCharacter_004_142_L_Arm_2_Hellraiser"),
        TEXT("HorrorMovieCharacter_004_143_L_Hand_2_Hellraiser"), TEXT("HorrorMovieCharacter_004_144_L_ForeArm_2_Hellraiser"),
        TEXT("HorrorMovieCharacter_004_145_Spine2_3_Hellraiser"), TEXT("HorrorMovieCharacter_004_146_Spine1_2_Hellraiser"),
        TEXT("HorrorMovieCharacter_004_147_R_Foot_1_Hellraiser"), TEXT("HorrorMovieCharacter_004_148_R_Calf_2_Hellraiser"),
        TEXT("HorrorMovieCharacter_004_149_R_Thigh_1_Hellraiser"), TEXT("HorrorMovieCharacter_004_150_L_Foot_2_Hellraiser"),
        TEXT("HorrorMovieCharacter_004_151_L_Calf_2_Hellraiser"), TEXT("HorrorMovieCharacter_004_152_Spine3_9_Hellraiser")});
}

AWyrmSadEchoCharacter::AWyrmSadEchoCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::SadEcho, TEXT("Sadoko"), 168.f, 960.f, 22.f, 35.f, 285.f, {
        TEXT("HorrorMovieCharacter_004_153_R_Calf_2_Sadoko"), TEXT("HorrorMovieCharacter_004_154_Head_Sadoko"),
        TEXT("HorrorMovieCharacter_004_155_R_Thigh_1_Sadoko"), TEXT("HorrorMovieCharacter_004_156_L_Arm_2_Sadoko"),
        TEXT("HorrorMovieCharacter_004_157_L_Hand_2_Sadoko"), TEXT("HorrorMovieCharacter_004_158_L_ForeArm_2_Sadoko"),
        TEXT("HorrorMovieCharacter_004_159_L_Arm_2_Sadoko"), TEXT("HorrorMovieCharacter_004_160_L_Hand_2_Sadoko"),
        TEXT("HorrorMovieCharacter_004_161_L_ForeArm_2_Sadoko"), TEXT("HorrorMovieCharacter_004_162_Spine2_3_Sadoko"),
        TEXT("HorrorMovieCharacter_004_163_Spine1_2_Sadoko"), TEXT("HorrorMovieCharacter_004_164_R_Foot_1_Sadoko"),
        TEXT("HorrorMovieCharacter_004_165_R_Calf_2_Sadoko"), TEXT("HorrorMovieCharacter_004_166_R_Thigh_1_Sadoko"),
        TEXT("HorrorMovieCharacter_004_167_L_Foot_2_Sadoko"), TEXT("HorrorMovieCharacter_004_168_Spine3_9_Sadoko")});
}

AWyrmDreadatorCharacter::AWyrmDreadatorCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::Dreadator, TEXT("Predator"), 208.f, 1000.f, 30.f, 37.f, 355.f, {
        TEXT("HorrorMovieCharacter_004_169_L_Arm_2_Predator"), TEXT("HorrorMovieCharacter_004_170_L_Arm_2_Predator"),
        TEXT("HorrorMovieCharacter_004_171_L_ForeArm_2_Predator"), TEXT("HorrorMovieCharacter_004_172_L_Arm_2_Predator"),
        TEXT("HorrorMovieCharacter_004_173_L_Hand_2_Predator"), TEXT("HorrorMovieCharacter_004_174_Head_Predator"),
        TEXT("HorrorMovieCharacter_004_175_L_Arm_2_Predator"), TEXT("HorrorMovieCharacter_004_176_L_Hand_2_Predator"),
        TEXT("HorrorMovieCharacter_004_177_L_ForeArm_2_Predator"), TEXT("HorrorMovieCharacter_004_178_Spine2_3_Predator"),
        TEXT("HorrorMovieCharacter_004_179_Spine1_2_Predator"), TEXT("HorrorMovieCharacter_004_180_R_Foot_1_Predator"),
        TEXT("HorrorMovieCharacter_004_181_R_Calf_2_Predator"), TEXT("HorrorMovieCharacter_004_182_L_Foot_2_Predator"),
        TEXT("HorrorMovieCharacter_004_183_L_Calf_2_Predator"), TEXT("HorrorMovieCharacter_004_184_Spine3_9_Predator")});
}

AWyrmRoastfaceCharacter::AWyrmRoastfaceCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::Roastface, TEXT("Ghostface"), 182.f, 1040.f, 24.f, 38.f, 350.f, {
        TEXT("HorrorMovieCharacter_004_185_Knife_Ghostface"), TEXT("HorrorMovieCharacter_004_186_R_Thigh_1_Ghostface"),
        TEXT("HorrorMovieCharacter_004_187_L_Arm_2_Ghostface"), TEXT("HorrorMovieCharacter_004_188_L_Hand_2_Ghostface"),
        TEXT("HorrorMovieCharacter_004_189_L_ForeArm_2_Ghostface"), TEXT("HorrorMovieCharacter_004_190_Head_Ghostface"),
        TEXT("HorrorMovieCharacter_004_191_L_Arm_2_Ghostface"), TEXT("HorrorMovieCharacter_004_192_L_Hand_2_Ghostface"),
        TEXT("HorrorMovieCharacter_004_193_L_ForeArm_2_Ghostface"), TEXT("HorrorMovieCharacter_004_194_Spine2_3_Ghostface"),
        TEXT("HorrorMovieCharacter_004_195_Spine1_2_Ghostface"), TEXT("HorrorMovieCharacter_004_196_R_Foot_1_Ghostface"),
        TEXT("HorrorMovieCharacter_004_197_R_Calf_2_Ghostface"), TEXT("HorrorMovieCharacter_004_198_R_Thigh_1_Ghostface"),
        TEXT("HorrorMovieCharacter_004_199_L_Foot_2_Ghostface"), TEXT("HorrorMovieCharacter_004_200_L_Calf_2_Ghostface"),
        TEXT("HorrorMovieCharacter_004_201_Spine3_9_Ghostface")});
}

AWyrmGravyDaughtersCharacter::AWyrmGravyDaughtersCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::GravyDaughters, TEXT("GradyDaughter"), 148.f, 1080.f, 23.f, 39.f, 310.f, {
        TEXT("HorrorMovieCharacter_004_202_L_Hand_2_GradyDaughter"), TEXT("HorrorMovieCharacter_004_203_L_Thigh_2_GradyDaughter"),
        TEXT("HorrorMovieCharacter_004_204_L_Arm_2_GradyDaughter"), TEXT("HorrorMovieCharacter_004_205_L_Hand_2_GradyDaughter"),
        TEXT("HorrorMovieCharacter_004_206_L_ForeArm_2_GradyDaughter"), TEXT("HorrorMovieCharacter_004_207_L_Arm_2_GradyDaughter"),
        TEXT("HorrorMovieCharacter_004_208_L_ForeArm_2_GradyDaughter"), TEXT("HorrorMovieCharacter_004_209_L_Calf_2_GradyDaughter"),
        TEXT("HorrorMovieCharacter_004_210_L_Thigh_2_GradyDaughter"), TEXT("HorrorMovieCharacter_004_211_R_Foot_1_GradyDaughter"),
        TEXT("HorrorMovieCharacter_004_212_R_Foot_1_GradyDaughter"), TEXT("HorrorMovieCharacter_004_213_Spine3_GradyDaughter"),
        TEXT("HorrorMovieCharacter_004_214_Head_GradyDaughter"), TEXT("HorrorMovieCharacter_004_215_Spine2_3_GradyDaughter"),
        TEXT("HorrorMovieCharacter_004_216_Spine1_2_GradyDaughter"), TEXT("HorrorMovieCharacter_004_217_L_Calf_2_GradyDaughter")});
}

AWyrmKnitCharacter::AWyrmKnitCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::Knit, TEXT("It"), 178.f, 1120.f, 25.f, 40.f, 330.f, {
        TEXT("HorrorMovieCharacter_004_218_Balloon_It"), TEXT("HorrorMovieCharacter_004_219_L_Arm_2_It"),
        TEXT("HorrorMovieCharacter_004_220_L_Hand_2_It"), TEXT("HorrorMovieCharacter_004_221_L_ForeArm_2_It"),
        TEXT("HorrorMovieCharacter_004_222_R_Foot_1_It"), TEXT("HorrorMovieCharacter_004_223_R_Foot_1_It"),
        TEXT("HorrorMovieCharacter_004_224_L_Calf_2_It"), TEXT("HorrorMovieCharacter_004_225_Spine3_It"),
        TEXT("HorrorMovieCharacter_004_226_Head_It"), TEXT("HorrorMovieCharacter_004_227_L_Arm_2_It"),
        TEXT("HorrorMovieCharacter_004_228_L_Hand_2_It"), TEXT("HorrorMovieCharacter_004_229_L_ForeArm_2_It"),
        TEXT("HorrorMovieCharacter_004_230_Spine2_3_It"), TEXT("HorrorMovieCharacter_004_231_Spine1_2_It"),
        TEXT("HorrorMovieCharacter_004_232_R_Thigh_1_It"), TEXT("HorrorMovieCharacter_004_233_L_Calf_2_It"),
        TEXT("HorrorMovieCharacter_004_234_L_Thigh_2_It")});
}

AWyrmCanniballCharacter::AWyrmCanniballCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::Canniball, TEXT("Hannibal"), 180.f, 1160.f, 28.f, 41.f, 295.f, {
        TEXT("HorrorMovieCharacter_004_235_Arms_Hannibal"), TEXT("HorrorMovieCharacter_004_236_L_Arm_2_Hannibal"),
        TEXT("HorrorMovieCharacter_004_237_R_Foot_1_Hannibal"), TEXT("HorrorMovieCharacter_004_238_R_Calf_2_Hannibal"),
        TEXT("HorrorMovieCharacter_004_239_R_Thigh_1_Hannibal"), TEXT("HorrorMovieCharacter_004_240_Head_Hannibal"),
        TEXT("HorrorMovieCharacter_004_241_L_Arm_2_Hannibal"), TEXT("HorrorMovieCharacter_004_242_Spine2_3_Hannibal"),
        TEXT("HorrorMovieCharacter_004_243_Spine1_2_Hannibal"), TEXT("HorrorMovieCharacter_004_244_R_Foot_1_Hannibal"),
        TEXT("HorrorMovieCharacter_004_245_R_Calf_2_Hannibal"), TEXT("HorrorMovieCharacter_004_246_R_Thigh_1_Hannibal"),
        TEXT("HorrorMovieCharacter_004_247_Spine3_9_Hannibal")});
}

AWyrmMumsTheWyrdCharacter::AWyrmMumsTheWyrdCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::MumsTheWyrd, TEXT("TheMummy"), 196.f, 1200.f, 31.f, 42.f, 305.f, {
        TEXT("HorrorMovieCharacter_004_248_R_Bandage_TheMummy"), TEXT("HorrorMovieCharacter_004_249_L_Bandage_TheMummy"),
        TEXT("HorrorMovieCharacter_004_250_L_Arm_2_TheMummy"), TEXT("HorrorMovieCharacter_004_251_L_Hand_2_TheMummy"),
        TEXT("HorrorMovieCharacter_004_252_L_ForeArm_2_TheMummy"), TEXT("HorrorMovieCharacter_004_253_R_Foot_1_TheMummy"),
        TEXT("HorrorMovieCharacter_004_254_R_Calf_2_TheMummy"), TEXT("HorrorMovieCharacter_004_255_R_Thigh_1_TheMummy"),
        TEXT("HorrorMovieCharacter_004_256_Head_TheMummy"), TEXT("HorrorMovieCharacter_004_257_L_Arm_2_TheMummy"),
        TEXT("HorrorMovieCharacter_004_258_L_Hand_2_TheMummy"), TEXT("HorrorMovieCharacter_004_259_L_ForeArm_2_TheMummy"),
        TEXT("HorrorMovieCharacter_004_260_Spine2_3_TheMummy"), TEXT("HorrorMovieCharacter_004_261_Spine1_2_TheMummy"),
        TEXT("HorrorMovieCharacter_004_262_R_Foot_1_TheMummy"), TEXT("HorrorMovieCharacter_004_263_R_Calf_2_TheMummy"),
        TEXT("HorrorMovieCharacter_004_264_R_Thigh_1_TheMummy"), TEXT("HorrorMovieCharacter_004_265_Spine3_9_TheMummy")});
}

AWyrmDreadyFreddieCharacter::AWyrmDreadyFreddieCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::DreadyFreddie, TEXT("Freddy"), 184.f, 1240.f, 29.f, 43.f, 345.f, {
        TEXT("HorrorMovieCharacter_004_298_L_Arm_2_Freddy"), TEXT("HorrorMovieCharacter_004_299_L_Hand_2_Freddy"),
        TEXT("HorrorMovieCharacter_004_300_L_ForeArm_2_Freddy"), TEXT("HorrorMovieCharacter_004_301_Head_Freddy"),
        TEXT("HorrorMovieCharacter_004_302_L_Arm_2_Freddy"), TEXT("HorrorMovieCharacter_004_303_L_Hand_2_Freddy"),
        TEXT("HorrorMovieCharacter_004_304_L_ForeArm_2_Freddy"), TEXT("HorrorMovieCharacter_004_305_Spine2_3_Freddy"),
        TEXT("HorrorMovieCharacter_004_306_Spine1_2_Freddy"), TEXT("HorrorMovieCharacter_004_307_R_Foot_1_Freddy"),
        TEXT("HorrorMovieCharacter_004_308_R_Calf_2_Freddy"), TEXT("HorrorMovieCharacter_004_309_R_Thigh_1_Freddy"),
        TEXT("HorrorMovieCharacter_004_310_L_Foot_2_Freddy"), TEXT("HorrorMovieCharacter_004_311_L_Calf_2_Freddy"),
        TEXT("HorrorMovieCharacter_004_312_L_Thigh_2_Freddy"), TEXT("HorrorMovieCharacter_004_313_Spine3_9_Freddy")});
}

AWyrmPyreMidheadCharacter::AWyrmPyreMidheadCharacter()
{
    Configure(EWyrmRequiredHorrorIdentity::PyreMidhead, TEXT("PyramidHead"), 220.f, 1280.f, 34.f, 45.f, 280.f, {
        TEXT("HorrorMovieCharacter_004_16_Sword_PyramidHead"), TEXT("HorrorMovieCharacter_004_17_R_Thigh_1_PyramidHead"),
        TEXT("HorrorMovieCharacter_004_18_R_Thigh_1_PyramidHead"), TEXT("HorrorMovieCharacter_004_19_L_Arm_2_PyramidHead"),
        TEXT("HorrorMovieCharacter_004_20_L_Hand_2_PyramidHead"), TEXT("HorrorMovieCharacter_004_21_L_ForeArm_2_PyramidHead"),
        TEXT("HorrorMovieCharacter_004_22_L_Arm_2_PyramidHead"), TEXT("HorrorMovieCharacter_004_23_L_Hand_2_PyramidHead"),
        TEXT("HorrorMovieCharacter_004_24_L_ForeArm_2_PyramidHead"), TEXT("HorrorMovieCharacter_004_25_Spine2_3_PyramidHead"),
        TEXT("HorrorMovieCharacter_004_26_Spine3_9_PyramidHead"), TEXT("HorrorMovieCharacter_004_27_Head_PyramidHead"),
        TEXT("HorrorMovieCharacter_004_28_Spine1_2_PyramidHead"), TEXT("HorrorMovieCharacter_004_29_R_Foot_1_PyramidHead"),
        TEXT("HorrorMovieCharacter_004_30_R_Thigh_1_PyramidHead"), TEXT("HorrorMovieCharacter_004_31_L_Foot_2_PyramidHead"),
        TEXT("HorrorMovieCharacter_004_32_L_Thigh_2_PyramidHead")});
}
