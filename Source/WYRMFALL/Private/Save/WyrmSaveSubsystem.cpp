#include "Save/WyrmSaveSubsystem.h"
#include "Save/WyrmSaveGame.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "Terrain/WyrmGeoForgeAdapter.h"
#include "Kismet/GameplayStatics.h"

void UWyrmSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UWyrmSaveSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

bool UWyrmSaveSubsystem::SaveGameSnapshot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    if (SaveSnapshotToSlot(SlotName, Character, TerrainProviderActor))
    {
        // Update generation ID if save succeeded
        if (UWyrmSaveGame* Loaded = Cast<UWyrmSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
        {
            LastSavedGenerationId = Loaded->SaveGenerationId;
        }
        return true;
    }
    return false;
}

bool UWyrmSaveSubsystem::LoadGameSnapshot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    if (SlotName.IsEmpty() || !DoesSaveExist(SlotName))
    {
        return false;
    }

    if (LoadSnapshotFromSlot(SlotName, Character, TerrainProviderActor))
    {
        if (UWyrmSaveGame* Loaded = Cast<UWyrmSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
        {
            LastLoadedGenerationId = Loaded->SaveGenerationId;
        }
        return true;
    }
    return false;
}

bool UWyrmSaveSubsystem::SaveSnapshotToSlot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    if (SlotName.IsEmpty())
    {
        return false;
    }

    UWyrmSaveGame* SaveObj = CreateSnapshotObject(SlotName, Character, TerrainProviderActor);
    if (!SaveObj)
    {
        return false;
    }

    return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, 0);
}

bool UWyrmSaveSubsystem::LoadSnapshotFromSlot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    if (SlotName.IsEmpty() || !DoesSaveExist(SlotName))
    {
        return false;
    }

    UWyrmSaveGame* SaveObj = Cast<UWyrmSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (!SaveObj)
    {
        return false;
    }

    return ApplySnapshotObject(SaveObj, Character, TerrainProviderActor);
}

