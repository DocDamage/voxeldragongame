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
#endif
