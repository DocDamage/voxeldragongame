#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Region/WyrmRegion01Types.h"
#include "WyrmRegion01Interactable.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmRegion01InteractableType : uint8
{
    CrownNotice,
    AuxiliaryRestraint,
    MachineEvidence,
    RecordsEvidence,
    WageRecord,
    CentralClaim,
    ServiceCache
};

/**
 * In-world interactive objects for Region 01 (Notice boards, controls, evidence inspectables, wage records)
 * that commit verified receipts to UWyrmRegion01Subsystem upon player interaction.
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Region01")
class WYRMFALL_API AWyrmRegion01Interactable : public AActor
{
    GENERATED_BODY()

public:
    AWyrmRegion01Interactable();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Region 01|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Region 01|Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01|Interactable")
    EWyrmRegion01InteractableType InteractableType = EWyrmRegion01InteractableType::CrownNotice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01|Interactable")
    float InteractionRadius = 250.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Region 01|Interactable")
    bool bHasBeenInteracted = false;

    UFUNCTION(BlueprintCallable, Category="Region 01|Interactable")
    bool Interact(AActor* Interactor);

    UFUNCTION(BlueprintPure, Category="Region 01|Interactable")
    bool CanInteract(const AActor* Interactor) const;
};
