#include "Combat/WyrmAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

UWyrmAttributeSet::UWyrmAttributeSet()
{
    InitCharacterLevel(1.f);
    InitMaxHealth(100.f);
    InitHealth(100.f);
    InitMaxFocus(100.f);
    InitFocus(100.f);
    InitPower(20.f);
    InitArmor(0.f);
    InitShield(0.f);
    InitIncomingDamage(0.f);
    InitIncomingHealing(0.f);
}

void UWyrmAttributeSet::SetCurrentHealth(float NewVal)
{
    UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
    if (AbilityComp && AbilityComp->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
    {
        SetHealth(NewVal);
    }
    else
    {
        InitHealth(NewVal);
    }
}

void UWyrmAttributeSet::SetCurrentMaxHealth(float NewVal)
{
    UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
    if (AbilityComp && AbilityComp->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
    {
        SetMaxHealth(NewVal);
    }
    else
    {
        InitMaxHealth(NewVal);
    }
}

void UWyrmAttributeSet::SetCurrentFocus(float NewVal)
{
    UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
    if (AbilityComp && AbilityComp->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
    {
        SetFocus(NewVal);
    }
    else
    {
        InitFocus(NewVal);
    }
}

void UWyrmAttributeSet::SetCurrentPower(float NewVal)
{
    UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
    if (AbilityComp && AbilityComp->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
    {
        SetPower(NewVal);
    }
    else
    {
        InitPower(NewVal);
    }
}

void UWyrmAttributeSet::SetCurrentArmor(float NewVal)
{
    UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
    if (AbilityComp && AbilityComp->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
    {
        SetArmor(NewVal);
    }
    else
    {
        InitArmor(NewVal);
    }
}

void UWyrmAttributeSet::SetCurrentShield(float NewVal)
{
    UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
    if (AbilityComp && AbilityComp->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
    {
        SetShield(NewVal);
    }
    else
    {
        InitShield(NewVal);
    }
}

float UWyrmAttributeSet::CalculateMaxHealthForLevel(float InLevel)
{
    const float SafeLevel = FMath::IsFinite(InLevel) ? FMath::Max(1.f, InLevel) : 1.f;
    return 100.f + 8.f * (SafeLevel - 1.f);
}

float UWyrmAttributeSet::CalculatePowerForLevel(float InLevel)
{
    const float SafeLevel = FMath::IsFinite(InLevel) ? FMath::Max(1.f, InLevel) : 1.f;
    return 20.f + 3.f * (SafeLevel - 1.f);
}

float UWyrmAttributeSet::CalculatePhysicalMitigation(float InArmor, float InAttackerLevel)
{
    if (!FMath::IsFinite(InArmor) || InArmor <= 0.f)
    {
        return 0.f;
    }
    const float SafeLevel = FMath::IsFinite(InAttackerLevel) ? FMath::Max(1.f, InAttackerLevel) : 1.f;
    const float Denom = InArmor + 50.f + 10.f * SafeLevel;
    if (Denom <= 0.f)
    {
        return 0.f;
    }
    return FMath::Clamp(InArmor / Denom, 0.f, 0.70f);
}

float UWyrmAttributeSet::CalculateRawDamage(float InWeaponBase, float InPower, float InPowerCoefficient)
{
    const float SafeBase = FMath::IsFinite(InWeaponBase) ? FMath::Max(0.f, InWeaponBase) : 0.f;
    const float SafePower = FMath::IsFinite(InPower) ? FMath::Max(0.f, InPower) : 0.f;
    const float SafeCoeff = FMath::IsFinite(InPowerCoefficient) ? FMath::Max(0.f, InPowerCoefficient) : 0.5f;
    return SafeBase + SafeCoeff * SafePower;
}

float UWyrmAttributeSet::CalculateMitigatedDamage(float InRawDamage, float InArmor, float InAttackerLevel)
{
    if (!FMath::IsFinite(InRawDamage) || InRawDamage <= 0.f)
    {
        return 0.f;
    }
    const float Mitigation = CalculatePhysicalMitigation(InArmor, InAttackerLevel);
    return InRawDamage * (1.0f - Mitigation);
}

void UWyrmAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (!FMath::IsFinite(NewValue))
    {
        NewValue = 0.f;
    }

    if (Attribute == GetHealthAttribute())
    {
        const float SafeMax = FMath::IsFinite(GetMaxHealth()) ? FMath::Max(1.f, GetMaxHealth()) : 1.f;
        NewValue = FMath::Clamp(NewValue, 0.f, SafeMax);
    }
    else if (Attribute == GetMaxHealthAttribute())
    {
        NewValue = FMath::Max(1.f, NewValue);
    }
    else if (Attribute == GetFocusAttribute())
    {
        const float SafeMaxFocus = FMath::IsFinite(GetMaxFocus()) ? FMath::Max(1.f, GetMaxFocus()) : 100.f;
        NewValue = FMath::Clamp(NewValue, 0.f, SafeMaxFocus);
    }
    else if (Attribute == GetMaxFocusAttribute())
    {
        NewValue = FMath::Max(1.f, NewValue);
    }
    else if (Attribute == GetPowerAttribute())
    {
        NewValue = FMath::Max(0.f, NewValue);
    }
    else if (Attribute == GetArmorAttribute())
    {
        NewValue = FMath::Max(0.f, NewValue);
    }
    else if (Attribute == GetCharacterLevelAttribute())
    {
        NewValue = FMath::Max(1.f, NewValue);
    }
    else if (Attribute == GetShieldAttribute())
    {
        NewValue = FMath::Max(0.f, NewValue);
    }
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
    else if (Attribute == GetMaxFocusAttribute() && GetFocus() > NewValue)
    {
        SetFocus(FMath::Max(0.f, NewValue));
    }
}

void UWyrmAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    UAbilitySystemComponent* TargetASC = Data.Target.AbilityActorInfo.IsValid() ? Data.Target.AbilityActorInfo->AbilitySystemComponent.Get() : nullptr;

    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        float LocalIncomingDamage = GetIncomingDamage();
        SetIncomingDamage(0.f);

        if (LocalIncomingDamage <= 0.f)
        {
            return;
        }

        // Boundary checks (COM-02): Check if target is already dead or invulnerable
        if (TargetASC)
        {
            static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")));
            static const FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Invulnerable")));

            if (TargetASC->HasMatchingGameplayTag(DeadTag) || GetHealth() <= 0.f)
            {
                return; // Target already dead; ignore damage
            }

            if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
            {
                return; // Target invulnerable; ignore damage
            }
        }

        // Determine attacker level from source ASC
        float AttackerLevel = 1.f;
        if (Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent())
        {
            const UWyrmAttributeSet* SourceSet = Cast<UWyrmAttributeSet>(
                Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent()->GetAttributeSet(UWyrmAttributeSet::StaticClass()));
            if (SourceSet)
            {
                AttackerLevel = SourceSet->GetCharacterLevel();
            }
        }

        // Mitigate incoming damage via canonical formula (COM-01)
        float MitigatedDamage = CalculateMitigatedDamage(LocalIncomingDamage, GetArmor(), AttackerLevel);

        // Absorb via Shield first
        if (GetShield() > 0.f)
        {
            const float ShieldAbsorbed = FMath::Min(GetShield(), MitigatedDamage);
            SetShield(GetShield() - ShieldAbsorbed);
            MitigatedDamage -= ShieldAbsorbed;
        }

        // Apply remaining to Health
        if (MitigatedDamage > 0.f)
        {
            const float OldHealth = GetHealth();
            const float NewHealth = FMath::Clamp(OldHealth - MitigatedDamage, 0.f, GetMaxHealth());
            SetHealth(NewHealth);

            if (NewHealth <= 0.f && TargetASC)
            {
                static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")));
                TargetASC->AddLooseGameplayTag(DeadTag);
            }
        }
    }
    else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
    {
        const float LocalIncomingHealing = GetIncomingHealing();
        SetIncomingHealing(0.f);

        if (LocalIncomingHealing > 0.f && GetHealth() > 0.f)
        {
            SetHealth(FMath::Clamp(GetHealth() + LocalIncomingHealing, 0.f, GetMaxHealth()));
        }
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute() ||
             Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        const float SafeMax = FMath::IsFinite(GetMaxHealth()) ? FMath::Max(1.f, GetMaxHealth()) : 1.f;
        if (SafeMax != GetMaxHealth()) { SetMaxHealth(SafeMax); }
        const float SafeHealth = FMath::IsFinite(GetHealth()) ? GetHealth() : 0.f;
        SetHealth(FMath::Clamp(SafeHealth, 0.f, SafeMax));
    }
    else if (Data.EvaluatedData.Attribute == GetFocusAttribute() ||
             Data.EvaluatedData.Attribute == GetMaxFocusAttribute())
    {
        const float SafeMaxFocus = FMath::IsFinite(GetMaxFocus()) ? FMath::Max(1.f, GetMaxFocus()) : 1.f;
        if (SafeMaxFocus != GetMaxFocus()) { SetMaxFocus(SafeMaxFocus); }
        const float SafeFocus = FMath::IsFinite(GetFocus()) ? GetFocus() : 0.f;
        SetFocus(FMath::Clamp(SafeFocus, 0.f, SafeMaxFocus));
    }
}
