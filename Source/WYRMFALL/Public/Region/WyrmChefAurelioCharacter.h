#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmChefAurelioCharacter.generated.h"

class AWyrmCharacter;
class USkeletalMeshComponent;

/** Supplied-art Chef Aurelio and bounded optional restaurant investigation. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Cogspire")
class WYRMFALL_API AWyrmChefAurelioCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmChefAurelioCharacter();

    UFUNCTION(BlueprintCallable, Category="Cogspire|Chef Aurelio") bool RecordPatronTestimony(AWyrmCharacter* Player);
    UFUNCTION(BlueprintCallable, Category="Cogspire|Chef Aurelio") bool ExamineKitchenEvidence(AWyrmCharacter* Player);
    UFUNCTION(BlueprintCallable, Category="Cogspire|Chef Aurelio") bool TraceIngredientSource(AWyrmCharacter* Player);
    UFUNCTION(BlueprintCallable, Category="Cogspire|Chef Aurelio") bool BeginConfrontation(AWyrmCharacter* Player);
    UFUNCTION(BlueprintCallable, Category="Cogspire|Chef Aurelio") bool ManifestCarversPrecision(AWyrmCharacter* Player);
    UFUNCTION(BlueprintPure, Category="Cogspire|Chef Aurelio") USkeletalMeshComponent* GetPresentationMesh() const { return PresentationMesh; }

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION() void HandleChefDefeated(AWyrmEnemyCharacter* DefeatedEnemy);

    UPROPERTY(VisibleAnywhere, Category="Cogspire|Chef Aurelio") TObjectPtr<USkeletalMeshComponent> PresentationMesh;
    UPROPERTY(VisibleInstanceOnly, Category="Cogspire|Chef Aurelio") bool bConfrontationActive = false;
};
