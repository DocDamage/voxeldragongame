#include "Crafting/WyrmCraftingSubsystem.h"
#include "Inventory/WyrmInventoryComponent.h"

namespace
{
bool BuildIngredientRequirements(const FWyrmRecipe& Recipe, TMap<FName, int32>& OutRequirements)
{
    OutRequirements.Reset();
    for (const FWyrmIngredientCost& Ingredient : Recipe.Ingredients)
    {
        if (Ingredient.ItemId.IsNone() || Ingredient.Quantity <= 0)
        {
            return false;
        }
        OutRequirements.FindOrAdd(Ingredient.ItemId) += Ingredient.Quantity;
    }
    return !OutRequirements.IsEmpty();
}

bool SimulateIngredientRemoval(
    const TArray<FWyrmItemInstance>& SourceBag,
    const TMap<FName, int32>& Requirements,
    TArray<FWyrmItemInstance>& OutBag,
    FName& OutMissingItem,
    int32& OutNeeded,
    int32& OutAvailable)
{
    OutBag = SourceBag;
    for (const TPair<FName, int32>& Requirement : Requirements)
    {
        int32 Remaining = Requirement.Value;
        int32 Available = 0;
        for (const FWyrmItemInstance& Item : SourceBag)
        {
            if (Item.ItemId == Requirement.Key)
            {
                Available += Item.StackCount;
            }
        }

        if (Available < Requirement.Value)
        {
            OutMissingItem = Requirement.Key;
            OutNeeded = Requirement.Value;
            OutAvailable = Available;
            return false;
        }

        for (int32 Index = OutBag.Num() - 1; Index >= 0 && Remaining > 0; --Index)
        {
            FWyrmItemInstance& Item = OutBag[Index];
            if (Item.ItemId != Requirement.Key)
            {
                continue;
            }

            const int32 Deducted = FMath::Min(Remaining, Item.StackCount);
            Item.StackCount -= Deducted;
            Remaining -= Deducted;
            if (Item.StackCount <= 0)
            {
                OutBag.RemoveAt(Index);
            }
        }
    }
    return true;
}

int64 GetAvailableOutputCapacity(
    const TArray<FWyrmItemInstance>& BagAfterIngredients,
    int32 MaxBagSlots,
    const FWyrmItemInstance& OutputItem)
{
    int64 Capacity = 0;
    for (const FWyrmItemInstance& Item : BagAfterIngredients)
    {
        if (Item.ItemId == OutputItem.ItemId)
        {
            Capacity += FMath::Max(0, Item.MaxStack - Item.StackCount);
        }
    }

    const int32 EmptySlots = FMath::Max(0, MaxBagSlots - BagAfterIngredients.Num());
    Capacity += static_cast<int64>(EmptySlots) * FMath::Max(1, OutputItem.MaxStack);
    return Capacity;
}
}

UWyrmCraftingSubsystem::UWyrmCraftingSubsystem()
{
}

void UWyrmCraftingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultRecipes();
}

void UWyrmCraftingSubsystem::RegisterRecipe(const FWyrmRecipe& Recipe)
{
    if (Recipe.IsValid())
    {
        RegisteredRecipes.Add(Recipe.RecipeId, Recipe);
    }
}

bool UWyrmCraftingSubsystem::GetRecipe(FName RecipeId, FWyrmRecipe& OutRecipe) const
{
    if (const FWyrmRecipe* Found = RegisteredRecipes.Find(RecipeId))
    {
        OutRecipe = *Found;
        return true;
    }
    return false;
}

TArray<FWyrmRecipe> UWyrmCraftingSubsystem::GetAllRecipes() const
{
    TArray<FWyrmRecipe> List;
    RegisteredRecipes.GenerateValueArray(List);
    return List;
}

bool UWyrmCraftingSubsystem::CanCraft(UWyrmInventoryComponent* Inventory, FName RecipeId,
                                     EWyrmCraftingStationType AvailableStation, FString& OutFailureReason) const
{
    if (!Inventory)
    {
        OutFailureReason = TEXT("NoInventory");
        return false;
    }

    const FWyrmRecipe* Recipe = RegisteredRecipes.Find(RecipeId);
    if (!Recipe)
    {
        OutFailureReason = TEXT("UnknownRecipe");
        return false;
    }

    // Station check (ACT-05)
    if (Recipe->RequiredStation != EWyrmCraftingStationType::Field && Recipe->RequiredStation != AvailableStation)
    {
        OutFailureReason = TEXT("InvalidStation");
        return false;
    }

    TMap<FName, int32> Requirements;
    if (!BuildIngredientRequirements(*Recipe, Requirements))
    {
        OutFailureReason = TEXT("InvalidIngredients");
        return false;
    }

    TArray<FWyrmItemInstance> BagAfterIngredients;
    FName MissingItem = NAME_None;
    int32 Needed = 0;
    int32 Available = 0;
    if (!SimulateIngredientRemoval(
            Inventory->GetBagItems(), Requirements, BagAfterIngredients,
            MissingItem, Needed, Available))
    {
        OutFailureReason = FString::Printf(TEXT("MissingIngredients: %s (need %d, have %d)"),
            *MissingItem.ToString(), Needed, Available);
        return false;
    }

    // Check exact capacity after the ingredient transaction has freed its slots.
    if (GetAvailableOutputCapacity(BagAfterIngredients, Inventory->MaxBagSlots, Recipe->OutputItem) <
        Recipe->OutputItem.StackCount)
    {
        OutFailureReason = TEXT("InventoryFull");
        return false;
    }

    return true;
}

