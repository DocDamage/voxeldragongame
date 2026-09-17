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
