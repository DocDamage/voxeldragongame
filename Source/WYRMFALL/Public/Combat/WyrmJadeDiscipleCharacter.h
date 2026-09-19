#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmJadeDiscipleCharacter.generated.h"

class AWyrmCharacter;

/** Bounded Jade Peaks disciple encounter with equal trust/combat capability rewards. */
UCLASS()
class WYRMFALL_API AWyrmJadeDiscipleCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmJadeDiscipleCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jade Peaks|Disciple")
    bool bEncounterResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jade Peaks|Disciple")
    bool bTrustedResolution = false;

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Disciple")
    bool ResolveWithTrust(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Disciple")
    bool ResolveAfterLivingDefeat(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Disciple")
    void ResetResolution();

protected:
    virtual void BeginPlay() override;

private:
    bool ResolveInternal(AWyrmCharacter* PlayerCharacter, bool bTrust);
};
