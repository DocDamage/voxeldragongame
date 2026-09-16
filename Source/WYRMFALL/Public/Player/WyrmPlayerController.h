#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Player/WyrmControlTypes.h"
#include "WyrmPlayerController.generated.h"
class UInputAction;
class UInputMappingContext;

UCLASS()
class WYRMFALL_API AWyrmPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    AWyrmPlayerController();

    UFUNCTION(BlueprintCallable, Category="Control", meta=(WorldContext="WorldContextObject"))
    static AWyrmPlayerController* SpawnWyrmPlayerController(UObject* WorldContextObject, const FTransform& SpawnTransform = FTransform());

    UFUNCTION(BlueprintPure, Category="Control")
    bool IsClickMoveEnabled() const { return bClickMoveEnabled; }

    UFUNCTION(BlueprintCallable, Category="Control")
    void SetClickMoveEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Control")
    bool IsMovementLocked() const { return bMovementLocked; }

    UFUNCTION(BlueprintCallable, Category="Control")
    void SetMovementLocked(bool bLocked);

    UFUNCTION(BlueprintPure, Category="Camera")
    EWyrmCameraMode GetActiveCameraMode() const;

    UFUNCTION(BlueprintCallable, Category="Camera")
    void SetActiveCameraMode(EWyrmCameraMode NewMode);

    UFUNCTION(BlueprintCallable, Category="Control")
    void CaptureControlState(FWyrmControlState& OutState) const;

    UFUNCTION(BlueprintCallable, Category="Control")
    void RestoreControlState(const FWyrmControlState& InState);

    UFUNCTION(BlueprintCallable, Category="Control")
    bool RequestClickMoveToDestination(const FVector& DestinationLocation);

#if WITH_EDITOR
    // BOOT-01 instrumentation: exercise repeated setup on the live PIE controller.
    UFUNCTION(Exec) void WyrmDevRebindInput();
#endif
protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
private:
    void BuildInputContext();
    void RefreshCursor();
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void LookGamepad(const FInputActionValue& Value);
    void SwitchCamera();
    void ToggleClickMove();
    void ClickMove();
    void JumpPressed();
    void JumpReleased();
    UPROPERTY(Transient) TObjectPtr<UInputMappingContext> Context;
    UPROPERTY(Transient) TObjectPtr<UInputAction> MoveAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> LookAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> PadLookAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> CameraAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> ClickToggleAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> ClickAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> JumpAction;
    bool bClickMoveEnabled = true;
    bool bMovementLocked = false;
};
