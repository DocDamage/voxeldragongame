#pragma once

#include "CoreMinimal.h"
#include "WyrmGloamingTypes.generated.h"

/** Stable Gloaming fact ledger serialized only by UWyrmSaveSubsystem. */
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmGloamingSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gloaming")
    TArray<FName> KnownFacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gloaming")
    TArray<FName> FactReceipts;
};
