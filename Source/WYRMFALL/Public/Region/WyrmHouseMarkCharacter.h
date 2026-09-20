#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmHouseMarkCharacter.generated.h"

class AWyrmCharacter;
class USkeletalMeshComponent;

/** Supplied-art House Mark champion and bounded optional investigation encounter. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Cogspire")
class WYRMFALL_API AWyrmHouseMarkCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmHouseMarkCharacter();

    UFUNCTION(BlueprintCallable, Category="Cogspire|House Mark")
    bool ExamineVictimEvidence(AWyrmCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="Cogspire|House Mark")
    bool ExamineRelicTradeEvidence(AWyrmCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="Cogspire|House Mark")
    bool IdentifyHouseMark(AWyrmCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="Cogspire|House Mark")
    bool BeginConfrontation(AWyrmCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="Cogspire|House Mark")
    bool ManifestDeathmark(AWyrmCharacter* Player);

    UFUNCTION(BlueprintPure, Category="Cogspire|House Mark")
    USkeletalMeshComponent* GetPresentationMesh() const { return PresentationMesh; }

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void HandleHouseMarkDefeated(AWyrmEnemyCharacter* DefeatedEnemy);

    UPROPERTY(VisibleAnywhere, Category="Cogspire|House Mark")
    TObjectPtr<USkeletalMeshComponent> PresentationMesh;

    UPROPERTY(VisibleInstanceOnly, Category="Cogspire|House Mark")
    bool bConfrontationActive = false;
};
