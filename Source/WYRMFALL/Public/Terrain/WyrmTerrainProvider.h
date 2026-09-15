#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WyrmTerrainProvider.generated.h"

// Project-facing contract only; no terrain engine or success-simulating mock.
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmTerrainCapabilities
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSmoothRemove = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSmoothAdd = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCollisionCompletion = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bNewSurfaceNavigation = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bPersistentEdits = false;
    bool HasMinimumG1APIs() const
    { return bSmoothRemove && bSmoothAdd && bCollisionCompletion && bNewSurfaceNavigation && bPersistentEdits; }
};
UENUM(BlueprintType)
enum class EWyrmTerrainEditOperation : uint8 { Remove, Add };
UENUM(BlueprintType)
enum class EWyrmTerrainSubmitResult : uint8 { Unsupported, Rejected, Queued };
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmTerrainEditRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGuid ActionId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector WorldCenter = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RadiusCm = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EWyrmTerrainEditOperation Operation = EWyrmTerrainEditOperation::Remove;
    bool IsWellFormed() const
    {
        return ActionId.IsValid() && !WorldCenter.ContainsNaN() && FMath::IsFinite(RadiusCm) && RadiusCm > 0.f
            && (Operation == EWyrmTerrainEditOperation::Remove || Operation == EWyrmTerrainEditOperation::Add);
    }
};
UINTERFACE(BlueprintType, Blueprintable)
class WYRMFALL_API UWyrmTerrainProvider : public UInterface { GENERATED_BODY() };
class WYRMFALL_API IWyrmTerrainProvider
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Terrain")
    FWyrmTerrainCapabilities GetTerrainCapabilities() const;
    virtual FWyrmTerrainCapabilities GetTerrainCapabilities_Implementation() const;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Terrain")
    EWyrmTerrainSubmitResult SubmitTerrainEdit(const FWyrmTerrainEditRequest& Request);
    virtual EWyrmTerrainSubmitResult SubmitTerrainEdit_Implementation(const FWyrmTerrainEditRequest& Request);
};
