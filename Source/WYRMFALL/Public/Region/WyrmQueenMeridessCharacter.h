#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WyrmQueenMeridessCharacter.generated.h"

class AWyrmEnemyCharacter;

/** Non-boss regional authority. Resolution targets the crown claim, never Meridess's life. */
UCLASS()
class WYRMFALL_API AWyrmQueenMeridessCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AWyrmQueenMeridessCharacter();

    UFUNCTION(BlueprintCallable, Category="Verdant Reach")
    bool ResolveWithEvidence();

    UFUNCTION(BlueprintCallable, Category="Verdant Reach")
    bool ResolveAfterGuardDefeat(AWyrmEnemyCharacter* DefeatedGuard);
};
