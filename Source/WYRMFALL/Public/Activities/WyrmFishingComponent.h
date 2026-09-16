#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "WyrmFishingComponent.generated.h"

class AWyrmWaterVolume;
class AWyrmCharacter;

UENUM(BlueprintType)
enum class EWyrmFishingState : uint8
{
    Ready,
    Aiming,
    Casting,
    Waiting,
    BiteWindow,
    Reeling,
    CatchPending,
    Caught,
    Cancelled
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyrmOnFishingStateChanged, EWyrmFishingState, NewState, EWyrmFishingState, OldState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyrmOnFishingBitePrompt, float, WindowDurationSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyrmOnFishingCatchCommitted, const FWyrmItemInstance&, FishItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyrmOnFishingCancelled, const FString&, Reason);

/**
 * Authoritative fishing interaction component for player character (ACT-01, ACT-03).
 * Enforces state machine, prompt-and-reel input, damage/movement cancellation, and inventory commit.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WYRMFALL_API UWyrmFishingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWyrmFishingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintAssignable, Category="Fishing|Events")
    FWyrmOnFishingStateChanged OnFishingStateChanged;

    UPROPERTY(BlueprintAssignable, Category="Fishing|Events")
    FWyrmOnFishingBitePrompt OnFishingBitePrompt;

    UPROPERTY(BlueprintAssignable, Category="Fishing|Events")
    FWyrmOnFishingCatchCommitted OnFishingCatchCommitted;

    UPROPERTY(BlueprintAssignable, Category="Fishing|Events")
    FWyrmOnFishingCancelled OnFishingCancelled;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fishing|Config")
    float BiteWaitTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fishing|Config")
    float BiteWindowDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fishing|Config")
    float ReelDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fishing|Config")
    float MaxCastDistance = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fishing|Config")
    float MovementCancelTolerance = 100.f;

    // --- Actions ---
    /** Starts fishing loop directed at target water location (ACT-01) */
    UFUNCTION(BlueprintCallable, Category="Fishing")
    bool StartFishing(const FVector& TargetWaterLocation, FString& OutFailureReason);

    /** Triggers a fish bite, entering prompted bite window (ACT-01) */
    UFUNCTION(BlueprintCallable, Category="Fishing")
    void TriggerBite();

    /** Responds to prompted bite to begin reeling (ACT-01) */
    UFUNCTION(BlueprintCallable, Category="Fishing")
    bool RespondToBite();

    /** Commits the catch directly into authoritative inventory (ACT-01, ACT-03) */
    UFUNCTION(BlueprintCallable, Category="Fishing")
    bool CommitCatch();

    /** Cancels active fishing immediately with zero resources lost or awarded (ACT-03) */
    UFUNCTION(BlueprintCallable, Category="Fishing")
    void CancelFishing(const FString& Reason);

    /** Called when owning character takes combat damage to interrupt activity (ACT-03) */
    UFUNCTION(BlueprintCallable, Category="Fishing")
    void NotifyCombatDamageTaken(float DamageAmount);

    // --- State Queries ---
    UFUNCTION(BlueprintPure, Category="Fishing")
    EWyrmFishingState GetFishingState() const { return FishingState; }

    UFUNCTION(BlueprintPure, Category="Fishing")
    bool IsFishingActive() const
    {
        return FishingState != EWyrmFishingState::Ready &&
               FishingState != EWyrmFishingState::Caught &&
               FishingState != EWyrmFishingState::Cancelled;
    }

    UFUNCTION(BlueprintPure, Category="Fishing")
    const FWyrmItemInstance& GetPendingCatch() const { return PendingCatchItem; }

    UFUNCTION(BlueprintPure, Category="Fishing")
    bool WasCatchRejectedBagFull() const { return bCatchRejectedBagFull; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fishing|State")
    EWyrmFishingState FishingState = EWyrmFishingState::Ready;

    UPROPERTY(Transient)
    TWeakObjectPtr<AWyrmWaterVolume> ActiveWaterVolume;

    UPROPERTY(Transient)
    FVector CastTargetLocation = FVector::ZeroVector;

    UPROPERTY(Transient)
    FVector CastOriginLocation = FVector::ZeroVector;

    UPROPERTY(Transient)
    FWyrmItemInstance PendingCatchItem;

    UPROPERTY(Transient)
    bool bCatchRejectedBagFull = false;

    FTimerHandle BiteTimerHandle;
    FTimerHandle WindowTimerHandle;
    FTimerHandle ReelTimerHandle;

    void SetFishingState(EWyrmFishingState NewState);
    void OnBiteWindowTimeout();
    void OnReelComplete();
    AWyrmWaterVolume* FindWaterVolumeAtLocation(const FVector& Location) const;
};
