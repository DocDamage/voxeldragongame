#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmCanopyHunterCharacter.generated.h"

class AWyrmCharacter;

/** Optional hunter encounter with trust and living-defeat parity. */
UCLASS()
class WYRMFALL_API AWyrmCanopyHunterCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmCanopyHunterCharacter();

    UFUNCTION(BlueprintCallable, Category="Verdant Reach")
    bool ResolveByTrust(AWyrmCharacter* Player);

    /** Submits while still alive at or below the authored 25% health threshold. */
    UFUNCTION(BlueprintCallable, Category="Verdant Reach")
    bool ResolveByLivingDefeat(AWyrmCharacter* Player);

private:
    bool GrantHuntersVeil(AWyrmCharacter* Player, bool bTrustRoute);
};
