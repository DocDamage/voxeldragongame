#include "Water/WyrmWaterVolume.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/WyrmCharacter.h"
#include "Terrain/WyrmGeoForgeAdapter.h"
#include "EngineUtils.h"

AWyrmWaterVolume::AWyrmWaterVolume()
{
    PrimaryActorTick.bCanEverTick = true;

    WaterBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WaterBox"));
    RootComponent = WaterBox;
    WaterBox->SetBoxExtent(FVector(1000.f, 1000.f, 300.f));
    WaterBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    WaterBox->SetGenerateOverlapEvents(true);

    WaterBox->OnComponentBeginOverlap.AddDynamic(this, &AWyrmWaterVolume::OnWaterBeginOverlap);
    WaterBox->OnComponentEndOverlap.AddDynamic(this, &AWyrmWaterVolume::OnWaterEndOverlap);
}

void AWyrmWaterVolume::BeginPlay()
{
    Super::BeginPlay();

    if (FMath::IsNearlyZero(SurfaceElevation))
    {
        SurfaceElevation = GetActorLocation().Z + WaterBox->GetScaledBoxExtent().Z;
    }

    // Register with any active terrain adapter in the world
    for (TActorIterator<AWyrmGeoForgeAdapter> It(GetWorld()); It; ++It)
    {
        It->RegisterWaterVolume(this);
    }
}

void AWyrmWaterVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (TActorIterator<AWyrmGeoForgeAdapter> It(GetWorld()); It; ++It)
    {
        It->UnregisterWaterVolume(this);
    }

    Super::EndPlay(EndPlayReason);
}

void AWyrmWaterVolume::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    for (int32 i = OverlappingCharacters.Num() - 1; i >= 0; --i)
    {
        if (AWyrmCharacter* Character = OverlappingCharacters[i].Get())
        {
            UpdateCharacterWaterState(Character);
        }
        else
        {
            OverlappingCharacters.RemoveAt(i);
        }
    }
}

bool AWyrmWaterVolume::IsPointInWater(const FVector& Point) const
{
    const FBox Bounds = GetWaterBounds();
    return Bounds.IsInsideOrOn(Point) && Point.Z <= SurfaceElevation;
}

float AWyrmWaterVolume::GetWaterDepth(const FVector& Point) const
{
    if (!IsPointInWater(Point))
    {
        return 0.f;
    }
    return FMath::Max(0.f, SurfaceElevation - Point.Z);
}

bool AWyrmWaterVolume::CanFishAtLocation(const FVector& CastLocation, FString& OutReason) const
{
    if (!IsPointInWater(CastLocation))
    {
        OutReason = TEXT("Cast location is not inside a valid water body");
        return false;
    }

    const float Depth = GetWaterDepth(CastLocation);
    if (Depth < 20.f)
    {
        OutReason = TEXT("Water is too shallow for fishing");
        return false;
    }

    return true;
}

FBox AWyrmWaterVolume::GetWaterBounds() const
{
    if (WaterBox)
    {
        return WaterBox->Bounds.GetBox();
    }
    return FBox(ForceInit);
}

