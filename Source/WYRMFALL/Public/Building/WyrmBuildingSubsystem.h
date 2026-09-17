#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Building/WyrmBuildingTypes.h"
#include "WyrmBuildingSubsystem.generated.h"

class AWyrmBuildingPiece;
class AWyrmRecoveryBundleActor;
class UWorld;

/**
 * Authoritative subsystem managing camp building piece definitions,
 * placement validation, atomic construction, demolition refunds,
 * recovery overflow bundles, and camp persistence.
 */
UCLASS(BlueprintType, Category="WYRMFALL|Building")
class WYRMFALL_API UWyrmBuildingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Registry ---
    UFUNCTION(BlueprintCallable, Category="Building|Registry")
    void RegisterDefaultDefinitions();

    UFUNCTION(BlueprintCallable, Category="Building|Registry")
    bool RegisterPieceDefinition(const FWyrmBuildingPieceDefinition& Definition);

    UFUNCTION(BlueprintPure, Category="Building|Registry")
    bool GetPieceDefinition(FName PieceId, FWyrmBuildingPieceDefinition& OutDef) const;

    UFUNCTION(BlueprintPure, Category="Building|Registry")
    TArray<FWyrmBuildingPieceDefinition> GetAllDefinitions() const;

    // --- Active Pieces ---
    UFUNCTION(BlueprintPure, Category="Building")
    TArray<AWyrmBuildingPiece*> GetActivePieces() const;

    // --- Validation & Placement ---
    /**
     * Previews placement and checks for materials, ground support, structural support,
     * pawn occupancy, and overlap. Zero side effects. (ACT-07, WRLD-08)
     */
    UFUNCTION(BlueprintCallable, Category="Building")
    EWyrmPlacementRejection ValidatePlacement(
        FName PieceId,
        const FTransform& Transform,
        AActor* InstigatorActor,
        FString& OutReason) const;

    /**
     * Atomically executes placement: validates, deducts materials, spawns piece,
     * and registers support footprint with terrain adapter. (ACT-06, ACT-07)
     */
    UFUNCTION(BlueprintCallable, Category="Building")
    AWyrmBuildingPiece* ExecutePlacement(
        FName PieceId,
        const FTransform& Transform,
        AActor* InstigatorActor,
        EWyrmPlacementRejection& OutRejection,
        FString& OutReason);

    /**
     * Demolishes a piece: refunds materials, empties storage, and deposits into instigator bag.
     * If bag is full, spawns an authoritative recovery bundle with remaining items. (ACT-09)
     */
    UFUNCTION(BlueprintCallable, Category="Building")
    bool DemolishPiece(
        AWyrmBuildingPiece* Piece,
        AActor* InstigatorActor,
        AWyrmRecoveryBundleActor*& OutRecoveryBundle,
        FString& OutReason);

    // --- Companion Growth Clearance (ACT-10) ---
    /**
     * Checks if a companion/dragon has vertical clearance to grow at the given location.
     * Physical roofs and ceilings block growth with explicit reason. (ACT-10)
     */
    UFUNCTION(BlueprintCallable, Category="Building")
    bool CheckCompanionGrowthClearance(
        UWorld* World,
        const FVector& Location,
        float RequiredHeight,
        float RequiredRadius,
        FString& OutReason) const;

    // --- Persistence (ACT-10) ---
    UFUNCTION(BlueprintCallable, Category="Building|Save")
    void BuildSaveRecord(FWyrmCampSaveRecord& OutRecord) const;

    UFUNCTION(BlueprintCallable, Category="Building|Save")
    void RestoreFromSaveRecord(const FWyrmCampSaveRecord& InRecord, UWorld* WorldContext);

    UFUNCTION(BlueprintCallable, Category="Building")
    void ClearAllPlacedPieces();

    UFUNCTION(BlueprintCallable, Category="Building")
    void SetWorldContext(UWorld* InWorld) { CachedWorld = InWorld; }

    UFUNCTION(BlueprintPure, Category="Building")
    UWorld* GetWorldContext() const { return CachedWorld.IsValid() ? CachedWorld.Get() : nullptr; }

private:
    UPROPERTY(Transient)
    TMap<FName, FWyrmBuildingPieceDefinition> PieceRegistry;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AWyrmBuildingPiece>> ActivePieces;

    UPROPERTY(Transient)
    TWeakObjectPtr<UWorld> CachedWorld;
};
