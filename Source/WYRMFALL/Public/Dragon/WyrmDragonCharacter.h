#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Dragon/WyrmDragonTypes.h"
#include "WyrmDragonCharacter.generated.h"

class UAbilitySystemComponent;
class UWyrmAttributeSet;
class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class AWyrmCharacter;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyrmDragonDefeatDelegate, AWyrmDragonCharacter*, Dragon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyrmTetherWarningDelegate, EWyrmTetherStatus, Status, float, DistanceToHumanoid);

// Authoritative Green Dragon (Verdance) Character implementing boss defeat, one-way bond, companion AI, and direct control.
UCLASS()
class WYRMFALL_API AWyrmDragonCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AWyrmDragonCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }
    UFUNCTION(BlueprintPure, Category="Combat") UAbilitySystemComponent* GetAbilitySystem() const { return AbilitySystem; }
    UFUNCTION(BlueprintPure, Category="Combat") UWyrmAttributeSet* GetAttributes() const { return Attributes; }

    // --- Identification & Role (DRG-01, WP-20) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon")
    FName DragonId = FName(TEXT("Verdance"));

    UFUNCTION(BlueprintCallable, Category="Dragon")
    void SetDragonId(FName NewDragonId);

    // Validates whether DragonId corresponds to an authoritative rig profile (Verdance, Jadefang)
    UFUNCTION(BlueprintPure, Category="Dragon|Rig")
    bool HasSupportedRigProfile() const { return FWyrmDragonRigProfile::IsValidDragonRig(DragonId); }

    UFUNCTION(BlueprintPure, Category="Dragon|Rig")
    const FWyrmDragonRigProfile& GetActiveRigProfile() const { return ActiveRigProfile; }

    UFUNCTION(BlueprintPure, Category="Dragon")
    EWyrmDragonRole GetDragonRole() const { return CurrentRole; }

    UFUNCTION(BlueprintCallable, Category="Dragon")
    void SetDragonRole(EWyrmDragonRole NewRole);

    UFUNCTION(BlueprintPure, Category="Dragon")
    bool HasBondReceipt() const { return bHasBondReceipt; }

    UFUNCTION(BlueprintCallable, Category="Dragon")
    void ResetBondReceipt() { bHasBondReceipt = false; }

    UFUNCTION(BlueprintCallable, Category="Dragon")
    bool PerformBossDefeat();

    UFUNCTION(BlueprintCallable, Category="Dragon")
    bool BondWithHumanoid(AWyrmCharacter* Humanoid);

    UFUNCTION(BlueprintPure, Category="Dragon|Bond")
    bool CanOfferVoluntaryBond(const AActor* Interactor) const;

    UFUNCTION(BlueprintCallable, Category="Dragon|Bond")
    bool InteractVoluntaryBond(AWyrmCharacter* Humanoid);

    // --- Form & Scale (DRG-01, DRG-03, DRG-08..13) ---
    UFUNCTION(BlueprintPure, Category="Dragon|Form")
    EWyrmDragonForm GetDragonForm() const { return CurrentForm; }

    // Fixture/save restoration helper. Normal gameplay must use RequestFormChange.
    UFUNCTION(BlueprintCallable, Category="Dragon|Form")
    void SetDragonForm(EWyrmDragonForm NewForm);

    UFUNCTION(BlueprintPure, Category="Dragon|Form")
    bool CanChangeForm(EWyrmDragonForm TargetForm, FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Dragon|Form")
    bool RequestFormChange(EWyrmDragonForm TargetForm);

    UFUNCTION(BlueprintCallable, Category="Dragon|Form")
    void InterruptFormTransition();

    UFUNCTION(BlueprintPure, Category="Dragon|Form")
    bool IsTransitioningForm() const { return bIsTransitioningForm; }

    UFUNCTION(BlueprintPure, Category="Dragon|Form")
    float GetFormTransitionProgress() const;

    UFUNCTION(BlueprintPure, Category="Dragon|Form")
    float GetFormTransitionCooldownRemaining() const { return FormTransitionCooldownRemaining; }

    // --- Town / Safe Zone Behavior (DRG-13) ---
    UFUNCTION(BlueprintCallable, Category="Dragon|Town")
    void SetTownModeEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Dragon|Town")
    bool IsTownModeEnabled() const { return bTownModeEnabled; }

    UFUNCTION(BlueprintPure, Category="Dragon|Locomotion")
    float GetCompanionGroundSpeed() const { return CompanionGroundSpeed; }

    UFUNCTION(BlueprintPure, Category="Dragon|Locomotion")
    float GetCompanionCatchUpSpeed() const { return CompanionCatchUpSpeed; }

    UFUNCTION(BlueprintPure, Category="Dragon|Locomotion")
    float GetTrueFormGroundSpeed() const { return TrueFormGroundSpeed; }

    // --- Companion Orders & AI (DRG-02) ---
    UFUNCTION(BlueprintPure, Category="Dragon|Companion")
    EWyrmCompanionOrder GetCompanionOrder() const { return CurrentOrder; }

    UFUNCTION(BlueprintCallable, Category="Dragon|Companion")
    void IssueOrder(EWyrmCompanionOrder NewOrder, AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintPure, Category="Dragon|Companion")
    AActor* GetCombatTarget() const { return CurrentCombatTarget.Get(); }

    UFUNCTION(BlueprintPure, Category="Dragon|Companion")
    AWyrmCharacter* GetWaitingHumanoid() const { return WaitingHumanoid.Get(); }

    // --- Combat & Attacks (DRG-02, DRG-03) ---
    UFUNCTION(BlueprintCallable, Category="Dragon|Combat")
    bool PerformPrimaryAttack(AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintCallable, Category="Dragon|Combat")
    bool PerformSecondaryAttack(AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintPure, Category="Dragon|Combat")
    float GetAreaAttackCooldownRemaining() const { return AreaAttackCooldownRemaining; }

    // --- Direct Control & Tethering (DRG-03, DRG-04) ---
    UFUNCTION(BlueprintPure, Category="Dragon|Control")
    bool IsDirectlyControlled() const { return bIsDirectlyControlled; }

    UFUNCTION(BlueprintCallable, Category="Dragon|Control")
    bool StartDirectControl(APlayerController* InPlayerController, AWyrmCharacter* HumanoidBody);

    UFUNCTION(BlueprintCallable, Category="Dragon|Control")
    bool EndDirectControl(APlayerController* InPlayerController);

    UFUNCTION(BlueprintPure, Category="Dragon|Control")
    EWyrmTetherStatus GetTetherStatus() const { return CurrentTetherStatus; }

    UFUNCTION(BlueprintCallable, Category="Dragon|Control")
    void CheckTetherStatus();

    UFUNCTION(BlueprintPure, Category="Dragon|Control")
    float GetDistanceToHumanoid() const;

    UFUNCTION(BlueprintCallable, Category="Dragon|Control")
    void HandleWaitingBodyDamaged(float DamageAmount);

    // --- Riding & Mounting (DRG-05) ---
    UFUNCTION(BlueprintPure, Category="Dragon|Mount")
    bool CanMount(AWyrmCharacter* Humanoid, FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Dragon|Mount")
    bool MountHumanoid(AWyrmCharacter* Humanoid);

    UFUNCTION(BlueprintPure, Category="Dragon|Mount")
    bool CanDismount(FVector& OutGroundLocation, FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Dragon|Mount")
    bool DismountHumanoid(FVector& OutDismountLocation);

    UFUNCTION(BlueprintPure, Category="Dragon|Mount")
    bool IsRiderMounted() const { return MountedRider.IsValid(); }

    UFUNCTION(BlueprintPure, Category="Dragon|Mount")
    AWyrmCharacter* GetMountedRider() const { return MountedRider.Get(); }

    // --- Flight Locomotion (DRG-06) ---
    UFUNCTION(BlueprintPure, Category="Dragon|Flight")
    EWyrmDragonFlightState GetFlightState() const { return CurrentFlightState; }

    UFUNCTION(BlueprintPure, Category="Dragon|Flight")
    bool IsInFlight() const { return CurrentFlightState == EWyrmDragonFlightState::Flying || CurrentFlightState == EWyrmDragonFlightState::TakingOff; }

    UFUNCTION(BlueprintPure, Category="Dragon|Flight")
    bool CanTakeOff(FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Dragon|Flight")
    bool TakeOff();

    UFUNCTION(BlueprintPure, Category="Dragon|Flight")
    bool CanLand(FVector& OutLandingLocation, FString& OutReason) const;

    UFUNCTION(BlueprintCallable, Category="Dragon|Flight")
    bool Land();

    UFUNCTION(BlueprintPure, Category="Dragon|Flight")
    FVector GetSafeGroundAnchor() const;

    // --- Mounted Defeat & Hub Recovery (DRG-07, DRG-14) ---
    UFUNCTION(BlueprintCallable, Category="Dragon|Recovery")
    void HandleMountedDefeat();

    UFUNCTION(BlueprintCallable, Category="Dragon|Recovery")
    bool RecoverCompanion();

    // --- Camera Preferences ---
    UFUNCTION(BlueprintCallable, Category="Dragon|Camera")
    void SetFlightCameraMode(bool bTopDown);

    // --- Mesh & Visuals ---
    UFUNCTION(BlueprintCallable, Category="Dragon|Visuals")
    void SetupModularMeshes();

    // --- Save/Load Persistence (SAVE-08) ---
    UFUNCTION(BlueprintCallable, Category="Dragon|Save")
    void BuildSaveRecord(FWyrmDragonSaveRecord& OutRecord) const;

    UFUNCTION(BlueprintCallable, Category="Dragon|Save")
    void RestoreFromSaveRecord(const FWyrmDragonSaveRecord& InRecord, AWyrmCharacter* HumanoidContext);

    // Static spawn helper
    UFUNCTION(BlueprintCallable, Category="Dragon", meta=(WorldContext="WorldContextObject"))
    static AWyrmDragonCharacter* SpawnWyrmDragon(UObject* WorldContextObject, EWyrmDragonRole InRole, const FTransform& SpawnTransform);

    // Events
    UPROPERTY(BlueprintAssignable, Category="Dragon|Events")
    FWyrmDragonDefeatDelegate OnDragonDefeated;

    UPROPERTY(BlueprintAssignable, Category="Dragon|Events")
    FWyrmTetherWarningDelegate OnTetherWarningChanged;

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
    void EnsureAbilitySystemInitialized();

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
    TObjectPtr<UAbilitySystemComponent> AbilitySystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
    TObjectPtr<UWyrmAttributeSet> Attributes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
    TArray<TObjectPtr<USkeletalMeshComponent>> FollowerMeshComponents;

private:
    void HandleHealthChanged(const struct FOnAttributeChangeData& Data);
    void UpdateCompanionAI(float DeltaSeconds);
    void ApplyFormDimensions();
    void CompleteFormTransition();
    void InitializeRigProfile();

    UPROPERTY(VisibleAnywhere, Category="Dragon|Rig")
    FWyrmDragonRigProfile ActiveRigProfile;

    UPROPERTY(VisibleAnywhere, Category="Dragon|Form")
    bool bIsTransitioningForm = false;

    UPROPERTY(VisibleAnywhere, Category="Dragon|Form")
    EWyrmDragonForm PendingForm = EWyrmDragonForm::CompanionForm;

    UPROPERTY(VisibleAnywhere, Category="Dragon|Form")
    float FormTransitionTimeRemaining = 0.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Form")
    float FormTransitionDuration = 1.0f;

    UPROPERTY(VisibleAnywhere, Category="Dragon|Form")
    float FormTransitionCooldownRemaining = 0.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Form")
    float FormTransitionCooldownDuration = 4.0f;

    UPROPERTY(VisibleAnywhere, Category="Dragon|Town")
    bool bTownModeEnabled = false;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Locomotion")
    float CompanionGroundSpeed = 450.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Locomotion")
    float CompanionCatchUpSpeed = 600.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Locomotion")
    float TrueFormGroundSpeed = 550.f;

    UPROPERTY(VisibleAnywhere, Category="Dragon|State")
    EWyrmDragonRole CurrentRole = EWyrmDragonRole::HostileBoss;

    UPROPERTY(VisibleAnywhere, Category="Dragon|State")
    EWyrmDragonForm CurrentForm = EWyrmDragonForm::CompanionForm;

    UPROPERTY(VisibleAnywhere, Category="Dragon|State")
    EWyrmCompanionOrder CurrentOrder = EWyrmCompanionOrder::Follow;

    UPROPERTY(VisibleAnywhere, Category="Dragon|State")
    EWyrmTetherStatus CurrentTetherStatus = EWyrmTetherStatus::WithinTether;

    UPROPERTY(VisibleAnywhere, Category="Dragon|State")
    bool bHasBondReceipt = false;

    UPROPERTY(VisibleAnywhere, Category="Dragon|State")
    bool bIsDirectlyControlled = false;

    UPROPERTY(VisibleAnywhere, Category="Dragon|Combat")
    float AreaAttackCooldownRemaining = 0.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Combat")
    float AreaAttackCooldownDuration = 6.0f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Tether")
    float TetherWarningDistance = 12000.f; // 120m

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Tether")
    float TetherLimitDistance = 15000.f; // 150m

    UPROPERTY(EditDefaultsOnly, Category="Dragon|AI")
    float CompanionFollowDistance = 350.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|AI")
    float AttackRange = 350.f;

    TWeakObjectPtr<AWyrmCharacter> WaitingHumanoid;
    TWeakObjectPtr<AActor> CurrentCombatTarget;

    UPROPERTY(VisibleAnywhere, Category="Dragon|State")
    EWyrmDragonFlightState CurrentFlightState = EWyrmDragonFlightState::Grounded;

    TWeakObjectPtr<AWyrmCharacter> MountedRider;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Mount")
    FVector MountSocketOffset = FVector(0.f, 0.f, 160.f);

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Flight")
    float FlightSpeed = 1600.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Flight")
    float GroundSpeed = 600.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Flight")
    float TakeoffClearanceHeight = 500.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Flight")
    float WingSpanSweepRadius = 350.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Flight")
    float LandingSearchDistance = 1200.f;

    UPROPERTY(EditDefaultsOnly, Category="Dragon|Flight")
    float MaxLandingSlopeAngle = 45.f;
};
