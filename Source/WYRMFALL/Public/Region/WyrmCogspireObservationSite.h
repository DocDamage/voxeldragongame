#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmCogspireObservationSite.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class EWyrmCogspireObservationType : uint8
{
    PublicMachinery,
    CoercionDiversion,
    BaronAcknowledgment
};

/** Invisible interaction authored beside supplied Cogspire assets; it only commits ordered facts. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Cogspire")
class WYRMFALL_API AWyrmCogspireObservationSite : public AActor
{
    GENERATED_BODY()

public:
    AWyrmCogspireObservationSite();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cogspire|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cogspire|Components")
    TObjectPtr<UBoxComponent> InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire|Observation")
    EWyrmCogspireObservationType ObservationType = EWyrmCogspireObservationType::PublicMachinery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire|Observation", meta=(ClampMin="1.0"))
    float InteractionRadius = 425.f;

    UFUNCTION(BlueprintPure, Category="Cogspire|Observation")
    bool CanInteract(const AActor* Interactor) const;

    UFUNCTION(BlueprintCallable, Category="Cogspire|Observation")
    bool Interact(AActor* Interactor);
};
