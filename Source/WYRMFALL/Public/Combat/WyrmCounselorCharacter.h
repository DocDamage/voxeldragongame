#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "WyrmCounselorCharacter.generated.h"

class AWyrmCharacter;

/**
 * Authoritative Counselor horror encounter character located at Silent Landing.
 * - Draft HP 600, Armor 10, Power 20 (14 damage normal strike).
 * - Demonstrates Relentless Advance stance (6s duration, 18s cooldown) where slows and
 *   light/medium stagger are resisted while still taking full damage.
 * - Has punishable recovery window following stance expiration.
 * - Defeat resolution grants echo.relentless_advance once + ordinary loot with full-bag safety
 *   and strict idempotency.
 */
UCLASS()
class WYRMFALL_API AWyrmCounselorCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmCounselorCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Counselor|Combat")
    bool bInRelentlessStance = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Counselor|Combat")
    bool bPunishableRecovery = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Counselor|Combat")
    float StanceRemainingTimer = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Counselor|Combat")
    float StanceCooldownTimer = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Counselor|Combat")
    float RecoveryRemainingTimer = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Counselor|Resolution")
    bool bEncounterResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Counselor|Resolution")
    bool bEchoGranted = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Counselor|Resolution")
    bool bOrdinaryLootDelivered = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Counselor|Resolution")
    TArray<FWyrmItemInstance> PreservedLoot;

    UFUNCTION(BlueprintCallable, Category="Counselor|Resolution")
    void ResetResolutionState()
    {
        bEncounterResolved = false;
        bEchoGranted = false;
        bOrdinaryLootDelivered = false;
        PreservedLoot.Empty();
    }

    UFUNCTION(BlueprintCallable, Category="Counselor|Combat")
    void ReviveCounselor(float NewHealth = 600.f);

    UFUNCTION(BlueprintCallable, Category="Counselor|Combat")
    bool EnterRelentlessStance();

    UFUNCTION(BlueprintCallable, Category="Counselor|Combat")
    bool PerformCounselorStrike(AActor* TargetActor);

    UFUNCTION(BlueprintPure, Category="Counselor|Combat")
    bool CanEnterStance() const;

    UFUNCTION(BlueprintPure, Category="Counselor|Combat")
    bool IsInRelentlessStance() const { return bInRelentlessStance; }

    UFUNCTION(BlueprintPure, Category="Counselor|Combat")
    bool IsInPunishableRecovery() const { return bPunishableRecovery; }

    /**
     * Authoritative resolution callback when Counselor is defeated.
     * Grants echo.relentless_advance once and ordinary loot once with full-bag safety.
     * Strictly idempotent: replaying returns false and rejects repeated rewards.
     */
    UFUNCTION(BlueprintCallable, Category="Counselor|Resolution")
    bool ResolveEncounter(AWyrmCharacter* PlayerCharacter);

    /** Safe loot claim method if inventory was full upon initial resolution */
    UFUNCTION(BlueprintCallable, Category="Counselor|Resolution")
    bool ClaimPreservedLoot(AWyrmCharacter* PlayerCharacter);

    virtual void ApplyStatusEffect(FGameplayTag StatusTag, float DurationSeconds, float Magnitude = 1.f) override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    void HandleCounselorDefeat(const struct FOnAttributeChangeData& Data);
};
