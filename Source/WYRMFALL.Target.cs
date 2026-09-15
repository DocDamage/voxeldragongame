using UnrealBuildTool;
using System.Collections.Generic;
public class WYRMFALLTarget : TargetRules
{
    public WYRMFALLTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("WYRMFALL");
    }
}
