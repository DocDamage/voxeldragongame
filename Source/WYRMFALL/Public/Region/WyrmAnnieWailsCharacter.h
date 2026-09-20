#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmAnnieWailsCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmAnnieWailsResolution : uint8
{
    Unresolved,
    DisarmedSurrender
};

/** Bounded required-horror encounter using the supplied rigid modular Annie Wails assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmAnnieWailsCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmAnnieWailsCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|AnnieWails")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|AnnieWails")
    EWyrmAnnieWailsResolution Resolution = EWyrmAnnieWailsResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|AnnieWails")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|AnnieWails")
    bool ResolveAfterDisarmedSurrender(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|AnnieWails")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|AnnieWails")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|AnnieWails")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|AnnieWails")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|AnnieWails")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|AnnieWails")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
