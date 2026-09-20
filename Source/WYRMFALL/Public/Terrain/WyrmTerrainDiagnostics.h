#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WyrmTerrainDiagnostics.generated.h"

class AActor;

// Development instrumentation only. Does not select a provider or attest readiness.
UCLASS()
class WYRMFALL_API UWyrmTerrainDiagnostics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    // Call only after collision is ready. Returned count is not nav completion.
    UFUNCTION(BlueprintCallable, Category="WYRMFALL|Development", meta=(DevelopmentOnly))
    static int32 RefreshNavigationDataForActor(AActor* TerrainActor);

    // Avoid Python's class-default-object access on NavigationSystemV1 (Within=World).
    UFUNCTION(BlueprintCallable, Category="WYRMFALL|Development", meta=(DevelopmentOnly))
    static bool ProjectNavigationPoint(AActor* WorldActor, FVector Point, FVector QueryExtent, FVector& Projected);

    // Avoid Python's class-default-object access on NavigationSystemV1 (Within=World).
    UFUNCTION(BlueprintCallable, Category="WYRMFALL|Development", meta=(DevelopmentOnly))
    static int32 FindCompleteNavigationPathPointCount(AActor* WorldActor, FVector Start, FVector End);

    UFUNCTION(BlueprintCallable, Category="WYRMFALL|Development", meta=(DevelopmentOnly))
    static bool IsNavigationBuildPending(AActor* WorldActor);
};
