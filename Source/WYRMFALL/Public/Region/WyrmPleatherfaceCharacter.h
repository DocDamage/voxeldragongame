#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmPleatherfaceCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmPleatherfaceResolution : uint8
{
    Unresolved,
    DisarmedSubmission
};

/** Bounded required-horror encounter using the supplied rigid modular Pleatherface assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmPleatherfaceCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmPleatherfaceCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Pleatherface")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Pleatherface")
    EWyrmPleatherfaceResolution Resolution = EWyrmPleatherfaceResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|Pleatherface")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Pleatherface")
    bool ResolveAfterDisarmedSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Pleatherface")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|Pleatherface")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|Pleatherface")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|Pleatherface")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|Pleatherface")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|Pleatherface")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
