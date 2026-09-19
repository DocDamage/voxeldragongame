#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmJadePeaksLandmark.generated.h"

class USphereComponent;

UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|JadePeaks")
class WYRMFALL_API AWyrmJadePeaksLandmark : public AActor
{
    GENERATED_BODY()

public:
    AWyrmJadePeaksLandmark();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jade Peaks|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jade Peaks|Components")
    TObjectPtr<USphereComponent> TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    FName LandmarkId = NAME_None;

    UFUNCTION(BlueprintCallable, Category="Jade Peaks")
    bool TriggerVisit(AActor* VisitingActor);

protected:
    virtual void PostInitializeComponents() override;

    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);
};
