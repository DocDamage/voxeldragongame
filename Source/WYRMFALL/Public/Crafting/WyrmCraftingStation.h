#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Crafting/WyrmCraftingTypes.h"
#include "WyrmCraftingStation.generated.h"

class UStaticMeshComponent;

/**
 * Authoritative crafting/cooking station in the world (ACT-02, ACT-05).
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Crafting")
class WYRMFALL_API AWyrmCraftingStation : public AActor
{
    GENERATED_BODY()

public:
    AWyrmCraftingStation();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crafting|Components")
    TObjectPtr<UStaticMeshComponent> StationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    EWyrmCraftingStationType StationType = EWyrmCraftingStationType::Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    float InteractionRadius = 250.f;

    UFUNCTION(BlueprintPure, Category="Crafting")
    bool IsActorInRange(const AActor* InActor) const;

    UFUNCTION(BlueprintPure, Category="Crafting")
    EWyrmCraftingStationType GetStationType() const { return StationType; }
};
