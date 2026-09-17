#pragma once

#include "CoreMinimal.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "WyrmBuildingTypes.generated.h"

UENUM(BlueprintType, meta=(ScriptName="WyrmBuildingPieceTypeEnum"))
enum class EWyrmBuildingPieceType : uint8
{
    None            UMETA(DisplayName="None"),
    Foundation      UMETA(DisplayName="Foundation"),
    Wall            UMETA(DisplayName="Wall"),
    Doorframe       UMETA(DisplayName="Doorframe"),
    Door            UMETA(DisplayName="Door"),
    Roof            UMETA(DisplayName="Roof"),
    Ceiling         UMETA(DisplayName="Ceiling"),
    StorageChest    UMETA(DisplayName="Storage Chest"),
    Workpoint       UMETA(DisplayName="Workpoint")
};

UENUM(BlueprintType, meta=(ScriptName="WyrmPlacementRejectionEnum"))
enum class EWyrmPlacementRejection : uint8
{
    None                    UMETA(DisplayName="None"),
    Unsupported             UMETA(DisplayName="Unsupported"),
    Overlapping             UMETA(DisplayName="Overlapping"),
    Occupied                UMETA(DisplayName="Occupied"),
    Protected               UMETA(DisplayName="Protected"),
    InsufficientMaterials   UMETA(DisplayName="Insufficient Materials"),
    InvalidDefinition       UMETA(DisplayName="Invalid Definition")
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmBuildingCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    int32 Count = 1;

    bool operator==(const FWyrmBuildingCost& Other) const
    {
        return ItemId == Other.ItemId && Count == Other.Count;
    }
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmBuildingPieceDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    FName PieceId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    EWyrmBuildingPieceType PieceType = EWyrmBuildingPieceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    FVector BoundsExtent = FVector(100.f, 100.f, 100.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    bool bRequiresGroundSupport = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    bool bRequiresStructuralSupport = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    TArray<FWyrmBuildingCost> Costs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building")
    TSubclassOf<class AWyrmBuildingPiece> CustomPieceClass;
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmCampPieceSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FGuid PieceGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FName PieceId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    EWyrmBuildingPieceType PieceType = EWyrmBuildingPieceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FTransform Transform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    bool bIsOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<FWyrmItemInstance> StoredItems;
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmCampSaveRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    TArray<FWyrmCampPieceSaveRecord> Pieces;
};
