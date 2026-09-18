#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmPracticeTarget.generated.h"

class AWyrmCharacter;
class UCapsuleComponent;

/**
 * Interactive practice target at Silent Landing demonstrating slow and stagger resistance
 * with Relentless Advance. Completely optional; skipping never revokes the power.
 */
UCLASS()
class WYRMFALL_API AWyrmPracticeTarget : public AActor
{
    GENERATED_BODY()

public:
    AWyrmPracticeTarget();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Practice")
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Practice")
    bool bPracticeAttempted = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Practice")
    bool bPracticeCompleted = false;

    /** Triggers a practice slow on target to demonstrate Relentless Advance suppression */
    UFUNCTION(BlueprintCallable, Category="Practice")
    bool TriggerPracticeSlow(AWyrmCharacter* TargetCharacter, float Duration = 5.0f, float Magnitude = 0.5f);

    /** Triggers a practice stagger on target to demonstrate Relentless Advance resistance */
    UFUNCTION(BlueprintCallable, Category="Practice")
    bool TriggerPracticeStagger(AWyrmCharacter* TargetCharacter, float Duration = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Practice")
    void CompletePractice() { bPracticeCompleted = true; }
};
