#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmCountMalvaineCharacter.generated.h"

class AWyrmCharacter;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EWyrmMalvaineResolution : uint8
{
    Unresolved,
    Parley,
    LivingDefeat
};

/** Bounded Count Malvaine encounter; Echo rewards are intentionally out of scope. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmCountMalvaineCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmCountMalvaineCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Malvaine")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|Malvaine")
    EWyrmMalvaineResolution Resolution = EWyrmMalvaineResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|Malvaine")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Malvaine")
    bool ResolveByParley(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Malvaine")
    bool ResolveAfterLivingDefeat(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Malvaine")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|Malvaine")
    USkeletalMeshComponent* GetPresentationMesh() const { return PresentationMesh; }

    UFUNCTION(BlueprintPure, Category="Gloaming|Malvaine")
    float GetPresentationHeight() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|Malvaine")
    TObjectPtr<USkeletalMeshComponent> PresentationMesh;

    bool ResolveInternal(AWyrmCharacter* PlayerCharacter, bool bParley);
};
