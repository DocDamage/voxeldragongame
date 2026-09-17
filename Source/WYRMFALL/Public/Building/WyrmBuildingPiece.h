#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building/WyrmBuildingTypes.h"
#include "WyrmBuildingPiece.generated.h"

class UStaticMeshComponent;

/**
 * Authoritative base actor for all placed camp building pieces
 * (foundations, walls, doorframes, doors, roofs, ceilings, workpoints).
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Building")
class WYRMFALL_API AWyrmBuildingPiece : public AActor
{
    GENERATED_BODY()

public:
    AWyrmBuildingPiece();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building|Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building|Properties")
    EWyrmBuildingPieceType PieceType = EWyrmBuildingPieceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building|Properties")
    FName PieceId = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building|Properties")
    FGuid PieceGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building|Properties")
    bool bIsOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building|Properties")
    FVector BoundsExtent = FVector(100.f, 100.f, 100.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building|Support")
    float SupportRadius = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building|Support")
    float SupportDepth = 200.f;

    /** Toggles door open/close state and updates visual transform (ACT-06) */
    UFUNCTION(BlueprintCallable, Category="Building")
    virtual void ToggleDoor();

    /** Returns demolition refund costs (ACT-09) */
    UFUNCTION(BlueprintPure, Category="Building")
    virtual TArray<FWyrmBuildingCost> GetDemolitionRefund(const TArray<FWyrmBuildingCost>& OriginalCosts, float RefundPercent = 1.0f) const;

    /** Returns the world space ground support column box beneath this piece (WRLD-09) */
    UFUNCTION(BlueprintPure, Category="Building|Support")
    virtual FBox GetSupportBounds() const;

    /** Returns the world space bounding box of this piece */
    UFUNCTION(BlueprintPure, Category="Building")
    virtual FBox GetPieceBounds() const;
};
