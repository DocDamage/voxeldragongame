#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmGloamingAshgraveSeal.generated.h"

class UBoxComponent;

/** Invisible interaction volume authored against the supplied Ashgrave crypt props. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmGloamingAshgraveSeal : public AActor
{
    GENERATED_BODY()

public:
    AWyrmGloamingAshgraveSeal();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Components")
    TObjectPtr<UBoxComponent> InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gloaming|Interaction", meta=(ClampMin="1.0"))
    float InteractionRadius = 325.f;

    UFUNCTION(BlueprintPure, Category="Gloaming|Interaction")
    bool CanInteract(const AActor* Interactor) const;

    UFUNCTION(BlueprintCallable, Category="Gloaming|Interaction")
    bool Interact(AActor* Interactor);
};
