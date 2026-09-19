// Copyright Epic Games, Inc. All Rights Reserved.

#include "Customization/WyrmCreatorSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UWyrmCreatorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializePresets();
}

void UWyrmCreatorSubsystem::InitializePresets()
{
	Presets.Empty();

	// Register canonical presets (CHAR-08)
	{
		FWyrmAppearanceSnapshot Preset;
		Preset.OptionParameters.Add(TEXT("BodyStyle"), TEXT("Knight"));
		Preset.OptionParameters.Add(TEXT("Helmet"), TEXT("Up"));
		Preset.ColorParameters.Add(TEXT("ArmorTint"), FLinearColor::White);
		Preset.CharacterScale = FVector(1.0f, 1.0f, 1.0f);
		Presets.Add(TEXT("Knight_Standard"), Preset);
	}
	{
		FWyrmAppearanceSnapshot Preset;
		Preset.OptionParameters.Add(TEXT("BodyStyle"), TEXT("Commander"));
		Preset.OptionParameters.Add(TEXT("Helmet"), TEXT("Up"));
		Preset.ColorParameters.Add(TEXT("ArmorTint"), FLinearColor(1.0f, 0.85f, 0.2f, 1.0f));
		Preset.CharacterScale = FVector(1.05f, 1.05f, 1.05f);
		Presets.Add(TEXT("Knight_Commander"), Preset);
	}
	{
		FWyrmAppearanceSnapshot Preset;
		Preset.OptionParameters.Add(TEXT("BodyStyle"), TEXT("Archer"));
		Preset.OptionParameters.Add(TEXT("Helmet"), TEXT("Up"));
		Preset.ColorParameters.Add(TEXT("ArmorTint"), FLinearColor(0.2f, 0.8f, 0.3f, 1.0f));
		Preset.CharacterScale = FVector(0.95f, 0.95f, 0.95f);
		Presets.Add(TEXT("Knight_Archer"), Preset);
	}
	{
		FWyrmAppearanceSnapshot Preset;
		Preset.OptionParameters.Add(TEXT("BodyStyle"), TEXT("Champion"));
		Preset.OptionParameters.Add(TEXT("Helmet"), TEXT("Down"));
		Preset.ColorParameters.Add(TEXT("ArmorTint"), FLinearColor(0.85f, 0.15f, 0.15f, 1.0f));
		Preset.CharacterScale = FVector(1.10f, 1.15f, 1.05f);
		Presets.Add(TEXT("Knight_Champion"), Preset);
	}
	{
		FWyrmAppearanceSnapshot Preset;
		Preset.OptionParameters.Add(TEXT("BodyStyle"), TEXT("Knight"));
		Preset.OptionParameters.Add(TEXT("Helmet"), TEXT("Up"));
		Preset.ColorParameters.Add(TEXT("ArmorTint"), FLinearColor(0.7f, 0.7f, 0.75f, 1.0f));
		Preset.CharacterScale = FVector(0.85f, 0.85f, 0.85f);
		Presets.Add(TEXT("Knight_Shortest"), Preset);
	}
	{
		FWyrmAppearanceSnapshot Preset;
		Preset.OptionParameters.Add(TEXT("BodyStyle"), TEXT("Knight"));
		Preset.OptionParameters.Add(TEXT("Helmet"), TEXT("Up"));
		Preset.ColorParameters.Add(TEXT("ArmorTint"), FLinearColor(0.7f, 0.7f, 0.75f, 1.0f));
		Preset.CharacterScale = FVector(1.15f, 1.15f, 1.15f);
		Presets.Add(TEXT("Knight_Tallest"), Preset);
	}
	{
		FWyrmAppearanceSnapshot Preset;
		Preset.OptionParameters.Add(TEXT("BodyStyle"), TEXT("Knight"));
		Preset.OptionParameters.Add(TEXT("Helmet"), TEXT("Up"));
		Preset.ColorParameters.Add(TEXT("ArmorTint"), FLinearColor(0.7f, 0.7f, 0.75f, 1.0f));
		Preset.CharacterScale = FVector(1.25f, 1.25f, 1.0f);
		Presets.Add(TEXT("Knight_Broadest"), Preset);
	}
}

void UWyrmCreatorSubsystem::SetCategoryLocked(FName Category, bool bLocked)
{
	if (Category.IsNone())
	{
		return;
	}

	if (bLocked)
	{
		LockedCategories.Add(Category);
		PlayCreatorSound(TEXT("Lock"));
	}
	else
	{
		LockedCategories.Remove(Category);
		PlayCreatorSound(TEXT("Unlock"));
	}
}

bool UWyrmCreatorSubsystem::IsCategoryLocked(FName Category) const
{
	return LockedCategories.Contains(Category);
}

