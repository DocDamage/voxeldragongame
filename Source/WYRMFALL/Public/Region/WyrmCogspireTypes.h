#pragma once

#include "CoreMinimal.h"
#include "WyrmCogspireTypes.generated.h"

/** Stable Cogspire fact and selective-engine state serialized only by UWyrmSaveSubsystem. */
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmCogspireSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire")
    TArray<FName> KnownFacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire")
    TArray<FName> FactReceipts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire")
    bool bCoercionGovernorActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cogspire")
    bool bCivicMachineryOperational = true;
};
