using System.IO;
using UnrealBuildTool;
public class WYRMFALL : ModuleRules
{
    public WYRMFALL(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
            "GameplayAbilities", "GameplayTags", "GameplayTasks"
        });
        PrivateDependencyModuleNames.AddRange(new string[] {
            "NavigationSystem", "AIModule", "ProceduralMeshComponent", "GeoForgeRuntime", "CustomizableObject"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "UnrealEd", "CustomizableObjectEditor", "MutableRuntime", "MutableTools"
            });
            PrivateIncludePaths.AddRange(new string[] {
                Path.Combine(EngineDirectory, "Plugins", "Mutable", "Source", "CustomizableObject", "Internal"),
                Path.Combine(EngineDirectory, "Plugins", "Mutable", "Source", "MutableRuntime", "Internal"),
                Path.Combine(EngineDirectory, "Plugins", "Mutable", "Source", "MutableTools", "Internal"),
                Path.Combine(EngineDirectory, "Plugins", "Mutable", "Source", "CustomizableObjectEditor", "Private"),
                Path.Combine(EngineDirectory, "Plugins", "Mutable", "Source", "CustomizableObjectEditor", "Private", "MuCOE"),
                Path.Combine(EngineDirectory, "Plugins", "Mutable", "Source", "CustomizableObjectEditor", "Private", "MuCOE", "Nodes")
            });
        }

    }
}
