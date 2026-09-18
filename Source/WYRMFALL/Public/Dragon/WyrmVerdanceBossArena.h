#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dragon/WyrmDragonTypes.h"
#include "WyrmVerdanceBossArena.generated.h"

class AWyrmDragonCharacter;
class AWyrmRegion01Interactable;
class AWyrmEnemyCharacter;
class AWyrmCharacter;
class USphereComponent;

UENUM(BlueprintType)
enum class EWyrmBossArenaState : uint8
{
    Inactive,
    BossCombat,
    LivingDefeat,
    ClaimBroken,
    CompanionBonded,
    ReliefCombat,
    ReliefResolved
};

/**
 * Authoritative encounter coordinator for the Verdance quarry arena in Region 01.
 * Governs boss combat phases, auxiliary restraint interference pulses, central claim
 * destruction, voluntary bond consent, and post-bond Crown relief squad combat.
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Dragon")
class WYRMFALL_API AWyrmVerdanceBossArena : public AActor
{
    GENERATED_BODY()

public:
    AWyrmVerdanceBossArena();

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Arena|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Arena|Components")
    TObjectPtr<USphereComponent> ArenaVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Setup")
    FVector ArenaCenter = FVector(2000.f, 0.f, 800.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Setup")
    float ArenaRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Setup")
    FVector BossSpawnOffset = FVector(0.f, 300.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Setup")
    FVector ClaimConsoleOffset = FVector(0.f, -200.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Setup")
    FVector ReliefSpawnOffset = FVector(800.f, -100.f, 0.f);

    // --- References ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Actors")
    TObjectPtr<AWyrmDragonCharacter> VerdanceBoss;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Actors")
    TObjectPtr<AWyrmRegion01Interactable> CentralClaimConsole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Actors")
    TArray<TObjectPtr<AWyrmEnemyCharacter>> ReliefSquad;

    // --- State & Auxiliary Restraint (REG-06) ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Arena|State")
    EWyrmBossArenaState ArenaState = EWyrmBossArenaState::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Auxiliary")
    float InterferencePulseInterval = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arena|Auxiliary")
    float InterferencePulseDamage = 15.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Arena|Auxiliary")
    int32 TotalInterferencePulsesEmitted = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Arena|Auxiliary")
    float InterferenceTimer = 0.0f;

    UFUNCTION(BlueprintPure, Category="Arena|Auxiliary")
    bool IsAuxiliaryInterferenceActive() const;

    UFUNCTION(BlueprintPure, Category="Arena|Auxiliary")
    int32 GetInterferencePulseCount() const { return TotalInterferencePulsesEmitted; }

    UFUNCTION(BlueprintCallable, Category="Arena|Auxiliary")
    void EmitInterferencePulse();

    // --- Boss Combat & Living Defeat (DRG-01, REG-06) ---
    UFUNCTION(BlueprintCallable, Category="Arena|Combat")
    void StartBossCombat();

    UFUNCTION(BlueprintCallable, Category="Arena|Combat")
    void HandleDragonDefeated(AWyrmDragonCharacter* DefeatedDragon);

    // --- Claim & Consent Sequence (REG-07) ---
    UFUNCTION(BlueprintPure, Category="Arena|Claim")
    bool CanBreakClaim() const;

    UFUNCTION(BlueprintCallable, Category="Arena|Claim")
    bool InteractBreakClaim(AActor* Interactor);

    UFUNCTION(BlueprintPure, Category="Arena|Bond")
    bool CanOfferVoluntaryBond(const AActor* Interactor) const;

    UFUNCTION(BlueprintCallable, Category="Arena|Bond")
    bool InteractVoluntaryBond(AWyrmCharacter* Humanoid);

    // --- Post-Bond Relief Encounter (REG-08) ---
    UFUNCTION(BlueprintCallable, Category="Arena|Relief")
    void TriggerReliefEncounter();

    UFUNCTION(BlueprintCallable, Category="Arena|Relief")
    bool ResolveReliefCombat();

    // --- Synchronization & Queries ---
    UFUNCTION(BlueprintPure, Category="Arena|State")
    EWyrmBossArenaState GetArenaState() const { return ArenaState; }

    UFUNCTION(BlueprintCallable, Category="Arena|State")
    void SyncArenaStateWithLedger();

    UFUNCTION(BlueprintPure, Category="Arena|State")
    bool IsPlayerInsideArena() const;

protected:
    virtual void BeginPlay() override;

private:
    AWyrmCharacter* GetPlayerCharacter() const;
};
