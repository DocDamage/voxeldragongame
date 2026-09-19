#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmJadeEmperorCharacter.generated.h"

/** Emperor Wei Longzhu: equal diplomacy/living-defeat pact-resolution paths. */
UCLASS()
class WYRMFALL_API AWyrmJadeEmperorCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmJadeEmperorCharacter();
    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Emperor") bool ResolveByDiplomacy();
    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Emperor") bool ResolveAfterLivingDefeat();
    UFUNCTION(BlueprintPure, Category="Jade Peaks|Emperor") bool IsPactResolved() const { return bPactResolved; }

protected:
    virtual void BeginPlay() override;

private:
    void GroundPresentationMesh();
    bool Resolve(bool bDiplomacy);
    UPROPERTY(VisibleInstanceOnly, Category="Jade Peaks|Emperor") bool bPactResolved = false;
};
