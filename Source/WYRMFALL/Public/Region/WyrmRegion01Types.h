#pragma once

#include "CoreMinimal.h"
#include "WyrmRegion01Types.generated.h"

/** The three named Crowncut workers are independent rescue facts, never a quest-step count. */
UENUM(BlueprintType)
enum class EWyrmRegion01Worker : uint8
{
    Pell,
    Iven,
    Sella,
};

/** Evidence sources deliberately overlap so one missed record cannot block the local conflict. */
UENUM(BlueprintType)
enum class EWyrmRegion01Evidence : uint8
{
    Machine,
    Records,
    SellaAccount,
};

/** Rusk remains unresolved until an actual custody branch is recorded. */
UENUM(BlueprintType)
enum class EWyrmRegion01RuskOutcome : uint8
{
    Unresolved,
    DefeatedCustody,
    SurrenderedCustody,
};

/**
 * A named, one-time receipt for a Region 01 fact. Receipt identity, rather
 * than a conversation index, prevents repeat rescue/reward state.
 */
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmRegion01FactReceipt
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    FName ReceiptId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    FName FactId = NAME_None;
};

/**
 * The authored relationship graph. These are logical landmarks; production
 * placement still requires the appropriate real environment/NPC assets.
 */
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmRegion01LandmarkDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    FName LandmarkId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    TArray<FName> ConnectedLandmarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    bool bOptional = false;

    /** True until a placed map instance is backed by audited real content. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    bool bRequiresRealContent = true;
};

/** Unified-save payload for persistent Region 01 facts and landmark visits. */
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmRegion01SaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    TArray<FName> KnownFacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    TArray<FWyrmRegion01FactReceipt> FactReceipts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    TArray<FName> VisitedLandmarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Region 01")
    EWyrmRegion01RuskOutcome RuskOutcome = EWyrmRegion01RuskOutcome::Unresolved;
};
