// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WyrmMutableRecipeBuilder.generated.h"

UCLASS()
class WYRMFALL_API UWyrmMutableRecipeBuilder : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	/**
	 * Builds and compiles the authoritative CO_Knight recipe asset from real knight voxel meshes.
	 * Configures:
	 *  - Root Object CO_Knight
	 *  - Skeletal Mesh Component 'Body' with ReferenceSkeletalMesh SK_Knight
	 *  - Mesh switch for Helmet (options Up / Down)
	 *  - Color parameter ArmorTint (default White)
	 *  - Synchronous compilation via Mutable engine
	 *
	 * @param OutMessage Status or error diagnostics output.
	 * @return True if compilation succeeded and asset was saved.
	 */
	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Mutable")
	static bool BuildKnightRecipe(FString& OutMessage);
#endif
};
