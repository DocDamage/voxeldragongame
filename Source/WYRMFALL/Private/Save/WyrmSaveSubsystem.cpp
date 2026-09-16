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
        if (UWyrmAttributeSet* Attrs = Character->GetAttributes())
        {
            float PowerBonus = Inv ? Inv->GetEquippedStatBonus(FName(TEXT("Power"))) : 0.f;
            float ArmorBonus = Inv ? Inv->GetEquippedStatBonus(FName(TEXT("Armor"))) : 0.f;
            float HealthBonus = Inv ? Inv->GetEquippedStatBonus(FName(TEXT("MaxHealth"))) : 0.f;

            SaveObj->CharacterRecord.MaxHealth = FMath::Max(1.f, Attrs->GetCurrentMaxHealth() - HealthBonus);
            SaveObj->CharacterRecord.Health = FMath::Min(Attrs->GetCurrentHealth(), SaveObj->CharacterRecord.MaxHealth);
            SaveObj->CharacterRecord.Focus = Attrs->GetCurrentFocus();
            SaveObj->CharacterRecord.MaxFocus = Attrs->GetCurrentMaxFocus();
            SaveObj->CharacterRecord.CharacterLevel = Attrs->GetCharacterLevel();
            SaveObj->CharacterRecord.Power = FMath::Max(0.f, Attrs->GetCurrentPower() - PowerBonus);
            SaveObj->CharacterRecord.Armor = FMath::Max(0.f, Attrs->GetCurrentArmor() - ArmorBonus);
            SaveObj->CharacterRecord.Shield = Attrs->GetCurrentShield();
        }

        SaveObj->CharacterRecord.CameraMode = Character->GetCameraMode();
        SaveObj->CharacterRecord.bMovementLocked = Character->IsMovementLocked();
        SaveObj->CharacterRecord.WorldLocation = Character->GetActorLocation();
        SaveObj->CharacterRecord.WorldRotation = Character->GetActorRotation();
        SaveObj->CharacterRecord.AppearanceDescriptor = Character->CaptureAppearanceDescriptor();

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
        if (AWyrmGeoForgeAdapter* Adapter = Cast<AWyrmGeoForgeAdapter>(TerrainProviderActor))
        {
            Adapter->BuildSavePayload(SaveObj->TerrainRecord.TerrainDeltaPayload);
            SaveObj->TerrainRecord.ProcessedActionIds = Adapter->ProcessedActionIds.Array();
        }
    }

    return SaveObj;
}

bool UWyrmSaveSubsystem::ApplySnapshotObject(const UWyrmSaveGame* SaveObj, AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    if (!SaveObj)
    {
        return false;
    }

    if (Character)
    {
        if (UWyrmAttributeSet* Attrs = Character->GetAttributes())
        {
            Attrs->SetCurrentMaxHealth(SaveObj->CharacterRecord.MaxHealth);
            Attrs->SetCurrentHealth(SaveObj->CharacterRecord.Health);
            Attrs->SetCurrentFocus(SaveObj->CharacterRecord.Focus);
            Attrs->SetCurrentPower(SaveObj->CharacterRecord.Power);
            Attrs->SetCurrentArmor(SaveObj->CharacterRecord.Armor);
            Attrs->SetCurrentShield(SaveObj->CharacterRecord.Shield);
        }

        Character->SetCameraMode(SaveObj->CharacterRecord.CameraMode);
        Character->SetMovementLocked(SaveObj->CharacterRecord.bMovementLocked);
        if (!SaveObj->CharacterRecord.WorldLocation.IsZero())
        {
            Character->SetActorLocationAndRotation(SaveObj->CharacterRecord.WorldLocation, SaveObj->CharacterRecord.WorldRotation);
        }
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
    }

    if (TerrainProviderActor)
    {
        if (AWyrmGeoForgeAdapter* Adapter = Cast<AWyrmGeoForgeAdapter>(TerrainProviderActor))
        {
            if (SaveObj->TerrainRecord.TerrainDeltaPayload.Num() > 0)
            {
                Adapter->ApplySavePayload(SaveObj->TerrainRecord.TerrainDeltaPayload);
            }
            Adapter->ProcessedActionIds = TSet<FGuid>(SaveObj->TerrainRecord.ProcessedActionIds);
        }
    }

    return true;
}
