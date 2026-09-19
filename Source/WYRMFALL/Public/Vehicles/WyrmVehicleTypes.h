#pragma once

#include "CoreMinimal.h"
#include "Player/WyrmControlTypes.h"
#include "WyrmVehicleTypes.generated.h"

UENUM(BlueprintType)
enum class EWyrmHovercarState : uint8
{
    Parked UMETA(DisplayName="Parked On Ground"),
    Hovering UMETA(DisplayName="Hovering Low Altitude"),
    Cruising UMETA(DisplayName="Cruising In Flight"),
    Landing UMETA(DisplayName="Landing Sequence"),
    Disabled UMETA(DisplayName="Disabled / Wrecked")
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmHovercarSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bHasBeenSpawned = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName VehicleId = FName(TEXT("ZenithHovercar_01"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    EWyrmHovercarState State = EWyrmHovercarState::Parked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float Health = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float MaxHealth = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FRotator WorldRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bCompanionBoarded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bMechaCircuitUnlocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    EWyrmCameraMode CameraMode = EWyrmCameraMode::ThirdPerson;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector LinearVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector DepotLocation = FVector(0.f, 0.f, 100.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FRotator DepotRotation = FRotator::ZeroRotator;
};
