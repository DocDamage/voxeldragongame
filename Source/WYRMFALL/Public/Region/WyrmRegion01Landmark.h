#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmRegion01Landmark.generated.h"

class USphereComponent;

/**
 * Placed in-world landmark volume that reports visits to UWyrmRegion01Subsystem
 * when overlapping the player character.
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Region01")
class WYRMFALL_API AWyrmRegion01Landmark : public AActor
{
    GENERATED_BODY()

public:
    AWyrmRegion01Landmark();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Region 01|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Region 01|Components")
    TObjectPtr<USphereComponent> TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    FName LandmarkId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    float TriggerRadius = 400.f;

    UFUNCTION(BlueprintPure, Category="Region 01")
    bool IsLandmarkAvailable() const;

    UFUNCTION(BlueprintPure, Category="Region 01")
    bool HasBeenVisited() const;

    UFUNCTION(BlueprintCallable, Category="Region 01")
    bool TriggerVisit(AActor* VisitingActor);

protected:
    virtual void PostInitializeComponents() override;

    UFUNCTION()
    void HandleOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);
};
