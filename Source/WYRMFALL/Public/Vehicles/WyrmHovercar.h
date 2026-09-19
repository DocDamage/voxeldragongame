#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Vehicles/WyrmVehicleTypes.h"
#include "Player/WyrmControlTypes.h"
#include "WyrmHovercar.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class AWyrmCharacter;
class AWyrmDragonCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyrmHovercarStateChangedDelegate, EWyrmHovercarState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyrmHovercarHealthChangedDelegate, float, NewHealth, float, MaxHealth);

/**
 * Authoritative Zenith Civilian Hovercar Pawn.
 * Supports 3D flight/hover locomotion, pilot humanoid entry/exit,
 * compact dragon companion passenger seating, collision damage,
 * depot recovery, traffic separation, and save persistence.
 */
UCLASS()
class WYRMFALL_API AWyrmHovercar : public APawn
{
    GENERATED_BODY()

public:
    AWyrmHovercar();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category="Vehicle", meta=(WorldContext="WorldContextObject"))
    static AWyrmHovercar* SpawnWyrmHovercar(UObject* WorldContextObject, const FTransform& SpawnTransform = FTransform());

    // --- State & Status ---
    UFUNCTION(BlueprintPure, Category="Vehicle")
    EWyrmHovercarState GetHovercarState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category="Vehicle")
    void SetHovercarState(EWyrmHovercarState NewState);

    UFUNCTION(BlueprintPure, Category="Vehicle")
    bool IsAirborne() const;

    UFUNCTION(BlueprintPure, Category="Vehicle")
    bool IsDisabled() const { return CurrentState == EWyrmHovercarState::Disabled; }

    // --- Health & Damage (VEH-06) ---
    UFUNCTION(BlueprintPure, Category="Vehicle|Health")
    float GetHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable, Category="Vehicle|Health")
    bool ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category="Vehicle|Health")
    void RepairFull();

    // --- Depot Recovery (VEH-06) ---
    UFUNCTION(BlueprintCallable, Category="Vehicle|Recovery")
    bool RecoverToDepot(const FVector& DepotLocation, const FRotator& DepotRotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintPure, Category="Vehicle|Recovery")
    FVector GetDepotLocation() const { return RegisteredDepotLocation; }

    UFUNCTION(BlueprintCallable, Category="Vehicle|Recovery")
    void SetDepotLocation(const FVector& InDepotLoc, const FRotator& InDepotRot)
    {
        RegisteredDepotLocation = InDepotLoc;
        RegisteredDepotRotation = InDepotRot;
    }

    // --- Occupancy & Humanoid Pilot (VEH-01, VEH-04) ---
    UFUNCTION(BlueprintPure, Category="Vehicle|Occupancy")
    bool IsOccupied() const { return Driver.IsValid(); }

    UFUNCTION(BlueprintPure, Category="Vehicle|Occupancy")
    AWyrmCharacter* GetDriver() const { return Driver.Get(); }

    UFUNCTION(BlueprintPure, Category="Vehicle|Occupancy")
    bool CanEnter(const AWyrmCharacter* Humanoid, FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Vehicle|Occupancy")
    bool EnterHovercar(AWyrmCharacter* Humanoid);

    UFUNCTION(BlueprintPure, Category="Vehicle|Occupancy")
    bool CanExit(FVector& OutGroundLocation, FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Vehicle|Occupancy")
    bool ExitHovercar(FVector& OutExitLocation);

    // --- Companion Boarding (VEH-05) ---
    UFUNCTION(BlueprintPure, Category="Vehicle|Companion")
    bool HasCompanionBoarded() const { return BoardedCompanion.IsValid(); }

    UFUNCTION(BlueprintPure, Category="Vehicle|Companion")
    AWyrmDragonCharacter* GetBoardedCompanion() const { return BoardedCompanion.Get(); }

    UFUNCTION(BlueprintPure, Category="Vehicle|Companion")
    bool CanBoardPet(const AWyrmDragonCharacter* Pet, FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Vehicle|Companion")
    bool BoardPet(AWyrmDragonCharacter* Pet);

    UFUNCTION(BlueprintCallable, Category="Vehicle|Companion")
    bool UnboardPet(FVector& OutUnboardLocation);

    UFUNCTION(BlueprintCallable, Category="Vehicle|Companion")
    void StagePetAtDepot(AWyrmDragonCharacter* Pet, const FVector& DepotLocation);

    // --- Flight Locomotion (VEH-02, VEH-03) ---
    UFUNCTION(BlueprintCallable, Category="Vehicle|Flight")
    void AddFlightInput(const FVector2D& MovementInput);

    UFUNCTION(BlueprintCallable, Category="Vehicle|Flight")
    void AddAltitudeInput(float Direction);

    UFUNCTION(BlueprintCallable, Category="Vehicle|Flight")
    void AddTurnInput(float Val);

    UFUNCTION(BlueprintPure, Category="Vehicle|Flight")
    bool CanTakeOff(FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Vehicle|Flight")
    bool TakeOff();

    UFUNCTION(BlueprintPure, Category="Vehicle|Flight")
    bool CanLand(FVector& OutLandingLocation, FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Vehicle|Flight")
    bool Land();

    // --- Speed & Mecha Progression Boost (VEH-09) ---
    UFUNCTION(BlueprintPure, Category="Vehicle|Speed")
    float GetCruiseSpeed() const;

    UFUNCTION(BlueprintPure, Category="Vehicle|Speed")
    float GetVerticalSpeed() const { return VerticalSpeed; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Speed")
    bool IsMechaCircuitUnlocked() const { return bMechaCircuitUnlocked; }

    UFUNCTION(BlueprintCallable, Category="Vehicle|Speed")
    void SetMechaCircuitUnlocked(bool bUnlocked) { bMechaCircuitUnlocked = bUnlocked; }

    // --- Camera Control ---
    UFUNCTION(BlueprintPure, Category="Vehicle|Camera")
    EWyrmCameraMode GetCameraMode() const { return CurrentCameraMode; }

    UFUNCTION(BlueprintCallable, Category="Vehicle|Camera")
    void SetCameraMode(EWyrmCameraMode NewMode);

    UFUNCTION(BlueprintCallable, Category="Vehicle|Camera")
    void ToggleCameraMode();

    // --- Sockets & Attachments ---
    UFUNCTION(BlueprintPure, Category="Vehicle|Sockets")
    FTransform GetDriverSeatTransform() const;

    UFUNCTION(BlueprintPure, Category="Vehicle|Sockets")
    FTransform GetPassengerSeatTransform() const;

    // --- Persistence (VEH-07) ---
    void BuildSaveRecord(FWyrmHovercarSaveRecord& OutRecord) const;
    void RestoreFromSaveRecord(const FWyrmHovercarSaveRecord& InRecord, AWyrmCharacter* PotentialDriver = nullptr);

    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UBoxComponent> CollisionBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> VehicleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(BlueprintAssignable, Category="Vehicle|Events")
    FWyrmHovercarStateChangedDelegate OnHovercarStateChanged;

    UPROPERTY(BlueprintAssignable, Category="Vehicle|Events")
    FWyrmHovercarHealthChangedDelegate OnHovercarHealthChanged;

protected:
    virtual void BeginPlay() override;

private:
    void UpdateFlightPhysics(float DeltaSeconds);
    void CheckSweptObstacles(const FVector& MovementDelta, float DeltaSeconds);
    float TraceAltitudeAboveGround(FVector& OutGroundPoint) const;

    UPROPERTY(EditAnywhere, Category="Vehicle|Config")
    float BaseCruiseSpeed = 1500.f; // cm/s

    UPROPERTY(EditAnywhere, Category="Vehicle|Config")
    float BoostedCruiseSpeed = 2000.f; // cm/s (+500 cm/s boost)

    UPROPERTY(EditAnywhere, Category="Vehicle|Config")
    float VerticalSpeed = 600.f; // cm/s

    UPROPERTY(EditAnywhere, Category="Vehicle|Config")
    float TurnRate = 90.f; // deg/s

    UPROPERTY(EditAnywhere, Category="Vehicle|Config")
    float BankingMaxRoll = 20.f; // deg

    UPROPERTY(EditAnywhere, Category="Vehicle|Config")
    float MinHoverAltitude = 80.f; // cm

    UPROPERTY(EditAnywhere, Category="Vehicle|Config")
    float MaxFlightAltitude = 8000.f; // cm (80m ceiling)

    UPROPERTY(EditAnywhere, Category="Vehicle|Config")
    float ImpactDamageVelocityThreshold = 400.f; // cm/s

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    EWyrmHovercarState CurrentState = EWyrmHovercarState::Parked;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    float CurrentHealth = 250.f;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    float MaxHealth = 250.f;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    bool bMechaCircuitUnlocked = false;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    EWyrmCameraMode CurrentCameraMode = EWyrmCameraMode::ThirdPerson;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    FVector CurrentLinearVelocity = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    FVector PendingInputVector = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    float PendingTurnInput = 0.f;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    float PendingAltitudeInput = 0.f;

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    FVector RegisteredDepotLocation = FVector(0.f, 0.f, 100.f);

    UPROPERTY(VisibleAnywhere, Category="Vehicle|State")
    FRotator RegisteredDepotRotation = FRotator::ZeroRotator;

    UPROPERTY(Transient)
    TWeakObjectPtr<AWyrmCharacter> Driver;

    UPROPERTY(Transient)
    TWeakObjectPtr<AWyrmDragonCharacter> BoardedCompanion;
};
