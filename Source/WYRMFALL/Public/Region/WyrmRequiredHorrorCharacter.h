#pragma once

#include "CoreMinimal.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "WyrmRequiredHorrorCharacter.generated.h"

class AWyrmCharacter;
class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWyrmRequiredHorrorResolution : uint8
{
    Unresolved,
    LivingSubmission
};

/** Shared rigid-assembly behavior for the remaining supplied required-horror roster. */
UCLASS(Abstract, BlueprintType, Category="WYRMFALL|Gloaming")
class WYRMFALL_API AWyrmRequiredHorrorCharacter : public AWyrmEnemyCharacter
{
    GENERATED_BODY()

public:
    AWyrmRequiredHorrorCharacter();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|RequiredHorror")
    bool bEncounterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gloaming|RequiredHorror")
    EWyrmRequiredHorrorResolution Resolution = EWyrmRequiredHorrorResolution::Unresolved;

    UFUNCTION(BlueprintCallable, Category="Gloaming|RequiredHorror")
    bool BeginEncounter(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|RequiredHorror")
    bool ResolveAfterLivingSubmission(AWyrmCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Gloaming|RequiredHorror")
    void ResetEncounter();

    UFUNCTION(BlueprintPure, Category="Gloaming|RequiredHorror")
    int32 GetPresentationPartCount() const { return PartAssetPaths.Num(); }

    UFUNCTION(BlueprintPure, Category="Gloaming|RequiredHorror")
    float GetPresentationHeight() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|RequiredHorror")
    bool HasCompletePresentation() const;

    UFUNCTION(BlueprintPure, Category="Gloaming|RequiredHorror")
    EWyrmRequiredHorrorIdentity GetHorrorIdentity() const { return HorrorIdentity; }

protected:
    virtual void BeginPlay() override;
    void Configure(EWyrmRequiredHorrorIdentity InIdentity, const TCHAR* InSourceToken,
        float InHeight, float InHealth, float InArmor, float InPower, float InWalkSpeed,
        const TArray<FString>& InAssetNames);

private:
    UPROPERTY(VisibleAnywhere, Category="Gloaming|RequiredHorror")
    TObjectPtr<USceneComponent> PresentationRoot;

    UPROPERTY(VisibleAnywhere, Category="Gloaming|RequiredHorror")
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationParts;

    EWyrmRequiredHorrorIdentity HorrorIdentity = EWyrmRequiredHorrorIdentity::AilYen;
    FString SourceToken;
    TArray<FString> PartAssetPaths;
    float TargetHeight = 180.f;
    float EncounterHealth = 800.f;
    float EncounterArmor = 20.f;
    float EncounterPower = 30.f;
    float PresentationScale = 1.f;
    float IdleTime = 0.f;
};

UCLASS(BlueprintType) class WYRMFALL_API AWyrmAilYenCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmAilYenCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmBellraiserCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmBellraiserCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmSadEchoCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmSadEchoCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmDreadatorCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmDreadatorCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmRoastfaceCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmRoastfaceCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmGravyDaughtersCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmGravyDaughtersCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmKnitCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmKnitCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmCanniballCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmCanniballCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmMumsTheWyrdCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmMumsTheWyrdCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmDreadyFreddieCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmDreadyFreddieCharacter(); };
UCLASS(BlueprintType) class WYRMFALL_API AWyrmPyreMidheadCharacter : public AWyrmRequiredHorrorCharacter
{ GENERATED_BODY() public: AWyrmPyreMidheadCharacter(); };
