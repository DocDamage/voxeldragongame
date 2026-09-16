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

	// 7. Mesh Switch with Enum Parameter for Helmet
	if (HelmDownMeshNode)
	{
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

		UEdGraphPin* SwitchOutputPin = nullptr;
		for (UEdGraphPin* P : SwitchNode->Pins)
		{
			if (P && P->Direction == EGPD_Output && P->PinType.PinCategory == UEdGraphSchema_CustomizableObject::PC_Mesh)
			{
				SwitchOutputPin = P;
				break;
			}
		}

		if (SwitchOutputPin && SectionNode->GetMeshPin())
		{
			SwitchOutputPin->MakeLinkTo(SectionNode->GetMeshPin());
		}
	}
	else
	{
		UEdGraphPin* KnightMeshPin = KnightMeshNode->GetMeshPin(0, 0);
		if (KnightMeshPin && SectionNode->GetMeshPin())
		{
			KnightMeshPin->MakeLinkTo(SectionNode->GetMeshPin());
		}
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
