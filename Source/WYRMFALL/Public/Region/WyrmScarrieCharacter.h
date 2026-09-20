#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmScarrieCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmScarrieResolution : uint8
{
    Unresolved,
    LivingSubmission
};

/** Bounded required-horror encounter using the supplied rigid modular Scarrie assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmScarrieCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmScarrieCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Scarrie")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Scarrie")
    EWyrmScarrieResolution Resolution = EWyrmScarrieResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|Scarrie")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Scarrie")
    bool ResolveAfterLivingSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Scarrie")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|Scarrie")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|Scarrie")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|Scarrie")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|Scarrie")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|Scarrie")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
