#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WyrmUnseenHandTarget.generated.h"

class UStaticMeshComponent;

/** Explicit authored movable prop target; generic physics bodies remain ineligible. */
UCLASS(BlueprintType, Blueprintable)
class WYRMFALL_API AWyrmUnseenHandTarget : public AActor
{
    GENERATED_BODY()

public:
    AWyrmUnseenHandTarget();

    UFUNCTION(BlueprintPure, Category="Unseen Hand")
    bool IsEligibleForUnseenHand() const { return bUnseenHandEligible && MassClassKg <= 250.f; }

    UFUNCTION(BlueprintCallable, Category="Unseen Hand")
    bool ApplyUnseenHandImpulse(const FVector& Impulse);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unseen Hand")
    bool bUnseenHandEligible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unseen Hand", meta=(ClampMin="1.0", ClampMax="250.0"))
    float MassClassKg = 50.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Unseen Hand")
    TObjectPtr<UStaticMeshComponent> MeshComponent;
};
