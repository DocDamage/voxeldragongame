#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmGloamingArrivalTrigger.generated.h"

class USphereComponent;

/** Arrival route trigger that establishes the first Gloaming regional fact. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmGloamingArrivalTrigger : public AActor
{
    GENERATED_BODY()

public:
    AWyrmGloamingArrivalTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Components")
    TObjectPtr<USphereComponent> TriggerVolume;

    UFUNCTION(BlueprintCallable, Category="Gloaming|Arrival")
    bool TriggerArrival(AActor* VisitingActor);

protected:
    virtual void PostInitializeComponents() override;

    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);
};
