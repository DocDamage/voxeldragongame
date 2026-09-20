#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmWherewolfCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmWherewolfResolution : uint8
{
    Unresolved,
    CalmedSubmission
};

/** Bounded required-horror encounter using the supplied rigid modular Wherewolf assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmWherewolfCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmWherewolfCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Wherewolf")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Wherewolf")
    EWyrmWherewolfResolution Resolution = EWyrmWherewolfResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|Wherewolf")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Wherewolf")
    bool ResolveAfterCalmedSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Wherewolf")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|Wherewolf")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|Wherewolf")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|Wherewolf")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|Wherewolf")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|Wherewolf")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
