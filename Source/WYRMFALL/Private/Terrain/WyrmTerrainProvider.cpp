#include "Terrain/WyrmTerrainProvider.h"
FWyrmTerrainCapabilities IWyrmTerrainProvider::GetTerrainCapabilities_Implementation() const
{ return FWyrmTerrainCapabilities(); }
EWyrmTerrainSubmitResult IWyrmTerrainProvider::SubmitTerrainEdit_Implementation(const FWyrmTerrainEditRequest& Request)
{ return EWyrmTerrainSubmitResult::Unsupported; }
bool IWyrmTerrainProvider::IsVolumeOccupied_Implementation(const FVector& Center, float RadiusCm) const
{ return false; }
bool IWyrmTerrainProvider::GetLastYield_Implementation(const FGuid& ActionId, FWyrmVoxelYield& OutYield) const
{ return false; }

