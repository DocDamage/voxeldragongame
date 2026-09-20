#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmCogspireArrivalTrigger.generated.h"

class USphereComponent;

/** Authored arrival trigger that only accepts the completed Cogspire travel context. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Cogspire")
class WYRMFALL_API AWyrmCogspireArrivalTrigger : public AActor
{
    GENERATED_BODY()

public:
    AWyrmCogspireArrivalTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cogspire|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cogspire|Components")
    TObjectPtr<USphereComponent> TriggerVolume;

    UFUNCTION(BlueprintCallable, Category="Cogspire|Arrival")
    bool TriggerArrival(AActor* VisitingActor);

protected:
    virtual void PostInitializeComponents() override;

    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);
};
