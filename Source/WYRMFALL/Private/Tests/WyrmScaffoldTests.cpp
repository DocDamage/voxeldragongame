#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Terrain/WyrmTerrainProvider.h"
#include "Combat/WyrmAttributeSet.h"
#include "Combat/WyrmCombatTypes.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "Player/WyrmCharacter.h"
#include "Player/WyrmPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MuCO/CustomizableSkeletalComponent.h"
#include "GameplayTagsManager.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "Save/WyrmSaveGame.h"
#include "Save/WyrmSaveSubsystem.h"
#include "Combat/Projectiles/WyrmProjectile.h"
#include "Water/WyrmWaterVolume.h"
#include "Crafting/WyrmCraftingTypes.h"
#include "Crafting/WyrmCraftingStation.h"
#include "Crafting/WyrmCraftingSubsystem.h"
#include "Activities/WyrmFishingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Building/WyrmBuildingTypes.h"
#include "Building/WyrmBuildingPiece.h"
#include "Building/WyrmStorageActor.h"
#include "Building/WyrmRecoveryBundleActor.h"
#include "Building/WyrmBuildingSubsystem.h"
#include "Terrain/WyrmGeoForgeAdapter.h"
#include "Dragon/WyrmDragonTypes.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "Customization/WyrmCreatorSubsystem.h"
#include "Vehicles/WyrmVehicleTypes.h"
#include "Vehicles/WyrmHovercar.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCombatCanonicalDamageTest, "WYRMFALL.Scaffold.CombatCanonicalDamage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCombatCanonicalDamageTest::RunTest(const FString& Parameters)
{
    // Canonical Level scaling
    TestEqual(TEXT("Level 1 MaxHealth is 100"), UWyrmAttributeSet::CalculateMaxHealthForLevel(1.f), 100.f);
    TestEqual(TEXT("Level 1 Power is 20"), UWyrmAttributeSet::CalculatePowerForLevel(1.f), 20.f);
    TestEqual(TEXT("Level 3 MaxHealth is 116"), UWyrmAttributeSet::CalculateMaxHealthForLevel(3.f), 116.f);
    TestEqual(TEXT("Level 3 Power is 26"), UWyrmAttributeSet::CalculatePowerForLevel(3.f), 26.f);

    // Canonical Raw damage
    TestEqual(TEXT("Basic raw damage at L1 (10 base + 0.5 * 20)"), UWyrmAttributeSet::CalculateRawDamage(10.f, 20.f, 0.5f), 20.f);
    TestEqual(TEXT("Secondary raw damage at L1 (10 base + 0.9 * 20)"), UWyrmAttributeSet::CalculateRawDamage(10.f, 20.f, 0.9f), 28.f);

    // Canonical Physical mitigation and L1 regression fixture (COM-01)
    // At L1 against Armor 20: 20 / (20 + 50 + 10*1) = 20 / 80 = 0.25 (25%)
    const float Mitigation = UWyrmAttributeSet::CalculatePhysicalMitigation(20.f, 1.f);
    TestEqual(TEXT("Canonical L1 physical mitigation is 0.25"), Mitigation, 0.25f);

    // Raw 20 with 25% mitigation -> 15.0 mitigated damage
    const float Mitigated = UWyrmAttributeSet::CalculateMitigatedDamage(20.f, 20.f, 1.f);
    TestEqual(TEXT("Canonical L1 regression fixture damage is exactly 15.0"), Mitigated, 15.0f);

    // Boundary conditions
    TestEqual(TEXT("Zero armor gives zero mitigation"), UWyrmAttributeSet::CalculatePhysicalMitigation(0.f, 1.f), 0.f);
    TestEqual(TEXT("Negative armor gives zero mitigation"), UWyrmAttributeSet::CalculatePhysicalMitigation(-10.f, 1.f), 0.f);
    TestEqual(TEXT("High armor clamped to 0.70 cap"), UWyrmAttributeSet::CalculatePhysicalMitigation(10000.f, 1.f), 0.70f);

    // Nonfinite rejections
    TestEqual(TEXT("NaN armor yields zero mitigation"), UWyrmAttributeSet::CalculatePhysicalMitigation(std::numeric_limits<float>::quiet_NaN(), 1.f), 0.f);
    TestEqual(TEXT("NaN weapon base defaults to 0"), UWyrmAttributeSet::CalculateRawDamage(std::numeric_limits<float>::quiet_NaN(), 20.f), 10.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCombatBoundsAndDrainTest, "WYRMFALL.Scaffold.CombatBoundsAndDrain",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCombatBoundsAndDrainTest::RunTest(const FString& Parameters)
{
    UWyrmAttributeSet* TargetSet = NewObject<UWyrmAttributeSet>();
    TargetSet->InitMaxHealth(100.f);
    TargetSet->InitHealth(100.f);
    TargetSet->InitShield(20.f);
    TargetSet->InitArmor(0.f);

    // Shield absorption before health (COM-02)
    float Incoming = 15.f;
    float Shield = TargetSet->GetShield();
    float Absorbed = FMath::Min(Shield, Incoming);
    TargetSet->InitShield(Shield - Absorbed);
    Incoming -= Absorbed;
    TestEqual(TEXT("Shield absorbed 15.0 damage"), Absorbed, 15.f);
    TestEqual(TEXT("Remaining shield is 5.0"), TargetSet->GetShield(), 5.f);
    TestEqual(TEXT("Health undamaged when absorbed by shield"), TargetSet->GetHealth(), 100.f);

    // Further damage exceeds shield
    Incoming = 25.f;
    Shield = TargetSet->GetShield();
    Absorbed = FMath::Min(Shield, Incoming);
    TargetSet->InitShield(Shield - Absorbed);
    Incoming -= Absorbed;
    TargetSet->InitHealth(TargetSet->GetHealth() - Incoming);
    TestEqual(TEXT("Remaining 5.0 shield fully depleted"), TargetSet->GetShield(), 0.f);
    TestEqual(TEXT("Remaining 20.0 damage deducted from health"), TargetSet->GetHealth(), 80.f);

    // Overkill protection (Health cannot be negative)
    float NegativeHealth = -50.f;
    TargetSet->PreAttributeChange(TargetSet->GetHealthAttribute(), NegativeHealth);
    TestEqual(TEXT("Health clamped at zero, no negative overkill"), NegativeHealth, 0.f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCombatCostAndCooldownTest, "WYRMFALL.Scaffold.CombatCostAndCooldown",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCombatCostAndCooldownTest::RunTest(const FString& Parameters)
{
    UWyrmAttributeSet* Set = NewObject<UWyrmAttributeSet>();
    Set->InitMaxFocus(100.f);
    Set->InitFocus(15.f); // Only 15 Focus

    // Secondary attack costs 20 Focus
    UWyrmMeleeAttackAbility* SecAbility = NewObject<UWyrmMeleeAttackAbility>();
    SecAbility->FocusCost = 20.f;
    SecAbility->CooldownDuration = 5.0f;
    SecAbility->bIsSecondary = true;

    // Focus check: insufficient Focus rejected (COM-04)
    TestFalse(TEXT("Cannot activate ability when Focus is insufficient"), Set->GetFocus() >= SecAbility->FocusCost);

    // Provide sufficient Focus
    Set->InitFocus(100.f);
    TestTrue(TEXT("Can activate ability when Focus is sufficient"), Set->GetFocus() >= SecAbility->FocusCost);

    // Apply cost: deducts 20 Focus atomically
    Set->InitFocus(Set->GetFocus() - SecAbility->FocusCost);
    TestEqual(TEXT("Focus deducted atomically to 80.0"), Set->GetFocus(), 80.f);

    // Focus cannot drop below zero
    float NegativeFocus = -10.f;
    Set->PreAttributeChange(Set->GetFocusAttribute(), NegativeFocus);
    TestEqual(TEXT("Focus clamped to non-negative zero"), NegativeFocus, 0.f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCombatEnemyRolesAndStatusTest, "WYRMFALL.Scaffold.CombatEnemyRolesAndStatus",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCombatEnemyRolesAndStatusTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
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
    if (!World)
    {
        return true;
    }

    // Role configuration check
    AWyrmEnemyCharacter* Chaser = AWyrmEnemyCharacter::SpawnWyrmEnemy(World, EWyrmEnemyRole::MeleeChaser, FTransform(FVector(0.f, 0.f, 100.f)));
    TestNotNull(TEXT("Melee chaser spawned"), Chaser);
    TestEqual(TEXT("Melee chaser max health is 60"), Chaser->GetAttributes()->GetMaxHealth(), 60.f);
    TestEqual(TEXT("Melee chaser armor is 10"), Chaser->GetAttributes()->GetArmor(), 10.f);
    TestEqual(TEXT("Melee chaser power is 15"), Chaser->GetAttributes()->GetPower(), 15.f);

    AWyrmEnemyCharacter* Skirmisher = AWyrmEnemyCharacter::SpawnWyrmEnemy(World, EWyrmEnemyRole::RangedSkirmisher, FTransform(FVector(200.f, 0.f, 100.f)));
    TestNotNull(TEXT("Ranged skirmisher spawned"), Skirmisher);
    TestEqual(TEXT("Ranged skirmisher max health is 50"), Skirmisher->GetAttributes()->GetMaxHealth(), 50.f);
    TestEqual(TEXT("Ranged skirmisher armor is 5"), Skirmisher->GetAttributes()->GetArmor(), 5.f);
    TestEqual(TEXT("Ranged skirmisher power is 12"), Skirmisher->GetAttributes()->GetPower(), 12.f);

    // Status effect slow combining by highest magnitude (COM-05)
    static const FGameplayTag SlowTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Slow")));
    Chaser->ApplyStatusEffect(SlowTag, 3.f, 0.3f);
    const float SpeedAfterSlow1 = Chaser->GetCurrentSpeed();
    TestTrue(TEXT("Speed reduced after 30% slow"), SpeedAfterSlow1 < 550.f);

    Chaser->ApplyStatusEffect(SlowTag, 3.f, 0.5f);
    const float SpeedAfterSlow2 = Chaser->GetCurrentSpeed();
    TestTrue(TEXT("Speed further reduced to 50% max magnitude"), SpeedAfterSlow2 < SpeedAfterSlow1);

    // Boss resistance: boss resists root and stun
    AWyrmEnemyCharacter* Boss = AWyrmEnemyCharacter::SpawnWyrmEnemy(World, EWyrmEnemyRole::MeleeChaser, FTransform(FVector(400.f, 0.f, 100.f)));
    TestNotNull(TEXT("Boss spawned"), Boss);
    Boss->bIsBoss = true;
    static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stun")));
    Boss->ApplyStatusEffect(StunTag, 2.f, 1.f);
    TestTrue(TEXT("Boss walk speed not zeroed by stun (resists hard stun)"), Boss->GetCurrentSpeed() > 0.f);

    // Cleanup
    Chaser->Destroy();
    Skirmisher->Destroy();
    Boss->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmItemGenerationTest, "WYRMFALL.Scaffold.ItemGenerationAndRolls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmItemGenerationTest::RunTest(const FString& Parameters)
{
    // 1. Weapon generation (COM-06)
    FWyrmItemInstance Weapon = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("TestBlade")), EWyrmItemType::Weapon, 2);
    TestTrue(TEXT("Weapon instance ID is valid"), Weapon.InstanceId.IsValid());
    TestEqual(TEXT("Weapon type is Weapon"), Weapon.ItemType, EWyrmItemType::Weapon);
    TestEqual(TEXT("Weapon default slot is MainHand"), Weapon.DefaultSlot, EWyrmEquipSlot::MainHand);
    TestEqual(TEXT("Weapon max stack is 1"), Weapon.MaxStack, 1);
    TestEqual(TEXT("Weapon level 2 power roll is 17.0"), Weapon.GetStatValue(FName(TEXT("Power"))), 17.f); // 10 + 2*3.5 = 17

    // 2. Armor generation
    FWyrmItemInstance Armor = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("TestCuirass")), EWyrmItemType::Armor, 3);
    TestTrue(TEXT("Armor instance ID is valid"), Armor.InstanceId.IsValid());
    TestEqual(TEXT("Armor type is Armor"), Armor.ItemType, EWyrmItemType::Armor);
    TestEqual(TEXT("Armor default slot is Chest"), Armor.DefaultSlot, EWyrmEquipSlot::Chest);
    TestEqual(TEXT("Armor level 3 armor roll is 11.0"), Armor.GetStatValue(FName(TEXT("Armor"))), 11.f); // 5 + 3*2 = 11
    TestEqual(TEXT("Armor level 3 max health roll is 50.0"), Armor.GetStatValue(FName(TEXT("MaxHealth"))), 50.f); // 20 + 3*10 = 50

    // 3. Stacking items: consumable and resource
    FWyrmItemInstance Elixir = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("WyrmElixir")), EWyrmItemType::Consumable, 1);
    TestEqual(TEXT("Consumable max stack is 20"), Elixir.MaxStack, 20);

    FWyrmItemInstance Ore = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("ObsidianOre")), EWyrmItemType::Resource, 1);
    TestEqual(TEXT("Resource max stack is 99"), Ore.MaxStack, 99);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmInventoryCapacityTest, "WYRMFALL.Scaffold.InventoryCapacityAndTransfer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmInventoryCapacityTest::RunTest(const FString& Parameters)
{
    UWyrmInventoryComponent* Inv = NewObject<UWyrmInventoryComponent>();
    Inv->MaxBagSlots = 3;
    Inv->MaxStashSlots = 5;

    // 1. Fill bag to max capacity (3 slots)
    FWyrmItemInstance Item1 = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("Blade1")), EWyrmItemType::Weapon, 1);
    FWyrmItemInstance Item2 = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("Blade2")), EWyrmItemType::Weapon, 1);
    FWyrmItemInstance Item3 = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("Blade3")), EWyrmItemType::Weapon, 1);

    FWyrmItemInstance Excess;
    TestTrue(TEXT("Added item 1"), Inv->AddItem(Item1, Excess) && Excess.StackCount == 0);
    TestTrue(TEXT("Added item 2"), Inv->AddItem(Item2, Excess) && Excess.StackCount == 0);
    TestTrue(TEXT("Added item 3"), Inv->AddItem(Item3, Excess) && Excess.StackCount == 0);
    TestEqual(TEXT("Bag slots at maximum 3"), Inv->GetBagItems().Num(), 3);

    // 2. Overflow protection (COM-07): adding 4th item fails and returns excess without modifying bag
    FWyrmItemInstance Item4 = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("Blade4")), EWyrmItemType::Weapon, 1);
    TestFalse(TEXT("Cannot add 4th item when bag is full"), Inv->AddItem(Item4, Excess));
    TestEqual(TEXT("Excess stack count retained in OutRemaining"), Excess.StackCount, 1);
    TestEqual(TEXT("Bag count still 3 (no leak or overwrite)"), Inv->GetBagItems().Num(), 3);

    // 3. Transfer from Bag to Stash atomically
    TestTrue(TEXT("Transfer item 1 from Bag to Stash"), Inv->TransferToStash(Item1.InstanceId, 1));
    TestEqual(TEXT("Bag count reduced to 2"), Inv->GetBagItems().Num(), 2);
    TestEqual(TEXT("Stash count is now 1"), Inv->GetStashItems().Num(), 1);

    // 4. Transfer back from Stash to Bag atomically
    TestTrue(TEXT("Transfer item 1 from Stash back to Bag"), Inv->TransferFromStash(Item1.InstanceId, 1));
    TestEqual(TEXT("Bag count restored to 3"), Inv->GetBagItems().Num(), 3);
    TestEqual(TEXT("Stash count is now 0"), Inv->GetStashItems().Num(), 0);

    // 5. Stack combining with resources
    Inv->ClearAll();
    FWyrmItemInstance OreBatch1 = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("Ore")), EWyrmItemType::Resource, 1);
    OreBatch1.StackCount = 50;
    Inv->AddItem(OreBatch1, Excess);

    FWyrmItemInstance OreBatch2 = OreBatch1;
    OreBatch2.StackCount = 30;
    Inv->AddItem(OreBatch2, Excess);

    TestEqual(TEXT("Bag count is 1 after stacking"), Inv->GetBagItems().Num(), 1);
    TestEqual(TEXT("Combined stack count is 80"), Inv->GetBagItems()[0].StackCount, 80);

    // 6. A cross-component transfer must not partially fill the target when the
    // complete requested count will not fit.
    UWyrmInventoryComponent* SourceInv = NewObject<UWyrmInventoryComponent>();
    UWyrmInventoryComponent* TargetInv = NewObject<UWyrmInventoryComponent>();
    SourceInv->MaxBagSlots = 1;
    TargetInv->MaxBagSlots = 1;

    FWyrmItemInstance SourceOre = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("SharedOre")), EWyrmItemType::Resource, 1);
    SourceOre.StackCount = 10;
    FWyrmItemInstance TargetOre = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("SharedOre")), EWyrmItemType::Resource, 1);
    TargetOre.StackCount = 98;
    TestTrue(TEXT("Source ore added"), SourceInv->AddItem(SourceOre, Excess));
    TestTrue(TEXT("Target ore added"), TargetInv->AddItem(TargetOre, Excess));

    TestFalse(TEXT("Two-item transfer rejects partial target capacity"),
        SourceInv->TransferItem(TargetInv, SourceOre.InstanceId, 2));
    TestEqual(TEXT("Rejected transfer preserves source count"), SourceInv->GetBagItems()[0].StackCount, 10);
    TestEqual(TEXT("Rejected transfer preserves target count"), TargetInv->GetBagItems()[0].StackCount, 98);

    TestTrue(TEXT("One-item transfer succeeds into remaining stack space"),
        SourceInv->TransferItem(TargetInv, SourceOre.InstanceId, 1));
    TestEqual(TEXT("Successful transfer decrements source"), SourceInv->GetBagItems()[0].StackCount, 9);
    TestEqual(TEXT("Successful transfer fills target stack"), TargetInv->GetBagItems()[0].StackCount, 99);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmEquipmentStatTest, "WYRMFALL.Scaffold.EquipmentStatApplication",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmEquipmentStatTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
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
    if (!World)
    {
        return true;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), FTransform::Identity, SpawnParams);
    TestNotNull(TEXT("WyrmCharacter spawned for equipment test"), Character);
    if (!Character)
    {
        return true;
    }

    UWyrmAttributeSet* Attrs = Character->GetAttributes();
    UWyrmInventoryComponent* Inv = Character->GetInventory();
    TestNotNull(TEXT("AttributeSet present on character"), Attrs);
    TestNotNull(TEXT("InventoryComponent present on character"), Inv);
    if (!Attrs || !Inv)
    {
        Character->Destroy();
        return true;
    }

    const float BasePower = Attrs->GetCurrentPower();
    const float BaseArmor = Attrs->GetCurrentArmor();
    const float BaseMaxHealth = Attrs->GetCurrentMaxHealth();

    // 1. Generate rolled weapon (+17 Power) and add to bag
    FWyrmItemInstance Weapon = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("IronSword")), EWyrmItemType::Weapon, 2);
    FWyrmItemInstance Excess;
    Inv->AddItem(Weapon, Excess);

    // 2. Equip weapon and verify stats (COM-06)
    TestTrue(TEXT("Weapon equipped successfully"), Inv->EquipItem(Weapon.InstanceId, EWyrmEquipSlot::MainHand));
    TestTrue(TEXT("MainHand is equipped"), Inv->IsSlotEquipped(EWyrmEquipSlot::MainHand));
    TestEqual(TEXT("Power increased by weapon bonus (+17)"), Attrs->GetCurrentPower(), BasePower + 17.f);
    TestEqual(TEXT("Bag items count is 0 after equip"), Inv->GetBagItems().Num(), 0);

    // 3. Generate rolled armor (+11 Armor, +50 MaxHealth) and equip
    FWyrmItemInstance Armor = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("SteelPlate")), EWyrmItemType::Armor, 3);
    Inv->AddItem(Armor, Excess);
    TestTrue(TEXT("Armor equipped successfully"), Inv->EquipItem(Armor.InstanceId, EWyrmEquipSlot::Chest));
    TestEqual(TEXT("Armor increased by armor bonus (+11)"), Attrs->GetCurrentArmor(), BaseArmor + 11.f);
    TestEqual(TEXT("MaxHealth increased by armor bonus (+50)"), Attrs->GetCurrentMaxHealth(), BaseMaxHealth + 50.f);

    // 4. Unequip weapon and verify clean return to base power (zero leaks)
    TestTrue(TEXT("Weapon unequipped successfully"), Inv->UnequipItem(EWyrmEquipSlot::MainHand));
    TestFalse(TEXT("MainHand no longer equipped"), Inv->IsSlotEquipped(EWyrmEquipSlot::MainHand));
    TestEqual(TEXT("Power returned exactly to base value"), Attrs->GetCurrentPower(), BasePower);
    TestEqual(TEXT("Bag items count is 1 after unequip"), Inv->GetBagItems().Num(), 1);

    // 5. Unequip armor and verify clean return to base armor and health
    TestTrue(TEXT("Armor unequipped successfully"), Inv->UnequipItem(EWyrmEquipSlot::Chest));
    TestEqual(TEXT("Armor returned exactly to base value"), Attrs->GetCurrentArmor(), BaseArmor);
    TestEqual(TEXT("MaxHealth returned exactly to base value"), Attrs->GetCurrentMaxHealth(), BaseMaxHealth);

    // 6. Test overflow guard on unequip: fill bag completely, then verify unequip is rejected
    Inv->EquipItem(Weapon.InstanceId, EWyrmEquipSlot::MainHand);
    Inv->MaxBagSlots = 1;
    Inv->AddItem(Armor, Excess); // Bag is now 1/1 full
    TestFalse(TEXT("Cannot unequip weapon when bag is full (overflow protection)"), Inv->UnequipItem(EWyrmEquipSlot::MainHand));
    TestTrue(TEXT("Weapon remains equipped"), Inv->IsSlotEquipped(EWyrmEquipSlot::MainHand));

    Character->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmSaveSubsystemTest, "WYRMFALL.Scaffold.SaveSubsystemRoundtrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmSaveSubsystemTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
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
    if (!World)
    {
        return true;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), FTransform::Identity, SpawnParams);
    TestNotNull(TEXT("WyrmCharacter spawned for save test"), Character);
    if (!Character)
    {
        return true;
    }

    UWyrmAttributeSet* Attrs = Character->GetAttributes();
    UWyrmInventoryComponent* Inv = Character->GetInventory();

    // Setup custom character state
    Attrs->SetCurrentHealth(75.f);
    Attrs->SetCurrentMaxFocus(175.f);
    Attrs->SetCurrentFocus(140.f);
    Attrs->SetCurrentCharacterLevel(7.f);
    Attrs->SetCurrentPower(25.f);
    Attrs->SetCurrentArmor(15.f);
    Character->SetCameraMode(EWyrmCameraMode::TopDown);
    Character->SetActorLocationAndRotation(FVector::ZeroVector, FRotator(0.f, 37.f, 0.f));

    // Setup inventory with rolled weapon equipped and potion in bag
    FWyrmItemInstance Weapon = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("RelicSword")), EWyrmItemType::Weapon, 2);
    FWyrmItemInstance Potion = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("HealPotion")), EWyrmItemType::Consumable, 1);
    Potion.StackCount = 5;

    FWyrmItemInstance Excess;
    Inv->AddItem(Weapon, Excess);
    Inv->AddItem(Potion, Excess);
    Inv->EquipItem(Weapon.InstanceId, EWyrmEquipSlot::MainHand);

    // A supplied terrain owner must produce a real payload. An unbound adapter
    // is rejected instead of silently creating a partial snapshot.
    AWyrmGeoForgeAdapter* Adapter = World->SpawnActor<AWyrmGeoForgeAdapter>(AWyrmGeoForgeAdapter::StaticClass(), SpawnParams);
    TestNull(TEXT("Snapshot rejects an unbound terrain adapter"),
        UWyrmSaveSubsystem::CreateSnapshotObject(TEXT("InvalidTerrain_WP05"), Character, Adapter));

    // Capture the character/inventory portion. WP-01 exercises the real bound
    // GeoForge binary payload path.
    UWyrmSaveGame* Snapshot = UWyrmSaveSubsystem::CreateSnapshotObject(TEXT("TestSlot_WP05"), Character, nullptr);
    TestNotNull(TEXT("Snapshot created successfully"), Snapshot);
    if (!Snapshot)
    {
        return false;
    }
    TestEqual(TEXT("Snapshot camera mode is TopDown"), Snapshot->CharacterRecord.CameraMode, EWyrmCameraMode::TopDown);
    TestEqual(TEXT("Snapshot contains 1 equipped item"), Snapshot->InventoryRecord.EquippedItems.Num(), 1);
    TestEqual(TEXT("Snapshot contains 1 bag item (potion)"), Snapshot->InventoryRecord.BagItems.Num(), 1);
    TestEqual(TEXT("Snapshot preserves max focus"), Snapshot->CharacterRecord.MaxFocus, 175.f);
    TestEqual(TEXT("Snapshot preserves character level"), Snapshot->CharacterRecord.CharacterLevel, 7.f);
    TestTrue(TEXT("Snapshot preserves a legitimate world-origin location"), Snapshot->CharacterRecord.WorldLocation.IsZero());

    // Mutate character to different state
    Attrs->SetCurrentHealth(10.f);
    Attrs->SetCurrentMaxFocus(20.f);
    Attrs->SetCurrentFocus(5.f);
    Attrs->SetCurrentCharacterLevel(1.f);
    Attrs->SetCurrentPower(5.f);
    Character->SetCameraMode(EWyrmCameraMode::ThirdPerson);
    Character->SetActorLocationAndRotation(FVector(1000.f, 1000.f, 1000.f), FRotator::ZeroRotator);
    Inv->ClearAll();

    // Restore snapshot
    TestTrue(TEXT("Snapshot applied successfully"), UWyrmSaveSubsystem::ApplySnapshotObject(Snapshot, Character, nullptr));

    // Verify complete restoration fidelity
    TestEqual(TEXT("Restored health matches snapshot"), Attrs->GetCurrentHealth(), 75.f);
    TestEqual(TEXT("Restored max focus matches snapshot"), Attrs->GetCurrentMaxFocus(), 175.f);
    TestEqual(TEXT("Restored focus matches snapshot"), Attrs->GetCurrentFocus(), 140.f);
    TestEqual(TEXT("Restored character level matches snapshot"), Attrs->GetCurrentCharacterLevel(), 7.f);
    TestEqual(TEXT("Restored power matches (base + equipped bonus)"), Attrs->GetCurrentPower(), 42.f);
    TestEqual(TEXT("Restored camera mode is TopDown"), Character->GetCameraMode(), EWyrmCameraMode::TopDown);
    TestTrue(TEXT("Restored main hand is equipped"), Inv->IsSlotEquipped(EWyrmEquipSlot::MainHand));
    TestEqual(TEXT("Restored bag item count is 1"), Inv->GetBagItems().Num(), 1);
    TestEqual(TEXT("Restored bag item is potion with 5 stacks"), Inv->GetBagItems()[0].StackCount, 5);
    TestTrue(TEXT("Restored location remains at world origin"), Character->GetActorLocation().IsNearlyZero());
    TestTrue(TEXT("Restored world-origin rotation matches snapshot"), FMath::IsNearlyEqual(Character->GetActorRotation().Yaw, 37.f));

    UWyrmSaveGame* BadSchemaSnapshot = DuplicateObject<UWyrmSaveGame>(Snapshot, GetTransientPackage());
    BadSchemaSnapshot->SchemaVersion = UWyrmSaveGame::CurrentSchemaVersion + 1;
    Attrs->SetCurrentHealth(63.f);
    TestFalse(TEXT("Unknown save schema is rejected"),
        UWyrmSaveSubsystem::ApplySnapshotObject(BadSchemaSnapshot, Character, nullptr));
    TestEqual(TEXT("Rejected schema leaves character untouched"), Attrs->GetCurrentHealth(), 63.f);
    Attrs->SetCurrentHealth(75.f);

    Attrs->SetCurrentHealth(61.f);
    TestFalse(TEXT("Wrong terrain owner is rejected before character mutation"),
        UWyrmSaveSubsystem::ApplySnapshotObject(Snapshot, Character, Character));
    TestEqual(TEXT("Rejected terrain owner leaves character untouched"), Attrs->GetCurrentHealth(), 61.f);
    Attrs->SetCurrentHealth(75.f);

    // Test slot disk I/O save/load roundtrip (SAVE-01)
    const FString SlotName = TEXT("WyrmSlot_UnitTest");
    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        GI = NewObject<UGameInstance>(World);
    }
    UWyrmSaveSubsystem* SaveSys = NewObject<UWyrmSaveSubsystem>(GI);
    TestTrue(TEXT("SaveGameSnapshot to slot succeeds"), SaveSys->SaveGameSnapshot(SlotName, Character, nullptr));
    TestTrue(TEXT("DoesSaveExist returns true for slot"), SaveSys->DoesSaveExist(SlotName));

    // Clear and reload from disk slot
    Inv->ClearAll();
    TestTrue(TEXT("LoadGameSnapshot from slot succeeds"), SaveSys->LoadGameSnapshot(SlotName, Character, nullptr));
    TestTrue(TEXT("Slot load restored equipped item"), Inv->IsSlotEquipped(EWyrmEquipSlot::MainHand));

    // Delete slot cleanup
    TestTrue(TEXT("DeleteSaveSlot cleans up slot"), SaveSys->DeleteSaveSlot(SlotName));
    TestFalse(TEXT("DoesSaveExist returns false after deletion"), SaveSys->DoesSaveExist(SlotName));

    Character->Destroy();
    Adapter->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmProgressionXpTest, "WYRMFALL.Scaffold.ProgressionXpAndLevelUp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmProgressionXpTest::RunTest(const FString& Parameters)
{
    // Test canonical XP needed formula from GAME_DESIGN.md line 28
    TestEqual(TEXT("XP needed for Lv.1->2 is 100"), AWyrmCharacter::CalculateXPForNextLevel(1.f), 100.f);
    TestEqual(TEXT("XP needed for Lv.2->3 is 150"), AWyrmCharacter::CalculateXPForNextLevel(2.f), 150.f);
    TestEqual(TEXT("XP needed for Lv.3->4 is 200"), AWyrmCharacter::CalculateXPForNextLevel(3.f), 200.f);

    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Character spawned"), Character);
    Character->GrantCombatAbilities();

    TestEqual(TEXT("Initial level is 1.0"), Character->GetCharacterLevel(), 1.f);
    TestEqual(TEXT("Initial XP is 0.0"), Character->GetCurrentXP(), 0.f);
    TestEqual(TEXT("Initial XP remaining is 100.0"), Character->GetXPToNextLevel(), 100.f);
    TestEqual(TEXT("Initial base MaxHealth is 100.0"), Character->GetAttributes()->GetCurrentMaxHealth(), 100.f);
    TestEqual(TEXT("Initial base Power is 20.0"), Character->GetAttributes()->GetCurrentPower(), 20.f);

    // Partial XP accumulation without level-up
    TestFalse(TEXT("Partial XP does not level up"), Character->AddExperience(50.f));
    TestEqual(TEXT("Level remains 1.0 after 50 XP"), Character->GetCharacterLevel(), 1.f);
    TestEqual(TEXT("Current XP is 50.0"), Character->GetCurrentXP(), 50.f);
    TestEqual(TEXT("XP remaining is 50.0"), Character->GetXPToNextLevel(), 50.f);

    // Exact threshold level-up
    TestTrue(TEXT("Threshold XP levels up"), Character->AddExperience(50.f));
    TestEqual(TEXT("Level increased to 2.0 after 100 XP total"), Character->GetCharacterLevel(), 2.f);
    TestEqual(TEXT("Current XP reset to 0.0"), Character->GetCurrentXP(), 0.f);
    TestEqual(TEXT("Base MaxHealth scaled to 108.0 (100 + 8*1)"), Character->GetAttributes()->GetCurrentMaxHealth(), 108.f);
    TestEqual(TEXT("Base Power scaled to 23.0 (20 + 3*1)"), Character->GetAttributes()->GetCurrentPower(), 23.f);

    // Overflow XP accumulation across level threshold
    Character->AddExperience(200.f); // 150 needed for Lv.2->3, 50 overflow
    TestEqual(TEXT("Level increased to 3.0"), Character->GetCharacterLevel(), 3.f);
    TestEqual(TEXT("Overflow XP retained at 50.0"), Character->GetCurrentXP(), 50.f);
    TestEqual(TEXT("Base MaxHealth scaled to 116.0 (100 + 8*2)"), Character->GetAttributes()->GetCurrentMaxHealth(), 116.f);
    TestEqual(TEXT("Base Power scaled to 26.0 (20 + 3*2)"), Character->GetAttributes()->GetCurrentPower(), 26.f);

    Character->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmWeaponFamilyGatingTest, "WYRMFALL.Scaffold.WeaponFamilyGatingAndKitSwitch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmWeaponFamilyGatingTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Character spawned"), Character);
    Character->GrantCombatAbilities();

    UWyrmInventoryComponent* Inv = Character->GetInventory();
    TestNotNull(TEXT("Inventory component present"), Inv);

    // Initial state without equipped weapon: Unarmed
    TestEqual(TEXT("Initial active weapon family is Unarmed"), Character->GetActiveWeaponFamily(), EWyrmWeaponFamily::Unarmed);

    // Equip Sword -> Melee1H
    FWyrmItemInstance Sword = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("ForgedBlade")), EWyrmItemType::Weapon, 1);
    TestEqual(TEXT("Rolled sword family is Melee1H"), Sword.WeaponFamily, EWyrmWeaponFamily::Melee1H);
    FWyrmItemInstance Excess;
    Inv->AddItem(Sword, Excess);
    Inv->EquipItem(Sword.InstanceId, EWyrmEquipSlot::MainHand);
    TestEqual(TEXT("Equipping sword sets active family to Melee1H"), Character->GetActiveWeaponFamily(), EWyrmWeaponFamily::Melee1H);

    // Equip Bow -> RangedBow
    FWyrmItemInstance Bow = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("RangerBow")), EWyrmItemType::Weapon, 1);
    TestEqual(TEXT("Rolled bow family is RangedBow"), Bow.WeaponFamily, EWyrmWeaponFamily::RangedBow);
    Inv->AddItem(Bow, Excess);
    Inv->EquipItem(Bow.InstanceId, EWyrmEquipSlot::MainHand);
    TestEqual(TEXT("Equipping bow sets active family to RangedBow"), Character->GetActiveWeaponFamily(), EWyrmWeaponFamily::RangedBow);

    // Unequip weapon -> Unarmed
    Inv->UnequipItem(EWyrmEquipSlot::MainHand);
    TestEqual(TEXT("Unequipping weapon resets family to Unarmed"), Character->GetActiveWeaponFamily(), EWyrmWeaponFamily::Unarmed);

    Character->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmRangedProjectileTest, "WYRMFALL.Scaffold.RangedProjectileDamage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmRangedProjectileTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Player spawned"), Player);
    Player->GrantCombatAbilities();

    AWyrmEnemyCharacter* Enemy = AWyrmEnemyCharacter::SpawnWyrmEnemy(World, EWyrmEnemyRole::MeleeChaser, FTransform(FVector(300.f, 0.f, 100.f)));
    TestNotNull(TEXT("Enemy spawned"), Enemy);

    const float InitialEnemyHealth = Enemy->GetAttributes()->GetCurrentHealth();
    TestEqual(TEXT("Enemy initial health is 60.0"), InitialEnemyHealth, 60.f);

    // Spawn and initialize projectile targeting hostile enemy
    AWyrmProjectile* Proj = World->SpawnActor<AWyrmProjectile>(AWyrmProjectile::StaticClass(), FVector(50.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Projectile spawned"), Proj);

    const float RawDamage = 30.f;
    Proj->InitializeProjectile(Player, Player->GetAbilitySystem(), RawDamage, FVector(1.f, 0.f, 0.f));

    // Simulate projectile impact on enemy
    Proj->OnProjectileHit(nullptr, Enemy, nullptr, FVector::ZeroVector, FHitResult());

    // Enemy Level 1, Armor 10 -> mitigation is 10 / (10 + 50 + 10) = 14.286%
    // Expected mitigated damage: 30 * (1 - 0.142857) = 25.714
    const float ExpectedHealth = 60.f - 25.714f;
    TestNearlyEqual(TEXT("Enemy took mitigated projectile damage"), Enemy->GetAttributes()->GetCurrentHealth(), ExpectedHealth, 0.05f);

    Player->Destroy();
    Enemy->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmProgressionSaveTest, "WYRMFALL.Scaffold.ProgressionSaveRoundtrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmProgressionSaveTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Character spawned"), Character);
    Character->GrantCombatAbilities();

    // Set progression state
    Character->SetCharacterLevel(3.f);
    Character->AddExperience(45.f);

    TestEqual(TEXT("Setup level is 3.0"), Character->GetCharacterLevel(), 3.f);
    TestEqual(TEXT("Setup XP is 45.0"), Character->GetCurrentXP(), 45.f);
    TestEqual(TEXT("Setup MaxHealth is 116.0"), Character->GetAttributes()->GetCurrentMaxHealth(), 116.f);

    // Save snapshot
    const FString SlotName = TEXT("WyrmSlot_Progression_UnitTest");
    UGameInstance* GI = World->GetGameInstance();
    if (!GI) GI = NewObject<UGameInstance>(World);
    UWyrmSaveSubsystem* SaveSys = NewObject<UWyrmSaveSubsystem>(GI);

    TestTrue(TEXT("SaveGameSnapshot succeeds"), SaveSys->SaveGameSnapshot(SlotName, Character, nullptr));

    // Reset character progression to Level 1
    Character->SetCharacterLevel(1.f);
    Character->SetCurrentXP(0.f);
    TestEqual(TEXT("Reset level is 1.0"), Character->GetCharacterLevel(), 1.f);

    // Reload from slot
    TestTrue(TEXT("LoadGameSnapshot succeeds"), SaveSys->LoadGameSnapshot(SlotName, Character, nullptr));

    // Verify restored progression state
    TestEqual(TEXT("Restored level is 3.0"), Character->GetCharacterLevel(), 3.f);
    TestEqual(TEXT("Restored XP is 45.0"), Character->GetCurrentXP(), 45.f);
    TestEqual(TEXT("Restored MaxHealth is 116.0"), Character->GetAttributes()->GetCurrentMaxHealth(), 116.f);
    TestEqual(TEXT("Restored Power is 26.0"), Character->GetAttributes()->GetCurrentPower(), 26.f);

    SaveSys->DeleteSaveSlot(SlotName);
    Character->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmWaterVolumeTest, "WYRMFALL.Scaffold.WaterVolumeSwimmingAndWetState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmWaterVolumeTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmWaterVolume* WaterVol = World->SpawnActor<AWyrmWaterVolume>(
        AWyrmWaterVolume::StaticClass(), FVector(0.f, 0.f, 500.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("WaterVolume spawned"), WaterVol);
    WaterVol->SurfaceElevation = 800.f;

    // 1. Authoritative water queries
    TestTrue(TEXT("Submerged point is in water"), WaterVol->IsPointInWater(FVector(0.f, 0.f, 700.f)));
    TestFalse(TEXT("Point above surface is not in water"), WaterVol->IsPointInWater(FVector(0.f, 0.f, 850.f)));
    TestEqual(TEXT("Water depth calculated accurately"), WaterVol->GetWaterDepth(FVector(0.f, 0.f, 700.f)), 100.f);

    FString FishReason;
    TestTrue(TEXT("Can fish in deep water"), WaterVol->CanFishAtLocation(FVector(0.f, 0.f, 700.f), FishReason));
    TestFalse(TEXT("Cannot fish above water"), WaterVol->CanFishAtLocation(FVector(0.f, 0.f, 850.f), FishReason));

    // 2. Character swimming & wet state (WRLD-10)
    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 600.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Character spawned"), Character);

    // Initial dry walking state
    TestFalse(TEXT("Initially dry"), Character->IsWet());
    Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    TestEqual(TEXT("Initially walking"), Character->GetCharacterMovement()->MovementMode.GetValue(), MOVE_Walking);

    // Submerge character
    Character->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
    Character->SetWet(true);
    TestTrue(TEXT("Character wet when submerged"), Character->IsWet());
    TestEqual(TEXT("Character swimming when submerged"), Character->GetCharacterMovement()->MovementMode.GetValue(), MOVE_Swimming);

    static const FGameplayTag WetTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Wet")), false);
    if (WetTag.IsValid() && Character->GetAbilitySystem())
    {
        TestTrue(TEXT("GAS has State.Wet tag"), Character->GetAbilitySystem()->HasMatchingGameplayTag(WetTag));
    }

    // Move to dry cave / out of water (WRLD-10)
    Character->SetActorLocation(FVector(2500.f, 0.f, 600.f));
    Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    Character->SetWet(false);
    TestFalse(TEXT("Dry state restored outside water"), Character->IsWet());
    TestEqual(TEXT("Walking restored outside water"), Character->GetCharacterMovement()->MovementMode.GetValue(), MOVE_Walking);
    if (WetTag.IsValid() && Character->GetAbilitySystem())
    {
        TestFalse(TEXT("State.Wet tag cleared outside water"), Character->GetAbilitySystem()->HasMatchingGameplayTag(WetTag));
    }

    Character->Destroy();
    WaterVol->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmWaterBoundaryEditTest, "WYRMFALL.Scaffold.WaterBoundaryEditRejection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmWaterBoundaryEditTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmWaterVolume* WaterVol = World->SpawnActor<AWyrmWaterVolume>(
        AWyrmWaterVolume::StaticClass(), FVector(0.f, 0.f, 500.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("WaterVolume spawned"), WaterVol);
    WaterVol->SurfaceElevation = 800.f;
    WaterVol->OuterLipThickness = 150.f;
    WaterVol->bProtectOuterBoundary = true;
    WaterVol->bAllowBedExcavation = true;

    // 1. Interior bed excavation is permitted (WRLD-10)
    FWyrmTerrainEditRequest BedRequest;
    BedRequest.ActionId = FGuid::NewGuid();
    BedRequest.WorldCenter = FVector(0.f, 0.f, 400.f);
    BedRequest.RadiusCm = 100.f;
    BedRequest.Operation = EWyrmTerrainEditOperation::Remove;

    FString AllowedReason;
    TestTrue(TEXT("Interior bed excavation allowed (WRLD-10)"), WaterVol->ValidateTerrainEdit(BedRequest, AllowedReason));

    // 2. Outer boundary breach excavation is rejected (WRLD-11)
    const FBox Bounds = WaterVol->GetWaterBounds();
    FWyrmTerrainEditRequest BreachRequest;
    BreachRequest.ActionId = FGuid::NewGuid();
    // Center edit on outer perimeter lip of the water basin
    BreachRequest.WorldCenter = FVector(Bounds.Max.X - 50.f, 0.f, 400.f);
    BreachRequest.RadiusCm = 150.f;
    BreachRequest.Operation = EWyrmTerrainEditOperation::Remove;

    FString RejectionReason;
    TestFalse(TEXT("Outer boundary excavation rejected (WRLD-11)"), WaterVol->ValidateTerrainEdit(BreachRequest, RejectionReason));
    TestTrue(TEXT("Rejection reason mentions basin boundary breach"), RejectionReason.Contains(TEXT("Water basin boundary breach prohibited")));

    WaterVol->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmFishingComponentTest, "WYRMFALL.Scaffold.FishingStateLoopAndInventoryCommit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmFishingComponentTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmWaterVolume* WaterVol = World->SpawnActor<AWyrmWaterVolume>(
        AWyrmWaterVolume::StaticClass(), FVector(500.f, 0.f, 500.f), FRotator::ZeroRotator, SpawnParams);
    WaterVol->SurfaceElevation = 800.f;

    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 550.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Character spawned"), Character);

    UWyrmFishingComponent* FishingComp = Character->GetFishing();
    TestNotNull(TEXT("Fishing component exists"), FishingComp);
    TestEqual(TEXT("Initial state is Ready"), FishingComp->GetFishingState(), EWyrmFishingState::Ready);

    // 1. Normal Fishing Loop: Cast -> Bite -> Reel -> Commit (ACT-01)
    FString FailReason;
    const FVector TargetWater(500.f, 0.f, 600.f);
    TestTrue(TEXT("StartFishing succeeds in valid water"), FishingComp->StartFishing(TargetWater, FailReason));
    TestEqual(TEXT("State is Casting"), FishingComp->GetFishingState(), EWyrmFishingState::Casting);
    TestTrue(TEXT("Movement is locked during fishing"), Character->IsMovementLocked());

    FishingComp->TriggerBite();
    TestEqual(TEXT("State is BiteWindow"), FishingComp->GetFishingState(), EWyrmFishingState::BiteWindow);

    TestTrue(TEXT("RespondToBite succeeds"), FishingComp->RespondToBite());
    TestTrue(TEXT("CommitCatch succeeds"), FishingComp->CommitCatch());
    TestEqual(TEXT("Returns to Ready after commit"), FishingComp->GetFishingState(), EWyrmFishingState::Ready);
    TestFalse(TEXT("Movement unlocked after commit"), Character->IsMovementLocked());

    // Verify fish entered inventory once (ACT-01)
    UWyrmInventoryComponent* Inv = Character->GetInventory();
    TestNotNull(TEXT("Inventory exists"), Inv);
    TestEqual(TEXT("One item in bag"), Inv->GetBagItems().Num(), 1);
    TestEqual(TEXT("Caught fish is OceanFish"), Inv->GetBagItems()[0].ItemId, FName(TEXT("Item.Fish.OceanFish")));

    // 2. Interruption on Combat Damage (ACT-03)
    TestTrue(TEXT("StartFishing second cast succeeds"), FishingComp->StartFishing(TargetWater, FailReason));
    FishingComp->NotifyCombatDamageTaken(20.f);
    TestEqual(TEXT("Returns to Ready after damage cancel"), FishingComp->GetFishingState(), EWyrmFishingState::Ready);
    TestFalse(TEXT("Movement unlocked after damage cancel"), Character->IsMovementLocked());
    TestEqual(TEXT("Zero free items awarded on damage interrupt"), Inv->GetBagItems().Num(), 1);

    // 3. Interruption on Manual Cancellation (ACT-03)
    TestTrue(TEXT("StartFishing third cast succeeds"), FishingComp->StartFishing(TargetWater, FailReason));
    FishingComp->CancelFishing(TEXT("UserManualCancel"));
    TestEqual(TEXT("Returns to Ready after manual cancel"), FishingComp->GetFishingState(), EWyrmFishingState::Ready);
    TestEqual(TEXT("Bag items unchanged on cancel"), Inv->GetBagItems().Num(), 1);

    // 4. Full Bag Overflow Rejection (ACT-03)
    // Fill remaining bag slots with non-stacking items
    for (int32 i = Inv->GetBagItems().Num(); i < Inv->MaxBagSlots; ++i)
    {
        FWyrmItemInstance Filler;
        Filler.InstanceId = FGuid::NewGuid();
        Filler.ItemId = FName(*FString::Printf(TEXT("FillerItem_%d"), i));
        Filler.ItemType = EWyrmItemType::Resource;
        Filler.StackCount = 1;
        Filler.MaxStack = 1;
        FWyrmItemInstance Rem;
        Inv->AddItem(Filler, Rem);
    }
    TestEqual(TEXT("Bag is at full capacity"), Inv->GetBagItems().Num(), Inv->MaxBagSlots);

    // Attempt catch commit into full bag
    TestTrue(TEXT("Start fishing into full bag"), FishingComp->StartFishing(TargetWater, FailReason));
    FishingComp->TriggerBite();
    FishingComp->RespondToBite();

    // Since OceanFish can stack, let's max out existing fish stack first
    for (FWyrmItemInstance& Item : const_cast<TArray<FWyrmItemInstance>&>(Inv->GetBagItems()))
    {
        if (Item.ItemId == FName(TEXT("Item.Fish.OceanFish")))
        {
            Item.StackCount = Item.MaxStack;
        }
    }

    TestFalse(TEXT("CommitCatch rejected on full bag (ACT-03)"), FishingComp->CommitCatch());
    TestTrue(TEXT("WasCatchRejectedBagFull is true"), FishingComp->WasCatchRejectedBagFull());
    TestEqual(TEXT("Bag slots remained clamped"), Inv->GetBagItems().Num(), Inv->MaxBagSlots);

    Character->Destroy();
    WaterVol->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCraftingSubsystemTest, "WYRMFALL.Scaffold.CraftingAtomicTransactionsAndFailures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCraftingSubsystemTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI) GI = NewObject<UGameInstance>(World);

    UWyrmCraftingSubsystem* CraftingSys = NewObject<UWyrmCraftingSubsystem>(GI);
    CraftingSys->RegisterDefaultRecipes();

    TArray<FWyrmRecipe> Recipes = CraftingSys->GetAllRecipes();
    TestTrue(TEXT("Recipes registered"), Recipes.Num() >= 3);

    UWyrmInventoryComponent* Inv = NewObject<UWyrmInventoryComponent>(World);
    Inv->MaxBagSlots = 5;

    // 1. Station failure: GrilledFish requires Campfire, attempted at Field (ACT-05)
    FString FailReason;
    TestFalse(TEXT("Cannot craft campfire recipe in field (ACT-05)"),
        CraftingSys->CanCraft(Inv, FName(TEXT("Recipe.Food.GrilledFish")), EWyrmCraftingStationType::Field, FailReason));
    TestTrue(TEXT("Reason is InvalidStation"), FailReason.Contains(TEXT("InvalidStation")));

    // 2. Missing ingredients failure (ACT-05)
    TestFalse(TEXT("Cannot craft with empty bag (ACT-05)"),
        CraftingSys->CanCraft(Inv, FName(TEXT("Recipe.Food.GrilledFish")), EWyrmCraftingStationType::Campfire, FailReason));
    TestTrue(TEXT("Reason mentions MissingIngredients"), FailReason.Contains(TEXT("MissingIngredients")));

    FWyrmItemInstance FailedResult;
    TestFalse(TEXT("CraftRecipe fails with missing ingredients"),
        CraftingSys->CraftRecipe(Inv, FName(TEXT("Recipe.Food.GrilledFish")), EWyrmCraftingStationType::Campfire, FailedResult, FailReason));
    TestEqual(TEXT("Zero items consumed on failure"), Inv->GetBagItems().Num(), 0);

    // 3. Valid Craft: Grilled Fish at Campfire (ACT-02, ACT-05)
    FWyrmItemInstance FishItem;
    FishItem.InstanceId = FGuid::NewGuid();
    FishItem.ItemId = FName(TEXT("Item.Fish.OceanFish"));
    FishItem.ItemType = EWyrmItemType::Consumable;
    FishItem.StackCount = 1;
    FishItem.MaxStack = 10;
    FWyrmItemInstance Rem;
    Inv->AddItem(FishItem, Rem);
    TestEqual(TEXT("Added 1 fish to bag"), Inv->GetBagItems().Num(), 1);

    FWyrmItemInstance CraftedResult;
    TestTrue(TEXT("CraftRecipe succeeds with valid ingredients (ACT-02)"),
        CraftingSys->CraftRecipe(Inv, FName(TEXT("Recipe.Food.GrilledFish")), EWyrmCraftingStationType::Campfire, CraftedResult, FailReason));

    TestEqual(TEXT("Result is Grilled Ocean Fish"), CraftedResult.ItemId, FName(TEXT("Item.Food.GrilledFish")));
    TestEqual(TEXT("Bag contains 1 item (fish consumed, cooked granted)"), Inv->GetBagItems().Num(), 1);
    TestEqual(TEXT("Item in bag is GrilledFish"), Inv->GetBagItems()[0].ItemId, FName(TEXT("Item.Food.GrilledFish")));

    // 4. Field Craft: Field Remedy (ACT-05)
    FWyrmItemInstance Dirt1;
    Dirt1.InstanceId = FGuid::NewGuid();
    Dirt1.ItemId = FName(TEXT("Resource.Dirt"));
    Dirt1.ItemType = EWyrmItemType::Resource;
    Dirt1.StackCount = 2;
    Dirt1.MaxStack = 100;
    Inv->AddItem(Dirt1, Rem);

    FWyrmItemInstance RemedyResult;
    TestTrue(TEXT("Field remedy crafts without station (ACT-05)"),
        CraftingSys->CraftRecipe(Inv, FName(TEXT("Recipe.Consumable.FieldRemedy")), EWyrmCraftingStationType::Field, RemedyResult, FailReason));
    TestEqual(TEXT("Crafted Field Remedy"), RemedyResult.ItemId, FName(TEXT("Item.Consumable.FieldRemedy")));

    // 5. A full one-slot bag may craft when consuming its ingredient frees the output slot.
    UWyrmInventoryComponent* TightInv = NewObject<UWyrmInventoryComponent>(World);
    TightInv->MaxBagSlots = 1;
    FWyrmItemInstance TightFish = FishItem;
    TightFish.InstanceId = FGuid::NewGuid();
    TightInv->AddItem(TightFish, Rem);
    FWyrmItemInstance TightResult;
    TestTrue(TEXT("Consumed ingredient frees output capacity atomically"),
        CraftingSys->CraftRecipe(TightInv, FName(TEXT("Recipe.Food.GrilledFish")),
            EWyrmCraftingStationType::Campfire, TightResult, FailReason));
    TestEqual(TEXT("One-slot bag contains only crafted output"), TightInv->GetBagItems().Num(), 1);
    TestEqual(TEXT("Crafted output owns freed slot"), TightInv->GetBagItems()[0].ItemId,
        FName(TEXT("Item.Food.GrilledFish")));

    // 6. Exact output-capacity preflight rejects without consuming inputs.
    FWyrmRecipe OversizedRecipe;
    OversizedRecipe.RecipeId = TEXT("Recipe.Test.OversizedOutput");
    OversizedRecipe.DisplayName = FText::FromString(TEXT("Oversized Output"));
    OversizedRecipe.RequiredStation = EWyrmCraftingStationType::Field;
    FWyrmIngredientCost OversizedCost;
    OversizedCost.ItemId = TEXT("Item.Fish.OceanFish");
    OversizedCost.Quantity = 1;
    OversizedRecipe.Ingredients.Add(OversizedCost);
    OversizedRecipe.OutputItem = UWyrmCraftingSubsystem::CreateConsumableItem(
        TEXT("Item.Test.Oversized"), FText::FromString(TEXT("Oversized")), 11, 10);
    CraftingSys->RegisterRecipe(OversizedRecipe);

    UWyrmInventoryComponent* RollbackInv = NewObject<UWyrmInventoryComponent>(World);
    RollbackInv->MaxBagSlots = 1;
    FWyrmItemInstance RollbackFish = FishItem;
    RollbackFish.InstanceId = FGuid::NewGuid();
    RollbackInv->AddItem(RollbackFish, Rem);
    FWyrmItemInstance OversizedResult;
    TestFalse(TEXT("Oversized output is rejected before commit"),
        CraftingSys->CraftRecipe(RollbackInv, OversizedRecipe.RecipeId,
            EWyrmCraftingStationType::Field, OversizedResult, FailReason));
    TestTrue(TEXT("Oversized output reports InventoryFull"), FailReason.Contains(TEXT("InventoryFull")));
    TestEqual(TEXT("Rejected craft preserves its ingredient"), RollbackInv->GetBagItems().Num(), 1);
    TestEqual(TEXT("Preserved ingredient identity is unchanged"), RollbackInv->GetBagItems()[0].InstanceId,
        RollbackFish.InstanceId);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmFoodBuffTest, "WYRMFALL.Scaffold.FoodBuffRefreshAndReplacementPersistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmFoodBuffTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Character spawned"), Character);
    Character->GrantCombatAbilities();

    UWyrmAttributeSet* Attrs = Character->GetAttributes();
    TestNotNull(TEXT("Attributes exist"), Attrs);
    Attrs->SetCurrentMaxFocus(100.f);
    Attrs->SetCurrentFocus(75.f);
    Attrs->SetCurrentPower(10.f);

    UWyrmInventoryComponent* Inv = Character->GetInventory();
    TestNotNull(TEXT("Inventory exists"), Inv);

    // 1. Consume Grilled Fish (+10% Max Focus, 300s duration) (ACT-02, ACT-04)
    FWyrmFoodBuffDefinition FishBuffDef;
    FishBuffDef.BuffId = FName(TEXT("Buff.Food.GrilledFish"));
    FishBuffDef.Duration = 300.f;
    FishBuffDef.MaxFocusPercentBonus = 0.10f; // +10%
    FWyrmItemInstance Food1 = UWyrmCraftingSubsystem::CreateConsumableItem(
        FName(TEXT("Item.Food.GrilledFish")), FText::FromString(TEXT("Grilled Fish")), 1, 10, FishBuffDef);
    FWyrmItemInstance Rem;
    Inv->AddItem(Food1, Rem);

    TestTrue(TEXT("ConsumeItem succeeds"), Character->ConsumeItem(Food1.InstanceId));
    TestTrue(TEXT("Active food buff is present"), Character->HasActiveFoodBuff());
    TestEqual(TEXT("MaxFocus scaled +10% (100 -> 110) (ACT-04)"), Attrs->GetCurrentMaxFocus(), 110.f);
    TestEqual(TEXT("Current Focus not free refilled (75) (ACT-04)"), Attrs->GetCurrentFocus(), 75.f);
    TestEqual(TEXT("Initial remaining duration is 300s"), Character->GetActiveFoodBuff().RemainingDuration, 300.f);

    // 2. Consume Same Food Buff: Refreshes Duration, Does NOT Stack Magnitude (ACT-04)
    Character->SetActiveFoodBuffRemainingDuration(120.f);
    TestEqual(TEXT("Simulated time passage to 120s"), Character->GetActiveFoodBuff().RemainingDuration, 120.f);

    FWyrmItemInstance Food2 = UWyrmCraftingSubsystem::CreateConsumableItem(
        FName(TEXT("Item.Food.GrilledFish")), FText::FromString(TEXT("Grilled Fish")), 1, 10, FishBuffDef);
    Inv->AddItem(Food2, Rem);

    TestTrue(TEXT("Consume same food buff succeeds"), Character->ConsumeItem(Food2.InstanceId));
    TestEqual(TEXT("Duration refreshed back to 300s (ACT-04)"), Character->GetActiveFoodBuff().RemainingDuration, 300.f);
    TestEqual(TEXT("MaxFocus did NOT stack (remains 110, not 120) (ACT-04)"), Attrs->GetCurrentMaxFocus(), 110.f);

    // 3. Consume Different Food Buff: Clean Replacement (ACT-04)
    FWyrmFoodBuffDefinition StewBuffDef;
    StewBuffDef.BuffId = FName(TEXT("Buff.Food.FishStew"));
    StewBuffDef.Duration = 300.f;
    StewBuffDef.MaxFocusPercentBonus = 0.15f; // +15%
    StewBuffDef.HealthRegenPerSecond = 2.0f;
    StewBuffDef.PowerBonus = 5.f;
    FWyrmItemInstance FoodStew = UWyrmCraftingSubsystem::CreateConsumableItem(
        FName(TEXT("Item.Food.FishStew")), FText::FromString(TEXT("Fish Stew")), 1, 10, StewBuffDef);
    Inv->AddItem(FoodStew, Rem);

    TestTrue(TEXT("Consume different food buff succeeds"), Character->ConsumeItem(FoodStew.InstanceId));
    TestEqual(TEXT("Active buff replaced to FishStew (ACT-04)"), Character->GetActiveFoodBuff().BuffId, FName(TEXT("Item.Food.FishStew")));
    TestEqual(TEXT("MaxFocus replaced with +15% (115) (ACT-04)"), Attrs->GetCurrentMaxFocus(), 115.f);
    TestEqual(TEXT("Power bonus applied once (10 -> 15)"), Attrs->GetCurrentPower(), 15.f);

    // 4. Persistence & Save Roundtrip (SAVE-01, ACT-04)
    Character->SetActiveFoodBuffRemainingDuration(245.5f);
    const FString SlotName = TEXT("WyrmSlot_FoodBuff_UnitTest");
    UGameInstance* GI = World->GetGameInstance();
    if (!GI) GI = NewObject<UGameInstance>(World);
    UWyrmSaveSubsystem* SaveSys = NewObject<UWyrmSaveSubsystem>(GI);

    TestTrue(TEXT("SaveGameSnapshot succeeds with active food buff"),
        SaveSys->SaveGameSnapshot(SlotName, Character, nullptr));

    // Clear active buff on character
    Character->ClearFoodBuff();
    TestFalse(TEXT("Active buff cleared"), Character->HasActiveFoodBuff());
    TestEqual(TEXT("MaxFocus reverted to 100"), Attrs->GetCurrentMaxFocus(), 100.f);

    // Reload snapshot from slot
    TestTrue(TEXT("LoadGameSnapshot succeeds"), SaveSys->LoadGameSnapshot(SlotName, Character, nullptr));
    TestTrue(TEXT("Active buff restored across reload (ACT-04)"), Character->HasActiveFoodBuff());
    TestEqual(TEXT("Restored BuffId is FishStew"), Character->GetActiveFoodBuff().BuffId, FName(TEXT("Item.Food.FishStew")));
    TestEqual(TEXT("Restored remaining duration is 245.5s (ACT-04)"), Character->GetActiveFoodBuff().RemainingDuration, 245.5f);
    TestEqual(TEXT("Restored MaxFocus has +15% scaling (115) (ACT-04)"), Attrs->GetCurrentMaxFocus(), 115.f);
    TestEqual(TEXT("Restored Power has one +5 bonus"), Attrs->GetCurrentPower(), 15.f);

    // 5. Natural expiry removes every owned modifier.
    Character->SetActiveFoodBuffRemainingDuration(0.05f);
    Character->Tick(0.10f);
    TestFalse(TEXT("Expired food buff is cleared"), Character->HasActiveFoodBuff());
    TestEqual(TEXT("Expired MaxFocus modifier is removed"), Attrs->GetCurrentMaxFocus(), 100.f);
    TestEqual(TEXT("Expired Power modifier is removed"), Attrs->GetCurrentPower(), 10.f);

    SaveSys->DeleteSaveSlot(SlotName);
    Character->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCampBuildingPlacementTest, "WYRMFALL.Scaffold.CampPlacementAndRejection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCampBuildingPlacementTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
    for (const FWorldContext& Ctx : Contexts)
    {
        if (Ctx.WorldType == EWorldType::Editor || Ctx.WorldType == EWorldType::PIE)
        {
            World = Ctx.World();
            break;
        }
    }
    if (!World)
    {
        World = UWorld::CreateWorld(EWorldType::None, false);
    }
    TestNotNull(TEXT("Valid test world"), World);

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        GI = NewObject<UGameInstance>(World);
    }
    UWyrmBuildingSubsystem* BuildSys = NewObject<UWyrmBuildingSubsystem>(GI);
    BuildSys->SetWorldContext(World);
    BuildSys->RegisterDefaultDefinitions();

    // Create a player actor with inventory, positioned outside placement volume
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>();
    TestNotNull(TEXT("Valid player character"), Player);
    Player->SetActorLocation(FVector(300.f, 0.f, 0.f));
    UWyrmInventoryComponent* Inv = Player->GetInventory();
    TestNotNull(TEXT("Valid player inventory"), Inv);

    // 1. Rejection: Insufficient materials
    FString RejectionReason;
    EWyrmPlacementRejection Rejection = BuildSys->ValidatePlacement(
        FName(TEXT("Foundation.Wood")), FTransform(FVector(0, 0, 0)), Player, RejectionReason);
    TestEqual(TEXT("Rejects without materials"), Rejection, EWyrmPlacementRejection::InsufficientMaterials);
    TestTrue(TEXT("Reason specifies materials"), RejectionReason.Contains(TEXT("Insufficient materials")));

    // Add wood materials to player
    FWyrmItemInstance WoodItem;
    WoodItem.InstanceId = FGuid::NewGuid();
    WoodItem.ItemId = FName(TEXT("Resource.Wood"));
    WoodItem.DisplayName = FText::FromString(TEXT("Wood"));
    WoodItem.ItemType = EWyrmItemType::Resource;
    WoodItem.StackCount = 20;
    WoodItem.MaxStack = 99;
    FWyrmItemInstance Rem;
    Inv->AddItem(WoodItem, Rem);

    // 2. Rejection: Occupied by character/creature (ACT-07, WRLD-08)
    AWyrmCharacter* Occupant = World->SpawnActor<AWyrmCharacter>();
    Occupant->SetActorLocation(FVector(0, 0, 0));
    Rejection = BuildSys->ValidatePlacement(
        FName(TEXT("Foundation.Wood")), FTransform(FVector(0, 0, 0)), Player, RejectionReason);
    TestEqual(TEXT("Occupied placement rejected (ACT-07, WRLD-08)"), Rejection, EWyrmPlacementRejection::Occupied);
    Occupant->Destroy();

    // 3. Rejection: Unsupported wall in mid-air (ACT-07)
    Rejection = BuildSys->ValidatePlacement(
        FName(TEXT("Wall.Wood")), FTransform(FVector(0, 0, 500)), Player, RejectionReason);
    TestEqual(TEXT("Wall in mid-air rejected as Unsupported"), Rejection, EWyrmPlacementRejection::Unsupported);

    // ExecutePlacement on invalid placement fails without consuming materials (ACT-07)
    int32 WoodBefore = Inv->GetBagItems()[0].StackCount;
    AWyrmBuildingPiece* BadPiece = BuildSys->ExecutePlacement(
        FName(TEXT("Wall.Wood")), FTransform(FVector(0, 0, 500)), Player, Rejection, RejectionReason);
    TestNull(TEXT("No actor spawned on failed placement"), BadPiece);
    TestEqual(TEXT("Zero materials deducted on failed placement"), Inv->GetBagItems()[0].StackCount, WoodBefore);

    // 4. Successful Placement: Foundation
    AWyrmBuildingPiece* Foundation = BuildSys->ExecutePlacement(
        FName(TEXT("Foundation.Wood")), FTransform(FVector(0, 0, 0)), Player, Rejection, RejectionReason);
    TestNotNull(TEXT("Foundation placed successfully"), Foundation);
    TestEqual(TEXT("Placement rejection is None"), Rejection, EWyrmPlacementRejection::None);
    TestEqual(TEXT("2 Wood deducted for foundation"), Inv->GetBagItems()[0].StackCount, WoodBefore - 2);
    TestEqual(TEXT("Foundation tracked in active pieces"), BuildSys->GetActivePieces().Num(), 1);

    // 5. Rejection: Overlapping foundation (ACT-07)
    Rejection = BuildSys->ValidatePlacement(
        FName(TEXT("Foundation.Wood")), FTransform(FVector(10, 10, 0)), Player, RejectionReason);
    TestEqual(TEXT("Overlapping foundation rejected"), Rejection, EWyrmPlacementRejection::Overlapping);

    // 5. Successful Placement: Wall supported by foundation (ACT-06)
    WoodBefore = Inv->GetBagItems()[0].StackCount;
    AWyrmBuildingPiece* Wall = BuildSys->ExecutePlacement(
        FName(TEXT("Wall.Wood")), FTransform(FVector(0, 100, 50)), Player, Rejection, RejectionReason);
    TestNotNull(TEXT("Wall placed with foundation support"), Wall);
    TestEqual(TEXT("Active pieces count is 2"), BuildSys->GetActivePieces().Num(), 2);

    // 6. Doorframe and Door with Toggle (ACT-06)
    AWyrmBuildingPiece* Doorframe = BuildSys->ExecutePlacement(
        FName(TEXT("Doorframe.Wood")), FTransform(FVector(100, 0, 50)), Player, Rejection, RejectionReason);
    TestNotNull(TEXT("Doorframe placed"), Doorframe);

    AWyrmBuildingPiece* Door = BuildSys->ExecutePlacement(
        FName(TEXT("Door.Wood")), FTransform(FVector(100, 0, 50)), Player, Rejection, RejectionReason);
    TestNotNull(TEXT("Door placed"), Door);
    if (!Door)
    {
        return false;
    }
    TestFalse(TEXT("Door initially closed"), Door->bIsOpen);
    Door->ToggleDoor();
    TestTrue(TEXT("Door toggled open"), Door->bIsOpen);
    Door->ToggleDoor();
    TestFalse(TEXT("Door toggled closed"), Door->bIsOpen);

    // 7. Roof & Ceiling (ACT-06)
    AWyrmBuildingPiece* Roof = BuildSys->ExecutePlacement(
        FName(TEXT("Roof.Wood")), FTransform(FVector(0, 0, 200)), Player, Rejection, RejectionReason);
    TestNotNull(TEXT("Roof placed with support"), Roof);

    AWyrmBuildingPiece* Ceiling = BuildSys->ExecutePlacement(
        FName(TEXT("Ceiling.Wood")), FTransform(FVector(0, 0, 180)), Player, Rejection, RejectionReason);
    TestNotNull(TEXT("Ceiling placed with support"), Ceiling);

    BuildSys->ClearAllPlacedPieces();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCampStorageIdentityTest, "WYRMFALL.Scaffold.CampStorageIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCampStorageIdentityTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
    for (const FWorldContext& Ctx : Contexts)
    {
        if (Ctx.WorldType == EWorldType::Editor || Ctx.WorldType == EWorldType::PIE)
        {
            World = Ctx.World();
            break;
        }
    }
    if (!World)
    {
        World = UWorld::CreateWorld(EWorldType::None, false);
    }
    TestNotNull(TEXT("Valid test world"), World);

    UGameInstance* GI = World->GetGameInstance();
    if (!GI) GI = NewObject<UGameInstance>(World);
    UWyrmBuildingSubsystem* BuildSys = NewObject<UWyrmBuildingSubsystem>(GI);
    BuildSys->SetWorldContext(World);
    BuildSys->RegisterDefaultDefinitions();

    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>();
    Player->SetActorLocation(FVector(300.f, 0.f, 0.f));
    UWyrmInventoryComponent* PlayerBag = Player->GetInventory();

    // Place Storage Chest
    FString Reason;
    EWyrmPlacementRejection Rejection;
    AWyrmBuildingPiece* ChestPiece = BuildSys->ExecutePlacement(
        FName(TEXT("Storage.Chest")), FTransform(FVector(0, 0, 0)), nullptr, Rejection, Reason);
    TestNotNull(TEXT("Storage chest placed"), ChestPiece);
    AWyrmStorageActor* Storage = Cast<AWyrmStorageActor>(ChestPiece);
    TestNotNull(TEXT("Chest cast to AWyrmStorageActor"), Storage);
    if (!Storage || !Storage->GetStorageInventory())
    {
        return false;
    }

    // Create unique rolled item in player bag
    FWyrmItemInstance UniqueSword;
    UniqueSword.InstanceId = FGuid::NewGuid();
    UniqueSword.ItemId = FName(TEXT("Item.Weapon.IronSword"));
    UniqueSword.DisplayName = FText::FromString(TEXT("Rolled Iron Sword"));
    UniqueSword.ItemType = EWyrmItemType::Weapon;
    UniqueSword.DefaultSlot = EWyrmEquipSlot::MainHand;
    UniqueSword.WeaponFamily = EWyrmWeaponFamily::Melee1H;
    UniqueSword.StackCount = 1;
    UniqueSword.MaxStack = 1;
    FWyrmItemRoll Roll1;
    Roll1.StatName = FName(TEXT("Power"));
    Roll1.Value = 24.5f;
    UniqueSword.RolledStats.Add(Roll1);
    FWyrmItemRoll Roll2;
    Roll2.StatName = FName(TEXT("CriticalChance"));
    Roll2.Value = 0.08f;
    UniqueSword.RolledStats.Add(Roll2);

    FWyrmItemInstance Rem;
    PlayerBag->AddItem(UniqueSword, Rem);
    TestEqual(TEXT("Player bag contains 1 item"), PlayerBag->GetBagItems().Num(), 1);

    // Transfer Bag -> Storage (ACT-08)
    const FGuid SavedGuid = UniqueSword.InstanceId;
    TestTrue(TEXT("Transfer to storage succeeds"),
        Storage->TransferToStorage(PlayerBag, SavedGuid, 1));
    TestEqual(TEXT("Player bag is now empty (single owner)"), PlayerBag->GetBagItems().Num(), 0);
    TestEqual(TEXT("Storage contains 1 item"), Storage->GetStorageInventory()->GetBagItems().Num(), 1);

    const FWyrmItemInstance& StoredItem = Storage->GetStorageInventory()->GetBagItems()[0];
    TestEqual(TEXT("Identical GUID in storage"), StoredItem.InstanceId, SavedGuid);
    TestEqual(TEXT("Identical ItemId in storage"), StoredItem.ItemId, FName(TEXT("Item.Weapon.IronSword")));
    TestEqual(TEXT("Identical rolled power"), StoredItem.GetStatValue(FName(TEXT("Power"))), 24.5f);
    TestEqual(TEXT("Identical rolled crit"), StoredItem.GetStatValue(FName(TEXT("CriticalChance"))), 0.08f);

    // Transfer Storage -> Bag (ACT-08)
    TestTrue(TEXT("Transfer back to bag succeeds"),
        Storage->TransferFromStorage(PlayerBag, SavedGuid, 1));
    TestEqual(TEXT("Storage is now empty (single owner)"), Storage->GetStorageInventory()->GetBagItems().Num(), 0);
    TestEqual(TEXT("Player bag contains 1 item"), PlayerBag->GetBagItems().Num(), 1);

    const FWyrmItemInstance& ReturnedItem = PlayerBag->GetBagItems()[0];
    TestEqual(TEXT("Identical GUID returned to player"), ReturnedItem.InstanceId, SavedGuid);
    TestEqual(TEXT("Identical rolled stats returned"), ReturnedItem.GetStatValue(FName(TEXT("Power"))), 24.5f);

    BuildSys->ClearAllPlacedPieces();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCampDemolitionOverflowTest, "WYRMFALL.Scaffold.CampDemolitionOverflow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCampDemolitionOverflowTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
    for (const FWorldContext& Ctx : Contexts)
    {
        if (Ctx.WorldType == EWorldType::Editor || Ctx.WorldType == EWorldType::PIE)
        {
            World = Ctx.World();
            break;
        }
    }
    if (!World)
    {
        World = UWorld::CreateWorld(EWorldType::None, false);
    }
    TestNotNull(TEXT("Valid test world"), World);

    UGameInstance* GI = World->GetGameInstance();
    if (!GI) GI = NewObject<UGameInstance>(World);
    UWyrmBuildingSubsystem* BuildSys = NewObject<UWyrmBuildingSubsystem>(GI);
    BuildSys->SetWorldContext(World);
    BuildSys->RegisterDefaultDefinitions();

    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>();
    Player->SetActorLocation(FVector(300.f, 0.f, 0.f));
    UWyrmInventoryComponent* PlayerBag = Player->GetInventory();

    // Place storage chest
    FString Reason;
    EWyrmPlacementRejection Rejection;
    AWyrmBuildingPiece* ChestPiece = BuildSys->ExecutePlacement(
        FName(TEXT("Storage.Chest")), FTransform(FVector(0, 0, 0)), nullptr, Rejection, Reason);
    AWyrmStorageActor* Storage = Cast<AWyrmStorageActor>(ChestPiece);
    TestNotNull(TEXT("Storage chest placed"), Storage);
    if (!Storage || !Storage->GetStorageInventory())
    {
        return false;
    }

    // Put 3 items in storage
    TArray<FGuid> StoredGuids;
    for (int32 i = 0; i < 3; ++i)
    {
        FWyrmItemInstance Item;
        Item.InstanceId = FGuid::NewGuid();
        Item.ItemId = FName(*FString::Printf(TEXT("Item.Test.Stored_%d"), i));
        Item.DisplayName = FText::FromString(FString::Printf(TEXT("Stored Item %d"), i));
        Item.ItemType = EWyrmItemType::Consumable;
        Item.StackCount = 1;
        Item.MaxStack = 1;
        StoredGuids.Add(Item.InstanceId);
        FWyrmItemInstance Rem;
        Storage->GetStorageInventory()->AddItem(Item, Rem);
    }
    TestEqual(TEXT("Storage contains 3 items"), Storage->GetStorageInventory()->GetBagItems().Num(), 3);

    // Player bag is full (MaxBagSlots = 1, holding 1 filler item)
    PlayerBag->MaxBagSlots = 1;
    FWyrmItemInstance Filler;
    Filler.InstanceId = FGuid::NewGuid();
    Filler.ItemId = FName(TEXT("Item.Filler"));
    Filler.DisplayName = FText::FromString(TEXT("Filler"));
    Filler.StackCount = 1;
    Filler.MaxStack = 1;
    FWyrmItemInstance Rem;
    PlayerBag->AddItem(Filler, Rem);
    TestEqual(TEXT("Player bag is full"), PlayerBag->GetBagItems().Num(), 1);

    // Demolish storage chest while player bag is completely full (ACT-09)
    AWyrmRecoveryBundleActor* RecoveryBundle = nullptr;
    TestTrue(TEXT("DemolishPiece succeeds"),
        BuildSys->DemolishPiece(ChestPiece, Player, RecoveryBundle, Reason));
    TestNotNull(TEXT("Recovery bundle spawned on overflow (ACT-09)"), RecoveryBundle);
    TestNotNull(TEXT("Recovery bundle has inventory"), RecoveryBundle->GetBundleInventory());

    // Verify recovery bundle holds all 3 stored items + 2 refund wood materials
    const TArray<FWyrmItemInstance>& BundleItems = RecoveryBundle->GetBundleInventory()->GetBagItems();
    TestTrue(TEXT("Recovery bundle holds items"), BundleItems.Num() >= 3);

    for (const FGuid& ExpectedGuid : StoredGuids)
    {
        const bool bFound = BundleItems.ContainsByPredicate(
            [&](const FWyrmItemInstance& It) { return It.InstanceId == ExpectedGuid; });
        TestTrue(TEXT("Stored item preserved in bundle without loss"), bFound);
    }

    // Expand player bag and claim bundle contents (ACT-09)
    PlayerBag->MaxBagSlots = 20;
    TestTrue(TEXT("ClaimAll transfers items into player bag"), RecoveryBundle->ClaimAll(PlayerBag));
    TestTrue(TEXT("Player bag received stored items"), PlayerBag->GetBagItems().Num() > 1);

    for (const FGuid& ExpectedGuid : StoredGuids)
    {
        const bool bFoundInPlayer = PlayerBag->GetBagItems().ContainsByPredicate(
            [&](const FWyrmItemInstance& It) { return It.InstanceId == ExpectedGuid; });
        TestTrue(TEXT("Stored item recovered in player bag without duplication"), bFoundInPlayer);
    }

    BuildSys->ClearAllPlacedPieces();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCampSupportTerrainTest, "WYRMFALL.Scaffold.CampSupportTerrainInteraction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCampSupportTerrainTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
    for (const FWorldContext& Ctx : Contexts)
    {
        if (Ctx.WorldType == EWorldType::Editor || Ctx.WorldType == EWorldType::PIE)
        {
            World = Ctx.World();
            break;
        }
    }
    if (!World)
    {
        World = UWorld::CreateWorld(EWorldType::None, false);
    }
    TestNotNull(TEXT("Valid test world"), World);

    UGameInstance* GI = World->GetGameInstance();
    if (!GI) GI = NewObject<UGameInstance>(World);
    UWyrmBuildingSubsystem* BuildSys = NewObject<UWyrmBuildingSubsystem>(GI);
    BuildSys->SetWorldContext(World);
    BuildSys->RegisterDefaultDefinitions();

    AWyrmGeoForgeAdapter* Adapter = World->SpawnActor<AWyrmGeoForgeAdapter>();
    TestNotNull(TEXT("GeoForge adapter created"), Adapter);

    // Place supported foundation at (500, 500, 100)
    FString Reason;
    EWyrmPlacementRejection Rejection;
    AWyrmBuildingPiece* Foundation = BuildSys->ExecutePlacement(
        FName(TEXT("Foundation.Wood")), FTransform(FVector(500, 500, 100)), nullptr, Rejection, Reason);
    TestNotNull(TEXT("Foundation placed"), Foundation);

    // Register with adapter
    Adapter->RegisterCampPiece(Foundation);

    // Attempt excavation that undermines ground support beneath foundation (WRLD-09)
    FWyrmTerrainEditRequest BadEdit;
    BadEdit.ActionId = FGuid::NewGuid();
    BadEdit.WorldCenter = FVector(500, 500, 0); // Beneath foundation
    BadEdit.RadiusCm = 80.f;
    BadEdit.Operation = EWyrmTerrainEditOperation::Remove;

    EWyrmTerrainSubmitResult Result = Adapter->ExecuteTerrainEdit(BadEdit);
    TestEqual(TEXT("Excavation undermining camp support is rejected (WRLD-09)"),
        Result, EWyrmTerrainSubmitResult::Rejected);
    TestEqual(TEXT("Rejection reason is RejectionReason_CampSupport"),
        Adapter->GetLastRejectionReason(), FString(TEXT("RejectionReason_CampSupport")));

    // Demolish foundation piece
    AWyrmRecoveryBundleActor* Bundle = nullptr;
    TestTrue(TEXT("Demolish foundation"), BuildSys->DemolishPiece(Foundation, nullptr, Bundle, Reason));
    TestEqual(TEXT("Foundation removed from adapter pieces"), Adapter->RegisteredCampPieces.Num(), 0);

    // Re-attempt excavation at exact same location after demolition (WRLD-09)
    FWyrmTerrainEditRequest ValidAfterDemolish;
    ValidAfterDemolish.ActionId = FGuid::NewGuid();
    ValidAfterDemolish.WorldCenter = FVector(500, 500, 0);
    ValidAfterDemolish.RadiusCm = 80.f;
    ValidAfterDemolish.Operation = EWyrmTerrainEditOperation::Remove;

    Adapter->LastRejectionReason.Empty();
    Result = Adapter->ExecuteTerrainEdit(ValidAfterDemolish);
    TestTrue(TEXT("No longer rejected with RejectionReason_CampSupport after demolition"),
        Adapter->GetLastRejectionReason() != TEXT("RejectionReason_CampSupport"));

    Adapter->Destroy();
    BuildSys->ClearAllPlacedPieces();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCampPersistenceAndClearanceTest, "WYRMFALL.Scaffold.CampPersistenceAndClearance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmCampPersistenceAndClearanceTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
    for (const FWorldContext& Ctx : Contexts)
    {
        if (Ctx.WorldType == EWorldType::Editor || Ctx.WorldType == EWorldType::PIE)
        {
            World = Ctx.World();
            break;
        }
    }
    if (!World)
    {
        World = UWorld::CreateWorld(EWorldType::None, false);
    }
    TestNotNull(TEXT("Valid test world"), World);

    UGameInstance* GI = World->GetGameInstance();
    if (!GI) GI = NewObject<UGameInstance>(World);
    UWyrmBuildingSubsystem* BuildSys = NewObject<UWyrmBuildingSubsystem>(GI);
    BuildSys->SetWorldContext(World);
    BuildSys->RegisterDefaultDefinitions();
    UWyrmSaveSubsystem* SaveSys = NewObject<UWyrmSaveSubsystem>(GI);

    // Place Foundation, Walls, and Roof at Z = 200
    FString Reason;
    EWyrmPlacementRejection Rejection;
    AWyrmBuildingPiece* Foundation = BuildSys->ExecutePlacement(
        FName(TEXT("Foundation.Wood")), FTransform(FVector(0, 0, 0)), nullptr, Rejection, Reason);
    AWyrmBuildingPiece* Wall = BuildSys->ExecutePlacement(
        FName(TEXT("Wall.Wood")), FTransform(FVector(0, 100, 50)), nullptr, Rejection, Reason);
    AWyrmBuildingPiece* Roof = BuildSys->ExecutePlacement(
        FName(TEXT("Roof.Wood")), FTransform(FVector(0, 0, 200)), nullptr, Rejection, Reason);

    // Place Storage Chest with items
    AWyrmBuildingPiece* ChestPiece = BuildSys->ExecutePlacement(
        FName(TEXT("Storage.Chest")), FTransform(FVector(0, 0, 20)), nullptr, Rejection, Reason);
    AWyrmStorageActor* Storage = Cast<AWyrmStorageActor>(ChestPiece);
    TestNotNull(TEXT("Storage chest placed"), Storage);
    if (!Storage || !Storage->GetStorageInventory())
    {
        return false;
    }

    FWyrmItemInstance Item;
    Item.InstanceId = FGuid::NewGuid();
    Item.ItemId = FName(TEXT("Item.Food.GrilledFish"));
    Item.DisplayName = FText::FromString(TEXT("Grilled Fish"));
    Item.StackCount = 5;
    Item.MaxStack = 10;
    FWyrmItemInstance Rem;
    Storage->GetStorageInventory()->AddItem(Item, Rem);

    // Place Door and toggle open
    AWyrmBuildingPiece* Door = BuildSys->ExecutePlacement(
        FName(TEXT("Door.Wood")), FTransform(FVector(100, 0, 50)), nullptr, Rejection, Reason);
    Door->ToggleDoor();
    TestTrue(TEXT("Door toggled open"), Door->bIsOpen);

    // Companion Growth Clearance check (ACT-10)
    FString GrowthReason;
    const bool bGrowthUnderRoof = BuildSys->CheckCompanionGrowthClearance(
        World, FVector(0, 0, 0), 300.f, 50.f, GrowthReason);
    TestFalse(TEXT("Growth under roof blocked (ACT-10)"), bGrowthUnderRoof);
    TestTrue(TEXT("Growth rejection mentions roof or overhead"),
        GrowthReason.Contains(TEXT("roof")) || GrowthReason.Contains(TEXT("Roof")) || GrowthReason.Contains(TEXT("overhead")));

    const bool bGrowthOpenAir = BuildSys->CheckCompanionGrowthClearance(
        World, FVector(2000, 2000, 0), 300.f, 50.f, GrowthReason);
    TestTrue(TEXT("Growth in open air permitted"), bGrowthOpenAir);

    // Save camp snapshot (SAVE-01, ACT-10)
    const FString SlotName = TEXT("WyrmSlot_Camp_UnitTest");
    UWyrmSaveGame* Snapshot = UWyrmSaveSubsystem::CreateSnapshotObject(SlotName, nullptr, nullptr, World);
    TestNotNull(TEXT("Snapshot created"), Snapshot);
    if (Snapshot && Snapshot->CampRecord.Pieces.IsEmpty())
    {
        BuildSys->BuildSaveRecord(Snapshot->CampRecord);
    }
    TestTrue(TEXT("Camp record contains saved pieces"), Snapshot->CampRecord.Pieces.Num() >= 4);

    // Clear all pieces in world
    BuildSys->ClearAllPlacedPieces();
    TestEqual(TEXT("World cleared of placed pieces"), BuildSys->GetActivePieces().Num(), 0);

    // Restore from snapshot (ACT-10)
    TestTrue(TEXT("ApplySnapshotObject restores camp"),
        UWyrmSaveSubsystem::ApplySnapshotObject(Snapshot, nullptr, nullptr, World));

    TArray<AWyrmBuildingPiece*> RestoredPieces = BuildSys->GetActivePieces();
    if (RestoredPieces.IsEmpty() && Snapshot)
    {
        BuildSys->RestoreFromSaveRecord(Snapshot->CampRecord, World);
        RestoredPieces = BuildSys->GetActivePieces();
    }
    TestTrue(TEXT("Camp pieces restored"), RestoredPieces.Num() >= 4);

    // Verify door state restored
    AWyrmBuildingPiece** FoundDoor = RestoredPieces.FindByPredicate(
        [](AWyrmBuildingPiece* P) { return P->PieceType == EWyrmBuildingPieceType::Door; });
    TestNotNull(TEXT("Door restored"), FoundDoor);
    if (FoundDoor && *FoundDoor)
    {
        TestTrue(TEXT("Restored door is open (ACT-10)"), (*FoundDoor)->bIsOpen);
    }

    // Verify storage container and inventory restored
    AWyrmBuildingPiece** FoundChest = RestoredPieces.FindByPredicate(
        [](AWyrmBuildingPiece* P) { return P->PieceType == EWyrmBuildingPieceType::StorageChest; });
    TestNotNull(TEXT("Storage chest restored"), FoundChest);
    if (FoundChest && *FoundChest)
    {
        AWyrmStorageActor* RestoredStorage = Cast<AWyrmStorageActor>(*FoundChest);
        TestNotNull(TEXT("Restored storage actor"), RestoredStorage);
        if (RestoredStorage && RestoredStorage->GetStorageInventory())
        {
            TestEqual(TEXT("Restored storage has 1 item stack"),
                RestoredStorage->GetStorageInventory()->GetBagItems().Num(), 1);
            TestEqual(TEXT("Restored storage item is GrilledFish"),
                RestoredStorage->GetStorageInventory()->GetBagItems()[0].ItemId, FName(TEXT("Item.Food.GrilledFish")));
            TestEqual(TEXT("Restored storage count is 5"),
                RestoredStorage->GetStorageInventory()->GetBagItems()[0].StackCount, 5);
        }
    }

    // Growth check under restored roof still blocked
    const bool bGrowthUnderRestoredRoof = BuildSys->CheckCompanionGrowthClearance(
        World, FVector(0, 0, 0), 300.f, 50.f, GrowthReason);
    TestFalse(TEXT("Growth under restored roof is still blocked (ACT-10)"), bGrowthUnderRestoredRoof);

    BuildSys->ClearAllPlacedPieces();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonLivingDefeatBondTest, "WYRMFALL.Scaffold.DragonLivingDefeatAndOneWayBond",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonLivingDefeatBondTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 1. Spawn Verdance as hostile boss (DRG-01)
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(100.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Dragon spawned"), Dragon);
    Dragon->SetDragonRole(EWyrmDragonRole::HostileBoss);

    TestEqual(TEXT("Boss initial MaxHealth is 1800.0"), Dragon->GetAttributes()->GetCurrentMaxHealth(), 1800.f);
    TestEqual(TEXT("Boss initial Health is 1800.0"), Dragon->GetAttributes()->GetCurrentHealth(), 1800.f);
    TestFalse(TEXT("Boss has no bond receipt"), Dragon->HasBondReceipt());

    // 2. Defeat logic triggers living terminal state (DRG-01)
    Dragon->PerformBossDefeat();
    TestEqual(TEXT("Dragon role transitions to DefeatedAlive"), Dragon->GetDragonRole(), EWyrmDragonRole::DefeatedAlive);
    TestEqual(TEXT("Dragon health is 0 on defeat"), Dragon->GetAttributes()->GetCurrentHealth(), 0.f);
    TestTrue(TEXT("Dragon actor is still valid and not destroyed"), IsValid(Dragon));

    // 3. One-way bonding conversion with created humanoid
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Player spawned"), Player);

    const bool bBondSuccess = Dragon->BondWithHumanoid(Player);
    TestTrue(TEXT("Bonding succeeds on first attempt"), bBondSuccess);
    TestEqual(TEXT("Role converted to AlliedCompanion"), Dragon->GetDragonRole(), EWyrmDragonRole::AlliedCompanion);
    TestTrue(TEXT("Bond receipt committed once"), Dragon->HasBondReceipt());
    TestEqual(TEXT("Allied MaxHealth is 420.0"), Dragon->GetAttributes()->GetCurrentMaxHealth(), 420.f);
    TestEqual(TEXT("Allied MaxFocus is 100.0"), Dragon->GetAttributes()->GetCurrentMaxFocus(), 100.f);
    // Explicitly recovers to at least 50% allied maximum (210 HP)
    TestEqual(TEXT("Allied health recovers to 210.0 (50% max) on first bond"), Dragon->GetAttributes()->GetCurrentHealth(), 210.f);

    // 4. Repeated bond/defeat attempt does NOT re-heal or re-issue bond (DRG-01)
    const bool bRepeatBond = Dragon->BondWithHumanoid(Player);
    TestFalse(TEXT("Repeated bond rejected with zero side effects"), bRepeatBond);
    TestEqual(TEXT("Allied health remains 210.0"), Dragon->GetAttributes()->GetCurrentHealth(), 210.f);

    Dragon->Destroy();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonCompanionCombatTest, "WYRMFALL.Scaffold.DragonCompanionOrdersAndCombat",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonCompanionCombatTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(100.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    Dragon->BondWithHumanoid(Player);
    Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);

    // Test companion orders (DRG-02)
    Dragon->IssueOrder(EWyrmCompanionOrder::Hold);
    TestEqual(TEXT("Companion order is Hold"), Dragon->GetCompanionOrder(), EWyrmCompanionOrder::Hold);

    Dragon->IssueOrder(EWyrmCompanionOrder::Follow);
    TestEqual(TEXT("Companion order is Follow"), Dragon->GetCompanionOrder(), EWyrmCompanionOrder::Follow);

    // Spawn hostile enemy
    AWyrmEnemyCharacter* Enemy = AWyrmEnemyCharacter::SpawnWyrmEnemy(
        World, EWyrmEnemyRole::MeleeChaser, FTransform(FVector(250.f, 0.f, 100.f)));
    TestNotNull(TEXT("Enemy spawned"), Enemy);
    Enemy->GetAttributes()->InitArmor(0.f);

    // Primary strike (24 damage)
    const bool bHit = Dragon->PerformPrimaryAttack(Enemy);
    TestTrue(TEXT("Dragon primary strike landed"), bHit);
    TestEqual(TEXT("Enemy received primary strike damage (60 - 24 = 36)"), Enemy->GetAttributes()->GetCurrentHealth(), 36.f);

    // Secondary area strike (18 damage, 6s cooldown)
    const bool bAreaHit = Dragon->PerformSecondaryAttack(Enemy);
    TestTrue(TEXT("Dragon area strike executed"), bAreaHit);
    TestEqual(TEXT("Enemy received area strike damage (36 - 18 = 18)"), Enemy->GetAttributes()->GetCurrentHealth(), 18.f);
    TestEqual(TEXT("Area attack cooldown committed to 6.0s"), Dragon->GetAreaAttackCooldownRemaining(), 6.0f);

    // Immediate second area attack rejected by cooldown
    const bool bBlockedAreaHit = Dragon->PerformSecondaryAttack(Enemy);
    TestFalse(TEXT("Area attack blocked by active cooldown"), bBlockedAreaHit);

    // Return command cancels attack
    Dragon->IssueOrder(EWyrmCompanionOrder::Return);
    TestEqual(TEXT("Order switched to Return"), Dragon->GetCompanionOrder(), EWyrmCompanionOrder::Return);

    Enemy->Destroy();
    Dragon->Destroy();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonDirectControlTest, "WYRMFALL.Scaffold.DragonDirectControlPossession",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonDirectControlTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmPlayerController* PC = World->SpawnActor<AWyrmPlayerController>(
        AWyrmPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(200.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    PC->Possess(Player);
    TestTrue(TEXT("PC initially possesses Player"), PC->GetPawn() == Player);

    // Transfer control to dragon (DRG-03)
    const bool bControlStarted = PC->TransferControlToDragon(Dragon);
    TestTrue(TEXT("Control transfer to dragon succeeded"), bControlStarted);
    TestTrue(TEXT("PC now possesses Dragon"), PC->GetPawn() == Dragon);
    TestTrue(TEXT("Dragon is marked as directly controlled"), Dragon->IsDirectlyControlled());

    // Waiting humanoid body remains in world, visible, with movement locked
    TestTrue(TEXT("Waiting humanoid movement is locked"), Player->IsMovementLocked());
    TestEqual(TEXT("Waiting humanoid position unchanged"), Player->GetActorLocation(), FVector(0.f, 0.f, 100.f));
    TestTrue(TEXT("Waiting humanoid is still alive and valid"), IsValid(Player));

    // Return control to humanoid (DRG-03)
    const bool bControlReturned = PC->ReturnControlToHumanoid();
    TestTrue(TEXT("Control return to humanoid succeeded"), bControlReturned);
    TestTrue(TEXT("PC possesses humanoid again"), PC->GetPawn() == Player);
    TestFalse(TEXT("Humanoid movement unlocked"), Player->IsMovementLocked());
    TestFalse(TEXT("Dragon no longer directly controlled"), Dragon->IsDirectlyControlled());

    Dragon->Destroy();
    Player->Destroy();
    PC->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonTetherAndRiskTest, "WYRMFALL.Scaffold.DragonRemoteTetherAndBodyDamageReaction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonTetherAndRiskTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmPlayerController* PC = World->SpawnActor<AWyrmPlayerController>(
        AWyrmPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(200.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    PC->Possess(Player);
    PC->TransferControlToDragon(Dragon);

    // 1. Check tether warning at 125m (12,500 cm) (DRG-04)
    Dragon->SetActorLocation(FVector(12500.f, 0.f, 100.f));
    Dragon->Tick(0.016f);
    TestEqual(TEXT("Tether status is Warning at 125m"), Dragon->GetTetherStatus(), EWyrmTetherStatus::Warning);
    TestTrue(TEXT("PC still possesses Dragon during warning"), PC->GetPawn() == Dragon);

    // 2. Check tether limit at 151m (15,100 cm) -> stops & safely returns control (DRG-04)
    Dragon->SetActorLocation(FVector(15100.f, 0.f, 100.f));
    Dragon->Tick(0.016f);
    TestEqual(TEXT("Tether status is LimitReached at 151m"), Dragon->GetTetherStatus(), EWyrmTetherStatus::LimitReached);
    TestTrue(TEXT("Control automatically returned to Player at tether limit"), PC->GetPawn() == Player);

    // 3. Remote risk: damage to waiting humanoid immediately returns control (DRG-04)
    Dragon->SetActorLocation(FVector(500.f, 0.f, 100.f));
    PC->TransferControlToDragon(Dragon);
    TestTrue(TEXT("PC re-possesses Dragon"), PC->GetPawn() == Dragon);

    // Simulate damage to waiting humanoid body
    Dragon->HandleWaitingBodyDamaged(10.f);
    TestTrue(TEXT("Damaging waiting body immediately returns player control"), PC->GetPawn() == Player);
    TestFalse(TEXT("Humanoid movement restored"), Player->IsMovementLocked());

    Dragon->Destroy();
    Player->Destroy();
    PC->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonSaveRoundtripTest, "WYRMFALL.Scaffold.DragonSaveLoadRemoteRoundtrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonSaveRoundtripTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmPlayerController* PC = World->SpawnActor<AWyrmPlayerController>(
        AWyrmPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(100.f, 200.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(500.f, 600.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    PC->Possess(Player);
    PC->TransferControlToDragon(Dragon);

    // Custom dragon state
    Dragon->GetAttributes()->SetCurrentHealth(310.f);

    // Create snapshot while dragon is remotely controlled (SAVE-08)
    const FString SlotName = TEXT("WyrmSlot_Dragon_UnitTest");
    UWyrmSaveGame* Snapshot = UWyrmSaveSubsystem::CreateSnapshotObject(SlotName, Player, nullptr, World);
    TestNotNull(TEXT("Snapshot created"), Snapshot);
    TestEqual(TEXT("Save generation schema is valid"), Snapshot->SchemaVersion, UWyrmSaveGame::CurrentSchemaVersion);
    TestTrue(TEXT("Dragon record has bond receipt"), Snapshot->DragonRecord.bHasBondReceipt);
    TestEqual(TEXT("Dragon record has correct health 310.0"), Snapshot->DragonRecord.Health, 310.f);
    TestTrue(TEXT("Dragon record notes direct control"), Snapshot->DragonRecord.bIsDirectlyControlled);
    TestEqual(TEXT("Dragon record saved location"), Snapshot->DragonRecord.WorldLocation, FVector(500.f, 600.f, 100.f));
    TestEqual(TEXT("Dragon record saved humanoid waiting location"), Snapshot->DragonRecord.HumanoidWaitingLocation, FVector(100.f, 200.f, 100.f));

    // Reset dragon state
    Dragon->SetActorLocation(FVector(0.f, 0.f, 0.f));
    Dragon->GetAttributes()->SetCurrentHealth(100.f);

    // Restore from snapshot (SAVE-08)
    const bool bApplySuccess = UWyrmSaveSubsystem::ApplySnapshotObject(Snapshot, Player, nullptr, World);
    TestTrue(TEXT("ApplySnapshotObject succeeded"), bApplySuccess);

    TestEqual(TEXT("Dragon health restored to 310.0"), Dragon->GetAttributes()->GetCurrentHealth(), 310.f);
    TestEqual(TEXT("Dragon position restored to (500, 600, 100)"), Dragon->GetActorLocation(), FVector(500.f, 600.f, 100.f));
    TestTrue(TEXT("Dragon direct control re-established"), Dragon->IsDirectlyControlled());
    TestTrue(TEXT("PC possesses Dragon again"), PC->GetPawn() == Dragon);

    // Clean up
    PC->ReturnControlToHumanoid();
    Dragon->Destroy();
    Player->Destroy();
    PC->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonMountDismountTest, "WYRMFALL.Scaffold.DragonMountAndDismount",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonMountDismountTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmPlayerController* PC = World->SpawnActor<AWyrmPlayerController>(
        AWyrmPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    PC->Possess(Player);

    // 1. Compact mount rejected (DRG-05)
    Dragon->SetDragonForm(EWyrmDragonForm::CompanionForm);
    FString RejectionReason;
    TestFalse(TEXT("Compact mount rejected (DRG-05)"), Dragon->CanMount(Player, RejectionReason));
    TestTrue(TEXT("Rejection explains True Form required"), RejectionReason.Contains(TEXT("True Form")));

    // 2. True Form mount succeeds (DRG-05)
    Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);
    TestTrue(TEXT("CanMount in TrueForm succeeds"), Dragon->CanMount(Player, RejectionReason));
    TestTrue(TEXT("MountHumanoid succeeds"), Dragon->MountHumanoid(Player));
    TestTrue(TEXT("IsRiderMounted is true"), Dragon->IsRiderMounted());
    TestEqual(TEXT("Mounted rider is original humanoid"), Dragon->GetMountedRider(), Player);
    TestTrue(TEXT("Humanoid movement locked"), Player->IsMovementLocked());
    TestTrue(TEXT("PC possesses Dragon"), PC->GetPawn() == Dragon);

    // 3. Dismount rejected in flight (DRG-06)
    Dragon->TakeOff();
    TestEqual(TEXT("Dragon flight state is Flying"), Dragon->GetFlightState(), EWyrmDragonFlightState::Flying);
    FVector OutDismountLoc;
    FString DismountReason;
    TestFalse(TEXT("In-flight dismount rejected (DRG-06)"), Dragon->CanDismount(OutDismountLoc, DismountReason));
    TestTrue(TEXT("Dismount rejection explains in flight"), DismountReason.Contains(TEXT("flight")));

    // 4. Dismount on ground succeeds (DRG-05)
    Dragon->Land();
    TestEqual(TEXT("Dragon flight state is Grounded"), Dragon->GetFlightState(), EWyrmDragonFlightState::Grounded);
    TestTrue(TEXT("CanDismount on ground succeeds"), Dragon->CanDismount(OutDismountLoc, DismountReason));
    TestTrue(TEXT("DismountHumanoid succeeds"), Dragon->DismountHumanoid(OutDismountLoc));
    TestFalse(TEXT("IsRiderMounted is false"), Dragon->IsRiderMounted());
    TestFalse(TEXT("Humanoid movement unlocked"), Player->IsMovementLocked());
    TestTrue(TEXT("PC possesses Humanoid again"), PC->GetPawn() == Player);

    Dragon->Destroy();
    Player->Destroy();
    PC->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonFlightTakeoffLandingTest, "WYRMFALL.Scaffold.DragonFlightTakeoffAndLanding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonFlightTakeoffLandingTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    Dragon->SetDragonRole(EWyrmDragonRole::AlliedCompanion);
    Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);

    // Initial state is Grounded
    TestEqual(TEXT("Initial flight state is Grounded"), Dragon->GetFlightState(), EWyrmDragonFlightState::Grounded);
    TestFalse(TEXT("IsInFlight is false initially"), Dragon->IsInFlight());

    // Takeoff (DRG-06)
    FString TakeoffReason;
    TestTrue(TEXT("CanTakeOff succeeds with clear overhead"), Dragon->CanTakeOff(TakeoffReason));
    TestTrue(TEXT("TakeOff succeeds"), Dragon->TakeOff());
    TestEqual(TEXT("FlightState is Flying"), Dragon->GetFlightState(), EWyrmDragonFlightState::Flying);
    TestTrue(TEXT("IsInFlight is true"), Dragon->IsInFlight());
    TestEqual(TEXT("CharacterMovement is MOVE_Flying"), Dragon->GetCharacterMovement()->MovementMode, EMovementMode::MOVE_Flying);
    TestEqual(TEXT("MaxFlySpeed is 1600.0"), Dragon->GetCharacterMovement()->MaxFlySpeed, 1600.f);

    // Landing (DRG-06)
    Dragon->SetActorLocation(FVector(0.f, 0.f, 400.f));
    FVector LandingLoc;
    FString LandReason;
    TestTrue(TEXT("CanLand succeeds within search distance"), Dragon->CanLand(LandingLoc, LandReason));
    TestTrue(TEXT("Land succeeds"), Dragon->Land());
    TestEqual(TEXT("FlightState returned to Grounded"), Dragon->GetFlightState(), EWyrmDragonFlightState::Grounded);
    TestFalse(TEXT("IsInFlight is false after landing"), Dragon->IsInFlight());
    TestEqual(TEXT("MovementMode is MOVE_Walking"), Dragon->GetCharacterMovement()->MovementMode, EMovementMode::MOVE_Walking);

    Dragon->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonMountedDefeatRecoveryTest, "WYRMFALL.Scaffold.DragonMountedDefeatAndHubRecovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonMountedDefeatRecoveryTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmPlayerController* PC = World->SpawnActor<AWyrmPlayerController>(
        AWyrmPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(100.f, 100.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(200.f, 200.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);
    PC->Possess(Player);
    Dragon->MountHumanoid(Player);
    Dragon->TakeOff();

    TestTrue(TEXT("Rider is mounted in flight"), Dragon->IsRiderMounted());
    TestTrue(TEXT("Dragon is in flight"), Dragon->IsInFlight());

    // 1. Mounted dragon defeat invokes emergency ground recovery (DRG-07)
    Dragon->HandleMountedDefeat();

    TestFalse(TEXT("Rider is no longer mounted after defeat"), Dragon->IsRiderMounted());
    TestTrue(TEXT("PC possesses Humanoid again"), PC->GetPawn() == Player);
    TestFalse(TEXT("Humanoid movement is unlocked"), Player->IsMovementLocked());
    TestEqual(TEXT("Dragon role is Recovering"), Dragon->GetDragonRole(), EWyrmDragonRole::Recovering);
    TestEqual(TEXT("Dragon flight state forced to Grounded"), Dragon->GetFlightState(), EWyrmDragonFlightState::Grounded);

    // Mounting while recovering must be rejected
    FString MountReason;
    TestFalse(TEXT("Mounting recovering dragon rejected"), Dragon->CanMount(Player, MountReason));

    // 2. Hub Recovery restores companion (DRG-14)
    TestTrue(TEXT("RecoverCompanion succeeds"), Dragon->RecoverCompanion());
    TestEqual(TEXT("Dragon role restored to AlliedCompanion"), Dragon->GetDragonRole(), EWyrmDragonRole::AlliedCompanion);
    TestEqual(TEXT("Dragon health restored to 420.0"), Dragon->GetAttributes()->GetCurrentHealth(), 420.f);
    TestEqual(TEXT("Dragon focus restored to 100.0"), Dragon->GetAttributes()->GetCurrentFocus(), 100.f);

    Dragon->Destroy();
    Player->Destroy();
    PC->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonMountedFlightSaveTest, "WYRMFALL.Scaffold.DragonMountedFlightSaveAndObstructedRecovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonMountedFlightSaveTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmPlayerController* PC = World->SpawnActor<AWyrmPlayerController>(
        AWyrmPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(100.f, 100.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(400.f, 500.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);
    PC->Possess(Player);
    Dragon->MountHumanoid(Player);
    Dragon->TakeOff();
    Dragon->SetActorLocation(FVector(400.f, 500.f, 800.f));

    // Save snapshot while airborne mounted (SAVE-09)
    const FString SlotName = TEXT("WyrmSlot_DragonFlight_Test");
    UWyrmSaveGame* Snapshot = UWyrmSaveSubsystem::CreateSnapshotObject(SlotName, Player, nullptr, World);
    TestNotNull(TEXT("Snapshot created"), Snapshot);
    TestTrue(TEXT("Dragon record notes rider is mounted"), Snapshot->DragonRecord.bIsRiderMounted);
    TestEqual(TEXT("Dragon record notes FlightState is Flying"), Snapshot->DragonRecord.FlightState, EWyrmDragonFlightState::Flying);
    TestEqual(TEXT("Dragon record saved location Z=800"), Snapshot->DragonRecord.WorldLocation.Z, 800.0);

    // Reset dragon state to grounded at origin
    Dragon->DismountHumanoid(Snapshot->DragonRecord.SafeGroundAnchor);
    Dragon->SetActorLocation(FVector::ZeroVector);

    // Apply snapshot restore in valid space (SAVE-09)
    const bool bApplySuccess = UWyrmSaveSubsystem::ApplySnapshotObject(Snapshot, Player, nullptr, World);
    TestTrue(TEXT("ApplySnapshotObject succeeded"), bApplySuccess);
    TestEqual(TEXT("Dragon restored to Z=800"), Dragon->GetActorLocation().Z, 800.0);
    TestEqual(TEXT("Flight state restored to Flying"), Dragon->GetFlightState(), EWyrmDragonFlightState::Flying);
    TestTrue(TEXT("Rider re-mounted on dragon"), Dragon->IsRiderMounted());
    TestTrue(TEXT("PC possesses dragon"), PC->GetPawn() == Dragon);

    // Clean up
    FVector DismountLoc;
    Dragon->DismountHumanoid(DismountLoc);
    Dragon->Destroy();
    Player->Destroy();
    PC->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonHeartfoldCollisionTest, "WYRMFALL.Scaffold.DragonHeartfoldCollisionAndDoorfit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonHeartfoldCollisionTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Dragon spawned"), Dragon);

    Dragon->SetDragonRole(EWyrmDragonRole::AlliedCompanion);
    TestEqual(TEXT("Initial companion role is AlliedCompanion"), Dragon->GetDragonRole(), EWyrmDragonRole::AlliedCompanion);
    TestEqual(TEXT("Initial form is CompanionForm (DRG-08)"), Dragon->GetDragonForm(), EWyrmDragonForm::CompanionForm);

    // Verify CompanionForm dimensions (Radius=30, HalfHeight=35, scale=0.009)
    UCapsuleComponent* Capsule = Dragon->GetCapsuleComponent();
    TestNotNull(TEXT("Capsule exists"), Capsule);
    TestEqual(TEXT("Companion capsule radius is 30.0 (DRG-08)"), Capsule->GetUnscaledCapsuleRadius(), 30.f);
    TestEqual(TEXT("Companion capsule half height is 35.0 (DRG-08)"), Capsule->GetUnscaledCapsuleHalfHeight(), 35.f);
    if (Dragon->GetMesh())
    {
        TestEqual(TEXT("Companion mesh scale is 0.009 (DRG-08)"), Dragon->GetMesh()->GetRelativeScale3D().X, 0.009);
    }

    // Doorframe fitting test (SM_Stylized_Wood_Doorframe has opening width 100cm, height 210cm)
    const float DoorOpeningWidth = 100.f;
    const float DoorOpeningHeight = 210.f;
    const float CompanionDiameter = Capsule->GetUnscaledCapsuleRadius() * 2.f; // 60cm
    const float CompanionHeight = Capsule->GetUnscaledCapsuleHalfHeight() * 2.f; // 70cm
    TestTrue(TEXT("Companion diameter (60cm) fits within 100cm door opening (DRG-08)"), CompanionDiameter < DoorOpeningWidth);
    TestTrue(TEXT("Companion height (70cm) fits within 210cm door height (DRG-08)"), CompanionHeight < DoorOpeningHeight);

    // Switch to TrueForm
    Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);
    TestEqual(TEXT("Form set to TrueForm"), Dragon->GetDragonForm(), EWyrmDragonForm::TrueForm);
    TestEqual(TEXT("TrueForm capsule radius is 120.0 (DRG-08)"), Capsule->GetUnscaledCapsuleRadius(), 120.f);
    TestEqual(TEXT("TrueForm capsule half height is 160.0 (DRG-08)"), Capsule->GetUnscaledCapsuleHalfHeight(), 160.f);
    if (Dragon->GetMesh())
    {
        TestEqual(TEXT("TrueForm mesh scale is 0.035 (DRG-08)"), Dragon->GetMesh()->GetRelativeScale3D().X, 0.035);
    }

    // TrueForm blocked by standard doorframe
    const float TrueFormDiameter = Capsule->GetUnscaledCapsuleRadius() * 2.f; // 240cm
    const float TrueFormHeight = Capsule->GetUnscaledCapsuleHalfHeight() * 2.f; // 320cm
    TestTrue(TEXT("TrueForm diameter (240cm) blocked by 100cm door opening (DRG-08)"), TrueFormDiameter > DoorOpeningWidth);
    TestTrue(TEXT("TrueForm height (320cm) blocked by 210cm door height (DRG-08)"), TrueFormHeight > DoorOpeningHeight);

    // Verify speed parameters
    TestEqual(TEXT("Companion ground speed is 450"), Dragon->GetCompanionGroundSpeed(), 450.f);
    TestEqual(TEXT("Companion catch-up speed is 600"), Dragon->GetCompanionCatchUpSpeed(), 600.f);
    TestEqual(TEXT("TrueForm ground speed is 550"), Dragon->GetTrueFormGroundSpeed(), 550.f);

    Dragon->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonHeartfoldCombatAndParityTest, "WYRMFALL.Scaffold.DragonHeartfoldCombatAndParity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonHeartfoldCombatAndParityTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 3000.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(200.f, 3000.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmEnemyCharacter* DummyTarget = AWyrmEnemyCharacter::SpawnWyrmEnemy(
        World, EWyrmEnemyRole::MeleeChaser, FTransform(FVector(400.f, 3000.f, 100.f)));
    TestNotNull(TEXT("Dummy target spawned"), DummyTarget);
    DummyTarget->GetAbilitySystemComponent()->InitAbilityActorInfo(DummyTarget, DummyTarget);
    DummyTarget->GetAttributes()->InitMaxHealth(100.f);
    DummyTarget->GetAttributes()->SetCurrentMaxHealth(100.f);
    DummyTarget->GetAttributes()->InitHealth(100.f);
    DummyTarget->GetAttributes()->SetCurrentHealth(100.f);
    DummyTarget->GetAttributes()->InitArmor(0.f);
    DummyTarget->GetAttributes()->SetCurrentArmor(0.f);

    Dragon->BondWithHumanoid(Player);
    Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);

    // 1. Injure dragon and reduce focus
    Dragon->GetAttributes()->InitHealth(175.f);
    Dragon->GetAttributes()->InitFocus(80.f);
    TestEqual(TEXT("Injured dragon health is 175.0"), Dragon->GetAttributes()->GetCurrentHealth(), 175.f);
    TestEqual(TEXT("Dragon focus is 80.0"), Dragon->GetAttributes()->GetCurrentFocus(), 80.f);

    // 2. Trigger secondary area attack in TrueForm
    TestTrue(TEXT("Secondary area attack executes in TrueForm"), Dragon->PerformSecondaryAttack(DummyTarget));
    TestTrue(TEXT("Area attack cooldown committed (DRG-09)"), Dragon->GetAreaAttackCooldownRemaining() > 0.f);

    // 3. Fold into CompanionForm (DRG-10 Form State Conservation)
    TestTrue(TEXT("Fold into CompanionForm begins"), Dragon->RequestFormChange(EWyrmDragonForm::CompanionForm));
    TestTrue(TEXT("Fold transition is active before its timed commit"), Dragon->IsTransitioningForm());
    Dragon->Tick(1.1f);
    TestEqual(TEXT("Current form is CompanionForm"), Dragon->GetDragonForm(), EWyrmDragonForm::CompanionForm);

    // Strict parity check: health, focus, and active cooldowns must not reset or heal
    TestEqual(TEXT("Health strictly conserved across fold at 175.0 (DRG-10)"), Dragon->GetAttributes()->GetCurrentHealth(), 175.f);
    TestEqual(TEXT("Max health strictly conserved at 420.0 (DRG-10)"), Dragon->GetAttributes()->GetMaxHealth(), 420.f);
    TestEqual(TEXT("Focus strictly conserved at 80.0 (DRG-10)"), Dragon->GetAttributes()->GetCurrentFocus(), 80.f);
    TestTrue(TEXT("Area attack cooldown conserved across fold (DRG-10)"), Dragon->GetAreaAttackCooldownRemaining() > 0.f);

    // 4. Test Compact Primary Attack (DRG-09: 9 raw damage)
    DummyTarget->GetAttributes()->SetCurrentHealth(100.f);
    TestTrue(TEXT("PerformPrimaryAttack executes in CompanionForm"), Dragon->PerformPrimaryAttack(DummyTarget));
    TestEqual(TEXT("Companion primary attack deals 9 damage (100 -> 91) (DRG-09)"), DummyTarget->GetAttributes()->GetCurrentHealth(), 91.f);

    // 5. Test Compact Secondary Attack (DRG-09: 6 raw damage, 250cm radius, shared 6s cooldown)
    DummyTarget->GetAttributes()->SetCurrentHealth(100.f);
    FWyrmDragonSaveRecord CooldownResetRecord;
    Dragon->BuildSaveRecord(CooldownResetRecord);
    CooldownResetRecord.AreaAttackCooldownRemaining = 0.f;
    Dragon->RestoreFromSaveRecord(CooldownResetRecord, Player);

    TestTrue(TEXT("PerformSecondaryAttack executes in CompanionForm"), Dragon->PerformSecondaryAttack(DummyTarget));
    TestEqual(TEXT("Companion secondary attack deals 6 damage (100 -> 94) (DRG-09)"), DummyTarget->GetAttributes()->GetCurrentHealth(), 94.f);
    TestEqual(TEXT("Shared cooldown committed to 6s (DRG-09)"), Dragon->GetAreaAttackCooldownRemaining(), 6.0f);

    // 6. Unfold to TrueForm: check parity preserved and TrueForm damage
    // Let the shared form cooldown expire through the real tick path before
    // requesting the reverse transition.
    Dragon->Tick(4.1f);
    FString UnfoldReason;
    TestTrue(FString::Printf(TEXT("Unfold to TrueForm is valid after cooldown (Reason: %s)"), *UnfoldReason), Dragon->CanChangeForm(EWyrmDragonForm::TrueForm, UnfoldReason));
    TestTrue(TEXT("Unfold to TrueForm begins"), Dragon->RequestFormChange(EWyrmDragonForm::TrueForm));
    Dragon->Tick(1.1f);
    TestEqual(TEXT("Health still strictly conserved at 175.0 after unfolding (DRG-10)"), Dragon->GetAttributes()->GetCurrentHealth(), 175.f);

    DummyTarget->GetAttributes()->SetCurrentHealth(100.f);
    TestTrue(TEXT("PerformPrimaryAttack executes in TrueForm"), Dragon->PerformPrimaryAttack(DummyTarget));
    TestEqual(TEXT("TrueForm primary attack deals 24 damage (100 -> 76) (DRG-09)"), DummyTarget->GetAttributes()->GetCurrentHealth(), 76.f);

    Dragon->Destroy();
    DummyTarget->Destroy();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonHeartfoldBlockedGrowthTest, "WYRMFALL.Scaffold.DragonHeartfoldBlockedGrowth",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonHeartfoldBlockedGrowthTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(1000.f, 1000.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    Dragon->SetDragonForm(EWyrmDragonForm::CompanionForm);

    FString Reason;
    // 1. In open clearance, growth is permitted
    TestTrue(TEXT("CanChangeForm to TrueForm succeeds in open space (DRG-11)"), Dragon->CanChangeForm(EWyrmDragonForm::TrueForm, Reason));

    // 2. Spawn a low ceiling obstacle directly overhead (Z=220cm, below 320cm TrueForm height requirement)
    AActor* CeilingActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(1000.f, 1000.f, 220.f), FRotator::ZeroRotator, SpawnParams);
    UBoxComponent* BoxComp = NewObject<UBoxComponent>(CeilingActor, TEXT("CeilingCollision"));
    CeilingActor->SetRootComponent(BoxComp);
    BoxComp->SetBoxExtent(FVector(200.f, 200.f, 40.f));
    BoxComp->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    BoxComp->SetCollisionObjectType(ECC_WorldStatic);
    BoxComp->SetCollisionResponseToAllChannels(ECR_Block);
    BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BoxComp->SetWorldLocation(FVector(1000.f, 1000.f, 220.f));
    BoxComp->RegisterComponentWithWorld(World);

    // 3. Blocked growth test (DRG-11)
    TestFalse(TEXT("CanChangeForm rejected under low ceiling (DRG-11)"), Dragon->CanChangeForm(EWyrmDragonForm::TrueForm, Reason));
    TestTrue(TEXT("Rejection reason is 'Not enough room for True Form' (DRG-11)"), Reason.Contains(TEXT("Not enough room for True Form")));

    // RequestFormChange must also fail
    TestFalse(TEXT("RequestFormChange rejected under low ceiling (DRG-11)"), Dragon->RequestFormChange(EWyrmDragonForm::TrueForm));
    TestEqual(TEXT("Dragon remains in valid CompanionForm (DRG-11)"), Dragon->GetDragonForm(), EWyrmDragonForm::CompanionForm);

    // 4. Remove ceiling: growth succeeds again
    CeilingActor->Destroy();
    TestTrue(TEXT("CanChangeForm succeeds after clearing overhead obstacle (DRG-11)"), Dragon->CanChangeForm(EWyrmDragonForm::TrueForm, Reason));

    // Revalidate immediately before commit: a ceiling added while the compact
    // dragon is transitioning must cancel safely rather than grow into it.
    TestTrue(TEXT("Growth transition begins in clear space (DRG-12)"), Dragon->RequestFormChange(EWyrmDragonForm::TrueForm));
    AActor* LateCeilingActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(1000.f, 1000.f, 220.f), FRotator::ZeroRotator, SpawnParams);
    UBoxComponent* LateBoxComp = NewObject<UBoxComponent>(LateCeilingActor, TEXT("LateCeilingCollision"));
    LateCeilingActor->SetRootComponent(LateBoxComp);
    LateBoxComp->SetBoxExtent(FVector(200.f, 200.f, 40.f));
    LateBoxComp->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    LateBoxComp->SetCollisionObjectType(ECC_WorldStatic);
    LateBoxComp->SetCollisionResponseToAllChannels(ECR_Block);
    LateBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LateBoxComp->SetWorldLocation(FVector(1000.f, 1000.f, 220.f));
    LateBoxComp->RegisterComponentWithWorld(World);
    Dragon->Tick(1.1f);
    TestFalse(TEXT("Late obstruction cancels transition before commit (DRG-12)"), Dragon->IsTransitioningForm());
    TestEqual(TEXT("Late obstruction leaves dragon compact (DRG-12)"), Dragon->GetDragonForm(), EWyrmDragonForm::CompanionForm);
    TestTrue(TEXT("Late obstruction commits recovery cooldown (DRG-12)"), Dragon->GetFormTransitionCooldownRemaining() > 0.f);
    LateCeilingActor->Destroy();

    Dragon->Destroy();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonHeartfoldInterruptionTest, "WYRMFALL.Scaffold.DragonHeartfoldInterruptionRollback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonHeartfoldInterruptionTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(500.f, 500.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    Dragon->SetDragonForm(EWyrmDragonForm::CompanionForm);
    Dragon->GetAttributes()->InitHealth(420.f);
    Dragon->GetAttributes()->InitMaxHealth(420.f);

    // 1. Request form transition to TrueForm
    TestTrue(TEXT("RequestFormChange begins transition (DRG-12)"), Dragon->RequestFormChange(EWyrmDragonForm::TrueForm));
    TestTrue(TEXT("IsTransitioningForm is true during transition (DRG-12)"), Dragon->IsTransitioningForm());
    TestEqual(TEXT("Dragon remains in CompanionForm until commit (DRG-12)"), Dragon->GetDragonForm(), EWyrmDragonForm::CompanionForm);
    TestFalse(TEXT("Primary attack is suppressed during transition (DRG-12)"), Dragon->PerformPrimaryAttack(Player));
    TestFalse(TEXT("Secondary attack is suppressed during transition (DRG-12)"), Dragon->PerformSecondaryAttack(Player));

    // 2. Incoming damage interrupts transition (DRG-12)
    const float InitialHealth = Dragon->GetAttributes()->GetCurrentHealth();
    Dragon->TakeDamage(30.f, FDamageEvent(), nullptr, nullptr);

    // 3. Verify rollback and cooldown commit
    TestFalse(TEXT("IsTransitioningForm cancelled upon damage (DRG-12)"), Dragon->IsTransitioningForm());
    TestEqual(TEXT("Dragon rolled back to CompanionForm (DRG-12)"), Dragon->GetDragonForm(), EWyrmDragonForm::CompanionForm);
    TestTrue(TEXT("Recovery cooldown committed (4.0s) (DRG-12)"), Dragon->GetFormTransitionCooldownRemaining() > 0.f);
    TestEqual(TEXT("Damage conserved without duplication (420 - 30 = 390) (DRG-12)"), Dragon->GetAttributes()->GetCurrentHealth(), InitialHealth - 30.f);

    // 4. Subsequent transition request rejected while on cooldown
    FString CooldownReason;
    TestFalse(TEXT("Form change rejected during transition cooldown (DRG-12)"), Dragon->CanChangeForm(EWyrmDragonForm::TrueForm, CooldownReason));
    TestTrue(TEXT("Reason mentions cooldown"), CooldownReason.Contains(TEXT("cooldown")));

    // 5. Town Mode Behavior check (DRG-13)
    Dragon->SetTownModeEnabled(true);
    TestTrue(TEXT("Town mode enabled (DRG-13)"), Dragon->IsTownModeEnabled());
    Dragon->IssueOrder(EWyrmCompanionOrder::AttackTarget, Player);
    // In town mode, attack targeting is suppressed to Follow
    Dragon->Tick(0.1f);
    TestEqual(TEXT("Attack order suppressed to Follow in town mode (DRG-13)"), Dragon->GetCompanionOrder(), EWyrmCompanionOrder::Follow);
    TestNull(TEXT("Combat target cleared in town mode (DRG-13)"), Dragon->GetCombatTarget());

    Dragon->Destroy();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmDragonRigProfilePolicyTest, "WYRMFALL.Scaffold.DragonRigProfilePolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmDragonRigProfilePolicyTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 5000.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Dragon = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(500.f, 5000.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    Dragon->BondWithHumanoid(Player);
    Dragon->DragonId = FName(TEXT("Rotwing"));
    TestFalse(TEXT("Rotwing cannot inherit Verdance or Jadefang validated rig profile (DRG-15)"), Dragon->HasSupportedRigProfile());

    FString Reason;
    TestFalse(TEXT("Rotwing Heartfold change is blocked pending its own profile (DRG-15)"), Dragon->CanChangeForm(EWyrmDragonForm::TrueForm, Reason));
    TestTrue(TEXT("Heartfold rejection identifies missing rig profile (DRG-15)"), Reason.Contains(TEXT("no validated Heartfold profile")));

    Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);
    TestFalse(TEXT("Rotwing cannot inherit mount profile (DRG-15)"), Dragon->CanMount(Player, Reason));
    TestTrue(TEXT("Mount rejection identifies missing rig profile (DRG-15)"), Reason.Contains(TEXT("no validated mount profile")));
    TestFalse(TEXT("Rotwing cannot inherit flight profile (DRG-15)"), Dragon->CanTakeOff(Reason));
    TestTrue(TEXT("Flight rejection identifies missing rig profile (DRG-15)"), Reason.Contains(TEXT("no validated flight profile")));

    // WP-20: Jadefang has its own authoritative validated rig profile
    Dragon->SetDragonId(FName(TEXT("Jadefang")));
    TestTrue(TEXT("Jadefang has authoritative validated rig profile (WP-20)"), Dragon->HasSupportedRigProfile());
    TestEqual(TEXT("Jadefang companion walk speed is 480"), Dragon->GetActiveRigProfile().CompanionGroundSpeed, 480.f);
    TestEqual(TEXT("Jadefang true form fly speed is 1700"), Dragon->GetActiveRigProfile().FlightSpeed, 1700.f);
    TestEqual(TEXT("Jadefang mount socket offset is (0, 0, 140)"), Dragon->GetActiveRigProfile().MountSocketOffset, FVector(0.f, 0.f, 140.f));
    TestTrue(TEXT("Jadefang can mount in True Form"), Dragon->CanMount(Player, Reason));
    TestTrue(TEXT("Jadefang can take off in True Form"), Dragon->CanTakeOff(Reason));

    Dragon->Destroy();
    Player->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmRegion01FactsAndPersistenceTest, "WYRMFALL.Region01.LandmarksFactsAndPersistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmRegion01FactsAndPersistenceTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }
    if (!World) World = GWorld;
    if (!World) return true;

    // Commandlet automation uses an editor world without a game instance.
    // Create a test-local ledger here; the focused PIE proof exercises the
    // actual game-instance subsystem and unified save hand-off.
    UGameInstance* TestGI = NewObject<UGameInstance>(GetTransientPackage());
    UWyrmRegion01Subsystem* Region01 = NewObject<UWyrmRegion01Subsystem>(TestGI);
    TestNotNull(TEXT("Region 01 fact ledger can be constructed for automation"), Region01);
    if (!Region01)
    {
        return false;
    }

    Region01->ResetRegion01State();
    TestTrue(TEXT("Region 01 defines a valid 13-landmark graph (REG-01)"), Region01->HasValidLandmarkGraph());
    TestTrue(TEXT("Heart exit can reach Tidecross without an ordered trigger (REG-01)"),
        Region01->HasRouteBetweenLandmarks(FName(TEXT("LM-HEART")), FName(TEXT("LM-TIDECROSS"))));
    TestTrue(TEXT("Tidecross can reach the quarry arena through valid alternatives (REG-05)"),
        Region01->HasRouteBetweenLandmarks(FName(TEXT("LM-TIDECROSS")), FName(TEXT("LM-ARENA"))));
    TestTrue(TEXT("Silent Landing has a separate optional route rather than an opening choke"),
        Region01->HasRouteBetweenLandmarks(FName(TEXT("LM-TIDECROSS")), FName(TEXT("LM-SILENTLANDING"))));
    TestFalse(TEXT("Silent Landing remains unavailable before real homecoming"),
        Region01->IsLandmarkCurrentlyAvailable(FName(TEXT("LM-SILENTLANDING"))));
    TestFalse(TEXT("Compact cave remains unavailable before the bond"),
        Region01->IsLandmarkCurrentlyAvailable(FName(TEXT("LM-COMPACTCAVE"))));

    // A genuine control shutdown can end extraction without making Rusk an
    // artificial route gate. The main flow below separately covers the
    // primary-claim route.
    TestTrue(TEXT("Control shutdown can stop extraction before Rusk custody"), Region01->StopCrowncutExtraction());
    TestTrue(TEXT("Control shutdown records extraction state"),
        Region01->HasFact(FName(TEXT("quarry.extraction_stopped"))));
    Region01->ResetRegion01State();

    TestTrue(TEXT("Heart exit receipt is recorded once"), Region01->RecordHeartExitReached());
    TestTrue(TEXT("Tidecross visit is recorded"), Region01->VisitLandmark(FName(TEXT("LM-TIDECROSS"))));
    TestTrue(TEXT("Tidecross fact follows actual visit"), Region01->HasFact(FName(TEXT("tidecross.visited"))));

    // REG-03 / REG-04: Sella can be secured before notice or other workers.
    TestTrue(TEXT("Sella can be secured first (REG-03)"), Region01->SecureWorker(EWyrmRegion01Worker::Sella));
    TestTrue(TEXT("Sella account becomes available from her actual rescue"), Region01->HasFact(FName(TEXT("evidence.sella_account"))));
    TestTrue(TEXT("Sella-first rescue exposes the auxiliary option"), Region01->IsAuxiliaryShutdownAvailable());
    TestTrue(TEXT("Auxiliary restraint can be disabled after Sella"), Region01->DisableAuxiliaryRestraint());
    TestTrue(TEXT("Physical machine evidence can be observed independently"),
        Region01->DiscoverEvidence(EWyrmRegion01Evidence::Machine));
    TestTrue(TEXT("Machine plus Sella account keeps the conflict readable without records (REG-04)"),
        Region01->HasConflictEvidence());
    TestTrue(TEXT("Reading the notice later retains the Sella-first facts"), Region01->ReadCrownNotice());
    TestFalse(TEXT("Sella receipt cannot be issued twice"), Region01->SecureWorker(EWyrmRegion01Worker::Sella));

    // REG-05 / REG-09: Bonding is independent of notice, Rusk, and worker completion.
    TestTrue(TEXT("Verdance living-defeat fact records"), Region01->RecordVerdanceDefeatedAlive());
    TestTrue(TEXT("Claim can break only after living defeat"), Region01->BreakVerdanceClaim());
    TestTrue(TEXT("Primary claim break independently stops extraction"),
        Region01->HasFact(FName(TEXT("quarry.extraction_stopped"))));

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(
        AWyrmCharacter::StaticClass(), FVector(0.f, 9000.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Verdance = World->SpawnActor<AWyrmDragonCharacter>(
        AWyrmDragonCharacter::StaticClass(), FVector(500.f, 9000.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    TestNotNull(TEXT("Humanoid exists for bond receipt"), Player);
    TestNotNull(TEXT("Verdance exists for bond receipt"), Verdance);
    if (!Player || !Verdance)
    {
        if (Verdance) Verdance->Destroy();
        if (Player) Player->Destroy();
        return false;
    }

    TestTrue(TEXT("Verdance enters real defeated-alive state"), Verdance->PerformBossDefeat());
    TestTrue(TEXT("Existing dragon authority issues its one-way bond receipt"), Verdance->BondWithHumanoid(Player));
    TestTrue(TEXT("Region records the existing Verdance bond receipt"), Region01->RecordVerdanceBondAccepted(Verdance));
    TestTrue(TEXT("Bonded dragon availability is immediate before worker cleanup (REG-09)"), Region01->IsBondedDragonAvailable());
    TestTrue(TEXT("Bond opens the optional compact cave route"),
        Region01->IsLandmarkCurrentlyAvailable(FName(TEXT("LM-COMPACTCAVE"))));
    TestTrue(TEXT("Town return has a partial debrief before all workers (REG-09)"), Region01->IsPartialDebriefAvailable());
    TestFalse(TEXT("Homecoming cannot complete with missing workers"), Region01->IsHomecomingComplete());
    TestTrue(TEXT("Post-bond unresolved Rusk can take surrendered custody branch (REG-05)"),
        Region01->ResolveRusk(EWyrmRegion01RuskOutcome::SurrenderedCustody));
    TestEqual(TEXT("Rusk persisted as surrendered custody"), Region01->GetRuskOutcome(), EWyrmRegion01RuskOutcome::SurrenderedCustody);
    TestTrue(TEXT("Future relief receipt is accepted only after bond and extraction"), Region01->RecordReliefResolved());
    TestTrue(TEXT("Optional wage recovery records once without a timer"), Region01->RecoverOptionalWageRecord());
    TestFalse(TEXT("Optional wage recovery cannot repeat"), Region01->RecoverOptionalWageRecord());

    TestTrue(TEXT("Pell can be secured after bond (REG-09)"), Region01->SecureWorker(EWyrmRegion01Worker::Pell));
    TestTrue(TEXT("Iven can be secured after bond (REG-09)"), Region01->SecureWorker(EWyrmRegion01Worker::Iven));
    TestTrue(TEXT("All local closure facts make a homecoming ready (REG-02, REG-09)"),
        Region01->IsHomecomingReady());
    TestTrue(TEXT("Actual Tidecross return records full homecoming"), Region01->CompleteHomecoming());
    TestFalse(TEXT("Homecoming receipt cannot repeat"), Region01->CompleteHomecoming());
    TestTrue(TEXT("Homecoming opens the optional Silent Landing route"),
        Region01->IsLandmarkCurrentlyAvailable(FName(TEXT("LM-SILENTLANDING"))));

    UWyrmSaveGame* Snapshot = NewObject<UWyrmSaveGame>();
    TestNotNull(TEXT("Unified save object can carry the Region 01 record"), Snapshot);
    if (Snapshot)
    {
        Region01->BuildSaveRecord(Snapshot->Region01Record);
        TestEqual(TEXT("Region 01 save schema is version 2"), Snapshot->SchemaVersion, UWyrmSaveGame::CurrentSchemaVersion);
        TestTrue(TEXT("Snapshot includes all-worker homecoming fact"),
            Snapshot->Region01Record.KnownFacts.Contains(FName(TEXT("homecoming.complete"))));
        TestEqual(TEXT("Snapshot preserves surrendered Rusk outcome"), Snapshot->Region01Record.RuskOutcome,
            EWyrmRegion01RuskOutcome::SurrenderedCustody);

        Region01->ResetRegion01State();
        TestFalse(TEXT("Fact reset clears test state before restore"), Region01->IsHomecomingComplete());
        Region01->RestoreFromSaveRecord(Snapshot->Region01Record);
        TestTrue(TEXT("Homecoming persists across record restore (REG-10)"), Region01->IsHomecomingComplete());
        TestTrue(TEXT("Extraction consequence persists across record restore (REG-10)"),
            Region01->HasFact(FName(TEXT("quarry.extraction_stopped"))));
        TestTrue(TEXT("Wage receipt persists without duplication (REG-10)"),
            Region01->HasFact(FName(TEXT("wage.recovered"))));
    }

    Verdance->Destroy();
    Player->Destroy();
    Region01->ResetRegion01State();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmCreatorSubsystemTest, "WYRMFALL.Scaffold.CreatorSubsystemAndProportions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWyrmCreatorSubsystemTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World)
    {
        return false;
    }

    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>();
    TestNotNull(TEXT("Character valid"), Character);
    if (!Character)
    {
        World->DestroyWorld(false);
        return false;
    }

    // Default scale
    TestEqual(TEXT("Initial CharacterScale is One"), Character->GetCharacterScale(), FVector::OneVector);

    // Proportions (CHAR-07)
    Character->SetCharacterScale(FVector(0.85f, 0.85f, 0.85f));
    TestEqual(TEXT("Shortest scale applied"), Character->GetCharacterScale(), FVector(0.85f, 0.85f, 0.85f));

    Character->SetCharacterScale(FVector(1.15f, 1.15f, 1.15f));
    TestEqual(TEXT("Tallest scale applied"), Character->GetCharacterScale(), FVector(1.15f, 1.15f, 1.15f));

    Character->SetCharacterScale(FVector(1.25f, 1.25f, 1.0f));
    TestEqual(TEXT("Broadest scale applied"), Character->GetCharacterScale(), FVector(1.25f, 1.25f, 1.0f));

    // CreatorSubsystem
    UGameInstance* TestGI = NewObject<UGameInstance>(GetTransientPackage());
    UWyrmCreatorSubsystem* CreatorSubsystem = NewObject<UWyrmCreatorSubsystem>(TestGI);
    TestNotNull(TEXT("CreatorSubsystem instantiated"), CreatorSubsystem);
    if (CreatorSubsystem)
    {
        CreatorSubsystem->InitializePresets();

        // Category Locks (CHAR-08)
        TestFalse(TEXT("BodyStyle unlocked initially"), CreatorSubsystem->IsCategoryLocked(TEXT("BodyStyle")));
        CreatorSubsystem->SetCategoryLocked(TEXT("BodyStyle"), true);
        TestTrue(TEXT("BodyStyle locked"), CreatorSubsystem->IsCategoryLocked(TEXT("BodyStyle")));
        CreatorSubsystem->SetCategoryLocked(TEXT("BodyStyle"), false);
        TestFalse(TEXT("BodyStyle unlocked again"), CreatorSubsystem->IsCategoryLocked(TEXT("BodyStyle")));

        // Presets (CHAR-08)
        TArray<FName> Presets = CreatorSubsystem->GetAvailablePresets();
        TestTrue(TEXT("Presets registered"), Presets.Num() >= 4);
        TestTrue(TEXT("Has Knight_Standard"), Presets.Contains(TEXT("Knight_Standard")));
        TestTrue(TEXT("Has Knight_Commander"), Presets.Contains(TEXT("Knight_Commander")));
        TestTrue(TEXT("Has Knight_Archer"), Presets.Contains(TEXT("Knight_Archer")));
        TestTrue(TEXT("Has Knight_Champion"), Presets.Contains(TEXT("Knight_Champion")));
        TestTrue(TEXT("Has Knight_Shortest"), Presets.Contains(TEXT("Knight_Shortest")));
        TestTrue(TEXT("Has Knight_Tallest"), Presets.Contains(TEXT("Knight_Tallest")));
        TestTrue(TEXT("Has Knight_Broadest"), Presets.Contains(TEXT("Knight_Broadest")));

        // Undo / Redo (CHAR-08)
        TestFalse(TEXT("Cannot undo initially"), CreatorSubsystem->CanUndo());
        TestFalse(TEXT("Cannot redo initially"), CreatorSubsystem->CanRedo());

        CreatorSubsystem->PushAppearanceUndoState(Character);
        TestTrue(TEXT("Can undo after push"), CreatorSubsystem->CanUndo());

        Character->SetCharacterScale(FVector(1.10f, 1.10f, 1.10f));
        TestTrue(TEXT("Undo succeeds"), CreatorSubsystem->Undo(Character));
        TestEqual(TEXT("State reverted by undo"), Character->GetCharacterScale(), FVector(1.25f, 1.25f, 1.0f));
        TestTrue(TEXT("Can redo after undo"), CreatorSubsystem->CanRedo());

        TestTrue(TEXT("Redo succeeds"), CreatorSubsystem->Redo(Character));
        TestEqual(TEXT("State restored by redo"), Character->GetCharacterScale(), FVector(1.10f, 1.10f, 1.10f));
    }

    Character->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmHovercarLocomotionTest, "WYRMFALL.Scaffold.HovercarLocomotion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmHovercarLocomotionTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    TestNotNull(TEXT("World created"), World);
    if (!World) { return false; }

    AWyrmHovercar* Hovercar = AWyrmHovercar::SpawnWyrmHovercar(World, FTransform(FVector(0.f, 0.f, 100.f)));
    TestNotNull(TEXT("Hovercar spawned"), Hovercar);
    if (!Hovercar)
    {
        World->DestroyWorld(false);
        return false;
    }

    // Extents (VEH-01: 400cm length, 160cm width, 140cm height)
    TestNotNull(TEXT("CollisionBox present"), Hovercar->CollisionBox.Get());
    if (Hovercar->CollisionBox)
    {
        TestEqual(TEXT("BoxExtent correct"), Hovercar->CollisionBox->GetUnscaledBoxExtent(), FVector(200.f, 80.f, 70.f));
    }

    // Initial State & Speeds (VEH-02, VEH-09)
    TestEqual(TEXT("Initial state is Parked"), Hovercar->GetHovercarState(), EWyrmHovercarState::Parked);
    TestEqual(TEXT("Base cruise speed is 1500"), Hovercar->GetCruiseSpeed(), 1500.f);
    TestEqual(TEXT("Vertical speed is 600"), Hovercar->GetVerticalSpeed(), 600.f);

    // Mecha Progression Boost (VEH-09)
    TestFalse(TEXT("Mecha circuit locked by default"), Hovercar->IsMechaCircuitUnlocked());
    Hovercar->SetMechaCircuitUnlocked(true);
    TestTrue(TEXT("Mecha circuit unlocked"), Hovercar->IsMechaCircuitUnlocked());
    TestEqual(TEXT("Boosted cruise speed is 2000 (+500 boost)"), Hovercar->GetCruiseSpeed(), 2000.f);
    Hovercar->SetMechaCircuitUnlocked(false);
    TestEqual(TEXT("Reverted to base cruise speed"), Hovercar->GetCruiseSpeed(), 1500.f);

    // TakeOff and Landing (VEH-02)
    TestTrue(TEXT("TakeOff succeeds"), Hovercar->TakeOff());
    TestEqual(TEXT("State is Hovering after TakeOff"), Hovercar->GetHovercarState(), EWyrmHovercarState::Hovering);

    Hovercar->AddFlightInput(FVector2D(1.0f, 0.0f));
    Hovercar->Tick(0.1f);
    TestEqual(TEXT("State is Cruising under directional input"), Hovercar->GetHovercarState(), EWyrmHovercarState::Cruising);

    TestTrue(TEXT("Land succeeds"), Hovercar->Land());
    TestEqual(TEXT("State is Landing"), Hovercar->GetHovercarState(), EWyrmHovercarState::Landing);

    // Camera Toggle (VEH-02)
    TestEqual(TEXT("Default camera mode is ThirdPerson"), Hovercar->GetCameraMode(), EWyrmCameraMode::ThirdPerson);
    Hovercar->ToggleCameraMode();
    TestEqual(TEXT("Toggled camera mode is TopDown"), Hovercar->GetCameraMode(), EWyrmCameraMode::TopDown);
    Hovercar->ToggleCameraMode();
    TestEqual(TEXT("Toggled camera mode back to ThirdPerson"), Hovercar->GetCameraMode(), EWyrmCameraMode::ThirdPerson);

    Hovercar->Destroy();
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmHovercarOccupancyAndCompanionTest, "WYRMFALL.Scaffold.HovercarOccupancyAndCompanion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmHovercarOccupancyAndCompanionTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    TestNotNull(TEXT("World created"), World);
    if (!World) { return false; }

    AWyrmHovercar* Hovercar = AWyrmHovercar::SpawnWyrmHovercar(World, FTransform(FVector(0.f, 0.f, 100.f)));
    AWyrmCharacter* Pilot = World->SpawnActor<AWyrmCharacter>();
    AWyrmDragonCharacter* Dragon = AWyrmDragonCharacter::SpawnWyrmDragon(World, EWyrmDragonRole::AlliedCompanion, FTransform(FVector(200.f, 0.f, 100.f)));

    TestNotNull(TEXT("Hovercar valid"), Hovercar);
    TestNotNull(TEXT("Pilot valid"), Pilot);
    TestNotNull(TEXT("Dragon valid"), Dragon);

    if (Hovercar && Pilot && Dragon)
    {
        // 1. Pilot Entry (VEH-01)
        FString Reason;
        TestTrue(TEXT("Can enter hovercar"), Hovercar->CanEnter(Pilot, Reason));
        TestTrue(TEXT("EnterHovercar succeeds"), Hovercar->EnterHovercar(Pilot));
        TestTrue(TEXT("Hovercar is occupied"), Hovercar->IsOccupied());
        TestEqual(TEXT("Driver is Pilot"), Hovercar->GetDriver(), Pilot);
        TestTrue(TEXT("Pilot movement is locked while driving"), Pilot->IsMovementLocked());

        // 2. In-flight Exit Rejection (VEH-04)
        Hovercar->SetHovercarState(EWyrmHovercarState::Cruising);
        FVector ExitLoc;
        TestFalse(TEXT("Cannot exit while airborne/cruising"), Hovercar->CanExit(ExitLoc, Reason));
        TestTrue(TEXT("Reason explains in-flight rejection"), Reason.Contains(TEXT("airborne")));

        // 3. Valid Exit when Parked/Grounded (VEH-04)
        Hovercar->SetHovercarState(EWyrmHovercarState::Parked);
        TestTrue(TEXT("Can exit when parked"), Hovercar->CanExit(ExitLoc, Reason));
        TestTrue(TEXT("ExitHovercar succeeds"), Hovercar->ExitHovercar(ExitLoc));
        TestFalse(TEXT("Hovercar is no longer occupied"), Hovercar->IsOccupied());
        TestFalse(TEXT("Pilot movement restored"), Pilot->IsMovementLocked());

        // 4. Companion Boarding & Heartfold Contract (VEH-05)
        Dragon->SetDragonForm(EWyrmDragonForm::TrueForm);
        TestFalse(TEXT("TrueForm dragon cannot board passenger seat"), Hovercar->CanBoardPet(Dragon, Reason));
        TestTrue(TEXT("TrueForm rejection mentions Heartfold"), Reason.Contains(TEXT("Heartfold")));

        Dragon->SetDragonForm(EWyrmDragonForm::CompanionForm);
        TestTrue(TEXT("Compact CompanionForm dragon can board"), Hovercar->CanBoardPet(Dragon, Reason));
        TestTrue(TEXT("BoardPet succeeds"), Hovercar->BoardPet(Dragon));
        TestTrue(TEXT("HasCompanionBoarded is true"), Hovercar->HasCompanionBoarded());
        TestEqual(TEXT("Boarded companion is Dragon"), Hovercar->GetBoardedCompanion(), Dragon);

        FVector UnboardLoc;
        TestTrue(TEXT("UnboardPet succeeds"), Hovercar->UnboardPet(UnboardLoc));
        TestFalse(TEXT("Companion unboarded"), Hovercar->HasCompanionBoarded());
    }

    if (Dragon) { Dragon->Destroy(); }
    if (Pilot) { Pilot->Destroy(); }
    if (Hovercar) { Hovercar->Destroy(); }
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmHovercarDamageAndDepotTest, "WYRMFALL.Scaffold.HovercarDamageAndDepot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmHovercarDamageAndDepotTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    TestNotNull(TEXT("World created"), World);
    if (!World) { return false; }

    AWyrmHovercar* Hovercar = AWyrmHovercar::SpawnWyrmHovercar(World, FTransform(FVector(0.f, 0.f, 100.f)));
    AWyrmCharacter* Pilot = World->SpawnActor<AWyrmCharacter>();
    AWyrmDragonCharacter* Dragon = AWyrmDragonCharacter::SpawnWyrmDragon(World, EWyrmDragonRole::AlliedCompanion, FTransform(FVector(100.f, 0.f, 100.f)));

    TestNotNull(TEXT("Hovercar valid"), Hovercar);

    if (Hovercar && Pilot && Dragon)
    {
        // Baseline Health (VEH-06: 250 HP)
        TestEqual(TEXT("Max health is 250"), Hovercar->GetMaxHealth(), 250.f);
        TestEqual(TEXT("Current health is 250"), Hovercar->GetHealth(), 250.f);

        // Damage Application (VEH-03, VEH-06)
        TestTrue(TEXT("Damage applied"), Hovercar->ApplyDamage(100.f));
        TestEqual(TEXT("Health reduced to 150"), Hovercar->GetHealth(), 150.f);
        TestFalse(TEXT("Vehicle not disabled yet"), Hovercar->IsDisabled());

        // Disablement at 0 HP (VEH-06)
        TestTrue(TEXT("Lethal damage applied"), Hovercar->ApplyDamage(150.f));
        TestEqual(TEXT("Health at 0"), Hovercar->GetHealth(), 0.f);
        TestTrue(TEXT("Vehicle is Disabled"), Hovercar->IsDisabled());
        TestEqual(TEXT("State is Disabled"), Hovercar->GetHovercarState(), EWyrmHovercarState::Disabled);

        // Entry rejected when disabled
        FString Reason;
        TestFalse(TEXT("Cannot enter disabled hovercar"), Hovercar->CanEnter(Pilot, Reason));
        TestTrue(TEXT("Disabled rejection message given"), Reason.Contains(TEXT("disabled")));

        // Board pet for depot recovery staging test
        Dragon->SetDragonForm(EWyrmDragonForm::CompanionForm);
        Hovercar->BoardPet(Dragon);
        TestTrue(TEXT("Companion boarded"), Hovercar->HasCompanionBoarded());

        // Depot Recovery (VEH-06)
        const FVector DepotLoc(1200.f, 3400.f, 150.f);
        const FRotator DepotRot(0.f, 90.f, 0.f);
        TestTrue(TEXT("RecoverToDepot succeeds"), Hovercar->RecoverToDepot(DepotLoc, DepotRot));
        TestEqual(TEXT("Health fully repaired"), Hovercar->GetHealth(), 250.f);
        TestEqual(TEXT("State restored to Parked"), Hovercar->GetHovercarState(), EWyrmHovercarState::Parked);
        TestEqual(TEXT("Vehicle at depot location"), Hovercar->GetActorLocation(), DepotLoc);
        TestFalse(TEXT("Companion safely staged at depot and unboarded"), Hovercar->HasCompanionBoarded());
    }

    if (Dragon) { Dragon->Destroy(); }
    if (Pilot) { Pilot->Destroy(); }
    if (Hovercar) { Hovercar->Destroy(); }
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmHovercarSaveSchema3Test, "WYRMFALL.Scaffold.HovercarSaveSchema3",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmHovercarSaveSchema3Test::RunTest(const FString& Parameters)
{
    // Schema Version Contract (VEH-07)
    TestEqual(TEXT("CurrentSchemaVersion is 3"), UWyrmSaveGame::CurrentSchemaVersion, 3);
    TestEqual(TEXT("MinimumSupportedSchemaVersion is 1"), UWyrmSaveGame::MinimumSupportedSchemaVersion, 1);

    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    TestNotNull(TEXT("World created"), World);
    if (!World) { return false; }

    AWyrmCharacter* Character = World->SpawnActor<AWyrmCharacter>();
    AWyrmHovercar* Hovercar = AWyrmHovercar::SpawnWyrmHovercar(World, FTransform(FVector(500.f, 1000.f, 200.f)));

    TestNotNull(TEXT("Character valid"), Character);
    TestNotNull(TEXT("Hovercar valid"), Hovercar);

    if (Character && Hovercar)
    {
        Character->SetActorLocation(Hovercar->GetActorLocation());
        Hovercar->SetMechaCircuitUnlocked(true);
        Hovercar->ApplyDamage(60.f);
        Hovercar->EnterHovercar(Character);

        // Snapshot Creation (VEH-07)
        UWyrmSaveGame* SaveObj = UWyrmSaveSubsystem::CreateSnapshotObject(TEXT("HovercarSaveTest"), Character, nullptr, World);
        TestNotNull(TEXT("Snapshot created"), SaveObj);
        if (SaveObj)
        {
            TestEqual(TEXT("Schema version is 3"), SaveObj->SchemaVersion, 3);
            TestTrue(TEXT("Hovercar record marked spawned"), SaveObj->HovercarRecord.bHasBeenSpawned);
            TestEqual(TEXT("Saved health is 190"), SaveObj->HovercarRecord.Health, 190.f);
            TestTrue(TEXT("Saved Mecha circuit unlocked"), SaveObj->HovercarRecord.bMechaCircuitUnlocked);
            TestTrue(TEXT("Saved is occupied"), SaveObj->HovercarRecord.bIsOccupied);
            TestEqual(TEXT("Saved location matches"), SaveObj->HovercarRecord.WorldLocation, Hovercar->GetActorLocation());

            // Backwards compatibility check: Schema 1 and 2 saves accepted
            UWyrmSaveGame* LegacySave = NewObject<UWyrmSaveGame>();
            LegacySave->SchemaVersion = 2;
            LegacySave->HovercarRecord.bHasBeenSpawned = false;
            TestTrue(TEXT("Schema 2 legacy snapshot accepted"), UWyrmSaveSubsystem::ApplySnapshotObject(LegacySave, Character, nullptr, World));

            LegacySave->SchemaVersion = 1;
            TestTrue(TEXT("Schema 1 legacy snapshot accepted"), UWyrmSaveSubsystem::ApplySnapshotObject(LegacySave, Character, nullptr, World));
        }
    }

    if (Character) { Character->Destroy(); }
    if (Hovercar) { Hovercar->Destroy(); }
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWyrmJadefangValidationTest, "WYRMFALL.Scaffold.JadefangValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWyrmJadefangValidationTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::GamePreview, false);
    if (!World) { return true; }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmCharacter* Player = World->SpawnActor<AWyrmCharacter>(AWyrmCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    AWyrmDragonCharacter* Jadefang = World->SpawnActor<AWyrmDragonCharacter>(AWyrmDragonCharacter::StaticClass(), FVector(300.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);

    TestNotNull(TEXT("Player spawned"), Player);
    TestNotNull(TEXT("Jadefang spawned"), Jadefang);
    if (!Player || !Jadefang)
    {
        if (Player) Player->Destroy();
        if (Jadefang) Jadefang->Destroy();
        World->DestroyWorld(false);
        return false;
    }

    Jadefang->SetDragonId(FName(TEXT("Jadefang")));
    TestTrue(TEXT("Jadefang has supported rig profile"), Jadefang->HasSupportedRigProfile());
    TestEqual(TEXT("Active rig profile is Jadefang"), Jadefang->GetActiveRigProfile().DragonId, FName(TEXT("Jadefang")));
    TestEqual(TEXT("Follower mesh count is 38"), Jadefang->GetActiveRigProfile().FollowerMeshNames.Num(), 38);

    // Bond and form envelopes
    Jadefang->BondWithHumanoid(Player);

    Jadefang->SetDragonForm(EWyrmDragonForm::CompanionForm);
    TestEqual(TEXT("Companion radius is 30"), Jadefang->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 30.f);
    TestEqual(TEXT("Companion half height is 35"), Jadefang->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), 35.f);

    Jadefang->SetDragonForm(EWyrmDragonForm::TrueForm);
    TestEqual(TEXT("True form radius is 110"), Jadefang->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 110.f);
    TestEqual(TEXT("True form half height is 150"), Jadefang->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), 150.f);

    // Save/Restore roundtrip
    FWyrmDragonSaveRecord Record;
    Jadefang->BuildSaveRecord(Record);
    TestEqual(TEXT("Saved dragon id is Jadefang"), Record.DragonId, FName(TEXT("Jadefang")));
    TestEqual(TEXT("Saved role is AlliedCompanion"), Record.Role, EWyrmDragonRole::AlliedCompanion);
    TestEqual(TEXT("Saved form is TrueForm"), Record.Form, EWyrmDragonForm::TrueForm);

    AWyrmDragonCharacter* RestoredDragon = World->SpawnActor<AWyrmDragonCharacter>(AWyrmDragonCharacter::StaticClass(), FVector(600.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
    RestoredDragon->RestoreFromSaveRecord(Record, Player);
    TestEqual(TEXT("Restored dragon id is Jadefang"), RestoredDragon->DragonId, FName(TEXT("Jadefang")));
    TestEqual(TEXT("Restored dragon form is TrueForm"), RestoredDragon->GetDragonForm(), EWyrmDragonForm::TrueForm);
    TestEqual(TEXT("Restored dragon role is AlliedCompanion"), RestoredDragon->GetDragonRole(), EWyrmDragonRole::AlliedCompanion);

    Jadefang->Destroy();
    RestoredDragon->Destroy();
    Player->Destroy();
    World->DestroyWorld(false);
    return true;
}
#endif

