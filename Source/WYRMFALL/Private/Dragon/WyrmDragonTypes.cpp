#include "Dragon/WyrmDragonTypes.h"

bool FWyrmDragonRigProfile::IsValidDragonRig(FName InDragonId)
{
    return InDragonId == FName(TEXT("Verdance")) || InDragonId == FName(TEXT("Jadefang"));
}

bool FWyrmDragonRigProfile::GetRigProfile(FName InDragonId, FWyrmDragonRigProfile& OutProfile)
{
    if (InDragonId == FName(TEXT("Verdance")))
    {
        OutProfile.DragonId = FName(TEXT("Verdance"));
        OutProfile.DisplayName = TEXT("Verdance");
        OutProfile.LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local.Hip-Local");
        OutProfile.FollowerMeshBasePath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes");
        OutProfile.SkeletonPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton");
        OutProfile.IdleAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonIdle_01.Green_DragonIdle_01");
        OutProfile.FlightAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonFlying_01.Green_DragonFlying_01");
        OutProfile.CompanionMeshScale = 0.009f;
        OutProfile.CompanionCapsuleRadius = 30.f;
        OutProfile.CompanionCapsuleHalfHeight = 35.f;
        OutProfile.CompanionGroundSpeed = 450.f;
        OutProfile.TrueFormMeshScale = 0.035f;
        OutProfile.TrueFormCapsuleRadius = 120.f;
        OutProfile.TrueFormCapsuleHalfHeight = 160.f;
        OutProfile.TrueFormGroundSpeed = 550.f;
        OutProfile.FlightSpeed = 1600.f;
        OutProfile.MountSocketOffset = FVector(0.f, 0.f, 160.f);
        OutProfile.TakeoffClearanceHeight = 500.f;
        OutProfile.WingSpanSweepRadius = 350.f;
        OutProfile.LandingSearchDistance = 1200.f;
        OutProfile.MaxLandingSlopeAngle = 45.f;
        OutProfile.FollowerMeshNames = {
            TEXT("Chest-Local"), TEXT("Claw-Local"), TEXT("Detail3-1-Local"), TEXT("Detail4-1-Local"),
            TEXT("Detail5-1-Local"), TEXT("Detail6-1-Local"), TEXT("ear_L-Local"), TEXT("ear_R-Local"),
            TEXT("Head-Local"), TEXT("horn1_ear1-2-Local"), TEXT("horn1_ear1-3-Local"), TEXT("horn2_ear1-2-Local"),
            TEXT("horn2_ear1-3-Local"), TEXT("Jaw-Local"), TEXT("L_Wing1-Local"), TEXT("L_Wing2-Local"),
            TEXT("L_Wing3-Local"), TEXT("LB_Claw-Local"), TEXT("LB_Foot-Local"), TEXT("LB_Leg1-Local"),
            TEXT("LB_Leg2-Local"), TEXT("LB_Thigh-Local"), TEXT("LF_Claw-Local"), TEXT("LF_Foot-Local"),
            TEXT("LF_Leg-Local"), TEXT("LF_Thigh-Local"), TEXT("Mini2-1-Local"), TEXT("Neck-Local"),
            TEXT("R_Wing1-Local"), TEXT("R_Wing2-Local"), TEXT("R_Wing3-Local"), TEXT("RB_Foot-Local"),
            TEXT("RB_Leg1-Local"), TEXT("RB_Leg2-Local"), TEXT("RB_Thigh-Local"), TEXT("RF_Claw-Local"),
            TEXT("RF_Foot-Local"), TEXT("RF_Leg-Local"), TEXT("RF_Thigh-Local"), TEXT("Tail_1-Local"),
            TEXT("Tail_2-Local"), TEXT("Tail_3-Local"), TEXT("Tail_4-Local")
        };
        return true;
    }
    else if (InDragonId == FName(TEXT("Jadefang")))
    {
        OutProfile.DragonId = FName(TEXT("Jadefang"));
        OutProfile.DisplayName = TEXT("Jadefang");
        OutProfile.LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/SkeletalMeshes/Hip-Local.Hip-Local");
        OutProfile.FollowerMeshBasePath = TEXT("/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/SkeletalMeshes");
        OutProfile.SkeletonPath = TEXT("/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton");
        OutProfile.IdleAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/SkeletalMeshes/Chinese+DragonIdle_01.Chinese+DragonIdle_01");
        OutProfile.FlightAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/SkeletalMeshes/Chinese+DragonFlying_01.Chinese+DragonFlying_01");
        OutProfile.CompanionMeshScale = 0.008f;
        OutProfile.CompanionCapsuleRadius = 30.f;
        OutProfile.CompanionCapsuleHalfHeight = 35.f;
        OutProfile.CompanionGroundSpeed = 480.f;
        OutProfile.TrueFormMeshScale = 0.030f;
        OutProfile.TrueFormCapsuleRadius = 110.f;
        OutProfile.TrueFormCapsuleHalfHeight = 150.f;
        OutProfile.TrueFormGroundSpeed = 600.f;
        OutProfile.FlightSpeed = 1700.f;
        OutProfile.MountSocketOffset = FVector(0.f, 0.f, 140.f);
        OutProfile.TakeoffClearanceHeight = 450.f;
        OutProfile.WingSpanSweepRadius = 300.f;
        OutProfile.LandingSearchDistance = 1200.f;
        OutProfile.MaxLandingSlopeAngle = 45.f;
        OutProfile.FollowerMeshNames = {
            TEXT("Chest-Local"), TEXT("Claw-Local"), TEXT("Detail3-1-Local"), TEXT("Detail6-1-Local"),
            TEXT("ear_L-Local"), TEXT("ear_R-Local"), TEXT("Head-Local"), TEXT("horn1_ear1-2-Local"),
            TEXT("horn1_ear1-3-Local"), TEXT("horn1_ear1-4-Local"), TEXT("horn2_ear1-2-Local"), TEXT("horn2_ear1-3-Local"),
            TEXT("horn2_ear1-4-Local"), TEXT("Jaw-Local"), TEXT("LB_Claw-Local"), TEXT("LB_Foot-Local"),
            TEXT("LB_Leg1-Local"), TEXT("LB_Leg2-Local"), TEXT("LB_Thigh-Local"), TEXT("LF_Claw-Local"),
            TEXT("LF_Foot-Local"), TEXT("LF_Leg-Local"), TEXT("LF_Thigh-Local"), TEXT("Mini2-1-Local"),
            TEXT("Mini3-1-Local"), TEXT("Neck-Local"), TEXT("RB_Foot-Local"), TEXT("RB_Leg1-Local"),
            TEXT("RB_Leg2-Local"), TEXT("RB_Thigh-Local"), TEXT("RF_Claw-Local"), TEXT("RF_Foot-Local"),
            TEXT("RF_Leg-Local"), TEXT("RF_Thigh-Local"), TEXT("Tail_1-Local"), TEXT("Tail_2-Local"),
            TEXT("Tail_3-Local"), TEXT("Tail_4-Local")
        };
        return true;
    }

    return false;
}
