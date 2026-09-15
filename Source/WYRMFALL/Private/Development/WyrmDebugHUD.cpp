#include "Development/WyrmDebugHUD.h"
#include "Player/WyrmCharacter.h"
#include "Player/WyrmPlayerController.h"
#include "Combat/WyrmAttributeSet.h"
#include "Engine/Canvas.h"
void AWyrmDebugHUD::DrawHUD()
{
    Super::DrawHUD();
#if !UE_BUILD_SHIPPING
    if (!Canvas) { return; }
    DrawText(TEXT("WYRMFALL | DIAGNOSTIC HOST - NOT THE GAME"), FLinearColor::Yellow, 24.f, 24.f);
    DrawText(TEXT("WASD / left stick: move | mouse / right stick: look | C / R3: camera | Space / A: jump"), FLinearColor::White, 24.f, 46.f);
    DrawText(TEXT("F6: click-move test toggle (requires real NavMesh; bootstrap has none)"), FLinearColor::White, 24.f, 66.f);
    DrawText(TEXT("Mutable assets, terrain, combat, dragons, Echoes, saves and vehicles: NOT INTEGRATED"), FLinearColor::Yellow, 24.f, 86.f);
    if (const auto* Body = Cast<AWyrmCharacter>(GetOwningPawn()))
    {
        const auto* PC = Cast<AWyrmPlayerController>(GetOwningPlayerController());
        DrawText(FString::Printf(TEXT("Camera: %s | GAS Health: %.0f / %.0f | Click test: %s"),
            Body->GetCameraMode() == EWyrmCameraMode::TopDown ? TEXT("Top-down") : TEXT("Third-person"),
            Body->GetAttributes()->GetHealth(), Body->GetAttributes()->GetMaxHealth(),
            PC && PC->IsClickMoveEnabled() ? TEXT("enabled") : TEXT("disabled")), FLinearColor::White, 24.f, 110.f);
    }
#endif
}