bool UWyrmCraftingSubsystem::CraftRecipe(UWyrmInventoryComponent* Inventory, FName RecipeId,
                                        EWyrmCraftingStationType AvailableStation, FWyrmItemInstance& OutResult,
                                        FString& OutFailureReason)
{
    // Atomic validation: zero inputs deducted on failure (ACT-05)
    if (!CanCraft(Inventory, RecipeId, AvailableStation, OutFailureReason))
    {
        return false;
    }

    const FWyrmRecipe* Recipe = RegisteredRecipes.Find(RecipeId);
    if (!Recipe)
    {
        OutFailureReason = TEXT("UnknownRecipe");
        return false;
    }

    const TArray<FWyrmItemInstance> BagSnapshot = Inventory->GetBagItems();
    const TArray<FWyrmItemInstance> StashSnapshot = Inventory->GetStashItems();
    const TMap<EWyrmEquipSlot, FWyrmItemInstance> EquippedSnapshot = Inventory->GetEquippedItems();

    TMap<FName, int32> Requirements;
    if (!BuildIngredientRequirements(*Recipe, Requirements))
    {
        OutFailureReason = TEXT("InvalidIngredients");
        return false;
    }

    // Deduct the preflighted inputs. Any unexpected failure restores all owners' inventory state.
    for (const TPair<FName, int32>& Requirement : Requirements)
    {
        int32 RemainingToDeduct = Requirement.Value;
        const TArray<FWyrmItemInstance> CurrentBag = Inventory->GetBagItems();
        for (const FWyrmItemInstance& Item : CurrentBag)
        {
            if (Item.ItemId == Requirement.Key)
            {
                const int32 DeductAmount = FMath::Min(RemainingToDeduct, Item.StackCount);
                if (!Inventory->RemoveItem(Item.InstanceId, DeductAmount))
                {
                    Inventory->RestoreInventoryState(BagSnapshot, StashSnapshot, EquippedSnapshot);
                    OutFailureReason = TEXT("IngredientRemovalFailed");
                    return false;
                }
                RemainingToDeduct -= DeductAmount;
                if (RemainingToDeduct <= 0)
                {
                    break;
                }
            }
        }

        if (RemainingToDeduct > 0)
        {
            Inventory->RestoreInventoryState(BagSnapshot, StashSnapshot, EquippedSnapshot);
            OutFailureReason = TEXT("IngredientRemovalFailed");
            return false;
        }
    }

    // Commit output item to inventory
    FWyrmItemInstance ItemToGrant = Recipe->OutputItem;
    ItemToGrant.InstanceId = FGuid::NewGuid();

    FWyrmItemInstance Remainder;
    const bool bAddOk = Inventory->AddItem(ItemToGrant, Remainder);
    if (!bAddOk || Remainder.StackCount > 0)
    {
        Inventory->RestoreInventoryState(BagSnapshot, StashSnapshot, EquippedSnapshot);
        OutFailureReason = TEXT("AddItemFailed");
        return false;
    }

    OutResult = ItemToGrant;
    OnRecipeCrafted.Broadcast(RecipeId, OutResult);
    return true;
}

FWyrmItemInstance UWyrmCraftingSubsystem::CreateConsumableItem(FName ItemId, const FText& DisplayName,
                                                               int32 StackCount, int32 MaxStack,
                                                               const FWyrmFoodBuffDefinition& BuffDef)
{
    FWyrmItemInstance Item;
    Item.InstanceId = FGuid::NewGuid();
    Item.ItemId = ItemId;
    Item.DisplayName = DisplayName;
    Item.ItemType = EWyrmItemType::Consumable;
    Item.StackCount = StackCount;
    Item.MaxStack = MaxStack;

    if (BuffDef.IsValid())
    {
        if (BuffDef.MaxFocusPercentBonus > 0.f)
        {
            FWyrmItemRoll Roll;
            Roll.StatName = TEXT("Buff.MaxFocusPercentBonus");
            Roll.Value = BuffDef.MaxFocusPercentBonus;
            Item.RolledStats.Add(Roll);
        }
        if (BuffDef.HealthRegenPerSecond > 0.f)
        {
            FWyrmItemRoll Roll;
            Roll.StatName = TEXT("Buff.HealthRegen");
            Roll.Value = BuffDef.HealthRegenPerSecond;
            Item.RolledStats.Add(Roll);
        }
        if (BuffDef.PowerBonus > 0.f)
        {
            FWyrmItemRoll Roll;
            Roll.StatName = TEXT("Buff.Power");
            Roll.Value = BuffDef.PowerBonus;
            Item.RolledStats.Add(Roll);
        }
        if (BuffDef.Duration > 0.f)
        {
            FWyrmItemRoll Roll;
            Roll.StatName = TEXT("Buff.Duration");
            Roll.Value = BuffDef.Duration;
            Item.RolledStats.Add(Roll);
        }
    }

    return Item;
}

