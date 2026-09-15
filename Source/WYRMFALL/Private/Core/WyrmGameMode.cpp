#include "Core/WyrmGameMode.h"
#include "Player/WyrmCharacter.h"
#include "Player/WyrmPlayerController.h"
#include "Development/WyrmDebugHUD.h"
AWyrmGameMode::AWyrmGameMode()
{
    DefaultPawnClass = AWyrmCharacter::StaticClass();
    PlayerControllerClass = AWyrmPlayerController::StaticClass();
    HUDClass = AWyrmDebugHUD::StaticClass();
}
