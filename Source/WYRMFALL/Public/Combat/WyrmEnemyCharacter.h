#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Combat/WyrmCombatTypes.h"
#include "WyrmEnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UWyrmAttributeSet;

UENUM(BlueprintType)
enum class EWyrmEnemyRole : uint8
{
    MeleeChaser UMETA(DisplayName="Melee Chaser (Wolf)"),
    RangedSkirmisher UMETA(DisplayName="Ranged Skirmisher")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyrmEnemyDeathDelegate, AWyrmEnemyCharacter*, DefeatedEnemy);

// Authoritative enemy character implementing GAS combat authority and status reactions.
UCLASS()
class WYRMFALL_API AWyrmEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AWyrmEnemyCharacter();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }
    UFUNCTION(BlueprintPure, Category="Combat") UAbilitySystemComponent* GetAbilitySystem() const { return AbilitySystem; }
    UFUNCTION(BlueprintPure, Category="Combat") UWyrmAttributeSet* GetAttributes() const { return Attributes; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
    EWyrmEnemyRole EnemyRole = EWyrmEnemyRole::MeleeChaser;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
    bool bIsBoss = false;

    UPROPERTY(BlueprintAssignable, Category="Enemy")
    FWyrmEnemyDeathDelegate OnEnemyDied;

    UFUNCTION(BlueprintCallable, Category="Enemy")
    void ConfigureForRole(EWyrmEnemyRole InRole);

    UFUNCTION(BlueprintCallable, Category="Enemy")
    bool AttackTarget(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category="Enemy")
    void ApplyStatusEffect(FGameplayTag StatusTag, float DurationSeconds, float Magnitude = 1.f);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void ApplyNamedStatusEffect(FName TagName, float DurationSeconds, float Magnitude = 1.f);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void SetInvulnerable(bool bInvulnerable);

    UFUNCTION(BlueprintPure, Category="Combat")
    bool HasMatchingGameplayTag(FName TagName) const;

    UFUNCTION(BlueprintPure, Category="Enemy")
    bool IsDefeated() const;

    UFUNCTION(BlueprintPure, Category="Enemy")
    float GetCurrentSpeed() const;

    // Static helper to spawn an enemy character in a world context
    UFUNCTION(BlueprintCallable, Category="Enemy", meta=(WorldContext="WorldContextObject"))
    static AWyrmEnemyCharacter* SpawnWyrmEnemy(UObject* WorldContextObject, EWyrmEnemyRole InRole, const FTransform& SpawnTransform);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
    TObjectPtr<UAbilitySystemComponent> AbilitySystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
    TObjectPtr<UWyrmAttributeSet> Attributes;

private:
    void HandleHealthChanged(const struct FOnAttributeChangeData& Data);
    void UpdateMovementForStatus();

    float BaseWalkSpeed = 600.f;
    float ActiveSlowMagnitude = 0.f;
    float SlowRemainingTimer = 0.f;
    float StunRemainingTimer = 0.f;
    float RootRemainingTimer = 0.f;
};
