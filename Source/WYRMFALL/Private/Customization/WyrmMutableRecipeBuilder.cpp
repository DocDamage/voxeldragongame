// Copyright Epic Games, Inc. All Rights Reserved.

#include "Customization/WyrmMutableRecipeBuilder.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableObjectPrivate.h"
#include "MuCOE/CustomizableObjectEditorFunctionLibrary.h"
#include "MuCOE/CustomizableObjectFactory.h"
#include "MuCOE/CustomizableObjectGraph.h"
#include "MuCOE/EdGraphSchema_CustomizableObject.h"
#include "MuCOE/Nodes/CONodeComponentSkeletalMesh.h"
#include "MuCOE/Nodes/CONodeSkeletalMeshObjectMake.h"
#include "MuCOE/Nodes/CONodeSkeletalMeshMake_V2.h"
#include "MuCOE/Nodes/CONodeSkeletalMeshSection.h"
#include "MuCOE/Nodes/CONodeSwitch.h"
#include "MuCOE/Nodes/CustomizableObjectNodeSkeletalMesh.h"
#include "MuCOE/Nodes/CustomizableObjectNodeEnumParameter.h"
#include "MuCOE/Nodes/CustomizableObjectNodeColorParameter.h"
#include "MuCOE/Nodes/CustomizableObjectNodeObject.h"
#include "UObject/SavePackage.h"

