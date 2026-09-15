#include "Terrain/WyrmTerrainProvider.h"
FWyrmTerrainCapabilities IWyrmTerrainProvider::GetTerrainCapabilities_Implementation() const
{ return FWyrmTerrainCapabilities(); }
EWyrmTerrainSubmitResult IWyrmTerrainProvider::SubmitTerrainEdit_Implementation(const FWyrmTerrainEditRequest& Request)
{ return EWyrmTerrainSubmitResult::Unsupported; }
// Queued is never completed. WP-01 must add provider-specific authoritative
// geometry/collision/nav completion and durable payload integration before use.
