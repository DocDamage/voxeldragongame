#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Save/WyrmSaveGame.h"
#include "WyrmSaveSubsystem.generated.h"

class AWyrmCharacter;
class AWyrmGeoForgeAdapter;

// Subsystem coordinating unified game save and load operations across all project subsystems.
UCLASS()
class WYRMFALL_API UWyrmSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Captures current character, appearance, inventory, and terrain delta into an authoritative save slot (SAVE-01) */
    UFUNCTION(BlueprintCallable, Category="Save")
    bool SaveGameSnapshot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor = nullptr);

    /** Restores character, appearance, inventory, and terrain delta from an authoritative save slot (SAVE-01) */
    UFUNCTION(BlueprintCallable, Category="Save")
    bool LoadGameSnapshot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor = nullptr);

    UFUNCTION(BlueprintPure, Category="Save")
    static bool DoesSaveExist(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category="Save")
    static bool DeleteSaveSlot(const FString& SlotName);

    UFUNCTION(BlueprintPure, Category="Save")
    static bool IsSchemaVersionSupported(int32 SchemaVersion);

    UFUNCTION(BlueprintCallable, Category="Save")
    static bool SaveSnapshotToSlot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor = nullptr, UWorld* WorldContext = nullptr);

    UFUNCTION(BlueprintCallable, Category="Save")
    static bool LoadSnapshotFromSlot(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor = nullptr, UWorld* WorldContext = nullptr);

    // --- Static snapshot helpers for direct memory or testing ---
    UFUNCTION(BlueprintCallable, Category="Save")
    static UWyrmSaveGame* CreateSnapshotObject(const FString& SlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor = nullptr, UWorld* WorldContext = nullptr);

    UFUNCTION(BlueprintCallable, Category="Save")
    static bool ApplySnapshotObject(const UWyrmSaveGame* SaveObj, AWyrmCharacter* Character, AActor* TerrainProviderActor = nullptr, UWorld* WorldContext = nullptr);

private:
    FGuid LastSavedGenerationId;
    FGuid LastLoadedGenerationId;
};
