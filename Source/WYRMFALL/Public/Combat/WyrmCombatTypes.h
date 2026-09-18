#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WyrmCombatTypes.generated.h"

UENUM(BlueprintType)
enum class EWyrmCombatTeam : uint8
{
    Player UMETA(DisplayName="Player"),
    Hostile UMETA(DisplayName="Hostile"),
    Neutral UMETA(DisplayName="Neutral")
};

UENUM(BlueprintType)
enum class EWyrmDamageGeneration : uint8
{
    Primary UMETA(DisplayName="Primary"),
    Periodic UMETA(DisplayName="Periodic"),
    Reflected UMETA(DisplayName="Reflected"),
    Repeated UMETA(DisplayName="Repeated"),
    Summoned UMETA(DisplayName="Summoned")
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmDamageResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    float RawDamage = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    float MitigatedDamage = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    float ShieldAbsorbed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    float HealthDamage = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    bool bTargetDied = false;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    bool bRejected = false;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    EWyrmDamageGeneration Generation = EWyrmDamageGeneration::Primary;
};
