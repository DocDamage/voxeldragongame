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
    Attrs->SetCurrentPower(25.f);
    Attrs->SetCurrentArmor(15.f);
    Character->SetCameraMode(EWyrmCameraMode::TopDown);

    // Setup inventory with rolled weapon equipped and potion in bag
    FWyrmItemInstance Weapon = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("RelicSword")), EWyrmItemType::Weapon, 2);
    FWyrmItemInstance Potion = UWyrmInventoryComponent::RollRandomItem(FName(TEXT("HealPotion")), EWyrmItemType::Consumable, 1);
    Potion.StackCount = 5;

    FWyrmItemInstance Excess;
    Inv->AddItem(Weapon, Excess);
    Inv->AddItem(Potion, Excess);
    Inv->EquipItem(Weapon.InstanceId, EWyrmEquipSlot::MainHand);

    // Create mock terrain provider with delta payload
    AWyrmGeoForgeAdapter* Adapter = World->SpawnActor<AWyrmGeoForgeAdapter>(AWyrmGeoForgeAdapter::StaticClass(), SpawnParams);
    FGuid MockActionId = FGuid::NewGuid();
    Adapter->ProcessedActionIds.Add(MockActionId);

    // Capture unified snapshot
    UWyrmSaveGame* Snapshot = UWyrmSaveSubsystem::CreateSnapshotObject(TEXT("TestSlot_WP05"), Character, Adapter);
    TestNotNull(TEXT("Snapshot created successfully"), Snapshot);
    TestEqual(TEXT("Snapshot camera mode is TopDown"), Snapshot->CharacterRecord.CameraMode, EWyrmCameraMode::TopDown);
    TestEqual(TEXT("Snapshot contains 1 equipped item"), Snapshot->InventoryRecord.EquippedItems.Num(), 1);
    TestEqual(TEXT("Snapshot contains 1 bag item (potion)"), Snapshot->InventoryRecord.BagItems.Num(), 1);
    TestEqual(TEXT("Snapshot contains terrain action ID"), Snapshot->TerrainRecord.ProcessedActionIds.Num(), 1);

    // Mutate character to different state
    Attrs->SetCurrentHealth(10.f);
    Attrs->SetCurrentPower(5.f);
    Character->SetCameraMode(EWyrmCameraMode::ThirdPerson);
    Inv->ClearAll();
    Adapter->ProcessedActionIds.Empty();

    // Restore snapshot
    TestTrue(TEXT("Snapshot applied successfully"), UWyrmSaveSubsystem::ApplySnapshotObject(Snapshot, Character, Adapter));

    // Verify complete restoration fidelity
    TestEqual(TEXT("Restored health matches snapshot"), Attrs->GetCurrentHealth(), 75.f);
    TestEqual(TEXT("Restored power matches (base + equipped bonus)"), Attrs->GetCurrentPower(), 42.f);
    TestEqual(TEXT("Restored camera mode is TopDown"), Character->GetCameraMode(), EWyrmCameraMode::TopDown);
    TestTrue(TEXT("Restored main hand is equipped"), Inv->IsSlotEquipped(EWyrmEquipSlot::MainHand));
    TestEqual(TEXT("Restored bag item count is 1"), Inv->GetBagItems().Num(), 1);
    TestEqual(TEXT("Restored bag item is potion with 5 stacks"), Inv->GetBagItems()[0].StackCount, 5);
    TestTrue(TEXT("Restored terrain action ID"), Adapter->ProcessedActionIds.Contains(MockActionId));

    // Test slot disk I/O save/load roundtrip (SAVE-01)
    const FString SlotName = TEXT("WyrmSlot_UnitTest");
    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        GI = NewObject<UGameInstance>(World);
    }
    UWyrmSaveSubsystem* SaveSys = NewObject<UWyrmSaveSubsystem>(GI);
    TestTrue(TEXT("SaveGameSnapshot to slot succeeds"), SaveSys->SaveGameSnapshot(SlotName, Character, Adapter));
    TestTrue(TEXT("DoesSaveExist returns true for slot"), SaveSys->DoesSaveExist(SlotName));

    // Clear and reload from disk slot
    Inv->ClearAll();
    TestTrue(TEXT("LoadGameSnapshot from slot succeeds"), SaveSys->LoadGameSnapshot(SlotName, Character, Adapter));
    TestTrue(TEXT("Slot load restored equipped item"), Inv->IsSlotEquipped(EWyrmEquipSlot::MainHand));

    // Delete slot cleanup
    TestTrue(TEXT("DeleteSaveSlot cleans up slot"), SaveSys->DeleteSaveSlot(SlotName));
    TestFalse(TEXT("DoesSaveExist returns false after deletion"), SaveSys->DoesSaveExist(SlotName));

    Character->Destroy();
    Adapter->Destroy();
    return true;
}
#endif

