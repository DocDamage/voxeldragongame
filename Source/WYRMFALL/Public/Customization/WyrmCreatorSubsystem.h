// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WyrmCreatorSubsystem.generated.h"

class AWyrmCharacter;
class USoundBase;

/**
 * Snapshot of character appearance, parameters, and physical proportions for undo/redo and presets.
 */
USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmAppearanceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FString AppearanceDescriptor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FVector CharacterScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TMap<FName, FString> OptionParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TMap<FName, FLinearColor> ColorParameters;
};

/**
 * Subsystem coordinating Character Creator UX, category locking, undo/redo history,
 * presets, randomized mutation, proportion adjustments, and audio feedback (WP-17, CHAR-07, CHAR-08).
 * Mutable remains the authoritative visual generation backbone.
 */
UCLASS()
class WYRMFALL_API UWyrmCreatorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Category Locks (CHAR-08) ---

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	void SetCategoryLocked(FName Category, bool bLocked);

	UFUNCTION(BlueprintPure, Category = "WYRMFALL|Creator")
	bool IsCategoryLocked(FName Category) const;

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	void ClearAllLocks();

	UFUNCTION(BlueprintPure, Category = "WYRMFALL|Creator")
	TArray<FName> GetLockedCategories() const;

	// --- Undo / Redo History Stack (CHAR-08) ---

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	void PushAppearanceUndoState(AWyrmCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	bool Undo(AWyrmCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	bool Redo(AWyrmCharacter* Character);

	UFUNCTION(BlueprintPure, Category = "WYRMFALL|Creator")
	bool CanUndo() const;

	UFUNCTION(BlueprintPure, Category = "WYRMFALL|Creator")
	bool CanRedo() const;

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	void ClearHistory();

	// --- Presets (CHAR-08) ---

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	void InitializePresets();

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	bool ApplyPreset(AWyrmCharacter* Character, FName PresetName);

	UFUNCTION(BlueprintPure, Category = "WYRMFALL|Creator")
	TArray<FName> GetAvailablePresets() const;

	UFUNCTION(BlueprintPure, Category = "WYRMFALL|Creator")
	bool GetPresetDetails(FName PresetName, FWyrmAppearanceSnapshot& OutSnapshot) const;

	// --- Randomization with Category Locks (CHAR-08) ---

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	void RandomizeAppearance(AWyrmCharacter* Character, bool bRespectLocks = true);

	// --- Proportions & Scaling (CHAR-07) ---

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	void SetCharacterProportions(AWyrmCharacter* Character, float HeightScale, float WidthScale);

	// --- Audio Feedback Integration (CHAR-08, Audio-Visual Pass) ---

	UFUNCTION(BlueprintCallable, Category = "WYRMFALL|Creator")
	void PlayCreatorSound(FName SoundAction);

private:
	/** Active locked categories that must not change during randomization */
	UPROPERTY()
	TSet<FName> LockedCategories;

	/** Undo history stack */
	UPROPERTY()
	TArray<FWyrmAppearanceSnapshot> UndoStack;

	/** Redo history stack */
	UPROPERTY()
	TArray<FWyrmAppearanceSnapshot> RedoStack;

	/** Registered canonical presets */
	UPROPERTY()
	TMap<FName, FWyrmAppearanceSnapshot> Presets;

	/** Maximum allowed history items */
	const int32 MaxHistoryDepth = 30;

	/** Helper to capture current character appearance state into a snapshot */
	FWyrmAppearanceSnapshot CaptureSnapshot(AWyrmCharacter* Character) const;

	/** Helper to restore snapshot state to character */
	void RestoreSnapshot(AWyrmCharacter* Character, const FWyrmAppearanceSnapshot& Snapshot);
};