bool AWyrmWaterVolume::ValidateTerrainEdit(const FWyrmTerrainEditRequest& Request, FString& OutRejectionReason) const
{
    if (Request.Operation != EWyrmTerrainEditOperation::Remove)
    {
        return true;
    }

    const FBox Bounds = GetWaterBounds();
    const FSphere EditSphere(Request.WorldCenter, Request.RadiusCm);

    // If edit does not intersect the water body at all, water volume has no objection
    if (!FMath::SphereAABBIntersection(EditSphere, Bounds))
    {
        return true;
    }

    // Outer boundary protection (WRLD-11):
    // If the edit sphere breaches the outer perimeter lip holding the water basin, reject it.
    if (bProtectOuterBoundary)
    {
        const float InnerMinX = Bounds.Min.X + OuterLipThickness;
        const float InnerMaxX = Bounds.Max.X - OuterLipThickness;
        const float InnerMinY = Bounds.Min.Y + OuterLipThickness;
        const float InnerMaxY = Bounds.Max.Y - OuterLipThickness;
        const float InnerMinZ = Bounds.Min.Z + (OuterLipThickness * 0.5f);

        const bool bBreachesLipX = (Request.WorldCenter.X - Request.RadiusCm < InnerMinX && Request.WorldCenter.X + Request.RadiusCm > Bounds.Min.X) ||
                                  (Request.WorldCenter.X + Request.RadiusCm > InnerMaxX && Request.WorldCenter.X - Request.RadiusCm < Bounds.Max.X);

        const bool bBreachesLipY = (Request.WorldCenter.Y - Request.RadiusCm < InnerMinY && Request.WorldCenter.Y + Request.RadiusCm > Bounds.Min.Y) ||
                                  (Request.WorldCenter.Y + Request.RadiusCm > InnerMaxY && Request.WorldCenter.Y - Request.RadiusCm < Bounds.Max.Y);

        const bool bBreachesBottom = (Request.WorldCenter.Z - Request.RadiusCm < InnerMinZ && Request.WorldCenter.Z + Request.RadiusCm > Bounds.Min.Z);

        if (bBreachesLipX || bBreachesLipY || bBreachesBottom)
        {
            OutRejectionReason = TEXT("Water basin boundary breach prohibited");
            return false;
        }
    }

    // Allowed bed excavation (WRLD-10): Interior excavation is permitted
    if (bAllowBedExcavation)
    {
        return true;
    }

    OutRejectionReason = TEXT("Excavation in this water body is disabled");
    return false;
}

bool AWyrmWaterVolume::ValidateTerrainEditAtLocation(
    const FVector& Center, float RadiusCm, bool bAddOperation, FString& OutRejectionReason) const
{
    FWyrmTerrainEditRequest Request;
    Request.ActionId = FGuid::NewGuid();
    Request.WorldCenter = Center;
    Request.RadiusCm = RadiusCm;
    Request.Operation = bAddOperation
        ? EWyrmTerrainEditOperation::Add
        : EWyrmTerrainEditOperation::Remove;
    return ValidateTerrainEdit(Request, OutRejectionReason);
}

bool AWyrmWaterVolume::IsTerrainEditAllowedAtLocation(
    const FVector& Center, float RadiusCm, bool bAddOperation) const
{
    FString RejectionReason;
    return ValidateTerrainEditAtLocation(Center, RadiusCm, bAddOperation, RejectionReason);
}

void AWyrmWaterVolume::OnWaterBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                           bool bFromSweep, const FHitResult& SweepResult)
{
    if (AWyrmCharacter* Character = Cast<AWyrmCharacter>(OtherActor))
    {
        OverlappingCharacters.AddUnique(Character);
        UpdateCharacterWaterState(Character);
    }
}

void AWyrmWaterVolume::OnWaterEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AWyrmCharacter* Character = Cast<AWyrmCharacter>(OtherActor))
    {
        OverlappingCharacters.Remove(Character);
        if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
        {
            MoveComp->SetMovementMode(MOVE_Walking);
        }
        Character->SetWet(false);
    }
}

void AWyrmWaterVolume::UpdateCharacterWaterState(AWyrmCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    const FVector PawnLoc = Character->GetActorLocation();
    // Submerged if feet/torso are below the water surface
    if (PawnLoc.Z <= SurfaceElevation)
    {
        if (MoveComp->MovementMode != MOVE_Swimming)
        {
            MoveComp->SetMovementMode(MOVE_Swimming);
        }
        Character->SetWet(true);
    }
    else
    {
        if (MoveComp->MovementMode == MOVE_Swimming)
        {
            MoveComp->SetMovementMode(MOVE_Walking);
        }
        // Still wet if within water box
        Character->SetWet(true);
    }
}
