#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmMichaelMireCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmMichaelMireResolution : uint8
{
    Unresolved,
    LivingSubmission
};

/** Bounded required-horror encounter using the supplied rigid modular Michael Mire assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmMichaelMireCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmMichaelMireCharacter();

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|MichaelMire")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|MichaelMire")
    EWyrmMichaelMireResolution Resolution = EWyrmMichaelMireResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|MichaelMire")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|MichaelMire")
    bool ResolveAfterLivingSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|MichaelMire")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|MichaelMire")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|MichaelMire")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|MichaelMire")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|MichaelMire")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|MichaelMire")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