bool UWyrmSaveSubsystem::DoesSaveExist(const FString& SlotName)
{
    if (SlotName.IsEmpty())
    {
        return false;
    }
    return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

bool UWyrmSaveSubsystem::DeleteSaveSlot(const FString& SlotName)
{
    if (SlotName.IsEmpty())
    {
        return false;
    }
    return UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

UWyrmSaveGame* UWyrmSaveSubsystem::CreateSnapshotObject(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    UWyrmSaveGame* SaveObj = Cast<UWyrmSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UWyrmSaveGame::StaticClass()));
    if (!SaveObj)
    {
        return nullptr;
    }

    SaveObj->SlotName = SlotName;
    SaveObj->Timestamp = FDateTime::UtcNow();
    SaveObj->SaveGenerationId = FGuid::NewGuid();

    if (Character)
    {
        UWyrmInventoryComponent* Inv = Character->GetInventory();
        const FWyrmActiveFoodBuff& ActiveBuff = Character->GetActiveFoodBuff();
        if (UWyrmAttributeSet* Attrs = Character->GetAttributes())
        {
            float PowerBonus = Inv ? Inv->GetEquippedStatBonus(FName(TEXT("Power"))) : 0.f;
            float ArmorBonus = Inv ? Inv->GetEquippedStatBonus(FName(TEXT("Armor"))) : 0.f;
            float HealthBonus = Inv ? Inv->GetEquippedStatBonus(FName(TEXT("MaxHealth"))) : 0.f;
            float FocusScale = (ActiveBuff.IsActive() && ActiveBuff.MaxFocusPercentBonus > 0.f)
                ? (1.f + ActiveBuff.MaxFocusPercentBonus)
                : 1.f;
            const float FoodPowerBonus = ActiveBuff.IsActive() ? ActiveBuff.PowerBonus : 0.f;

            SaveObj->CharacterRecord.MaxHealth = FMath::Max(1.f, Attrs->GetCurrentMaxHealth() - HealthBonus);
            SaveObj->CharacterRecord.Health = FMath::Min(Attrs->GetCurrentHealth(), SaveObj->CharacterRecord.MaxHealth);
            SaveObj->CharacterRecord.MaxFocus = FMath::Max(1.f, Attrs->GetCurrentMaxFocus() / FocusScale);
            SaveObj->CharacterRecord.Focus = Attrs->GetCurrentFocus();
            SaveObj->CharacterRecord.CharacterLevel = Character->GetCharacterLevel();
            SaveObj->CharacterRecord.CurrentXP = Character->GetCurrentXP();
            SaveObj->CharacterRecord.Power = FMath::Max(0.f, Attrs->GetCurrentPower() - PowerBonus - FoodPowerBonus);
            SaveObj->CharacterRecord.Armor = FMath::Max(0.f, Attrs->GetCurrentArmor() - ArmorBonus);
            SaveObj->CharacterRecord.Shield = Attrs->GetCurrentShield();
        }

        SaveObj->CharacterRecord.CameraMode = Character->GetCameraMode();
        SaveObj->CharacterRecord.bMovementLocked = Character->IsMovementLocked();
        SaveObj->CharacterRecord.WorldLocation = Character->GetActorLocation();
        SaveObj->CharacterRecord.WorldRotation = Character->GetActorRotation();
        SaveObj->CharacterRecord.AppearanceDescriptor = Character->CaptureAppearanceDescriptor();

        // Save active food buff (ACT-04)
        SaveObj->CharacterRecord.bHasActiveBuff = ActiveBuff.IsActive();
        SaveObj->CharacterRecord.ActiveBuffId = ActiveBuff.BuffId;
        SaveObj->CharacterRecord.ActiveBuffName = ActiveBuff.BuffName;
        SaveObj->CharacterRecord.ActiveBuffRemainingDuration = ActiveBuff.RemainingDuration;
        SaveObj->CharacterRecord.ActiveBuffTotalDuration = ActiveBuff.TotalDuration;
        SaveObj->CharacterRecord.ActiveBuffMaxFocusPercentBonus = ActiveBuff.MaxFocusPercentBonus;
        SaveObj->CharacterRecord.ActiveBuffHealthRegenPerSecond = ActiveBuff.HealthRegenPerSecond;
        SaveObj->CharacterRecord.ActiveBuffPowerBonus = ActiveBuff.PowerBonus;

        if (Inv)
        {
            SaveObj->InventoryRecord.MaxBagSlots = Inv->MaxBagSlots;
            SaveObj->InventoryRecord.MaxStashSlots = Inv->MaxStashSlots;
            SaveObj->InventoryRecord.BagItems = Inv->GetBagItems();
            SaveObj->InventoryRecord.StashItems = Inv->GetStashItems();
            SaveObj->InventoryRecord.EquippedItems = Inv->GetEquippedItems();
        }
    }

    if (TerrainProviderActor)
    {
        AWyrmGeoForgeAdapter* Adapter = Cast<AWyrmGeoForgeAdapter>(TerrainProviderActor);
        if (!Adapter || !Adapter->BuildSavePayload(SaveObj->TerrainRecord.TerrainDeltaPayload))
        {
            return nullptr;
        }
        SaveObj->TerrainRecord.ProcessedActionIds = Adapter->ProcessedActionIds.Array();
    }

    return SaveObj;
}

