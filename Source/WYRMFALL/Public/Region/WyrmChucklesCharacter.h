#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmChucklesCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmChucklesResolution : uint8
{
    Unresolved,
    ContainedSubmission
};

/** Bounded required-horror encounter using the supplied rigid modular Chuckles assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmChucklesCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmChucklesCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Chuckles")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Chuckles")
    EWyrmChucklesResolution Resolution = EWyrmChucklesResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|Chuckles")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Chuckles")
    bool ResolveAfterContainedSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Chuckles")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|Chuckles")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|Chuckles")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|Chuckles")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|Chuckles")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|Chuckles")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
