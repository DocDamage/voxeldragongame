#include "Save/WyrmSaveSubsystem.h"
#include "Save/WyrmSaveGame.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "Terrain/WyrmGeoForgeAdapter.h"
#include "Building/WyrmBuildingSubsystem.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "Region/WyrmJadePeaksSubsystem.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Region/WyrmCogspireSubsystem.h"
#include "Region/WyrmWorldTravelSubsystem.h"
#include "Vehicles/WyrmHovercar.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectIterator.h"

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
    if (SaveSnapshotToSlot(SlotName, Character, TerrainProviderActor, GetWorld()))
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

    if (LoadSnapshotFromSlot(SlotName, Character, TerrainProviderActor, GetWorld()))
    {
        if (UWyrmSaveGame* Loaded = Cast<UWyrmSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
        {
            LastLoadedGenerationId = Loaded->SaveGenerationId;
        }
        return true;
    }
    return false;
}

bool UWyrmSaveSubsystem::SaveSnapshotToSlot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor, UWorld* WorldContext)
{
    if (SlotName.IsEmpty())
    {
        return false;
    }

    UWyrmSaveGame* SaveObj = CreateSnapshotObject(SlotName, Character, TerrainProviderActor, WorldContext);
    if (!SaveObj)
    {
        return false;
    }

    return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, 0);
}

bool UWyrmSaveSubsystem::LoadSnapshotFromSlot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor, UWorld* WorldContext)
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

    return ApplySnapshotObject(SaveObj, Character, TerrainProviderActor, WorldContext);
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

bool UWyrmSaveSubsystem::IsSchemaVersionSupported(int32 SchemaVersion)
{
    return SchemaVersion >= UWyrmSaveGame::MinimumSupportedSchemaVersion &&
           SchemaVersion <= UWyrmSaveGame::CurrentSchemaVersion;
}