void UWyrmCraftingSubsystem::RegisterDefaultRecipes()
{
    // 1. Recipe.Food.GrilledFish (ACT-02: Cook actual catch -> usable preparation buff)
    {
        FWyrmRecipe Recipe;
        Recipe.RecipeId = TEXT("Recipe.Food.GrilledFish");
        Recipe.DisplayName = FText::FromString(TEXT("Grilled Ocean Fish"));
        Recipe.RequiredStation = EWyrmCraftingStationType::Campfire;

        FWyrmIngredientCost FishCost;
        FishCost.ItemId = TEXT("Item.Fish.OceanFish");
        FishCost.Quantity = 1;
        Recipe.Ingredients.Add(FishCost);

        Recipe.BuffDefinition.BuffId = TEXT("Buff.Food.GrilledFish");
        Recipe.BuffDefinition.BuffName = FText::FromString(TEXT("Grilled Fish Savor (+10% Max Focus)"));
        Recipe.BuffDefinition.Duration = 300.f;
        Recipe.BuffDefinition.MaxFocusPercentBonus = 0.10f; // +10% max Focus (ACT-04)

        Recipe.OutputItem = CreateConsumableItem(
            TEXT("Item.Food.GrilledFish"),
            FText::FromString(TEXT("Grilled Ocean Fish")),
            1, 10, Recipe.BuffDefinition);

        RegisterRecipe(Recipe);
    }

    // 2. Recipe.Food.FishStew (ACT-02: Multi-ingredient recipe)
    {
        FWyrmRecipe Recipe;
        Recipe.RecipeId = TEXT("Recipe.Food.FishStew");
        Recipe.DisplayName = FText::FromString(TEXT("Hearty Fish Stew"));
        Recipe.RequiredStation = EWyrmCraftingStationType::Campfire;

        FWyrmIngredientCost FishCost;
        FishCost.ItemId = TEXT("Item.Fish.OceanFish");
        FishCost.Quantity = 1;
        Recipe.Ingredients.Add(FishCost);

        FWyrmIngredientCost PotatoCost;
        PotatoCost.ItemId = TEXT("Item.Ingredient.Potato");
        PotatoCost.Quantity = 1;
        Recipe.Ingredients.Add(PotatoCost);

        Recipe.BuffDefinition.BuffId = TEXT("Buff.Food.FishStew");
        Recipe.BuffDefinition.BuffName = FText::FromString(TEXT("Fish Stew Vigor (+15% Max Focus, 2 HP/s)"));
        Recipe.BuffDefinition.Duration = 300.f;
        Recipe.BuffDefinition.MaxFocusPercentBonus = 0.15f;
        Recipe.BuffDefinition.HealthRegenPerSecond = 2.0f;

        Recipe.OutputItem = CreateConsumableItem(
            TEXT("Item.Food.FishStew"),
            FText::FromString(TEXT("Hearty Fish Stew")),
            1, 10, Recipe.BuffDefinition);

        RegisterRecipe(Recipe);
    }

    // 3. Recipe.Consumable.FieldRemedy (ACT-05: Field crafting)
    {
        FWyrmRecipe Recipe;
        Recipe.RecipeId = TEXT("Recipe.Consumable.FieldRemedy");
        Recipe.DisplayName = FText::FromString(TEXT("Field Remedy"));
        Recipe.RequiredStation = EWyrmCraftingStationType::Field;

        FWyrmIngredientCost DirtCost;
        DirtCost.ItemId = TEXT("Resource.Dirt");
        DirtCost.Quantity = 2;
        Recipe.Ingredients.Add(DirtCost);

        Recipe.BuffDefinition.BuffId = TEXT("Buff.Remedy.FieldRemedy");
        Recipe.BuffDefinition.BuffName = FText::FromString(TEXT("Field Remedy (Instant Heal)"));
        Recipe.BuffDefinition.Duration = 0.5f;
        Recipe.BuffDefinition.HealthRegenPerSecond = 25.0f;

        Recipe.OutputItem = CreateConsumableItem(
            TEXT("Item.Consumable.FieldRemedy"),
            FText::FromString(TEXT("Field Remedy")),
            1, 10, Recipe.BuffDefinition);

        RegisterRecipe(Recipe);
    }
}
