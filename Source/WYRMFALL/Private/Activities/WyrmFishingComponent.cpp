#include "Activities/WyrmFishingComponent.h"
#include "Water/WyrmWaterVolume.h"
#include "Player/WyrmCharacter.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"

UWyrmFishingComponent::UWyrmFishingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWyrmFishingComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWyrmFishingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsFishingActive())
    {
        if (AActor* Owner = GetOwner())
        {
            const float DistFromCast = FVector::Dist(Owner->GetActorLocation(), CastOriginLocation);
            if (DistFromCast > MovementCancelTolerance)
            {
                CancelFishing(TEXT("MovementInterrupted"));
            }
        }
    }
}

bool UWyrmFishingComponent::StartFishing(const FVector& TargetWaterLocation, FString& OutFailureReason)
{
    if (FishingState != EWyrmFishingState::Ready)
    {
        OutFailureReason = TEXT("FishingAlreadyActive");
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        OutFailureReason = TEXT("NoOwner");
        return false;
    }

    const float CastDist = FVector::Dist2D(Owner->GetActorLocation(), TargetWaterLocation);
    if (CastDist > MaxCastDistance)
    {
        OutFailureReason = TEXT("OutOfRange");
        return false;
    }

    AWyrmWaterVolume* Water = FindWaterVolumeAtLocation(TargetWaterLocation);
    if (!Water)
    {
        OutFailureReason = TEXT("InvalidWaterLocation");
        return false;
    }

    FString WaterReason;
    if (!Water->CanFishAtLocation(TargetWaterLocation, WaterReason))
    {
        OutFailureReason = WaterReason;
        return false;
    }

    ActiveWaterVolume = Water;
    CastTargetLocation = TargetWaterLocation;
    CastOriginLocation = Owner->GetActorLocation();
    bCatchRejectedBagFull = false;
    PendingCatchItem = FWyrmItemInstance();

    if (AWyrmCharacter* Character = Cast<AWyrmCharacter>(Owner))
    {
        Character->SetMovementLocked(true);
    }

    SetFishingState(EWyrmFishingState::Casting);

    // Schedule bite trigger
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            BiteTimerHandle, this, &UWyrmFishingComponent::TriggerBite, BiteWaitTime, false);
    }

    return true;
}

void UWyrmFishingComponent::TriggerBite()
{
    if (FishingState == EWyrmFishingState::Casting || FishingState == EWyrmFishingState::Waiting)
    {
        SetFishingState(EWyrmFishingState::BiteWindow);
        OnFishingBitePrompt.Broadcast(BiteWindowDuration);

        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                WindowTimerHandle, this, &UWyrmFishingComponent::OnBiteWindowTimeout, BiteWindowDuration, false);
        }
    }
}

void UWyrmFishingComponent::OnBiteWindowTimeout()
{
    if (FishingState == EWyrmFishingState::BiteWindow)
    {
        CancelFishing(TEXT("BiteTimeout"));
    }
}

bool UWyrmFishingComponent::RespondToBite()
{
    if (FishingState != EWyrmFishingState::BiteWindow)
    {
        return false;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(WindowTimerHandle);
    }

    SetFishingState(EWyrmFishingState::Reeling);

    if (ReelDuration > 0.f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ReelTimerHandle, this, &UWyrmFishingComponent::OnReelComplete, ReelDuration, false);
            return true;
        }
    }

    OnReelComplete();
    return true;
}

void UWyrmFishingComponent::OnReelComplete()
{
    if (FishingState == EWyrmFishingState::Reeling)
    {
        SetFishingState(EWyrmFishingState::CatchPending);
        CommitCatch();
    }
}

bool UWyrmFishingComponent::CommitCatch()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ReelTimerHandle);
    }

    AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetOwner());
    if (!Character)
    {
        CancelFishing(TEXT("NoCharacter"));
        return false;
    }

    UWyrmInventoryComponent* Inventory = Character->GetInventory();
    if (!Inventory)
    {
        CancelFishing(TEXT("NoInventory"));
        return false;
    }

    // Generate real ocean fish item
    FWyrmItemInstance FishItem;
    FishItem.InstanceId = FGuid::NewGuid();
    FishItem.ItemId = TEXT("Item.Fish.OceanFish");
    FishItem.DisplayName = FText::FromString(TEXT("Ocean Fish"));
    FishItem.ItemType = EWyrmItemType::Consumable;
    FishItem.StackCount = 1;
    FishItem.MaxStack = 10;

    FWyrmItemInstance Remainder;
    const bool bAdded = Inventory->AddItem(FishItem, Remainder);

    // Full bag handling (ACT-03)
    if (!bAdded && Remainder.StackCount > 0)
    {
        bCatchRejectedBagFull = true;
        PendingCatchItem = FishItem;
        Character->SetMovementLocked(false);
        SetFishingState(EWyrmFishingState::Ready);
        OnFishingCancelled.Broadcast(TEXT("InventoryFull"));
        return false;
    }

    bCatchRejectedBagFull = false;
    PendingCatchItem = FishItem;
    Character->SetMovementLocked(false);

    SetFishingState(EWyrmFishingState::Caught);
    OnFishingCatchCommitted.Broadcast(FishItem);
    SetFishingState(EWyrmFishingState::Ready);
    return true;
}

void UWyrmFishingComponent::CancelFishing(const FString& Reason)
{
    if (!IsFishingActive())
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BiteTimerHandle);
        World->GetTimerManager().ClearTimer(WindowTimerHandle);
        World->GetTimerManager().ClearTimer(ReelTimerHandle);
    }

    if (AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetOwner()))
    {
        Character->SetMovementLocked(false);
    }

    PendingCatchItem = FWyrmItemInstance();
    SetFishingState(EWyrmFishingState::Cancelled);
    OnFishingCancelled.Broadcast(Reason);
    SetFishingState(EWyrmFishingState::Ready);
}

void UWyrmFishingComponent::NotifyCombatDamageTaken(float DamageAmount)
{
    if (DamageAmount > 0.f && IsFishingActive())
    {
        CancelFishing(TEXT("CombatDamage"));
    }
}

void UWyrmFishingComponent::SetFishingState(EWyrmFishingState NewState)
{
    if (FishingState != NewState)
    {
        const EWyrmFishingState OldState = FishingState;
        FishingState = NewState;
        OnFishingStateChanged.Broadcast(NewState, OldState);
    }
}

AWyrmWaterVolume* UWyrmFishingComponent::FindWaterVolumeAtLocation(const FVector& Location) const
{
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AWyrmWaterVolume> It(World); It; ++It)
        {
            if (It->IsPointInWater(Location))
            {
                return *It;
            }
        }
    }
    return nullptr;
}
