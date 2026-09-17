#include "Building/WyrmBuildingPiece.h"
#include "Components/StaticMeshComponent.h"

AWyrmBuildingPiece::AWyrmBuildingPiece()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

void AWyrmBuildingPiece::ToggleDoor()
{
    if (PieceType != EWyrmBuildingPieceType::Door)
    {
        return;
    }

    bIsOpen = !bIsOpen;
    if (MeshComponent)
    {
        MeshComponent->SetRelativeRotation(bIsOpen ? FRotator(0.f, 90.f, 0.f) : FRotator::ZeroRotator);
    }
}

TArray<FWyrmBuildingCost> AWyrmBuildingPiece::GetDemolitionRefund(const TArray<FWyrmBuildingCost>& OriginalCosts, float RefundPercent) const
{
    TArray<FWyrmBuildingCost> Refunds;
    const float ClampedPercent = FMath::Clamp(RefundPercent, 0.f, 1.f);
    if (ClampedPercent <= 0.f)
    {
        return Refunds;
    }

    for (const FWyrmBuildingCost& Cost : OriginalCosts)
    {
        int32 RefundCount = FMath::FloorToInt(Cost.Count * ClampedPercent);
        if (RefundCount <= 0 && Cost.Count > 0 && ClampedPercent > 0.f)
        {
            RefundCount = 1;
        }

        if (RefundCount > 0)
        {
            FWyrmBuildingCost Ref;
            Ref.ItemId = Cost.ItemId;
            Ref.Count = RefundCount;
            Refunds.Add(Ref);
        }
    }
    return Refunds;
}

FBox AWyrmBuildingPiece::GetSupportBounds() const
{
    if (PieceType != EWyrmBuildingPieceType::Foundation)
    {
        return FBox(ForceInit);
    }
    const FVector Center = GetActorLocation();
    const FVector Min = Center - FVector(SupportRadius, SupportRadius, SupportDepth);
    const FVector Max = Center + FVector(SupportRadius, SupportRadius, 20.f);
    return FBox(Min, Max);
}

FBox AWyrmBuildingPiece::GetPieceBounds() const
{
    if (MeshComponent && MeshComponent->GetStaticMesh())
    {
        return MeshComponent->Bounds.GetBox();
    }
    const FVector Center = GetActorLocation();
    return FBox(Center - BoundsExtent, Center + BoundsExtent);
}
