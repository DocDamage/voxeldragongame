#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Terrain/WyrmTerrainProvider.h"
#include "Combat/WyrmAttributeSet.h"
#include "Player/WyrmCharacter.h"
#include "Player/WyrmPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MuCO/CustomizableSkeletalComponent.h"
#include "GameplayTagsManager.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmTerrainRequestTest, "WYRMFALL.Scaffold.TerrainRequestValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmTerrainRequestTest::RunTest(const FString& Parameters)
{
    FWyrmTerrainEditRequest Request;
    TestFalse(TEXT("Default request cannot authorize an edit"), Request.IsWellFormed());
    Request.ActionId = FGuid::NewGuid(); Request.RadiusCm = 50.f;
    TestTrue(TEXT("Explicit finite request"), Request.IsWellFormed());
    Request.RadiusCm = -1.f;
    TestFalse(TEXT("Reject negative radius"), Request.IsWellFormed());
    Request.RadiusCm = std::numeric_limits<float>::infinity();
    TestFalse(TEXT("Reject infinite radius"), Request.IsWellFormed());
    Request.RadiusCm = 50.f; Request.WorldCenter.X = std::numeric_limits<double>::quiet_NaN();
    TestFalse(TEXT("Reject nonfinite center"), Request.IsWellFormed());
    Request.WorldCenter = FVector::ZeroVector; Request.Operation = static_cast<EWyrmTerrainEditOperation>(99);
    TestFalse(TEXT("Reject unknown operation"), Request.IsWellFormed());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCapabilityTest, "WYRMFALL.Scaffold.NoImplicitTerrainSupport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCapabilityTest::RunTest(const FString& Parameters)
{
    FWyrmTerrainCapabilities C;
    TestFalse(TEXT("No provider is not a pass"), C.HasMinimumG1APIs());
    C.bSmoothRemove = C.bSmoothAdd = C.bCollisionCompletion = C.bPersistentEdits = true;
    TestFalse(TEXT("Missing new-surface navigation blocks minimum API coverage"), C.HasMinimumG1APIs());
    C.bNewSurfaceNavigation = true;
    TestTrue(TEXT("All declared minimum APIs; still not runtime evidence"), C.HasMinimumG1APIs());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmAttributeTest, "WYRMFALL.Scaffold.AttributeInputClamps",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmAttributeTest::RunTest(const FString& Parameters)
{
    UWyrmAttributeSet* Set = NewObject<UWyrmAttributeSet>();
    float V = -10.f; Set->PreAttributeChange(Set->GetHealthAttribute(), V);
    TestEqual(TEXT("No negative health"), V, 0.f);
    V = 1000.f; Set->PreAttributeChange(Set->GetHealthAttribute(), V);
    TestEqual(TEXT("Health bounded by maximum"), V, Set->GetMaxHealth());
    V = 0.f; Set->PreAttributeChange(Set->GetMaxHealthAttribute(), V);
    TestEqual(TEXT("Nonzero maximum"), V, 1.f);
    V = std::numeric_limits<float>::quiet_NaN(); Set->PreAttributeChange(Set->GetHealthAttribute(), V);
    TestEqual(TEXT("Nonfinite health rejected"), V, 0.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmTagTest, "WYRMFALL.Scaffold.TagRegistration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmTagTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Dead tag configured"), FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")), false).IsValid());
    TestTrue(TEXT("Echo tag configured, not unlocked"), FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.RelentlessAdvance")), false).IsValid());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmBaseAttributeTest, "WYRMFALL.Scaffold.AttributeBaseClamps",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmBaseAttributeTest::RunTest(const FString& Parameters)
{
    UWyrmAttributeSet* Set = NewObject<UWyrmAttributeSet>();
    float V = -20.f; Set->PreAttributeBaseChange(Set->GetHealthAttribute(), V);
    TestEqual(TEXT("Base health cannot be negative"), V, 0.f);
    V = std::numeric_limits<float>::infinity(); Set->PreAttributeBaseChange(Set->GetMaxHealthAttribute(), V);
    TestEqual(TEXT("Nonfinite base maximum rejected"), V, 1.f);
    V = 200.f; Set->PreAttributeBaseChange(Set->GetHealthAttribute(), V);
    TestEqual(TEXT("Base health bounded"), V, 100.f);
    Set->InitMaxHealth(std::numeric_limits<float>::quiet_NaN());
    V = 50.f; Set->PreAttributeChange(Set->GetHealthAttribute(), V);
    TestEqual(TEXT("Invalid stored maximum cannot propagate NaN"), V, 1.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmTerrainNumericTest, "WYRMFALL.Scaffold.TerrainNumericBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmTerrainNumericTest::RunTest(const FString& Parameters)
{
    FWyrmTerrainEditRequest Request;
    Request.ActionId = FGuid::NewGuid(); Request.RadiusCm = 1.f;
    Request.Operation = EWyrmTerrainEditOperation::Add;
    TestTrue(TEXT("Finite addition request is structurally valid only"), Request.IsWellFormed());
    Request.RadiusCm = 0.f;
    TestFalse(TEXT("Zero-radius edit rejected"), Request.IsWellFormed());
    Request.RadiusCm = std::numeric_limits<float>::quiet_NaN();
    TestFalse(TEXT("NaN radius rejected"), Request.IsWellFormed());
    Request.RadiusCm = 1.f; Request.WorldCenter.Z = std::numeric_limits<double>::infinity();
    TestFalse(TEXT("Infinite center rejected"), Request.IsWellFormed());
    Request.WorldCenter = FVector::ZeroVector; Request.ActionId.Invalidate();
    TestFalse(TEXT("No stable action identity rejected"), Request.IsWellFormed());
    return true;
}

#include "Terrain/WyrmGeoForgeAdapter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmAdapterCapabilityTest, "WYRMFALL.Scaffold.GeoForgeAdapterCapabilities",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmAdapterCapabilityTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.World())
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World)
    {
        World = GWorld;
    }
    TestNotNull(TEXT("World exists for test"), World);
    if (!World)
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmGeoForgeAdapter* Adapter = World->SpawnActor<AWyrmGeoForgeAdapter>(AWyrmGeoForgeAdapter::StaticClass(), SpawnParams);
    TestNotNull(TEXT("GeoForge adapter actor spawned"), Adapter);
    if (Adapter)
    {
        IWyrmTerrainProvider* Provider = Cast<IWyrmTerrainProvider>(Adapter);
        TestNotNull(TEXT("Adapter implements IWyrmTerrainProvider"), Provider);
        if (Provider)
        {
            FWyrmTerrainCapabilities Caps = Provider->GetTerrainCapabilities_Implementation();
            TestTrue(TEXT("GeoForge adapter declares minimum G1 APIs"), Caps.HasMinimumG1APIs());
            TestTrue(TEXT("GeoForge adapter supports smooth removal"), Caps.bSmoothRemove);
            TestTrue(TEXT("GeoForge adapter supports smooth addition"), Caps.bSmoothAdd);
            TestTrue(TEXT("GeoForge adapter supports collision completion"), Caps.bCollisionCompletion);
            TestTrue(TEXT("GeoForge adapter supports new-surface navigation"), Caps.bNewSurfaceNavigation);
            TestTrue(TEXT("GeoForge adapter supports persistent edits"), Caps.bPersistentEdits);
        }

        FWyrmTerrainCapabilities DirectCaps = Adapter->GetCapabilities();
        TestTrue(TEXT("Direct helper reports minimum G1 APIs"), DirectCaps.HasMinimumG1APIs());

        Adapter->Destroy();
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmAdapterYieldTest, "WYRMFALL.Scaffold.GeoForgeAdapterYieldContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmAdapterYieldTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.World())
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World)
    {
        World = GWorld;
    }
    TestNotNull(TEXT("World exists for test"), World);
    if (!World)
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmGeoForgeAdapter* Adapter = World->SpawnActor<AWyrmGeoForgeAdapter>(AWyrmGeoForgeAdapter::StaticClass(), SpawnParams);
    TestNotNull(TEXT("GeoForge adapter actor spawned"), Adapter);
    if (!Adapter)
    {
        return false;
    }

    IWyrmTerrainProvider* Provider = Cast<IWyrmTerrainProvider>(Adapter);
    TestNotNull(TEXT("Adapter implements IWyrmTerrainProvider"), Provider);
    if (!Provider)
    {
        Adapter->Destroy();
        return false;
    }

    // 1. Unbound adapter returns Unsupported for well-formed request
    FWyrmTerrainEditRequest Req;
    Req.ActionId = FGuid::NewGuid();
    Req.RadiusCm = 100.f;
    Req.WorldCenter = FVector(0.f, 0.f, 0.f);
    Req.Operation = EWyrmTerrainEditOperation::Remove;

    EWyrmTerrainSubmitResult Result = Provider->SubmitTerrainEdit_Implementation(Req);
    TestEqual(TEXT("Unbound adapter returns Unsupported"), Result, EWyrmTerrainSubmitResult::Unsupported);

    // 2. Malformed request returns Rejected
    FWyrmTerrainEditRequest BadReq;
    BadReq.RadiusCm = -50.f;
    EWyrmTerrainSubmitResult BadResult = Provider->SubmitTerrainEdit_Implementation(BadReq);
    TestEqual(TEXT("Malformed request returns Rejected"), BadResult, EWyrmTerrainSubmitResult::Rejected);

    // Direct helper check
    EWyrmTerrainSubmitResult DirectBadResult = Adapter->ExecuteTerrainEdit(BadReq);
    TestEqual(TEXT("Direct helper malformed request returns Rejected"), DirectBadResult, EWyrmTerrainSubmitResult::Rejected);

    // 3. Yield queries on unknown action
    FWyrmVoxelYield OutYield;
    TestFalse(TEXT("Querying unknown ActionId returns false"), Provider->GetLastYield_Implementation(FGuid::NewGuid(), OutYield));

    // 4. Duplicate action ID protection: simulate an already processed action ID
    Adapter->ProcessedActionIds.Add(Req.ActionId);
    EWyrmTerrainSubmitResult DupResult = Provider->SubmitTerrainEdit_Implementation(Req);
    TestEqual(TEXT("Duplicate action ID returns Rejected"), DupResult, EWyrmTerrainSubmitResult::Rejected);

    TestTrue(TEXT("Duplicate action ID sets yield record"), Provider->GetLastYield_Implementation(Req.ActionId, OutYield));
    TestTrue(TEXT("Duplicate prevented flag is set"), OutYield.bDuplicatePrevented);
    TestEqual(TEXT("Duplicate yield extracted count is zero"), OutYield.ExtractedCount, 0);

    Adapter->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCharacterMutableTest, "WYRMFALL.Scaffold.CharacterMutableBinding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCharacterMutableTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.World())
            {
                World = Context.World();
                break;
            }
        }
    }
    TestNotNull(TEXT("World exists for character mutable test"), World);
    if (!World)
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), SpawnParams);
    TestNotNull(TEXT("WyrmCharacter spawned successfully"), Character);
    if (Character)
    {
        UCustomizableSkeletalComponent* Comp = Character->GetCustomizableComponent();
        TestNotNull(TEXT("CustomizableSkeletalComponent exists on character"), Comp);
        if (Comp)
        {
            TestTrue(TEXT("CustomizableSkeletalComponent attached to GetMesh()"), Comp->GetAttachParent() == Character->GetMesh());
        }

        TestEqual(TEXT("Default Color parameter returns White"), Character->GetColorParameter(TEXT("NonExistent")), FLinearColor::White);
        TestEqual(TEXT("Default Option parameter returns empty string"), Character->GetOptionParameter(TEXT("NonExistent")), FString());
        TestEqual(TEXT("Default Float parameter returns zero"), Character->GetFloatParameter(TEXT("NonExistent")), 0.f);

        TestFalse(TEXT("Capture descriptor without instance returns empty"), Character->CaptureAppearanceDescriptor().Len() > 0);
        TestFalse(TEXT("Restore empty descriptor returns false"), Character->RestoreAppearanceDescriptor(TEXT("")));

        Character->Destroy();
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmSharedControlTest, "WYRMFALL.Scaffold.SharedControlFoundation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmSharedControlTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.World())
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World)
    {
        World = GWorld;
    }
    TestNotNull(TEXT("World exists for shared control test"), World);
    if (!World)
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), SpawnParams);
    TestNotNull(TEXT("WyrmCharacter spawned"), Character);
    if (!Character)
    {
        return false;
    }

    AWyrmPlayerController* PC = World->SpawnActor<AWyrmPlayerController>(AWyrmPlayerController::StaticClass(), SpawnParams);
    TestNotNull(TEXT("WyrmPlayerController spawned"), PC);
    if (!PC)
    {
        Character->Destroy();
        return false;
    }

    PC->Possess(Character);
    TestTrue(TEXT("Possessed pawn matches character"), PC->GetPawn() == Character);

    // 1. Initial ThirdPerson camera defaults
    TestEqual(TEXT("Initial camera mode is ThirdPerson"), Character->GetCameraMode(), EWyrmCameraMode::ThirdPerson);
    TestEqual(TEXT("Active camera mode on controller is ThirdPerson"), PC->GetActiveCameraMode(), EWyrmCameraMode::ThirdPerson);
    TestEqual(TEXT("ThirdPerson target arm length is 450"), Character->GetCameraBoom()->TargetArmLength, 450.f);
    TestEqual(TEXT("ThirdPerson FOV is 80"), Character->GetFollowCamera()->FieldOfView, 80.f);
    TestTrue(TEXT("ThirdPerson uses pawn control rotation"), Character->GetCameraBoom()->bUsePawnControlRotation);
    TestFalse(TEXT("ThirdPerson cursor is hidden"), PC->bShowMouseCursor);

    // 2. Camera Mode Toggle to TopDown
    Character->ToggleCamera();
    TestEqual(TEXT("Toggled camera mode is TopDown"), Character->GetCameraMode(), EWyrmCameraMode::TopDown);
    TestEqual(TEXT("TopDown target arm length is 1100"), Character->GetCameraBoom()->TargetArmLength, 1100.f);
    TestEqual(TEXT("TopDown FOV is 55"), Character->GetFollowCamera()->FieldOfView, 55.f);
    TestFalse(TEXT("TopDown does not use pawn control rotation"), Character->GetCameraBoom()->bUsePawnControlRotation);

    // Refresh cursor test on controller
    PC->SetActiveCameraMode(EWyrmCameraMode::TopDown);
    TestTrue(TEXT("TopDown cursor is visible on controller"), PC->bShowMouseCursor);

    // 3. Set camera mode explicitly
    Character->SetCameraMode(EWyrmCameraMode::ThirdPerson);
    TestEqual(TEXT("Explicit camera mode is ThirdPerson"), Character->GetCameraMode(), EWyrmCameraMode::ThirdPerson);
    PC->SetActiveCameraMode(EWyrmCameraMode::ThirdPerson);
    TestFalse(TEXT("ThirdPerson cursor is hidden again"), PC->bShowMouseCursor);

    // 4. Movement Lock state gating
    TestFalse(TEXT("Initial movement lock is false"), Character->IsMovementLocked());
    TestFalse(TEXT("Controller initial movement lock is false"), PC->IsMovementLocked());

    PC->SetMovementLocked(true);
    TestTrue(TEXT("Controller movement is locked"), PC->IsMovementLocked());
    TestTrue(TEXT("Character movement is locked via controller"), Character->IsMovementLocked());

    PC->SetMovementLocked(false);
    TestFalse(TEXT("Controller movement is unlocked"), PC->IsMovementLocked());
    TestFalse(TEXT("Character movement is unlocked"), Character->IsMovementLocked());

    // 5. Control State persistence capture and restore
    FWyrmControlState State;
    PC->CaptureControlState(State);
    TestEqual(TEXT("Captured state camera mode matches"), State.CameraMode, EWyrmCameraMode::ThirdPerson);
    TestTrue(TEXT("Captured click-move enabled"), State.bClickMoveEnabled);
    TestFalse(TEXT("Captured movement locked is false"), State.bMovementLocked);

    // Mutate and serialize
    State.CameraMode = EWyrmCameraMode::TopDown;
    State.bClickMoveEnabled = false;
    State.bMovementLocked = true;
    FString JsonStr = State.ToJsonString();
    TestTrue(TEXT("State JSON serialized"), JsonStr.Contains(TEXT("\"camera_mode\":\"TopDown\"")));

    FWyrmControlState RestoredState;
    TestTrue(TEXT("State JSON deserialized"), FWyrmControlState::FromJsonString(JsonStr, RestoredState));
    TestEqual(TEXT("Restored camera mode is TopDown"), RestoredState.CameraMode, EWyrmCameraMode::TopDown);
    TestFalse(TEXT("Restored click-move is false"), RestoredState.bClickMoveEnabled);
    TestTrue(TEXT("Restored movement locked is true"), RestoredState.bMovementLocked);

    PC->RestoreControlState(RestoredState);
    TestEqual(TEXT("Controller restored camera mode is TopDown"), PC->GetActiveCameraMode(), EWyrmCameraMode::TopDown);
    TestFalse(TEXT("Controller restored click move is false"), PC->IsClickMoveEnabled());
    TestTrue(TEXT("Controller restored movement locked is true"), PC->IsMovementLocked());

    // Cleanup
    PC->UnPossess();
    PC->Destroy();
    Character->Destroy();
    return true;
}
#endif

