#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Region/WyrmRegion01Types.h"
#include "WyrmRegion01Npc.generated.h"

/**
 * Authoritative NPC character in Region 01 representing supplied cast members
 * (Tamsin, Mara, Sella, Pell, Iven, Rusk) with normalized 26-bone mesh,
 * authenticated palette material, validated (0.5, 0.5, 0.5) scale, and
 * direct interaction wiring to UWyrmRegion01Subsystem.
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Region01")
class WYRMFALL_API AWyrmRegion01Npc : public ACharacter
{
    GENERATED_BODY()

public:
    AWyrmRegion01Npc();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01|NPC")
    FName RoleName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01|NPC")
    float InteractionRadius = 250.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Region 01|NPC")
    bool bRescued = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Region 01|NPC")
    FText LastDialogue;

    UFUNCTION(BlueprintCallable, Category="Region 01|NPC")
    bool ConfigureRole(FName InRoleName);

    UFUNCTION(BlueprintCallable, Category="Region 01|NPC")
    bool Interact(AActor* Interactor);

    UFUNCTION(BlueprintPure, Category="Region 01|NPC")
    bool CanInteract(const AActor* Interactor) const;

    UFUNCTION(BlueprintCallable, Category="Region 01|NPC")
    void RelocateToSafeStaging(const FVector& StagingLocation);

    UFUNCTION(BlueprintPure, Category="Region 01|NPC")
    FName GetRoleName() const { return RoleName; }

    UFUNCTION(BlueprintPure, Category="Region 01|NPC")
    bool IsRescued() const { return bRescued; }

    UFUNCTION(BlueprintPure, Category="Region 01|NPC")
    FText GetLastDialogue() const { return LastDialogue; }

    static FString GetNormalizedMeshPathForRole(FName InRole);
    static FString GetNormalizedIdleAnimPathForRole(FName InRole);
    static FString GetPaletteMaterialPathForRole(FName InRole);

protected:
    virtual void BeginPlay() override;
};
