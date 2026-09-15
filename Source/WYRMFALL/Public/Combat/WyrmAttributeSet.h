#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "WyrmAttributeSet.generated.h"

// Only GAS owns these values. 100 is a diagnostic initial value, not balance.
UCLASS()
class WYRMFALL_API UWyrmAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
public:
    UWyrmAttributeSet();
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData Health;
    UPROPERTY(BlueprintReadOnly, Category="Combat") FGameplayAttributeData MaxHealth;
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UWyrmAttributeSet, Health)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Health)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Health)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Health)
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UWyrmAttributeSet, MaxHealth)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(MaxHealth)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxHealth)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxHealth)

    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
private:
    void ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const;
};
