#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Crafting/WyrmCraftingTypes.h"
#include "WyrmCraftingSubsystem.generated.h"

class UWyrmInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyrmOnRecipeCrafted, FName, RecipeId, const FWyrmItemInstance&, ResultItem);

/**
 * Authoritative crafting and cooking subsystem (ACT-02, ACT-05).
 * Validates stations, ingredients, and inventory capacity before committing atomic transactions.
 */
UCLASS()
class WYRMFALL_API UWyrmCraftingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UWyrmCraftingSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UPROPERTY(BlueprintAssignable, Category="Crafting|Events")
    FWyrmOnRecipeCrafted OnRecipeCrafted;

    UFUNCTION(BlueprintCallable, Category="Crafting")
    void RegisterRecipe(const FWyrmRecipe& Recipe);

    UFUNCTION(BlueprintPure, Category="Crafting")
    bool GetRecipe(FName RecipeId, FWyrmRecipe& OutRecipe) const;

    UFUNCTION(BlueprintPure, Category="Crafting")
    TArray<FWyrmRecipe> GetAllRecipes() const;

    /** Validates whether recipe can be crafted without mutating inventory (ACT-05) */
    UFUNCTION(BlueprintPure, Category="Crafting")
    bool CanCraft(UWyrmInventoryComponent* Inventory, FName RecipeId,
                  EWyrmCraftingStationType AvailableStation, FString& OutFailureReason) const;

    /** Atomically crafts a recipe, deducting inputs and granting output item (ACT-02, ACT-05) */
    UFUNCTION(BlueprintCallable, Category="Crafting")
    bool CraftRecipe(UWyrmInventoryComponent* Inventory, FName RecipeId,
                     EWyrmCraftingStationType AvailableStation, FWyrmItemInstance& OutResult,
                     FString& OutFailureReason);

    UFUNCTION(BlueprintCallable, Category="Crafting")
    static FWyrmItemInstance CreateConsumableItem(FName ItemId, const FText& DisplayName,
                                                  int32 StackCount = 1, int32 MaxStack = 10,
                                                  const FWyrmFoodBuffDefinition& BuffDef = FWyrmFoodBuffDefinition());

    UFUNCTION(BlueprintCallable, Category="Crafting")
    void RegisterDefaultRecipes();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crafting|Recipes")
    TMap<FName, FWyrmRecipe> RegisteredRecipes;
};
