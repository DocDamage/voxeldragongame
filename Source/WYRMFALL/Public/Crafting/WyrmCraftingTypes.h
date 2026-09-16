#pragma once

#include "CoreMinimal.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "WyrmCraftingTypes.generated.h"

UENUM(BlueprintType, meta=(ScriptName="WyrmCraftingStationType"))
enum class EWyrmCraftingStationType : uint8
{
    Field       UMETA(DisplayName="Field (No Station)"),
    Campfire    UMETA(DisplayName="Campfire"),
    CookingPot  UMETA(DisplayName="Cooking Pot"),
    Workbench   UMETA(DisplayName="Workbench")
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmIngredientCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    int32 Quantity = 1;

    bool operator==(const FWyrmIngredientCost& Other) const
    {
        return ItemId == Other.ItemId && Quantity == Other.Quantity;
    }
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmFoodBuffDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    FName BuffId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    FText BuffName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float Duration = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float MaxFocusPercentBonus = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float HealthRegenPerSecond = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float PowerBonus = 0.f;

    bool IsValid() const
    {
        return !BuffId.IsNone() && Duration > 0.f;
    }
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    FName RecipeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    EWyrmCraftingStationType RequiredStation = EWyrmCraftingStationType::Field;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    TArray<FWyrmIngredientCost> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    FWyrmItemInstance OutputItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
    FWyrmFoodBuffDefinition BuffDefinition;

    bool IsValid() const
    {
        return !RecipeId.IsNone() && Ingredients.Num() > 0 && OutputItem.IsValid();
    }
};