void UWyrmCreatorSubsystem::ClearAllLocks()
{
	LockedCategories.Empty();
}

TArray<FName> UWyrmCreatorSubsystem::GetLockedCategories() const
{
	return LockedCategories.Array();
}

FWyrmAppearanceSnapshot UWyrmCreatorSubsystem::CaptureSnapshot(AWyrmCharacter* Character) const
{
	FWyrmAppearanceSnapshot Snapshot;
	if (!Character)
	{
		return Snapshot;
	}

	Snapshot.AppearanceDescriptor = Character->CaptureAppearanceDescriptor();
	Snapshot.CharacterScale = Character->GetCharacterScale();

	static const TArray<FName> TrackedOptions = { TEXT("BodyStyle"), TEXT("Helmet") };
	for (const FName& Opt : TrackedOptions)
	{
		Snapshot.OptionParameters.Add(Opt, Character->GetOptionParameter(Opt));
	}

	static const TArray<FName> TrackedColors = { TEXT("ArmorTint") };
	for (const FName& Col : TrackedColors)
	{
		Snapshot.ColorParameters.Add(Col, Character->GetColorParameter(Col));
	}

	return Snapshot;
}

void UWyrmCreatorSubsystem::RestoreSnapshot(AWyrmCharacter* Character, const FWyrmAppearanceSnapshot& Snapshot)
{
	if (!Character)
	{
		return;
	}

	for (const auto& Pair : Snapshot.OptionParameters)
	{
		Character->SetOptionParameter(Pair.Key, Pair.Value);
	}
	for (const auto& Pair : Snapshot.ColorParameters)
	{
		Character->SetColorParameter(Pair.Key, Pair.Value);
	}
	Character->SetCharacterScale(Snapshot.CharacterScale);

	if (!Snapshot.AppearanceDescriptor.IsEmpty())
	{
		Character->RestoreAppearanceDescriptor(Snapshot.AppearanceDescriptor);
	}
	else
	{
		Character->ApplyAppearance(false);
	}
}

void UWyrmCreatorSubsystem::PushAppearanceUndoState(AWyrmCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	FWyrmAppearanceSnapshot Snapshot = CaptureSnapshot(Character);
	UndoStack.Add(Snapshot);
	if (UndoStack.Num() > MaxHistoryDepth)
	{
		UndoStack.RemoveAt(0);
	}
	RedoStack.Empty();
}

bool UWyrmCreatorSubsystem::Undo(AWyrmCharacter* Character)
{
	if (!Character || UndoStack.Num() == 0)
	{
		return false;
	}

	FWyrmAppearanceSnapshot CurrentSnapshot = CaptureSnapshot(Character);
	RedoStack.Add(CurrentSnapshot);

	FWyrmAppearanceSnapshot PreviousSnapshot = UndoStack.Pop();
	RestoreSnapshot(Character, PreviousSnapshot);

	PlayCreatorSound(TEXT("Undo"));
	return true;
}

bool UWyrmCreatorSubsystem::Redo(AWyrmCharacter* Character)
{
	if (!Character || RedoStack.Num() == 0)
	{
		return false;
	}

	FWyrmAppearanceSnapshot CurrentSnapshot = CaptureSnapshot(Character);
	UndoStack.Add(CurrentSnapshot);

	FWyrmAppearanceSnapshot NextSnapshot = RedoStack.Pop();
	RestoreSnapshot(Character, NextSnapshot);

	PlayCreatorSound(TEXT("Redo"));
	return true;
}

bool UWyrmCreatorSubsystem::CanUndo() const
{
	return UndoStack.Num() > 0;
}

bool UWyrmCreatorSubsystem::CanRedo() const
{
	return RedoStack.Num() > 0;
}

void UWyrmCreatorSubsystem::ClearHistory()
{
	UndoStack.Empty();
	RedoStack.Empty();
}

bool UWyrmCreatorSubsystem::ApplyPreset(AWyrmCharacter* Character, FName PresetName)
{
	if (!Character || !Presets.Contains(PresetName))
	{
		return false;
	}

	PushAppearanceUndoState(Character);

	const FWyrmAppearanceSnapshot& Preset = Presets[PresetName];
	RestoreSnapshot(Character, Preset);

	PlayCreatorSound(TEXT("PresetApply"));
	return true;
}

TArray<FName> UWyrmCreatorSubsystem::GetAvailablePresets() const
{
	TArray<FName> OutNames;
	Presets.GetKeys(OutNames);
	return OutNames;
}

bool UWyrmCreatorSubsystem::GetPresetDetails(FName PresetName, FWyrmAppearanceSnapshot& OutSnapshot) const
{
	if (const FWyrmAppearanceSnapshot* Found = Presets.Find(PresetName))
	{
		OutSnapshot = *Found;
		return true;
	}
	return false;
}

