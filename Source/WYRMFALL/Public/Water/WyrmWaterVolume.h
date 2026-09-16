#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Terrain/WyrmTerrainProvider.h"
#include "WyrmWaterVolume.generated.h"

class UBoxComponent;
class AWyrmCharacter;

/**
 * Authoritative bounded water volume actor for WYRMFALL.
 * Supplies single-owner wet/dry/swimming state (WRLD-10) and protects outer basin boundaries from breach (WRLD-11).
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Water")
class WYRMFALL_API AWyrmWaterVolume : public AActor
{
    GENERATED_BODY()

public:
    AWyrmWaterVolume();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Water|Components")
    TObjectPtr<UBoxComponent> WaterBox;

    /** Z elevation of the water surface. If 0, initialized from top of WaterBox */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Water|Properties")
    float SurfaceElevation = 0.f;

    /** Perimeter width in cm representing the basin's retaining edge that cannot be excavated (WRLD-11) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Water|Properties")
    float OuterLipThickness = 150.f;

    /** Whether excavation inside the interior water bed is permitted (WRLD-10) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Water|Properties")
    bool bAllowBedExcavation = true;

    /** Whether to reject excavations that breach the basin's outer retaining boundary (WRLD-11) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Water|Properties")
    bool bProtectOuterBoundary = true;

    // --- Authoritative Queries ---
    UFUNCTION(BlueprintPure, Category="Water")
    bool IsPointInWater(const FVector& Point) const;

    UFUNCTION(BlueprintPure, Category="Water")
    float GetWaterSurfaceElevation(const FVector& Point) const { return SurfaceElevation; }

    UFUNCTION(BlueprintPure, Category="Water")
    float GetWaterDepth(const FVector& Point) const;

    UFUNCTION(BlueprintPure, Category="Water")
    bool CanFishAtLocation(const FVector& CastLocation, FString& OutReason) const;

    /** Validates whether a terrain edit is permitted relative to this water body (WRLD-10, WRLD-11) */
    UFUNCTION(BlueprintCallable, Category="Water")
    bool ValidateTerrainEdit(const FWyrmTerrainEditRequest& Request, FString& OutRejectionReason) const;

    /** Script-facing convenience wrapper used by focused runtime verification. */
    UFUNCTION(BlueprintCallable, Category="Water")
    bool ValidateTerrainEditAtLocation(
        const FVector& Center, float RadiusCm, bool bAddOperation, FString& OutRejectionReason) const;

    UFUNCTION(BlueprintPure, Category="Water")
    bool IsTerrainEditAllowedAtLocation(const FVector& Center, float RadiusCm, bool bAddOperation) const;

    UFUNCTION(BlueprintPure, Category="Water")
    FBox GetWaterBounds() const;

protected:
    UFUNCTION()
    void OnWaterBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                             bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnWaterEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateCharacterWaterState(AWyrmCharacter* Character);

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AWyrmCharacter>> OverlappingCharacters;
};
