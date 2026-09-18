#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "WyrmAttributeSet.generated.h"

#define WYRM_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// GAS is the sole authority for combat attributes, damage, and resources.
UCLASS()
class WYRMFALL_API UWyrmAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
public:
    UWyrmAttributeSet();

    // --- Health & Vitality ---
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Health;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, Health)

    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData MaxHealth;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, MaxHealth)

    // --- Focus & Resources (COM-04) ---
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Focus;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, Focus)

    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData MaxFocus;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, MaxFocus)

    // --- Combat Power & Defense ---
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Power;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, Power)

    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Armor;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, Armor)

    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData CharacterLevel;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, CharacterLevel)

    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Shield;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, Shield)

    // --- Meta Attributes (Transient for Execution / ModCallbacks) ---
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData IncomingDamage;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, IncomingDamage)

    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData IncomingHealing;
    WYRM_ATTRIBUTE_ACCESSORS(UWyrmAttributeSet, IncomingHealing)

    // --- Blueprint / Python Accessors ---
    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentHealth() const { return GetHealth(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentHealth(float NewVal);

    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentMaxHealth() const { return GetMaxHealth(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentMaxHealth(float NewVal);

    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentFocus() const { return GetFocus(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentFocus(float NewVal);

    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentMaxFocus() const { return GetMaxFocus(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentMaxFocus(float NewVal);

    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentCharacterLevel() const { return GetCharacterLevel(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentCharacterLevel(float NewVal);

    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentPower() const { return GetPower(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentPower(float NewVal);

    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentArmor() const { return GetArmor(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentArmor(float NewVal);

    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentShield() const { return GetShield(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentShield(float NewVal);

    UFUNCTION(BlueprintPure, Category="Combat") float GetCurrentIncomingDamage() const { return GetIncomingDamage(); }
    UFUNCTION(BlueprintCallable, Category="Combat") void SetCurrentIncomingDamage(float NewVal) { SetIncomingDamage(NewVal); }

    // --- Attribute Lifecycle ---
    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    // --- Canonical Combat Formulas (COM-01) ---
    UFUNCTION(BlueprintPure, Category="Combat")
    static float CalculatePhysicalMitigation(float InArmor, float InAttackerLevel);

    UFUNCTION(BlueprintPure, Category="Combat")
    static float CalculateRawDamage(float InWeaponBase, float InPower, float InPowerCoefficient = 0.5f);

    UFUNCTION(BlueprintPure, Category="Combat")
    static float CalculateMitigatedDamage(float InRawDamage, float InArmor, float InAttackerLevel);

    UFUNCTION(BlueprintPure, Category="Combat")
    static float CalculateMitigatedDamageWithAbilityReduction(float InRawDamage, float InArmor, float InAttackerLevel, float InAbilityReductionPercent);

    UFUNCTION(BlueprintPure, Category="Combat")
    static float GetMaxAbilityDamageReductionPercent() { return 0.75f; } // 75% explicit cap per Section 9

    UFUNCTION(BlueprintPure, Category="Combat")
    static float CalculateMaxHealthForLevel(float InLevel);

    UFUNCTION(BlueprintPure, Category="Combat")
    static float CalculatePowerForLevel(float InLevel);

private:
    void ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const;
};