bool UWyrmMutableRecipeBuilder::BuildKnightRecipe(FString& OutMessage)
{
	USkeletalMesh* SK_Knight = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/WYRMFALL/Characters/Player/SK_Knight.SK_Knight"));
	USkeletalMesh* SK_KnightHelmDown = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/WYRMFALL/Characters/Player/SK_KnightHelmDown.SK_KnightHelmDown"));
	USkeletalMesh* SK_KnightArcher = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/WYRMFALL/Characters/Player/SK_KnightArcher.SK_KnightArcher"));
	USkeletalMesh* SK_KnightCaptain = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/WYRMFALL/Characters/Player/SK_KnightCaptain.SK_KnightCaptain"));
	USkeletalMesh* SK_KnightChampion = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/WYRMFALL/Characters/Player/SK_KnightChampion.SK_KnightChampion"));
	USkeletalMesh* SK_KnightCommander = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/WYRMFALL/Characters/Player/SK_KnightCommander.SK_KnightCommander"));
	UMaterialInterface* M_Knight = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/WYRMFALL/Characters/Player/M_Knight.M_Knight"));

	if (!SK_Knight)
	{
		OutMessage = TEXT("Missing required base skeletal mesh: /Game/WYRMFALL/Characters/Player/SK_Knight");
		return false;
	}

	const FString PackagePath = TEXT("/Game/WYRMFALL/Characters/Player");
	const FString AssetName = TEXT("CO_Knight");
	const FString FullPackageName = PackagePath + TEXT("/") + AssetName;

	UPackage* ExistingPackage = FindPackage(nullptr, *FullPackageName);
	UCustomizableObject* CO = nullptr;
	if (ExistingPackage)
	{
		CO = FindObject<UCustomizableObject>(ExistingPackage, *AssetName);
	}
	if (!CO)
	{
		CO = LoadObject<UCustomizableObject>(nullptr, *(FullPackageName + TEXT(".") + AssetName));
	}
	if (!CO)
	{
		UPackage* Pkg = ExistingPackage ? ExistingPackage : CreatePackage(*FullPackageName);
		UCustomizableObjectFactory* Factory = NewObject<UCustomizableObjectFactory>();
		CO = Cast<UCustomizableObject>(Factory->FactoryCreateNew(UCustomizableObject::StaticClass(), Pkg, *AssetName, RF_Public | RF_Standalone, nullptr, GWarn));
	}
	if (!CO)
	{
		OutMessage = FString::Printf(TEXT("Failed to create or load CustomizableObject at %ls"), *FullPackageName);
		return false;
	}

	UCustomizableObjectPrivate* ObjectPrivate = CO->GetPrivate();
	if (!ObjectPrivate)
	{
		OutMessage = TEXT("CustomizableObject has no private implementation object");
		return false;
	}

	UEdGraph* Graph = ObjectPrivate->GetSource();
	if (!Graph)
	{
		Graph = NewObject<UCustomizableObjectGraph>(CO, NAME_None, RF_Transactional);
		ObjectPrivate->GetSource() = Graph;
	}

	auto AddNode = [Graph]<typename T>() -> T*
	{
		T* Node = NewObject<T>(Graph);
		Node->SetFlags(RF_Transactional);
		Graph->AddNode(Node, true);
		Node->CreateNewGuid();
		Node->PostPlacedNewNode();
		if (UCustomizableObjectNode* TypedNode = Cast<UCustomizableObjectNode>(Node))
		{
			TypedNode->BeginConstruct();
			TypedNode->PostBackwardsCompatibleFixup();
		}
		Node->ReconstructNode();
		return Node;
	};

	// 1. Root Object Node
	UCustomizableObjectNodeObject* RootNode = nullptr;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (UCustomizableObjectNodeObject* ObjNode = Cast<UCustomizableObjectNodeObject>(Node))
		{
			if (ObjNode->bIsBase)
			{
				RootNode = ObjNode;
				break;
			}
		}
	}
	if (!RootNode)
	{
		RootNode = AddNode.operator()<UCustomizableObjectNodeObject>();
		RootNode->bIsBase = true;
	}
	RootNode->SetObjectName(TEXT("CO_Knight"));

	// Clean up any other existing nodes so we build a fresh, deterministic graph
	TArray<UEdGraphNode*> NodesToRemove;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node != RootNode)
		{
			NodesToRemove.Add(Node);
		}
	}
	for (UEdGraphNode* Node : NodesToRemove)
	{
		Graph->RemoveNode(Node);
	}

	// 2. Component Skeletal Mesh Node
	UCONodeComponentSkeletalMesh* CompNode = AddNode.operator()<UCONodeComponentSkeletalMesh>();
	CompNode->SetComponentName(TEXT("Body"));
	CompNode->ReferenceSkeletalMesh = SK_Knight;
	if (CompNode->OutputPin.Get() && RootNode->ComponentsPin())
	{
		CompNode->OutputPin.Get()->MakeLinkTo(RootNode->ComponentsPin());
	}

	// 3. Skeletal Mesh Object Make Node
	UCONodeSkeletalMeshObjectMake* MeshObjMakeNode = AddNode.operator()<UCONodeSkeletalMeshObjectMake>();
	if (MeshObjMakeNode->PassthroughSkeletalMeshPin.Get() && CompNode->SkeletalMeshPin.Get())
	{
		MeshObjMakeNode->PassthroughSkeletalMeshPin.Get()->MakeLinkTo(CompNode->SkeletalMeshPin.Get());
	}

	// 4. Skeletal Mesh Make V2 Node
	UCONodeSkeletalMeshMake_V2* SkelMakeNode = AddNode.operator()<UCONodeSkeletalMeshMake_V2>();
	if (SkelMakeNode->SkeletalMeshPin.Get() && MeshObjMakeNode->SkeletalMeshPin.Get())
	{
		SkelMakeNode->SkeletalMeshPin.Get()->MakeLinkTo(MeshObjMakeNode->SkeletalMeshPin.Get());
	}

	// 5. Skeletal Mesh Section Node
	UCONodeSkeletalMeshSection* SectionNode = AddNode.operator()<UCONodeSkeletalMeshSection>();
	if (M_Knight)
	{
		SectionNode->SetMaterial(M_Knight);
		SectionNode->ReconstructNode();
	}
	if (SectionNode->OutputPin() && SkelMakeNode->LODPins.Num() > 0 && SkelMakeNode->LODPins[0].Get())
	{
		SectionNode->OutputPin()->MakeLinkTo(SkelMakeNode->LODPins[0].Get());
	}

	// 6. Skeletal Mesh Nodes
	UCustomizableObjectNodeSkeletalMesh* KnightMeshNode = AddNode.operator()<UCustomizableObjectNodeSkeletalMesh>();
	KnightMeshNode->SkeletalMesh = SK_Knight;
	KnightMeshNode->ReconstructNode();

	UCustomizableObjectNodeSkeletalMesh* HelmDownMeshNode = nullptr;
	if (SK_KnightHelmDown)
	{
		HelmDownMeshNode = AddNode.operator()<UCustomizableObjectNodeSkeletalMesh>();
		HelmDownMeshNode->SkeletalMesh = SK_KnightHelmDown;
		HelmDownMeshNode->ReconstructNode();
	}

	UCustomizableObjectNodeSkeletalMesh* ArcherMeshNode = nullptr;
	if (SK_KnightArcher)
	{
		ArcherMeshNode = AddNode.operator()<UCustomizableObjectNodeSkeletalMesh>();
		ArcherMeshNode->SkeletalMesh = SK_KnightArcher;
		ArcherMeshNode->ReconstructNode();
	}

	UCustomizableObjectNodeSkeletalMesh* CaptainMeshNode = nullptr;
	if (SK_KnightCaptain)
	{
		CaptainMeshNode = AddNode.operator()<UCustomizableObjectNodeSkeletalMesh>();
		CaptainMeshNode->SkeletalMesh = SK_KnightCaptain;
		CaptainMeshNode->ReconstructNode();
	}

	UCustomizableObjectNodeSkeletalMesh* ChampionMeshNode = nullptr;
	if (SK_KnightChampion)
	{
		ChampionMeshNode = AddNode.operator()<UCustomizableObjectNodeSkeletalMesh>();
		ChampionMeshNode->SkeletalMesh = SK_KnightChampion;
		ChampionMeshNode->ReconstructNode();
	}

	UCustomizableObjectNodeSkeletalMesh* CommanderMeshNode = nullptr;
	if (SK_KnightCommander)
	{
		CommanderMeshNode = AddNode.operator()<UCustomizableObjectNodeSkeletalMesh>();
		CommanderMeshNode->SkeletalMesh = SK_KnightCommander;
		CommanderMeshNode->ReconstructNode();
	}

	// 7. Mesh Switches: BodyStyle and Helmet
	UEdGraphPin* FinalMeshOutputPin = nullptr;

	if (ArcherMeshNode && CaptainMeshNode && ChampionMeshNode && CommanderMeshNode)
	{
		// BodyStyle Enum Node (5 options)
		UCustomizableObjectNodeEnumParameter* BodyEnumNode = AddNode.operator()<UCustomizableObjectNodeEnumParameter>();
		BodyEnumNode->SetParameterName(TEXT("BodyStyle"));
		BodyEnumNode->Values.Empty();
		FCustomizableObjectNodeEnumValue VKnight; VKnight.Name = TEXT("Knight");
		FCustomizableObjectNodeEnumValue VArcher; VArcher.Name = TEXT("Archer");
		FCustomizableObjectNodeEnumValue VCaptain; VCaptain.Name = TEXT("Captain");
		FCustomizableObjectNodeEnumValue VChampion; VChampion.Name = TEXT("Champion");
		FCustomizableObjectNodeEnumValue VCommander; VCommander.Name = TEXT("Commander");
		BodyEnumNode->Values.Add(VKnight);
		BodyEnumNode->Values.Add(VArcher);
		BodyEnumNode->Values.Add(VCaptain);
		BodyEnumNode->Values.Add(VChampion);
		BodyEnumNode->Values.Add(VCommander);
		BodyEnumNode->DefaultIndex = 0;
		BodyEnumNode->ReconstructNode();

		UCONodeSwitch* BodySwitchNode = AddNode.operator()<UCONodeSwitch>();
		BodySwitchNode->PinType = UEdGraphSchema_CustomizableObject::PC_Mesh;

		UEdGraphPin* BodyEnumOutputPin = nullptr;
		for (UEdGraphPin* P : BodyEnumNode->Pins)
		{
			if (P && P->Direction == EGPD_Output && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Enum)
			{
				BodyEnumOutputPin = P;
				break;
			}
		}

		if (BodyEnumOutputPin && BodySwitchNode->SwitchParameterPinReference.Get())
		{
			BodyEnumOutputPin->MakeLinkTo(BodySwitchNode->SwitchParameterPinReference.Get());
			BodySwitchNode->ReconstructNode();
		}

		if (BodySwitchNode->SwitchPins.Num() >= 5)
		{
			if (KnightMeshNode->GetMeshPin(0, 0) && BodySwitchNode->SwitchPins[0].Get())
			{
				KnightMeshNode->GetMeshPin(0, 0)->MakeLinkTo(BodySwitchNode->SwitchPins[0].Get());
			}
			if (ArcherMeshNode->GetMeshPin(0, 0) && BodySwitchNode->SwitchPins[1].Get())
			{
				ArcherMeshNode->GetMeshPin(0, 0)->MakeLinkTo(BodySwitchNode->SwitchPins[1].Get());
			}
			if (CaptainMeshNode->GetMeshPin(0, 0) && BodySwitchNode->SwitchPins[2].Get())
			{
				CaptainMeshNode->GetMeshPin(0, 0)->MakeLinkTo(BodySwitchNode->SwitchPins[2].Get());
			}
			if (ChampionMeshNode->GetMeshPin(0, 0) && BodySwitchNode->SwitchPins[3].Get())
			{
				ChampionMeshNode->GetMeshPin(0, 0)->MakeLinkTo(BodySwitchNode->SwitchPins[3].Get());
			}
			if (CommanderMeshNode->GetMeshPin(0, 0) && BodySwitchNode->SwitchPins[4].Get())
			{
				CommanderMeshNode->GetMeshPin(0, 0)->MakeLinkTo(BodySwitchNode->SwitchPins[4].Get());
			}
		}

		UEdGraphPin* BodySwitchOutputPin = nullptr;
		for (UEdGraphPin* P : BodySwitchNode->Pins)
		{
			if (P && P->Direction == EGPD_Output && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Mesh)
			{
				BodySwitchOutputPin = P;
				break;
			}
		}

		// Helmet Switch (Up = selected body style, Down = HelmDown)
		if (HelmDownMeshNode && BodySwitchOutputPin)
		{
			UCustomizableObjectNodeEnumParameter* HelmetEnumNode = AddNode.operator()<UCustomizableObjectNodeEnumParameter>();
			HelmetEnumNode->SetParameterName(TEXT("Helmet"));
			HelmetEnumNode->Values.Empty();
			FCustomizableObjectNodeEnumValue ValUp; ValUp.Name = TEXT("Up");
			FCustomizableObjectNodeEnumValue ValDown; ValDown.Name = TEXT("Down");
			HelmetEnumNode->Values.Add(ValUp);
			HelmetEnumNode->Values.Add(ValDown);
			HelmetEnumNode->DefaultIndex = 0;
			HelmetEnumNode->ReconstructNode();

			UCONodeSwitch* HelmetSwitchNode = AddNode.operator()<UCONodeSwitch>();
			HelmetSwitchNode->PinType = UEdGraphSchema_CustomizableObject::PC_Mesh;

			UEdGraphPin* HelmetEnumOutputPin = nullptr;
			for (UEdGraphPin* P : HelmetEnumNode->Pins)
			{
				if (P && P->Direction == EGPD_Output && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Enum)
				{
					HelmetEnumOutputPin = P;
					break;
				}
			}

			if (HelmetEnumOutputPin && HelmetSwitchNode->SwitchParameterPinReference.Get())
			{
				HelmetEnumOutputPin->MakeLinkTo(HelmetSwitchNode->SwitchParameterPinReference.Get());
				HelmetSwitchNode->ReconstructNode();
			}

			if (HelmetSwitchNode->SwitchPins.Num() >= 2)
			{
				BodySwitchOutputPin->MakeLinkTo(HelmetSwitchNode->SwitchPins[0].Get());
				if (HelmDownMeshNode->GetMeshPin(0, 0) && HelmetSwitchNode->SwitchPins[1].Get())
				{
					HelmDownMeshNode->GetMeshPin(0, 0)->MakeLinkTo(HelmetSwitchNode->SwitchPins[1].Get());
				}
			}

			for (UEdGraphPin* P : HelmetSwitchNode->Pins)
			{
				if (P && P->Direction == EGPD_Output && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Mesh)
				{
					FinalMeshOutputPin = P;
					break;
				}
			}
		}
		else
		{
			FinalMeshOutputPin = BodySwitchOutputPin;
		}
	}
	else if (HelmDownMeshNode)
	{
		// Fallback 2-mesh switch
		UCustomizableObjectNodeEnumParameter* EnumNode = AddNode.operator()<UCustomizableObjectNodeEnumParameter>();
		EnumNode->SetParameterName(TEXT("Helmet"));
		EnumNode->Values.Empty();
		FCustomizableObjectNodeEnumValue ValUp; ValUp.Name = TEXT("Up");
		FCustomizableObjectNodeEnumValue ValDown; ValDown.Name = TEXT("Down");
		EnumNode->Values.Add(ValUp);
		EnumNode->Values.Add(ValDown);
		EnumNode->DefaultIndex = 0;
		EnumNode->ReconstructNode();

		UCONodeSwitch* SwitchNode = AddNode.operator()<UCONodeSwitch>();
		SwitchNode->PinType = UEdGraphSchema_CustomizableObject::PC_Mesh;

		UEdGraphPin* EnumOutputPin = nullptr;
		for (UEdGraphPin* P : EnumNode->Pins)
		{
			if (P && P->Direction == EGPD_Output && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Enum)
			{
				EnumOutputPin = P;
				break;
			}
		}

		if (EnumOutputPin && SwitchNode->SwitchParameterPinReference.Get())
		{
			EnumOutputPin->MakeLinkTo(SwitchNode->SwitchParameterPinReference.Get());
			SwitchNode->ReconstructNode();
		}

		if (SwitchNode->SwitchPins.Num() >= 2)
		{
			UEdGraphPin* KnightMeshPin = KnightMeshNode->GetMeshPin(0, 0);
			UEdGraphPin* HelmDownMeshPin = HelmDownMeshNode->GetMeshPin(0, 0);
			if (KnightMeshPin && SwitchNode->SwitchPins[0].Get())
			{
				KnightMeshPin->MakeLinkTo(SwitchNode->SwitchPins[0].Get());
			}
			if (HelmDownMeshPin && SwitchNode->SwitchPins[1].Get())
			{
				HelmDownMeshPin->MakeLinkTo(SwitchNode->SwitchPins[1].Get());
			}
		}

		for (UEdGraphPin* P : SwitchNode->Pins)
		{
			if (P && P->Direction == EGPD_Output && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Mesh)
			{
				FinalMeshOutputPin = P;
				break;
			}
		}
	}
	else
	{
		FinalMeshOutputPin = KnightMeshNode->GetMeshPin(0, 0);
	}

	if (FinalMeshOutputPin && SectionNode->GetMeshPin())
	{
		FinalMeshOutputPin->MakeLinkTo(SectionNode->GetMeshPin());
	}

	// 8. Color Parameter: ArmorTint
	UCustomizableObjectNodeColorParameter* ColorNode = AddNode.operator()<UCustomizableObjectNodeColorParameter>();
	ColorNode->SetParameterName(TEXT("ArmorTint"));
	ColorNode->DefaultValue = FLinearColor::White;
	ColorNode->ReconstructNode();

	UEdGraphPin* ColorOutputPin = nullptr;
	for (UEdGraphPin* P : ColorNode->Pins)
	{
		if (P && P->Direction == EGPD_Output && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Color)
		{
			ColorOutputPin = P;
			break;
		}
	}

	UEdGraphPin* SectionColorPin = nullptr;
	for (UEdGraphPin* P : SectionNode->Pins)
	{
		if (P && P->Direction == EGPD_Input && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Color)
		{
			SectionColorPin = P;
			break;
		}
	}

	if (ColorOutputPin && SectionColorPin)
	{
		ColorOutputPin->MakeLinkTo(SectionColorPin);
		UE_LOG(LogMutable, Display, TEXT("[RecipeBuilder] Linked ColorNode output to SectionNode ArmorTint input"));
	}

	// 9. Synchronous compilation
	ECustomizableObjectCompilationState State = UCustomizableObjectEditorFunctionLibrary::CompileCustomizableObjectSynchronously(CO);
	if (State != ECustomizableObjectCompilationState::Completed)
	{
		OutMessage = FString::Printf(TEXT("Compilation failed for %ls (state %d)"), *FullPackageName, static_cast<int32>(State));
		return false;
	}

	// 10. Save asset package
	UPackage* Package = CO->GetPackage();
	if (Package)
	{
		Package->MarkPackageDirty();
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		const FString PackageFileName = FPackageName::LongPackageNameToFilename(FullPackageName, FPackageName::GetAssetPackageExtension());
		UPackage::Save(Package, CO, *PackageFileName, SaveArgs);
		FAssetRegistryModule::AssetCreated(CO);
	}

	OutMessage = FString::Printf(TEXT("Successfully compiled and saved %ls"), *FullPackageName);
	return true;
}
#endif
