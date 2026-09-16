#pragma once

#include "CoreMinimal.h"
#include "WyrmControlTypes.generated.h"

UENUM(BlueprintType)
enum class EWyrmCameraMode : uint8
{
    ThirdPerson UMETA(DisplayName="Third Person"),
    TopDown     UMETA(DisplayName="Top Down")
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmControlState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control")
    EWyrmCameraMode CameraMode = EWyrmCameraMode::ThirdPerson;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control")
    bool bClickMoveEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control")
    bool bInvertY = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control")
    bool bMovementLocked = false;

    FString ToJsonString() const
    {
        return FString::Printf(TEXT("{\"camera_mode\":\"%s\",\"click_move\":%s,\"invert_y\":%s,\"movement_locked\":%s}"),
            CameraMode == EWyrmCameraMode::TopDown ? TEXT("TopDown") : TEXT("ThirdPerson"),
            bClickMoveEnabled ? TEXT("true") : TEXT("false"),
            bInvertY ? TEXT("true") : TEXT("false"),
            bMovementLocked ? TEXT("true") : TEXT("false"));
    }

    static bool FromJsonString(const FString& InJson, FWyrmControlState& OutState)
    {
        if (InJson.IsEmpty()) { return false; }
        OutState.CameraMode = InJson.Contains(TEXT("\"camera_mode\":\"TopDown\"")) ? EWyrmCameraMode::TopDown : EWyrmCameraMode::ThirdPerson;
        OutState.bClickMoveEnabled = InJson.Contains(TEXT("\"click_move\":true"));
        OutState.bInvertY = InJson.Contains(TEXT("\"invert_y\":true"));
        OutState.bMovementLocked = InJson.Contains(TEXT("\"movement_locked\":true"));
        return true;
    }
};
