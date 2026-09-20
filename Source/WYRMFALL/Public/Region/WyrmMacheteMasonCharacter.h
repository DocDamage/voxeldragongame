#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmMacheteMasonCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmMacheteMasonResolution : uint8
{
    Unresolved,
    DisarmedSubmission
};

/** Bounded required-horror encounter using the supplied rigid modular Machete Mason assembly. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmMacheteMasonCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmMacheteMasonCharacter();

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|MacheteMason")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|MacheteMason")
    EWyrmMacheteMasonResolution Resolution = EWyrmMacheteMasonResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|MacheteMason")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|MacheteMason")
    bool ResolveAfterDisarmedSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|MacheteMason")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|MacheteMason")
    int32 GetPresentationPartCount() const { return PresentationParts.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|MacheteMason")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|MacheteMason")
    bool HasCompletePresentation() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|MacheteMason")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|MacheteMason")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};