bool UWyrmSaveSubsystem::ApplySnapshotObject(const UWyrmSaveGame* SaveObj, AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    if (!SaveObj || SaveObj->SchemaVersion != UWyrmSaveGame::CurrentSchemaVersion)
    {
        return false;
    }

    // Validate and restore the terrain owner first so a failed terrain payload
    // cannot leave the remaining owners partially restored.
    if (TerrainProviderActor)
    {
        AWyrmGeoForgeAdapter* Adapter = Cast<AWyrmGeoForgeAdapter>(TerrainProviderActor);
        if (!Adapter || SaveObj->TerrainRecord.TerrainDeltaPayload.IsEmpty() ||
            !Adapter->ApplySavePayload(SaveObj->TerrainRecord.TerrainDeltaPayload))
        {
            return false;
        }
        Adapter->ProcessedActionIds = TSet<FGuid>(SaveObj->TerrainRecord.ProcessedActionIds);
    }

    if (Character)
    {
        // Remove any live modifier before restoring the snapshot's base attributes.
        Character->ClearFoodBuff();

        if (UWyrmAttributeSet* Attrs = Character->GetAttributes())
        {
            Character->SetCharacterLevel(SaveObj->CharacterRecord.CharacterLevel);
            Character->SetCurrentXP(SaveObj->CharacterRecord.CurrentXP);
            Attrs->SetCurrentMaxHealth(SaveObj->CharacterRecord.MaxHealth);
            Attrs->SetCurrentHealth(SaveObj->CharacterRecord.Health);
            Attrs->SetCurrentMaxFocus(SaveObj->CharacterRecord.MaxFocus);
            Attrs->SetCurrentPower(SaveObj->CharacterRecord.Power);
            Attrs->SetCurrentArmor(SaveObj->CharacterRecord.Armor);
            Attrs->SetCurrentShield(SaveObj->CharacterRecord.Shield);
        }

        Character->SetCameraMode(SaveObj->CharacterRecord.CameraMode);
        Character->SetMovementLocked(SaveObj->CharacterRecord.bMovementLocked);
        Character->SetActorLocationAndRotation(SaveObj->CharacterRecord.WorldLocation, SaveObj->CharacterRecord.WorldRotation);
        if (!SaveObj->CharacterRecord.AppearanceDescriptor.IsEmpty())
        {
            Character->RestoreAppearanceDescriptor(SaveObj->CharacterRecord.AppearanceDescriptor);
        }

        if (UWyrmInventoryComponent* Inv = Character->GetInventory())
        {
            Inv->MaxBagSlots = SaveObj->InventoryRecord.MaxBagSlots;
            Inv->MaxStashSlots = SaveObj->InventoryRecord.MaxStashSlots;
            Inv->RestoreInventoryState(SaveObj->InventoryRecord.BagItems, SaveObj->InventoryRecord.StashItems, SaveObj->InventoryRecord.EquippedItems);
        }

        // Restore active food buff (ACT-04)
        if (SaveObj->CharacterRecord.bHasActiveBuff &&
            !SaveObj->CharacterRecord.ActiveBuffId.IsNone() &&
            SaveObj->CharacterRecord.ActiveBuffRemainingDuration > 0.f)
        {
            FWyrmActiveFoodBuff RestoredBuff;
            RestoredBuff.BuffId = SaveObj->CharacterRecord.ActiveBuffId;
            RestoredBuff.BuffName = SaveObj->CharacterRecord.ActiveBuffName;
            RestoredBuff.RemainingDuration = SaveObj->CharacterRecord.ActiveBuffRemainingDuration;
            RestoredBuff.TotalDuration = SaveObj->CharacterRecord.ActiveBuffTotalDuration;
            RestoredBuff.MaxFocusPercentBonus = SaveObj->CharacterRecord.ActiveBuffMaxFocusPercentBonus;
            RestoredBuff.HealthRegenPerSecond = SaveObj->CharacterRecord.ActiveBuffHealthRegenPerSecond;
            RestoredBuff.PowerBonus = SaveObj->CharacterRecord.ActiveBuffPowerBonus;
            Character->ApplyFoodBuff(RestoredBuff);
            // Re-apply remaining duration exactly
            Character->SetActiveFoodBuffRemainingDuration(SaveObj->CharacterRecord.ActiveBuffRemainingDuration);
        }

        // Restore Focus only after its base and active-buff maximum are in place.
        if (UWyrmAttributeSet* Attrs = Character->GetAttributes())
        {
            Attrs->SetCurrentFocus(SaveObj->CharacterRecord.Focus);
        }
    }

    return true;
}