UWyrmSaveGame* UWyrmSaveSubsystem::CreateSnapshotObject(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor, UWorld* WorldContext)
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

    // Carry forward other regions when updating an existing Schema 5+ slot.
    if (!SlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        if (const UWyrmSaveGame* Previous = Cast<UWyrmSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
        {
            if (Previous->SchemaVersion >= 5) SaveObj->RegionalWorldRecords = Previous->RegionalWorldRecords;
        }
    }

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
        SaveObj->CharacterRecord.CharacterScale = Character->GetCharacterScale();

        // Save active food buff (ACT-04)
        SaveObj->CharacterRecord.bHasActiveBuff = ActiveBuff.IsActive();
        SaveObj->CharacterRecord.ActiveBuffId = ActiveBuff.BuffId;
        SaveObj->CharacterRecord.ActiveBuffName = ActiveBuff.BuffName;
        SaveObj->CharacterRecord.ActiveBuffRemainingDuration = ActiveBuff.RemainingDuration;
        SaveObj->CharacterRecord.ActiveBuffTotalDuration = ActiveBuff.TotalDuration;
        SaveObj->CharacterRecord.ActiveBuffMaxFocusPercentBonus = ActiveBuff.MaxFocusPercentBonus;
        SaveObj->CharacterRecord.ActiveBuffHealthRegenPerSecond = ActiveBuff.HealthRegenPerSecond;
        SaveObj->CharacterRecord.ActiveBuffPowerBonus = ActiveBuff.PowerBonus;

        // Save Echo state (WP-15)
        SaveObj->CharacterRecord.LearnedEchoes = Character->GetLearnedEchoes();
        SaveObj->CharacterRecord.EquippedEcho = Character->GetEquippedEcho();
        SaveObj->CharacterRecord.bRelentlessAdvanceActive = Character->IsRelentlessAdvanceActive();
        SaveObj->CharacterRecord.RelentlessAdvanceRemainingDuration = Character->GetRelentlessAdvanceRemainingDuration();
        SaveObj->CharacterRecord.RelentlessAdvanceRemainingCooldown = Character->GetRelentlessAdvanceRemainingCooldown();

        // Save Moonbound Beast state (WP-21 / SAVE-11)
        SaveObj->CharacterRecord.bMoonboundActive = Character->IsMoonboundActive();
        SaveObj->CharacterRecord.MoonboundRemainingDuration = Character->GetMoonboundRemainingDuration();
        SaveObj->CharacterRecord.MoonboundRemainingCooldown = Character->GetMoonboundRemainingCooldown();
        SaveObj->CharacterRecord.bMoonboundReturnPending = Character->IsMoonboundReturnPending();
        SaveObj->CharacterRecord.LastSafeHumanoidLocation = Character->GetLastSafeHumanoidLocation();
        SaveObj->CharacterRecord.MirrorStepRemainingCooldown = Character->GetMirrorStepRemainingCooldown();
        SaveObj->CharacterRecord.UnseenHandRemainingCooldown = Character->GetUnseenHandRemainingCooldown();
        SaveObj->CharacterRecord.bHuntersVeilActive = Character->IsHuntersVeilActive();
        SaveObj->CharacterRecord.HuntersVeilRemainingDuration = Character->GetHuntersVeilRemainingDuration();
        SaveObj->CharacterRecord.HuntersVeilRemainingCooldown = Character->GetHuntersVeilRemainingCooldown();
        SaveObj->CharacterRecord.SanguineStrikeRemainingCooldown = Character->GetSanguineStrikeRemainingCooldown();
        SaveObj->CharacterRecord.SecondTurnRemainingCooldown = Character->GetSecondTurnRemainingCooldown();

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

    // Save placed camp pieces (ACT-10)
    if (!WorldContext)
    {
        if (Character) { WorldContext = Character->GetWorld(); }
        else if (TerrainProviderActor) { WorldContext = TerrainProviderActor->GetWorld(); }
    }

    UWyrmBuildingSubsystem* BuildingSub = nullptr;
    if (WorldContext)
    {
        if (UGameInstance* GI = WorldContext->GetGameInstance())
        {
            BuildingSub = GI->GetSubsystem<UWyrmBuildingSubsystem>();
            if (BuildingSub && BuildingSub->GetActivePieces().Num() == 0)
            {
                for (TObjectIterator<UWyrmBuildingSubsystem> It; It; ++It)
                {
                    if (!It->IsTemplate() && *It != BuildingSub)
                    {
                        if (It->GetActivePieces().Num() > 0 && (!WorldContext || It->GetWorldContext() == WorldContext))
                        {
                            BuildingSub = *It;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!BuildingSub)
    {
        for (TObjectIterator<UWyrmBuildingSubsystem> It; It; ++It)
        {
            if (!It->IsTemplate())
            {
                if (!WorldContext || It->GetWorldContext() == WorldContext || !BuildingSub)
                {
                    BuildingSub = *It;
                    if (WorldContext && It->GetWorldContext() == WorldContext)
                    {
                        break;
                    }
                }
            }
        }
    }

    if (BuildingSub)
    {
        BuildingSub->BuildSaveRecord(SaveObj->CampRecord);
    }

    const FName CurrentRegionId = UWyrmWorldTravelSubsystem::RegionIdForMapName(
        WorldContext ? WorldContext->GetMapName() : FString());
    if (!CurrentRegionId.IsNone())
    {
        SaveObj->RegionalWorldRecords.RemoveAll([CurrentRegionId](const FWyrmRegionalWorldSaveRecord& Record)
        {
            return Record.RegionId == CurrentRegionId || Record.RegionId.IsNone();
        });
        FWyrmRegionalWorldSaveRecord RegionalRecord;
        RegionalRecord.RegionId = CurrentRegionId;
        RegionalRecord.TerrainRecord = SaveObj->TerrainRecord;
        RegionalRecord.CampRecord = SaveObj->CampRecord;
        SaveObj->RegionalWorldRecords.Add(RegionalRecord);
        SaveObj->RegionalWorldRecords.Sort([](const FWyrmRegionalWorldSaveRecord& A, const FWyrmRegionalWorldSaveRecord& B)
        {
            return A.RegionId.LexicalLess(B.RegionId);
        });
    }

    if (UWyrmWorldTravelSubsystem* Travel = UWyrmWorldTravelSubsystem::GetWorldTravelSubsystem(WorldContext))
    {
        Travel->BuildSaveRecord(SaveObj->WorldTravelRecord);
        if (SaveObj->WorldTravelRecord.CurrentRegionId.IsNone())
            SaveObj->WorldTravelRecord.CurrentRegionId = CurrentRegionId;
    }

    // Region 01 owns facts only; this unified subsystem remains the sole
    // serializer and slot owner for those facts.
    if (UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(WorldContext))
    {
        Region01->BuildSaveRecord(SaveObj->Region01Record);
    }

    if (UWyrmJadePeaksSubsystem* JadePeaks = UWyrmJadePeaksSubsystem::GetJadePeaksSubsystem(WorldContext))
    {
        JadePeaks->BuildSaveRecord(SaveObj->JadePeaksRecord);
    }

    if (UWyrmGloamingSubsystem* Gloaming = UWyrmGloamingSubsystem::GetGloamingSubsystem(WorldContext))
    {
        Gloaming->BuildSaveRecord(SaveObj->GloamingRecord);
    }

    if (UWyrmCogspireSubsystem* Cogspire = UWyrmCogspireSubsystem::GetCogspireSubsystem(WorldContext))
    {
        Cogspire->BuildSaveRecord(SaveObj->CogspireRecord);
    }

    // Capture active Dragon Companion / Boss (DRG-01..04, SAVE-08)
    if (WorldContext)
    {
        for (TActorIterator<AWyrmDragonCharacter> It(WorldContext); It; ++It)
        {
            if (*It && IsValid(*It))
            {
                FWyrmDragonSaveRecord Record;
                (*It)->BuildSaveRecord(Record);
                SaveObj->DragonRecords.Add(Record);
            }
        }
        SaveObj->DragonRecords.Sort([](const FWyrmDragonSaveRecord& A, const FWyrmDragonSaveRecord& B)
        {
            return A.DragonId.LexicalLess(B.DragonId);
        });
        if (!SaveObj->DragonRecords.IsEmpty())
        {
            SaveObj->DragonRecord = SaveObj->DragonRecords[0];
        }

        // Capture active Zenith Hovercar (VEH-07)
        for (TActorIterator<AWyrmHovercar> It(WorldContext); It; ++It)
        {
            if (*It && IsValid(*It))
            {
                (*It)->BuildSaveRecord(SaveObj->HovercarRecord);
                break;
            }
        }
    }

    return SaveObj;
}

bool UWyrmSaveSubsystem::ApplySnapshotObject(const UWyrmSaveGame* SaveObj, AWyrmCharacter* Character, AActor* TerrainProviderActor, UWorld* WorldContext)
{
    if (!SaveObj || !IsSchemaVersionSupported(SaveObj->SchemaVersion))
    {
        return false;
    }

    if (SaveObj->SchemaVersion >= 8)
    {
        const bool bBondLedgerPresent =
            SaveObj->CogspireRecord.KnownFacts.Contains(FName(TEXT("cogspire.cogfang_bonded"))) &&
            SaveObj->CogspireRecord.FactReceipts.Contains(FName(TEXT("cogspire.cogfang.voluntary_bond")));
        int32 BondedCogfangRecords = 0;
        for (const FWyrmDragonSaveRecord& Record : SaveObj->DragonRecords)
        {
            if (Record.DragonId == FName(TEXT("Cogfang")) && Record.bHasBondReceipt &&
                Record.Role == EWyrmDragonRole::AlliedCompanion)
            {
                ++BondedCogfangRecords;
            }
        }
        if (BondedCogfangRecords > 1 || (bBondLedgerPresent && BondedCogfangRecords != 1))
        {
            return false;
        }
    }

    // Restore placed camp pieces (ACT-10)
    if (!WorldContext)
    {
        if (Character) { WorldContext = Character->GetWorld(); }
        else if (TerrainProviderActor) { WorldContext = TerrainProviderActor->GetWorld(); }
    }

    if (SaveObj->SchemaVersion >= 8 && WorldContext &&
        SaveObj->CogspireRecord.KnownFacts.Contains(FName(TEXT("cogspire.cogfang_bonded"))) &&
        SaveObj->CogspireRecord.FactReceipts.Contains(FName(TEXT("cogspire.cogfang.voluntary_bond"))))
    {
        int32 ExistingCogfangCount = 0;
        for (TActorIterator<AWyrmDragonCharacter> It(WorldContext); It; ++It)
        {
            if (*It && IsValid(*It) && (*It)->DragonId == FName(TEXT("Cogfang")))
            {
                ++ExistingCogfangCount;
            }
        }
        if (ExistingCogfangCount > 1)
        {
            return false;
        }
    }

    const FName CurrentRegionId = UWyrmWorldTravelSubsystem::RegionIdForMapName(
        WorldContext ? WorldContext->GetMapName() : FString());
    const FWyrmRegionalWorldSaveRecord* RegionalRecord = SaveObj->SchemaVersion >= 5
        ? SaveObj->RegionalWorldRecords.FindByPredicate([CurrentRegionId](const FWyrmRegionalWorldSaveRecord& Record)
          { return Record.RegionId == CurrentRegionId; })
        : nullptr;
    const FWyrmCampSaveRecord& CampToRestore = RegionalRecord ? RegionalRecord->CampRecord : SaveObj->CampRecord;
    const FWyrmTerrainSaveRecord& TerrainToRestore = RegionalRecord ? RegionalRecord->TerrainRecord : SaveObj->TerrainRecord;

    UWyrmBuildingSubsystem* RestoreBuildingSub = nullptr;
    if (WorldContext)
    {
        if (UGameInstance* GI = WorldContext->GetGameInstance())
        {
            RestoreBuildingSub = GI->GetSubsystem<UWyrmBuildingSubsystem>();
            if (RestoreBuildingSub)
            {
                for (TObjectIterator<UWyrmBuildingSubsystem> It; It; ++It)
                {
                    if (!It->IsTemplate() && *It != RestoreBuildingSub)
                    {
                        if (It->GetWorldContext() == WorldContext)
                        {
                            RestoreBuildingSub = *It;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!RestoreBuildingSub)
    {
        for (TObjectIterator<UWyrmBuildingSubsystem> It; It; ++It)
        {
            if (!It->IsTemplate())
            {
                if (!WorldContext || It->GetWorldContext() == WorldContext || !RestoreBuildingSub)
                {
                    RestoreBuildingSub = *It;
                    if (WorldContext && It->GetWorldContext() == WorldContext)
                    {
                        break;
                    }
                }
            }
        }
    }

    if (RestoreBuildingSub)
    {
        RestoreBuildingSub->RestoreFromSaveRecord(CampToRestore, WorldContext);
    }

    // Validate and restore the terrain owner first so a failed terrain payload
    // cannot leave the remaining owners partially restored.
    if (TerrainProviderActor)
    {
        AWyrmGeoForgeAdapter* Adapter = Cast<AWyrmGeoForgeAdapter>(TerrainProviderActor);
        if (!Adapter || TerrainToRestore.TerrainDeltaPayload.IsEmpty() ||
            !Adapter->ApplySavePayload(TerrainToRestore.TerrainDeltaPayload))
        {
            return false;
        }
        Adapter->ProcessedActionIds = TSet<FGuid>(TerrainToRestore.ProcessedActionIds);
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
        Character->SetCharacterScale(SaveObj->CharacterRecord.CharacterScale);
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

        // Restore Echo state (WP-15)
        Character->RestoreEchoState(
            SaveObj->CharacterRecord.LearnedEchoes,
            SaveObj->CharacterRecord.EquippedEcho,
            SaveObj->CharacterRecord.bRelentlessAdvanceActive,
            SaveObj->CharacterRecord.RelentlessAdvanceRemainingDuration,
            SaveObj->CharacterRecord.RelentlessAdvanceRemainingCooldown);

        // Restore Moonbound Beast state (WP-21 / SAVE-11)
        Character->RestoreMoonboundState(
            SaveObj->CharacterRecord.bMoonboundActive,
            SaveObj->CharacterRecord.MoonboundRemainingDuration,
            SaveObj->CharacterRecord.MoonboundRemainingCooldown,
            SaveObj->CharacterRecord.bMoonboundReturnPending,
            SaveObj->CharacterRecord.LastSafeHumanoidLocation);
        Character->RestoreMirrorStepState(SaveObj->CharacterRecord.MirrorStepRemainingCooldown);
        Character->RestoreUnseenHandState(SaveObj->CharacterRecord.UnseenHandRemainingCooldown);
        Character->RestoreHuntersVeilState(
            SaveObj->CharacterRecord.bHuntersVeilActive,
            SaveObj->CharacterRecord.HuntersVeilRemainingDuration,
            SaveObj->CharacterRecord.HuntersVeilRemainingCooldown);
        Character->RestoreSanguineStrikeState(SaveObj->CharacterRecord.SanguineStrikeRemainingCooldown);
        Character->RestoreSecondTurnState(SaveObj->CharacterRecord.SecondTurnRemainingCooldown);
    }

    // Restore active Dragon Companion / Boss (SAVE-08)
    if (WorldContext)
    {
        TArray<FWyrmDragonSaveRecord> RecordsToRestore = SaveObj->DragonRecords;
        if (RecordsToRestore.IsEmpty() &&
            (SaveObj->DragonRecord.bHasBondReceipt || SaveObj->DragonRecord.Role != EWyrmDragonRole::HostileBoss))
        {
            RecordsToRestore.Add(SaveObj->DragonRecord);
        }

        TMap<FName, AWyrmDragonCharacter*> ExistingDragons;
        for (TActorIterator<AWyrmDragonCharacter> It(WorldContext); It; ++It)
        {
            if (*It && IsValid(*It) && !ExistingDragons.Contains((*It)->DragonId))
            {
                ExistingDragons.Add((*It)->DragonId, *It);
            }
        }

        for (const FWyrmDragonSaveRecord& Record : RecordsToRestore)
        {
            if (!Record.bHasBondReceipt && Record.Role == EWyrmDragonRole::HostileBoss)
            {
                continue;
            }
            AWyrmDragonCharacter* Dragon = ExistingDragons.FindRef(Record.DragonId);
            if (!Dragon)
            {
                FTransform SpawnTransform(Record.WorldRotation, Record.WorldLocation);
                Dragon = AWyrmDragonCharacter::SpawnWyrmDragon(WorldContext, Record.Role, SpawnTransform);
                if (Dragon)
                {
                    Dragon->SetDragonId(Record.DragonId);
                    ExistingDragons.Add(Record.DragonId, Dragon);
                }
            }
            if (Dragon)
            {
                Dragon->RestoreFromSaveRecord(Record, Character);
            }
        }

        // Restore the fact ledger only after the existing terrain, character,
        // inventory, camp, and dragon owners have accepted the snapshot.
        if (UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(WorldContext))
        {
            Region01->RestoreFromSaveRecord(SaveObj->Region01Record);
        }

        if (UWyrmJadePeaksSubsystem* JadePeaks = UWyrmJadePeaksSubsystem::GetJadePeaksSubsystem(WorldContext))
        {
            JadePeaks->RestoreFromSaveRecord(SaveObj->JadePeaksRecord);
        }

        if (UWyrmGloamingSubsystem* Gloaming = UWyrmGloamingSubsystem::GetGloamingSubsystem(WorldContext))
        {
            Gloaming->RestoreFromSaveRecord(SaveObj->GloamingRecord);
        }

        if (UWyrmCogspireSubsystem* Cogspire = UWyrmCogspireSubsystem::GetCogspireSubsystem(WorldContext))
        {
            if (SaveObj->SchemaVersion >= 8)
            {
                Cogspire->RestoreFromSaveRecord(SaveObj->CogspireRecord);
            }
            else
            {
                Cogspire->ResetCogspireState();
            }
        }

        if (UWyrmWorldTravelSubsystem* Travel = UWyrmWorldTravelSubsystem::GetWorldTravelSubsystem(WorldContext))
        {
            Travel->RestoreFromSaveRecord(SaveObj->WorldTravelRecord);
        }

        // Restore active Zenith Hovercar (VEH-07)
        if (SaveObj->HovercarRecord.bHasBeenSpawned)
        {
            AWyrmHovercar* ActiveHovercar = nullptr;
            for (TActorIterator<AWyrmHovercar> It(WorldContext); It; ++It)
            {
                if (*It && IsValid(*It))
                {
                    ActiveHovercar = *It;
                    break;
                }
            }

            if (!ActiveHovercar)
            {
                FTransform SpawnTransform(SaveObj->HovercarRecord.WorldRotation, SaveObj->HovercarRecord.WorldLocation);
                ActiveHovercar = AWyrmHovercar::SpawnWyrmHovercar(WorldContext, SpawnTransform);
            }

            if (ActiveHovercar)
            {
                ActiveHovercar->RestoreFromSaveRecord(SaveObj->HovercarRecord, Character);
            }
        }
    }

    return true;
}
