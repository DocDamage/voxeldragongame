#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "WyrmHollowTwinCharacter.generated.h"

class AWyrmCharacter;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EWyrmHollowTwinIdentity : uint8
{
    Morrow,
    Mourn
};

UENUM(BlueprintType)
enum class EWyrmHollowTwinsResolution : uint8
{
    Unresolved,
    Released,
    LivingSubmission
};

/** One authored half of the bounded Hollow Twins haunting encounter. */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmHollowTwinCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmHollowTwinCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gloaming|HollowTwins")
    EWyrmHollowTwinIdentity TwinIdentity = EWyrmHollowTwinIdentity::Morrow;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Gloaming|HollowTwins")
    TObjectPtr<AWyrmHollowTwinCharacter> Partner;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|HollowTwins")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|HollowTwins")
    EWyrmHollowTwinsResolution Resolution = EWyrmHollowTwinsResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|HollowTwins")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|HollowTwins")
    bool ResolveByRelease(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|HollowTwins")
    bool ResolveAfterLivingSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|HollowTwins")
    bool ManifestSecondTurn(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|HollowTwins")
    void ResetEncounterPair();

    UFUNCTION(BlueprintPure, Category="Gloaming|HollowTwins")
    bool HasDistinctValidPartner() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|HollowTwins")
    USkeletalMeshComponent* GetPresentationMesh() const { return PresentationMesh; }

    UFUNCTION(BlueprintPure, Category="Gloaming|HollowTwins")
    float GetPresentationHeight() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|HollowTwins")
    TObjectPtr<USkeletalMeshComponent> PresentationMesh;

    bool ResolvePair(AWyrmCharacter* PlayerCharacter, bool bReleased);
    bool IsAtLivingSubmissionThreshold() const;
    void SetPairEncounterActive(bool bActive);
    void ApplyResolution(EWyrmHollowTwinsResolution NewResolution);
    void ResetSelf();
};
