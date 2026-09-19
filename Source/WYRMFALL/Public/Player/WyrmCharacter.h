#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "Player/WyrmControlTypes.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "WyrmCharacter.generated.h"
class UAbilitySystemComponent;
class UWyrmAttributeSet;
class USpringArmComponent;
class UCameraComponent;
class UCustomizableSkeletalComponent;
class UCustomizableObject;
class UCustomizableObjectInstance;
class UWyrmInventoryComponent;
class UWyrmFishingComponent;

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmActiveFoodBuff
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    FName BuffId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    FText BuffName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float RemainingDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float TotalDuration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float MaxFocusPercentBonus = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float HealthRegenPerSecond = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
    float PowerBonus = 0.f;

    bool IsActive() const { return !BuffId.IsNone() && RemainingDuration > 0.f; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyrmCharacterDamagedDelegate, float, DamageAmount);

// Humanoid host with Mutable visual authority.
UCLASS()
class WYRMFALL_API AWyrmCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AWyrmCharacter();

    UPROPERTY(BlueprintAssignable, Category="Combat")
    FWyrmCharacterDamagedDelegate OnCharacterDamaged;
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UFUNCTION(BlueprintPure, Category="Combat") UAbilitySystemComponent* GetAbilitySystem() const { return AbilitySystem; }
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintCallable, Category="Camera") void SetCameraMode(EWyrmCameraMode NewMode);
    UFUNCTION(BlueprintCallable, Category="Camera") void ToggleCamera();
    UFUNCTION(BlueprintPure, Category="Camera") EWyrmCameraMode GetCameraMode() const { return CameraMode; }
    UFUNCTION(BlueprintPure, Category="Camera") USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    UFUNCTION(BlueprintPure, Category="Camera") UCameraComponent* GetFollowCamera() const { return Camera; }

    // --- Control and Gating (WP-03) ---
    UFUNCTION(BlueprintCallable, Category="Control") void SetMovementLocked(bool bLocked);
    UFUNCTION(BlueprintPure, Category="Control") bool IsMovementLocked() const { return bMovementLocked; }
    UFUNCTION(BlueprintCallable, Category="Control") void CaptureControlState(FWyrmControlState& OutState) const;
    UFUNCTION(BlueprintCallable, Category="Control") void RestoreControlState(const FWyrmControlState& InState);

    // --- Combat and Abilities (WP-04, WP-06) ---
    UFUNCTION(BlueprintPure, Category="Combat") UWyrmAttributeSet* GetAttributes() const { return Attributes; }
    UFUNCTION(BlueprintCallable, Category="Combat") bool PerformPrimaryAttack();
    UFUNCTION(BlueprintCallable, Category="Combat") bool PerformSecondaryAttack();
    UFUNCTION(BlueprintCallable, Category="Combat") bool AttackTarget(AActor* TargetActor);
    UFUNCTION(BlueprintCallable, Category="Combat") bool PerformEvade();
    UFUNCTION(BlueprintCallable, Category="Combat") void GrantCombatAbilities();
    UFUNCTION(BlueprintPure, Category="Combat") EWyrmWeaponFamily GetActiveWeaponFamily() const { return ActiveWeaponFamily; }
    UFUNCTION(BlueprintCallable, Category="Combat") void UpdateActiveWeaponKit();

    // --- Status Reactions & Gating (WP-15) ---
    UFUNCTION(BlueprintCallable, Category="Combat|Status")
    void ApplyStatusEffect(FGameplayTag StatusTag, float DurationSeconds, float Magnitude = 1.f);

    UFUNCTION(BlueprintCallable, Category="Combat|Status")
    void ApplyNamedStatusEffect(FName TagName, float DurationSeconds, float Magnitude = 1.f);

    UFUNCTION(BlueprintCallable, Category="Combat|Status")
    void ClearNamedStatusEffect(FName TagName);

    UFUNCTION(BlueprintCallable, Category="Combat|Status")
    void UpdateMovementForStatus();

    UFUNCTION(BlueprintPure, Category="Combat|Status")
    float GetCurrentSpeed() const;

    UFUNCTION(BlueprintPure, Category="Combat|Status")
    bool HasMatchingGameplayTag(FName TagName) const;

    // --- Horror Echo Powers (WP-15) ---
    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    bool LearnEcho(FName EchoId);

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    bool EquipEcho(FName EchoId);

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void UnequipEcho();

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    bool ActivateEquippedEcho();

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void ActivateRelentlessAdvanceStance(float Duration);

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool IsEchoUnlocked(FName EchoId) const;

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool IsEchoEquipped(FName EchoId) const;

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    FName GetEquippedEcho() const { return EquippedEcho; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    const TArray<FName>& GetLearnedEchoes() const { return LearnedEchoes; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool IsRelentlessAdvanceActive() const { return bRelentlessAdvanceActive; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    float GetRelentlessAdvanceRemainingDuration() const { return RelentlessAdvanceRemainingTimer; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    float GetRelentlessAdvanceRemainingCooldown() const { return RelentlessAdvanceCooldownTimer; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool CanActivateRelentlessAdvance(FString& OutFailureReason) const;

    // --- Moonbound Form (WP-21 / ECHO-08, ECHO-09) ---
    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void ActivateMoonboundForm(float Duration);

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void DeactivateMoonboundForm();

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool IsMoonboundActive() const { return bMoonboundActive; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    float GetMoonboundRemainingDuration() const { return MoonboundRemainingTimer; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    float GetMoonboundRemainingCooldown() const { return MoonboundCooldownTimer; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool CanActivateMoonboundForm(FString& OutFailureReason) const;

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool IsMoonboundReturnPending() const { return bMoonboundReturnPending; }

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    bool ResolveMoonboundReturnBlockage();

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    FVector GetLastSafeHumanoidLocation() const { return LastSafeHumanoidLocation; }

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void SetLastSafeHumanoidLocation(const FVector& InLocation) { LastSafeHumanoidLocation = InLocation; }

    UFUNCTION(BlueprintPure, Category="Appearance")
    USkeletalMeshComponent* GetBeastMeshComponent() const { return BeastMeshComponent; }

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void RestoreEchoState(const TArray<FName>& InLearnedEchoes, FName InEquippedEcho, bool bActive, float RemainingDuration, float RemainingCooldown);

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void RestoreMoonboundState(bool bInActive, float RemainingDuration, float RemainingCooldown, bool bInPending, const FVector& InLastSafeLoc);

    // --- Mirror Step (WP-22 / JP-05) ---
    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool CanActivateMirrorStep(const FVector& Destination, FString& OutFailureReason) const;

    /** Reflection-friendly single-value view of the authoritative validator. Empty means valid. */
    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    FString GetMirrorStepFailureReason(const FVector& Destination) const;

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    bool ActivateMirrorStep(const FVector& Destination);

    /** Called only by UWyrmMirrorStepAbility after GAS commits cost/cooldown. */
    bool CommitMirrorStep();

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    FVector GetPendingMirrorStepDestination() const { return PendingMirrorStepDestination; }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    float GetMirrorStepRemainingCooldown() const { return MirrorStepCooldownTimer; }

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void RestoreMirrorStepState(float RemainingCooldown);

    // --- Unseen Hand (WP-23.2) ---
    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    bool CanActivateUnseenHand(AActor* Target, FString& OutFailureReason) const;

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    FString GetUnseenHandFailureReason(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    bool ActivateUnseenHand(AActor* Target);

    bool CommitUnseenHand();

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    AActor* GetPendingUnseenHandTarget() const { return PendingUnseenHandTarget.Get(); }

    UFUNCTION(BlueprintPure, Category="Combat|Echo")
    float GetUnseenHandRemainingCooldown() const { return UnseenHandCooldownTimer; }

    UFUNCTION(BlueprintCallable, Category="Combat|Echo")
    void RestoreUnseenHandState(float RemainingCooldown);

    // --- Progression & XP (WP-06) ---
    UFUNCTION(BlueprintCallable, Category="Progression") bool AddExperience(float Amount);
    UFUNCTION(BlueprintPure, Category="Progression") float GetCurrentXP() const { return CurrentXP; }
    UFUNCTION(BlueprintCallable, Category="Progression") void SetCurrentXP(float InXP);
    UFUNCTION(BlueprintPure, Category="Progression") float GetCharacterLevel() const;
    UFUNCTION(BlueprintCallable, Category="Progression") void SetCharacterLevel(float NewLevel);
    UFUNCTION(BlueprintPure, Category="Progression") static float CalculateXPForNextLevel(float InLevel);
    UFUNCTION(BlueprintPure, Category="Progression") float GetXPForNextLevel() const;
    UFUNCTION(BlueprintPure, Category="Progression") float GetXPToNextLevel() const;

    // --- Mutable Appearance Authority (WP-02) ---
    UFUNCTION(BlueprintPure, Category="Appearance")
    UCustomizableSkeletalComponent* GetCustomizableComponent() const { return CustomizableSkeletalComponent; }

    UFUNCTION(BlueprintPure, Category="Appearance")
    UCustomizableObjectInstance* GetCustomizableInstance() const { return CustomizableInstance; }

    UFUNCTION(BlueprintCallable, Category="Appearance")
    bool SetCustomizableObject(UCustomizableObject* InCustomizableObject);

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetCustomizableInstance(UCustomizableObjectInstance* InInstance);

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetColorParameter(FName ParamName, FLinearColor Color);

    UFUNCTION(BlueprintPure, Category="Appearance")
    FLinearColor GetColorParameter(FName ParamName) const;

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetOptionParameter(FName ParamName, const FString& OptionName);

    UFUNCTION(BlueprintPure, Category="Appearance")
    FString GetOptionParameter(FName ParamName) const;

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetFloatParameter(FName ParamName, float Value);

    UFUNCTION(BlueprintPure, Category="Appearance")
    float GetFloatParameter(FName ParamName) const;

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void ApplyAppearance(bool bAsync = true);

    UFUNCTION(BlueprintCallable, Category="Appearance")
    FString CaptureAppearanceDescriptor() const;

    UFUNCTION(BlueprintCallable, Category="Appearance")
    bool RestoreAppearanceDescriptor(const FString& InDescriptor);

    // --- Scale & Proportions Authority (CHAR-07) ---
    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetCharacterScale(FVector NewScale);

    UFUNCTION(BlueprintPure, Category="Appearance")
    FVector GetCharacterScale() const { return CharacterScale; }

    // --- Equipment Attachment (CHAR-04) ---
    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool AttachEquipmentMesh(USceneComponent* ItemMesh, FName SocketName);

    UFUNCTION(BlueprintPure, Category="Equipment")
    bool IsSocketValid(FName SocketName) const;

    // --- Inventory & Equipment (WP-05) ---
    UFUNCTION(BlueprintPure, Category="Inventory")
    UWyrmInventoryComponent* GetInventory() const { return InventoryComponent; }

    // --- Wet / Water Status (WRLD-10) ---
    UFUNCTION(BlueprintPure, Category="Status")
    bool IsWet() const { return bIsWet; }

    UFUNCTION(BlueprintCallable, Category="Status")
    void SetWet(bool bInWet);

    // --- Activities & Fishing (ACT-01, ACT-03) ---
    UFUNCTION(BlueprintPure, Category="Activities")
    UWyrmFishingComponent* GetFishing() const { return FishingComponent; }

    // --- Consumables & Food Preparation Buffs (ACT-02, ACT-04) ---
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool ConsumeItem(const FGuid& ItemInstanceId);

    UFUNCTION(BlueprintPure, Category="Buff")
    bool HasActiveFoodBuff() const { return ActiveFoodBuff.IsActive(); }

    UFUNCTION(BlueprintPure, Category="Buff")
    const FWyrmActiveFoodBuff& GetActiveFoodBuff() const { return ActiveFoodBuff; }

    UFUNCTION(BlueprintCallable, Category="Buff")
    void ApplyFoodBuff(const FWyrmActiveFoodBuff& InBuff);

    UFUNCTION(BlueprintCallable, Category="Buff")
    void ClearFoodBuff();

    UFUNCTION(BlueprintCallable, Category="Buff")
    void SetActiveFoodBuffRemainingDuration(float InDuration) { ActiveFoodBuff.RemainingDuration = InDuration; }

protected:
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    UPROPERTY(VisibleAnywhere, Category="Combat") TObjectPtr<UAbilitySystemComponent> AbilitySystem;
    UPROPERTY(VisibleAnywhere, Category="Combat") TObjectPtr<UWyrmAttributeSet> Attributes;
    UPROPERTY(VisibleAnywhere, Category="Camera") TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, Category="Camera") TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Appearance")
    TObjectPtr<UCustomizableSkeletalComponent> CustomizableSkeletalComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Appearance")
    TObjectPtr<USkeletalMeshComponent> BeastMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
    TObjectPtr<UCustomizableObjectInstance> CustomizableInstance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
    FVector CharacterScale = FVector::OneVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    TObjectPtr<UWyrmInventoryComponent> InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Activities")
    TObjectPtr<UWyrmFishingComponent> FishingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Buff")
    FWyrmActiveFoodBuff ActiveFoodBuff;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Status")
    bool bIsWet = false;

private:
    void ApplyCamera();
    void CheckLevelUp();
    void HandleHealthChanged(const struct FOnAttributeChangeData& Data);

    UPROPERTY(VisibleAnywhere, Category="Camera") EWyrmCameraMode CameraMode = EWyrmCameraMode::ThirdPerson;
    UPROPERTY(VisibleAnywhere, Category="Control") bool bMovementLocked = false;
    UPROPERTY(VisibleAnywhere, Category="Combat") EWyrmWeaponFamily ActiveWeaponFamily = EWyrmWeaponFamily::Unarmed;
    UPROPERTY(VisibleAnywhere, Category="Progression") float CurrentXP = 0.f;

    FGameplayAbilitySpecHandle PrimaryMeleeHandle;
    FGameplayAbilitySpecHandle SecondaryMeleeHandle;
    FGameplayAbilitySpecHandle PrimaryRangedHandle;
    FGameplayAbilitySpecHandle SecondaryRangedHandle;
    FGameplayAbilitySpecHandle EvadeHandle;
    FGameplayAbilitySpecHandle RelentlessAdvanceHandle;
    FGameplayAbilitySpecHandle MoonboundFormHandle;
    FGameplayAbilitySpecHandle MirrorStepHandle;
    FGameplayAbilitySpecHandle UnseenHandHandle;
    FGameplayAbilitySpecHandle PrimaryBeastClawHandle;
    FGameplayAbilitySpecHandle SecondaryBeastPounceHandle;

    float BaseWalkSpeed = 600.f;
    float ActiveSlowMagnitude = 0.f;
    float SlowRemainingTimer = 0.f;
    float StunRemainingTimer = 0.f;
    float RootRemainingTimer = 0.f;
    float StaggerRemainingTimer = 0.f;

    bool bRelentlessAdvanceActive = false;
    float RelentlessAdvanceRemainingTimer = 0.f;
    float RelentlessAdvanceCooldownTimer = 0.f;

    bool bMoonboundActive = false;
    float MoonboundRemainingTimer = 0.f;
    float MoonboundCooldownTimer = 0.f;
    bool bMoonboundReturnPending = false;
    FVector LastSafeHumanoidLocation = FVector::ZeroVector;
    float SavedHumanoidCapsuleRadius = 42.f;
    float SavedHumanoidCapsuleHalfHeight = 96.f;

    float MirrorStepCooldownTimer = 0.f;
    FVector PendingMirrorStepDestination = FVector::ZeroVector;
    bool bHasPendingMirrorStepDestination = false;

    float UnseenHandCooldownTimer = 0.f;
    TWeakObjectPtr<AActor> PendingUnseenHandTarget;

    TArray<FName> LearnedEchoes;
    FName EquippedEcho = NAME_None;
};
