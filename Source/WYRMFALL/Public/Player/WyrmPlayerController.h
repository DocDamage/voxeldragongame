#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "WyrmPlayerController.generated.h"
class UInputAction;
class UInputMappingContext;

UCLASS()
class WYRMFALL_API AWyrmPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    AWyrmPlayerController();
    bool IsClickMoveEnabled() const { return bClickMoveEnabled; }
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
    bool bClickMoveEnabled = false;
};
