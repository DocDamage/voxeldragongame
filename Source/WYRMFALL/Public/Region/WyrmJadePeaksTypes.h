#pragma once

#include "CoreMinimal.h"
#include "WyrmJadePeaksTypes.generated.h"

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmJadePeaksFactReceipt
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    FName ReceiptId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    FName FactId = NAME_None;
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmJadePeaksLandmarkDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    FName LandmarkId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    TArray<FName> ConnectedLandmarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    bool bSupportsTrueForm = false;
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmJadePeaksSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    TArray<FName> KnownFacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    TArray<FWyrmJadePeaksFactReceipt> FactReceipts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jade Peaks")
    TArray<FName> VisitedLandmarks;
};
