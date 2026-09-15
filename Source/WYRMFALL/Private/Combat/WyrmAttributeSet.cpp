#include "Combat/WyrmAttributeSet.h"
#include "GameplayEffectExtension.h"

UWyrmAttributeSet::UWyrmAttributeSet()
{
    InitMaxHealth(100.f);
    InitHealth(100.f);
}
void UWyrmAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const
{
    // Do not clamp future unrelated attributes through a global fallback.
    if (Attribute != GetHealthAttribute() && Attribute != GetMaxHealthAttribute()) { return; }
    if (!FMath::IsFinite(NewValue)) { NewValue = 0.f; }
    const float SafeMax = FMath::IsFinite(GetMaxHealth()) ? FMath::Max(1.f, GetMaxHealth()) : 1.f;
    if (Attribute == GetMaxHealthAttribute()) { NewValue = FMath::Max(1.f, NewValue); }
    else { NewValue = FMath::Clamp(NewValue, 0.f, SafeMax); }
}
void UWyrmAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);
    ClampAttributeValue(Attribute, NewValue);
}
void UWyrmAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);
    ClampAttributeValue(Attribute, NewValue);
}
void UWyrmAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);
    if (Attribute == GetMaxHealthAttribute() && GetHealth() > NewValue)
    {
        SetHealth(FMath::Max(0.f, NewValue));
    }
}
void UWyrmAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
    if (Data.EvaluatedData.Attribute == GetHealthAttribute() ||
        Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        const float SafeMax = FMath::IsFinite(GetMaxHealth()) ? FMath::Max(1.f, GetMaxHealth()) : 1.f;
        if (SafeMax != GetMaxHealth()) { SetMaxHealth(SafeMax); }
        const float SafeHealth = FMath::IsFinite(GetHealth()) ? GetHealth() : 0.f;
        SetHealth(FMath::Clamp(SafeHealth, 0.f, SafeMax));
    }
    // Death, respawn, damage formula and ability grants belong to WP-04/05.
}
