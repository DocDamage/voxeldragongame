#include "Save/WyrmSaveGame.h"

UWyrmSaveGame::UWyrmSaveGame()
{
    SchemaVersion = CurrentSchemaVersion;
    SaveGenerationId = FGuid::NewGuid();
    Timestamp = FDateTime::UtcNow();
    SlotName = TEXT("WyrmfallSlot_0");
}
