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
            "NavigationSystem", "AIModule"
        });
        // Mutable is enabled in the project; add CustomizableObject only when
        // WP-02 consumes its verified headers. No invented terrain dependency.
    }
}
