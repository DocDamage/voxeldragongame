#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmFrankNShrineCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmFrankNShrineResolution : uint8
{
    Unresolved,
    GroundedSubmission
};

/** Bounded required-horror encounter using the supplied rigid modular Frank N. Shrine assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmFrankNShrineCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmFrankNShrineCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|FrankNShrine")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|FrankNShrine")
    EWyrmFrankNShrineResolution Resolution = EWyrmFrankNShrineResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|FrankNShrine")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|FrankNShrine")
    bool ResolveAfterGroundedSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|FrankNShrine")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|FrankNShrine")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|FrankNShrine")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|FrankNShrine")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|FrankNShrine")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|FrankNShrine")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