void UWyrmCreatorSubsystem::RandomizeAppearance(AWyrmCharacter* Character, bool bRespectLocks)
{
	if (!Character)
	{
		return;
	}

	PushAppearanceUndoState(Character);

	static const TArray<FString> BodyOptions = { TEXT("Knight"), TEXT("Archer"), TEXT("Captain"), TEXT("Champion"), TEXT("Commander") };
	static const TArray<FString> HelmetOptions = { TEXT("Up"), TEXT("Down") };
	static const TArray<FLinearColor> TintPalette = {
		FLinearColor::White,
		FLinearColor(1.0f, 0.85f, 0.2f, 1.0f),  // Gold
		FLinearColor(0.85f, 0.15f, 0.15f, 1.0f), // Crimson
		FLinearColor(0.2f, 0.5f, 0.9f, 1.0f),   // Azure
		FLinearColor(0.2f, 0.8f, 0.3f, 1.0f),   // Emerald
		FLinearColor(0.25f, 0.25f, 0.3f, 1.0f)   // Shadow
	};

	if (!bRespectLocks || !IsCategoryLocked(TEXT("BodyStyle")))
	{
		const int32 Index = FMath::RandRange(0, BodyOptions.Num() - 1);
		Character->SetOptionParameter(TEXT("BodyStyle"), BodyOptions[Index]);
	}

	if (!bRespectLocks || !IsCategoryLocked(TEXT("Helmet")))
	{
		const int32 Index = FMath::RandRange(0, HelmetOptions.Num() - 1);
		Character->SetOptionParameter(TEXT("Helmet"), HelmetOptions[Index]);
	}

	if (!bRespectLocks || !IsCategoryLocked(TEXT("ArmorTint")))
	{
		const int32 Index = FMath::RandRange(0, TintPalette.Num() - 1);
		Character->SetColorParameter(TEXT("ArmorTint"), TintPalette[Index]);
	}

	if (!bRespectLocks || !IsCategoryLocked(TEXT("Scale")))
	{
		const float ScaleH = FMath::RandRange(0.90f, 1.12f);
		const float ScaleW = FMath::RandRange(0.90f, 1.15f);
		SetCharacterProportions(Character, ScaleH, ScaleW);
	}

	Character->ApplyAppearance(false);
	PlayCreatorSound(TEXT("Randomize"));
}

void UWyrmCreatorSubsystem::SetCharacterProportions(AWyrmCharacter* Character, float HeightScale, float WidthScale)
{
	if (!Character)
	{
		return;
	}

	const float ClampedH = FMath::Clamp(HeightScale, 0.80f, 1.25f);
	const float ClampedW = FMath::Clamp(WidthScale, 0.80f, 1.35f);
	Character->SetCharacterScale(FVector(ClampedW, ClampedW, ClampedH));
}

void UWyrmCreatorSubsystem::PlayCreatorSound(FName SoundAction)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FString CuePath;
	if (SoundAction == TEXT("Select"))
	{
		CuePath = TEXT("/Game/UltimateUIMenusSFX/Casual_Mobile/Cue/CasualMobile_01_Cue.CasualMobile_01_Cue");
	}
	else if (SoundAction == TEXT("Randomize"))
	{
		CuePath = TEXT("/Game/UltimateUIMenusSFX/Casual_Mobile/Cue/CasualMobile_03_Cue.CasualMobile_03_Cue");
	}
	else if (SoundAction == TEXT("Lock"))
	{
		CuePath = TEXT("/Game/UltimateUIMenusSFX/Casual_Mobile/Cue/CasualMobile_06_Cue.CasualMobile_06_Cue");
	}
	else if (SoundAction == TEXT("Unlock"))
	{
		CuePath = TEXT("/Game/UltimateUIMenusSFX/Casual_Mobile/Cue/CasualMobile_07_Cue.CasualMobile_07_Cue");
	}
	else if (SoundAction == TEXT("Undo"))
	{
		CuePath = TEXT("/Game/UltimateUIMenusSFX/Casual_Mobile/Cue/CasualMobile_08_Cue.CasualMobile_08_Cue");
	}
	else if (SoundAction == TEXT("Redo"))
	{
		CuePath = TEXT("/Game/UltimateUIMenusSFX/Casual_Mobile/Cue/CasualMobile_09_Cue.CasualMobile_09_Cue");
	}
	else if (SoundAction == TEXT("PresetApply"))
	{
		CuePath = TEXT("/Game/UltimateUIMenusSFX/Fantasy_RPG/Cue/FantasyRPG_01_Cue.FantasyRPG_01_Cue");
	}

	if (!CuePath.IsEmpty())
	{
		if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, *CuePath))
		{
			UGameplayStatics::PlaySound2D(World, Sound);
		}
	}
}
