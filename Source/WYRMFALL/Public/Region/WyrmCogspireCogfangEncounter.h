#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmCogspireCogfangEncounter.generated.h"

class AWyrmCharacter;
class AWyrmDragonCharacter;
class USphereComponent;

/** Coordinates the bounded Cogfang fight, coercion-governor shutdown, and voluntary bond. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Cogspire")
class WYRMFALL_API AWyrmCogspireCogfangEncounter : public AActor
{
    GENERATED_BODY()

public:
    AWyrmCogspireCogfangEncounter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cogspire|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cogspire|Components")
    TObjectPtr<USphereComponent> EncounterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire|Actors")
    TObjectPtr<AWyrmDragonCharacter> Cogfang;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire|Interaction", meta=(ClampMin="1.0"))
    float EncounterRadius = 1100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire|Interaction", meta=(ClampMin="1.0"))
    float GovernorInteractionRadius = 425.f;

    UFUNCTION(BlueprintPure, Category="Cogspire|Combat")
    bool CanBeginEncounter(const AWyrmCharacter* Player) const;

    UFUNCTION(BlueprintCallable, Category="Cogspire|Combat")
    bool BeginEncounter(AWyrmCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="Cogspire|Engine")
    bool InteractShutdownGovernor(AWyrmCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="Cogspire|Bond")
    bool InteractVoluntaryBond(AWyrmCharacter* Player);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleDragonDefeated(AWyrmDragonCharacter* DefeatedDragon);
};
