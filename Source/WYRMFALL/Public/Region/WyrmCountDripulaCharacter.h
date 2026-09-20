#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmCountDripulaCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmCountDripulaResolution : uint8
{
    Unresolved,
    BloodlessSurrender
};

/** Bounded required-horror encounter using the supplied rigid modular Count Dripula assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmCountDripulaCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmCountDripulaCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|CountDripula")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|CountDripula")
    EWyrmCountDripulaResolution Resolution = EWyrmCountDripulaResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|CountDripula")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|CountDripula")
    bool ResolveAfterBloodlessSurrender(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|CountDripula")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|CountDripula")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|CountDripula")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|CountDripula")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|CountDripula")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|CountDripula")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
