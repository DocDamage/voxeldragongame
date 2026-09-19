#pragma once

#include "CoreMinimal.h"
#include "WyrmDragonTypes.generated.h"

UENUM(BlueprintType)
enum class EWyrmDragonRole : uint8
{
    HostileBoss UMETA(DisplayName="Hostile Boss"),
    DefeatedAlive UMETA(DisplayName="Defeated Alive (Living Terminal State)"),
    AlliedCompanion UMETA(DisplayName="Allied Companion"),
    Recovering UMETA(DisplayName="Recovering / Incapacitated")
};

UENUM(BlueprintType)
enum class EWyrmCompanionOrder : uint8
{
    Follow UMETA(DisplayName="Follow Humanoid"),
    Hold UMETA(DisplayName="Hold Position"),
    AttackTarget UMETA(DisplayName="Attack Target"),
    Return UMETA(DisplayName="Return / Regroup")
};

UENUM(BlueprintType)
enum class EWyrmDragonForm : uint8
{
    CompanionForm UMETA(DisplayName="Compact Companion Form"),
    TrueForm UMETA(DisplayName="True Dragon Form")
};

UENUM(BlueprintType)
enum class EWyrmTetherStatus : uint8
{
    WithinTether UMETA(DisplayName="Within Safe Tether (< 120m)"),
    Warning UMETA(DisplayName="Tether Warning Distance (120m - 150m)"),
    LimitReached UMETA(DisplayName="Tether Limit Reached (>= 150m)")
};

UENUM(BlueprintType)
enum class EWyrmDragonFlightState : uint8
{
    Grounded UMETA(DisplayName="Grounded"),
    TakingOff UMETA(DisplayName="Taking Off"),
    Flying UMETA(DisplayName="Flying"),
    Landing UMETA(DisplayName="Landing")
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmDragonRigProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    FName DragonId = FName(TEXT("Verdance"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    FString DisplayName = TEXT("Verdance");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    FString LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local.Hip-Local");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    FString FollowerMeshBasePath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    TArray<FString> FollowerMeshNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    FString SkeletonPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    FString IdleAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonIdle_01.Green_DragonIdle_01");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    FString FlightAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonFlying_01.Green_DragonFlying_01");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float CompanionMeshScale = 0.009f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float CompanionCapsuleRadius = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float CompanionCapsuleHalfHeight = 35.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float CompanionGroundSpeed = 450.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float TrueFormMeshScale = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float TrueFormCapsuleRadius = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float TrueFormCapsuleHalfHeight = 160.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float TrueFormGroundSpeed = 550.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float FlightSpeed = 1600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    FVector MountSocketOffset = FVector(0.f, 0.f, 160.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float TakeoffClearanceHeight = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float WingSpanSweepRadius = 350.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float LandingSearchDistance = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon|Rig")
    float MaxLandingSlopeAngle = 45.f;

    static bool GetRigProfile(FName InDragonId, FWyrmDragonRigProfile& OutProfile);
    static bool IsValidDragonRig(FName InDragonId);
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmDragonSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName DragonId = FName(TEXT("Verdance"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    EWyrmDragonRole Role = EWyrmDragonRole::HostileBoss;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    EWyrmDragonForm Form = EWyrmDragonForm::CompanionForm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    EWyrmCompanionOrder Order = EWyrmCompanionOrder::Follow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bHasBondReceipt = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float Health = 420.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float MaxHealth = 420.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float Focus = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float MaxFocus = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float AreaAttackCooldownRemaining = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FRotator WorldRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bIsDirectlyControlled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector HumanoidWaitingLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FRotator HumanoidWaitingRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bIsRiderMounted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    EWyrmDragonFlightState FlightState = EWyrmDragonFlightState::Grounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector SafeGroundAnchor = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float FormTransitionCooldownRemaining = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bTownModeEnabled = false;
};

