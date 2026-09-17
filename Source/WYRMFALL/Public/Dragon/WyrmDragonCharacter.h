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

    // --- Identification & Role (DRG-01) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dragon")
    FName DragonId = FName(TEXT("Verdance"));

    UFUNCTION(BlueprintPure, Category="Dragon")
    EWyrmDragonRole GetDragonRole() const { return CurrentRole; }

    UFUNCTION(BlueprintCallable, Category="Dragon")
    void SetDragonRole(EWyrmDragonRole NewRole);

    UFUNCTION(BlueprintPure, Category="Dragon")
    bool HasBondReceipt() const { return bHasBondReceipt; }

    UFUNCTION(BlueprintCallable, Category="Dragon")
    bool PerformBossDefeat();

    UFUNCTION(BlueprintCallable, Category="Dragon")
    bool BondWithHumanoid(AWyrmCharacter* Humanoid);

    // --- Form & Scale (DRG-01, DRG-03) ---
    UFUNCTION(BlueprintPure, Category="Dragon")
    EWyrmDragonForm GetDragonForm() const { return CurrentForm; }

    UFUNCTION(BlueprintCallable, Category="Dragon")
    void SetDragonForm(EWyrmDragonForm NewForm);

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
};
