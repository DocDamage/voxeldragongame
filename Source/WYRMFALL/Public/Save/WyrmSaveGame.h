#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "Building/WyrmBuildingTypes.h"
#include "Player/WyrmControlTypes.h"
#include "Dragon/WyrmDragonTypes.h"
#include "Region/WyrmRegion01Types.h"
#include "Region/WyrmJadePeaksTypes.h"
#include "Region/WyrmGloamingTypes.h"
#include "Region/WyrmCogspireTypes.h"
#include "Vehicles/WyrmVehicleTypes.h"
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
    float CurrentXP = 0.f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector CharacterScale = FVector::OneVector;

    // --- Active Food Preparation Buff (ACT-04) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bHasActiveBuff = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName ActiveBuffId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FText ActiveBuffName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float ActiveBuffRemainingDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float ActiveBuffTotalDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float ActiveBuffMaxFocusPercentBonus = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float ActiveBuffHealthRegenPerSecond = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float ActiveBuffPowerBonus = 0.f;

    // --- Horror Echo State (WP-15) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<FName> LearnedEchoes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName EquippedEcho = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bRelentlessAdvanceActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float RelentlessAdvanceRemainingDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float RelentlessAdvanceRemainingCooldown = 0.f;

    // --- Moonbound Beast State (WP-21 / SAVE-11) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bMoonboundActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float MoonboundRemainingDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float MoonboundRemainingCooldown = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bMoonboundReturnPending = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FVector LastSafeHumanoidLocation = FVector::ZeroVector;

    // --- Jade Peaks Echo State (WP-22 / JP-05) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float MirrorStepRemainingCooldown = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float UnseenHandRemainingCooldown = 0.f;

    // --- Verdant Reach Echo State (WP-23.1) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bHuntersVeilActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float HuntersVeilRemainingDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float HuntersVeilRemainingCooldown = 0.f;

    // --- Gloaming Marches Echo State (WP-23.5 / Schema 7) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float SanguineStrikeRemainingCooldown = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    float SecondTurnRemainingCooldown = 0.f;
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

/** Schema 5+ region-keyed mutable-world payload. Legacy singular fields remain readable. */
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmRegionalWorldSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName RegionId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmTerrainSaveRecord TerrainRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmCampSaveRecord CampRecord;
};

/** Pending/last completed route only. UWyrmSaveSubsystem remains the slot owner. */
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmWorldTravelSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName CurrentRegionId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName ArrivalLandmarkId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName ReturnRegionId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName ReturnLandmarkId = NAME_None;
};

// Authoritative single save container for WYRMFALL.
UCLASS(BlueprintType)
class WYRMFALL_API UWyrmSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UWyrmSaveGame();

    // Schema 8 adds Cogspire facts and selective-engine state while retaining
    // Schemas 1-7 and the region-keyed Schema 5 world payload.
    static const int32 MinimumSupportedSchemaVersion = 1;
    static const int32 CurrentSchemaVersion = 8;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmCampSaveRecord CampRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<FWyrmRegionalWorldSaveRecord> RegionalWorldRecords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmWorldTravelSaveRecord WorldTravelRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmDragonSaveRecord DragonRecord;

    /** Schema 4 multi-dragon authority. DragonRecord remains the legacy first entry. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<FWyrmDragonSaveRecord> DragonRecords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmRegion01SaveRecord Region01Record;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmJadePeaksSaveRecord JadePeaksRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmGloamingSaveRecord GloamingRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmCogspireSaveRecord CogspireRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FWyrmHovercarSaveRecord HovercarRecord;
};
