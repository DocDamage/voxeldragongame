#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Region/WyrmGloamingTypes.h"
#include "WyrmGloamingSubsystem.generated.h"

class AWyrmDragonCharacter;
class UWyrmWorldTravelSubsystem;

UENUM(BlueprintType)
enum class EWyrmRequiredHorrorIdentity : uint8
{
    AilYen,
    Bellraiser,
    SadEcho,
    Dreadator,
    Roastface,
    GravyDaughters,
    Knit,
    Canniball,
    MumsTheWyrd,
    DreadyFreddie,
    PyreMidhead
};

/** Bounded Gloaming Marches fact owner; UWyrmSaveSubsystem owns persistence. */
UCLASS(BlueprintType, Category="WYRMFALL|Gloaming")
class WYRMFALL_API UWyrmGloamingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintPure, Category="Gloaming", meta=(WorldContext="WorldContextObject"))
    static UWyrmGloamingSubsystem* GetGloamingSubsystem(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Facts")
    void ResetGloamingState();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Arrival")
    bool RecordArrival();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Ashgrave")
    bool ResolveAshgraveExtractionSeal();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Malvaine")
    bool RecordMalvaineResolution(bool bParley);

    UFUNCTION(BlueprintCallable, Category="Gloaming|HollowTwins")
    bool RecordHollowTwinsResolution(bool bReleased);

    UFUNCTION(BlueprintCallable, Category="Gloaming|MichaelMire")
    bool RecordMichaelMireResolution();

    UFUNCTION(BlueprintCallable, Category="Gloaming|MacheteMason")
    bool RecordMacheteMasonResolution();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Pleatherface")
    bool RecordPleatherfaceResolution();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Wherewolf")
    bool RecordWherewolfResolution();

    UFUNCTION(BlueprintCallable, Category="Gloaming|AnnieWails")
    bool RecordAnnieWailsResolution();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Scarrie")
    bool RecordScarrieResolution();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Chuckles")
    bool RecordChucklesResolution();

    UFUNCTION(BlueprintCallable, Category="Gloaming|CountDripula")
    bool RecordCountDripulaResolution();

    UFUNCTION(BlueprintCallable, Category="Gloaming|FrankNShrine")
    bool RecordFrankNShrineResolution();

    UFUNCTION(BlueprintPure, Category="Gloaming|RequiredHorror")
    bool CanResolveRequiredHorror(EWyrmRequiredHorrorIdentity Identity) const;

    UFUNCTION(BlueprintCallable, Category="Gloaming|RequiredHorror")
    bool RecordRequiredHorrorResolution(EWyrmRequiredHorrorIdentity Identity);

    /** Commits the one-time regional closure only after the roster, bonded Nyxaroth, and safe return route are present. */
    UFUNCTION(BlueprintPure, Category="Gloaming|Completion")
    bool CanCompleteRegion(const AWyrmDragonCharacter* Nyxaroth, const UWyrmWorldTravelSubsystem* Travel) const;

    UFUNCTION(BlueprintCallable, Category="Gloaming|Completion")
    bool RecordRegionalCompletion(AWyrmDragonCharacter* Nyxaroth, UWyrmWorldTravelSubsystem* Travel);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Echo")
    bool RecordSanguineStrikeUnlock();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Echo")
    bool RecordSecondTurnUnlock();

    UFUNCTION(BlueprintPure, Category="Gloaming|Facts")
    bool HasFact(FName FactId) const;

    UFUNCTION(BlueprintPure, Category="Gloaming|Facts")
    bool HasReceipt(FName ReceiptId) const;

    void BuildSaveRecord(FWyrmGloamingSaveRecord& OutRecord) const;
    void RestoreFromSaveRecord(const FWyrmGloamingSaveRecord& InRecord);

private:
    bool CommitFact(FName FactId, FName ReceiptId);
    void NormalizeRestoredState();

    UPROPERTY(VisibleInstanceOnly, Category="Gloaming")
    TArray<FName> KnownFacts;

    UPROPERTY(VisibleInstanceOnly, Category="Gloaming")
    TArray<FName> FactReceipts;
};
