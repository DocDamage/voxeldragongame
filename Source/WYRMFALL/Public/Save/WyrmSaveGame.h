#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "Player/WyrmControlTypes.h"
#include "WyrmSaveGame.generated.h"

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmCharacterSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float Health = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float Focus = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float MaxFocus = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float CharacterLevel = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float Power = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float Armor = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float Shield = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    EWyrmCameraMode CameraMode = EWyrmCameraMode::ThirdPerson;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bMovementLocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FRotator WorldRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FString AppearanceDescriptor;
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmInventorySaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    int32 MaxBagSlots = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    int32 MaxStashSlots = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<FWyrmItemInstance> BagItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<FWyrmItemInstance> StashItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TMap<EWyrmEquipSlot, FWyrmItemInstance> EquippedItems;
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmTerrainSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    int32 WorldRevision = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<uint8> TerrainDeltaPayload;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<FGuid> ProcessedActionIds;
};

// Authoritative single save container for WYRMFALL.
UCLASS(BlueprintType)
class WYRMFALL_API UWyrmSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UWyrmSaveGame();

    static const int32 CurrentSchemaVersion = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Save")
    int32 SchemaVersion = CurrentSchemaVersion;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Save")
    FGuid SaveGenerationId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Save")
    FDateTime Timestamp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Save")
    FString SlotName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmCharacterSaveRecord CharacterRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmInventorySaveRecord InventoryRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmTerrainSaveRecord TerrainRecord;
};
