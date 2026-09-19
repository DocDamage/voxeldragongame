#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "WyrmCorvynCharacter.generated.h"

class AWyrmCharacter;

/**
 * Authoritative Ser Corvyn the Cursed encounter character.
 * - Boss profile: HP 750, Armor 12, Power 22.
 * - Demonstrates beast combat kit (beast claw strike and pounce).
 * - Implements dual resolution paths with strict parity (ECHO-07):
 *   - Hostile living defeat: commits corvyn.defeated_hostile.
 *   - Authored mercy/cure: commits corvyn.cured.
 *   - Both grant equivalent signature capability: Unlock.Echo.MoonboundForm,
 *     echo.moonbound_form fact, and Item_CorvynRelic loot.
 * - Strict idempotency: replaying resolution rejects duplicate grants.
 * - Full-bag safety: ordinary loot safely preserved in PreservedLoot if bag is full.
 */
UCLASS()
class WYRMFALL_API AWyrmCorvynCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmCorvynCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Corvyn|Resolution")
    bool bEncounterResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Corvyn|Resolution")
    bool bHostileResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Corvyn|Resolution")
    bool bMercyResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Corvyn|Resolution")
    bool bEchoGranted = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Corvyn|Resolution")
    bool bOrdinaryLootDelivered = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Corvyn|Resolution")
    TArray<FWyrmItemInstance> PreservedLoot;

    UFUNCTION(BlueprintCallable, Category="Corvyn|Resolution")
    void ResetResolutionState()
    {
        bEncounterResolved = false;
        bHostileResolved = false;
        bMercyResolved = false;
        bEchoGranted = false;
        bOrdinaryLootDelivered = false;
        PreservedLoot.Empty();
    }

    UFUNCTION(BlueprintCallable, Category="Corvyn|Combat")
    void ReviveCorvyn(float NewHealth = 750.f);

    UFUNCTION(BlueprintCallable, Category="Corvyn|Combat")
    bool PerformBeastStrike(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category="Corvyn|Combat")
    bool PerformBeastPounce(AActor* TargetActor);

    /**
     * Authoritative hostile resolution callback when Corvyn is defeated in combat.
     * Commits corvyn.defeated_hostile and grants MoonboundForm + loot.
     */
    UFUNCTION(BlueprintCallable, Category="Corvyn|Resolution")
    bool ResolveEncounterHostile(AWyrmCharacter* PlayerCharacter);

    /**
     * Authoritative mercy/cure resolution callback via authored cure/trust action.
     * Commits corvyn.cured and grants MoonboundForm + loot with zero reward penalty.
     */
    UFUNCTION(BlueprintCallable, Category="Corvyn|Resolution")
    bool ResolveEncounterMercy(AWyrmCharacter* PlayerCharacter);

    /** Safe loot claim method if inventory was full upon initial resolution */
    UFUNCTION(BlueprintCallable, Category="Corvyn|Resolution")
    bool ClaimPreservedLoot(AWyrmCharacter* PlayerCharacter);

protected:
    virtual void BeginPlay() override;

private:
    bool InternalResolve(AWyrmCharacter* PlayerCharacter, bool bIsMercy);
};
