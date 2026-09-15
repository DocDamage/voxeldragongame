#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WyrmDebugHUD.generated.h"
UCLASS()
class WYRMFALL_API AWyrmDebugHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
