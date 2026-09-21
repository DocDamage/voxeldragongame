#include "Dragon/WyrmDragonTypes.h"

bool FWyrmDragonRigProfile::IsValidDragonRig(FName InDragonId)
{
    return InDragonId == FName(TEXT("Verdance")) ||
        InDragonId == FName(TEXT("Jadefang")) ||
        InDragonId == FName(TEXT("Nyxaroth")) ||
        InDragonId == FName(TEXT("Cogfang")) ||
        InDragonId == FName(TEXT("Grovemaw")) ||
        InDragonId == FName(TEXT("Rotwing")) ||
        InDragonId == FName(TEXT("Ossuroth"));
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
    else if (InDragonId == FName(TEXT("Nyxaroth")))
    {
        OutProfile.DragonId = FName(TEXT("Nyxaroth"));
        OutProfile.DisplayName = TEXT("Nyxaroth");
        OutProfile.LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Nyxaroth/Dark_Dragon/SkeletalMeshes/Hip-Local.Hip-Local");
        OutProfile.FollowerMeshBasePath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Nyxaroth/Dark_Dragon/SkeletalMeshes");
        OutProfile.SkeletonPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Nyxaroth/Dark_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton");
        OutProfile.IdleAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Nyxaroth/Dark_Dragon/SkeletalMeshes/Dark_DragonIdle_01.Dark_DragonIdle_01");
        OutProfile.FlightAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_5/Nyxaroth/Dark_Dragon/SkeletalMeshes/Dark_DragonFlying_01.Dark_DragonFlying_01");
        OutProfile.CompanionMeshScale = 0.0085f;
        OutProfile.CompanionCapsuleRadius = 28.f;
        OutProfile.CompanionCapsuleHalfHeight = 34.f;
        OutProfile.CompanionGroundSpeed = 460.f;
        OutProfile.TrueFormMeshScale = 0.032f;
        OutProfile.TrueFormCapsuleRadius = 115.f;
        OutProfile.TrueFormCapsuleHalfHeight = 155.f;
        OutProfile.TrueFormGroundSpeed = 575.f;
        OutProfile.FlightSpeed = 1650.f;
        OutProfile.MountSocketOffset = FVector(0.f, 0.f, 150.f);
        OutProfile.TakeoffClearanceHeight = 480.f;
        OutProfile.WingSpanSweepRadius = 325.f;
        OutProfile.LandingSearchDistance = 1200.f;
        OutProfile.MaxLandingSlopeAngle = 45.f;
        OutProfile.FollowerMeshNames = {
            TEXT("Chest-Local"), TEXT("Head-Local"), TEXT("Jaw-Local"),
            TEXT("LB_Foot-Local"), TEXT("LB_Leg1-Local"), TEXT("LB_Leg2-Local"), TEXT("LB_Thigh-Local"),
            TEXT("LF_Claw-Local"), TEXT("LF_Foot-Local"), TEXT("LF_Leg-Local"), TEXT("LF_Thigh-Local"),
            TEXT("L_Wing1-Local"), TEXT("L_Wing2-Local"), TEXT("L_Wing3-Local"), TEXT("Neck-Local"),
            TEXT("RB_Foot-Local"), TEXT("RB_Leg1-Local"), TEXT("RB_Leg2-Local"), TEXT("RB_Thigh-Local"),
            TEXT("RF_Claw-Local"), TEXT("RF_Foot-Local"), TEXT("RF_Leg-Local"), TEXT("RF_Thigh-Local"),
            TEXT("R_Wing1-Local"), TEXT("R_Wing2-Local"), TEXT("R_Wing3-Local"),
            TEXT("Tail_1-Local"), TEXT("Tail_2-Local"), TEXT("Tail_3-Local"), TEXT("Tail_4-Local"),
            TEXT("ear_L-Local"), TEXT("ear_R-Local")
        };
        return true;
    }
    else if (InDragonId == FName(TEXT("Cogfang")))
    {
        OutProfile.DragonId = FName(TEXT("Cogfang"));
        OutProfile.DisplayName = TEXT("Cogfang");
        OutProfile.LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/Cogfang/Steampunk_Dragon/SkeletalMeshes/Hip-Local.Hip-Local");
        OutProfile.FollowerMeshBasePath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/Cogfang/Steampunk_Dragon/SkeletalMeshes");
        OutProfile.SkeletonPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/Cogfang/Steampunk_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton");
        OutProfile.IdleAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/Cogfang/Steampunk_Dragon/SkeletalMeshes/Steampunk_DragonIdle_01.Steampunk_DragonIdle_01");
        OutProfile.FlightAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_6/Cogfang/Steampunk_Dragon/SkeletalMeshes/Steampunk_DragonFlying_01.Steampunk_DragonFlying_01");
        OutProfile.CompanionMeshScale = 0.009f;
        OutProfile.CompanionCapsuleRadius = 32.f;
        OutProfile.CompanionCapsuleHalfHeight = 38.f;
        OutProfile.CompanionGroundSpeed = 420.f;
        OutProfile.TrueFormMeshScale = 0.036f;
        OutProfile.TrueFormCapsuleRadius = 125.f;
        OutProfile.TrueFormCapsuleHalfHeight = 165.f;
        OutProfile.TrueFormGroundSpeed = 525.f;
        OutProfile.FlightSpeed = 1500.f;
        OutProfile.MountSocketOffset = FVector(0.f, 0.f, 170.f);
        OutProfile.TakeoffClearanceHeight = 520.f;
        OutProfile.WingSpanSweepRadius = 360.f;
        OutProfile.LandingSearchDistance = 1200.f;
        OutProfile.MaxLandingSlopeAngle = 42.f;
        OutProfile.FollowerMeshNames = {
            TEXT("Chest-Local"), TEXT("Claw-Local"), TEXT("Head-Local"), TEXT("Jaw-Local"),
            TEXT("LB_Claw-Local"), TEXT("LB_Foot-Local"), TEXT("LB_Leg1-Local"), TEXT("LB_Leg2-Local"),
            TEXT("LB_Thigh-Local"), TEXT("LF_Claw-Local"), TEXT("LF_Foot-Local"), TEXT("LF_Leg-Local"),
            TEXT("LF_Thigh-Local"), TEXT("L_Wing1-Local"), TEXT("L_Wing2-Local"), TEXT("L_Wing3-Local"),
            TEXT("Neck-Local"), TEXT("RB_Foot-Local"), TEXT("RB_Leg1-Local"), TEXT("RB_Leg2-Local"),
            TEXT("RB_Thigh-Local"), TEXT("RF_Claw-Local"), TEXT("RF_Foot-Local"), TEXT("RF_Leg-Local"),
            TEXT("RF_Thigh-Local"), TEXT("R_Wing1-Local"), TEXT("R_Wing2-Local"), TEXT("R_Wing3-Local"),
            TEXT("Tail_1-Local"), TEXT("Tail_2-Local"), TEXT("Tail_3-Local"), TEXT("Tail_4-Local"),
            TEXT("ear_L-Local"), TEXT("ear_R-Local")
        };
        return true;
    }
    else if (InDragonId == FName(TEXT("Grovemaw")))
    {
        OutProfile.DragonId = FName(TEXT("Grovemaw"));
        OutProfile.DisplayName = TEXT("Grovemaw");
        OutProfile.LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_3/Grovemaw/Wooden_Dragon/SkeletalMeshes/Hip-Local.Hip-Local");
        OutProfile.FollowerMeshBasePath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_3/Grovemaw/Wooden_Dragon/SkeletalMeshes");
        OutProfile.SkeletonPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_3/Grovemaw/Wooden_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton");
        OutProfile.IdleAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_3/Grovemaw/Wooden_Dragon/SkeletalMeshes/Wooden_DragonIdle_01.Wooden_DragonIdle_01");
        OutProfile.FlightAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_3/Grovemaw/Wooden_Dragon/SkeletalMeshes/Wooden_DragonFlying_01.Wooden_DragonFlying_01");
        OutProfile.CompanionMeshScale = 0.0085f;
        OutProfile.CompanionCapsuleRadius = 29.f;
        OutProfile.CompanionCapsuleHalfHeight = 35.f;
        OutProfile.CompanionGroundSpeed = 440.f;
        OutProfile.TrueFormMeshScale = 0.034f;
        OutProfile.TrueFormCapsuleRadius = 118.f;
        OutProfile.TrueFormCapsuleHalfHeight = 158.f;
        OutProfile.TrueFormGroundSpeed = 540.f;
        OutProfile.FlightSpeed = 1550.f;
        OutProfile.MountSocketOffset = FVector(0.f, 0.f, 155.f);
        OutProfile.TakeoffClearanceHeight = 500.f;
        OutProfile.WingSpanSweepRadius = 340.f;
        OutProfile.LandingSearchDistance = 1200.f;
        OutProfile.MaxLandingSlopeAngle = 44.f;
        OutProfile.FollowerMeshNames = {
            TEXT("Chest-Local"), TEXT("Claw-Local"), TEXT("Head-Local"), TEXT("Jaw-Local"),
            TEXT("LB_Claw-Local"), TEXT("LB_Foot-Local"), TEXT("LB_Leg1-Local"), TEXT("LB_Leg2-Local"),
            TEXT("LB_Thigh-Local"), TEXT("LF_Claw-Local"), TEXT("LF_Foot-Local"), TEXT("LF_Leg-Local"),
            TEXT("LF_Thigh-Local"), TEXT("L_Wing1-Local"), TEXT("L_Wing2-Local"), TEXT("L_Wing3-Local"),
            TEXT("Neck-Local"), TEXT("RB_Foot-Local"), TEXT("RB_Leg1-Local"), TEXT("RB_Leg2-Local"),
            TEXT("RB_Thigh-Local"), TEXT("RF_Claw-Local"), TEXT("RF_Foot-Local"), TEXT("RF_Leg-Local"),
            TEXT("RF_Thigh-Local"), TEXT("R_Wing1-Local"), TEXT("R_Wing2-Local"), TEXT("R_Wing3-Local"),
            TEXT("Tail_1-Local"), TEXT("Tail_2-Local"), TEXT("Tail_3-Local"), TEXT("Tail_4-Local"),
            TEXT("ear_L-Local"), TEXT("ear_R-Local")
        };
        return true;
    }
    else if (InDragonId == FName(TEXT("Rotwing")))
    {
        OutProfile.DragonId = FName(TEXT("Rotwing"));
        OutProfile.DisplayName = TEXT("Rotwing");
        OutProfile.LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_8/Rotwing/Zombie_Dragon/SkeletalMeshes/Hip-Local.Hip-Local");
        OutProfile.FollowerMeshBasePath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_8/Rotwing/Zombie_Dragon/SkeletalMeshes");
        OutProfile.SkeletonPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_8/Rotwing/Zombie_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton");
        OutProfile.IdleAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_8/Rotwing/Zombie_Dragon/SkeletalMeshes/Zombie_DragonIdle_01.Zombie_DragonIdle_01");
        OutProfile.FlightAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_8/Rotwing/Zombie_Dragon/SkeletalMeshes/Zombie_DragonFlying_01.Zombie_DragonFlying_01");
        OutProfile.CompanionMeshScale = 0.0088f;
        OutProfile.CompanionCapsuleRadius = 31.f;
        OutProfile.CompanionCapsuleHalfHeight = 37.f;
        OutProfile.CompanionGroundSpeed = 410.f;
        OutProfile.TrueFormMeshScale = 0.035f;
        OutProfile.TrueFormCapsuleRadius = 122.f;
        OutProfile.TrueFormCapsuleHalfHeight = 162.f;
        OutProfile.TrueFormGroundSpeed = 510.f;
        OutProfile.FlightSpeed = 1450.f;
        OutProfile.MountSocketOffset = FVector(0.f, 0.f, 165.f);
        OutProfile.TakeoffClearanceHeight = 510.f;
        OutProfile.WingSpanSweepRadius = 345.f;
        OutProfile.LandingSearchDistance = 1200.f;
        OutProfile.MaxLandingSlopeAngle = 40.f;
        OutProfile.FollowerMeshNames = {
            TEXT("Chest-Local"), TEXT("Claw-Local"), TEXT("Head-Local"), TEXT("Jaw-Local"),
            TEXT("LB_Claw-Local"), TEXT("LB_Foot-Local"), TEXT("LB_Leg1-Local"), TEXT("LB_Leg2-Local"),
            TEXT("LB_Thigh-Local"), TEXT("LF_Claw-Local"), TEXT("LF_Foot-Local"), TEXT("LF_Leg-Local"),
            TEXT("LF_Thigh-Local"), TEXT("L_Wing1-Local"), TEXT("L_Wing2-Local"), TEXT("L_Wing3-Local"),
            TEXT("Neck-Local"), TEXT("RB_Foot-Local"), TEXT("RB_Leg1-Local"), TEXT("RB_Leg2-Local"),
            TEXT("RB_Thigh-Local"), TEXT("RF_Claw-Local"), TEXT("RF_Foot-Local"), TEXT("RF_Leg-Local"),
            TEXT("RF_Thigh-Local"), TEXT("R_Wing1-Local"), TEXT("R_Wing2-Local"), TEXT("R_Wing3-Local"),
            TEXT("Tail_1-Local"), TEXT("Tail_2-Local"), TEXT("Tail_3-Local"), TEXT("Tail_4-Local"),
            TEXT("ear_L-Local"), TEXT("ear_R-Local")
        };
        return true;
    }
    else if (InDragonId == FName(TEXT("Ossuroth")))
    {
        OutProfile.DragonId = FName(TEXT("Ossuroth"));
        OutProfile.DisplayName = TEXT("Ossuroth");
        OutProfile.LeaderMeshPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_9/Ossuroth/Skull_Dragon/SkeletalMeshes/Hip-Local.Hip-Local");
        OutProfile.FollowerMeshBasePath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_9/Ossuroth/Skull_Dragon/SkeletalMeshes");
        OutProfile.SkeletonPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_9/Ossuroth/Skull_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton");
        OutProfile.IdleAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_9/Ossuroth/Skull_Dragon/SkeletalMeshes/Skull_DragonIdle_01.Skull_DragonIdle_01");
        OutProfile.FlightAnimPath = TEXT("/Game/WYRMFALL/Development/Intake/WP23_9/Ossuroth/Skull_Dragon/SkeletalMeshes/Skull_DragonFlying_01.Skull_DragonFlying_01");
        OutProfile.CompanionMeshScale = 0.009f;
        OutProfile.CompanionCapsuleRadius = 33.f;
        OutProfile.CompanionCapsuleHalfHeight = 40.f;
        OutProfile.CompanionGroundSpeed = 390.f;
        OutProfile.TrueFormMeshScale = 0.036f;
        OutProfile.TrueFormCapsuleRadius = 128.f;
        OutProfile.TrueFormCapsuleHalfHeight = 170.f;
        OutProfile.TrueFormGroundSpeed = 490.f;
        OutProfile.FlightSpeed = 1400.f;
        OutProfile.MountSocketOffset = FVector(0.f, 0.f, 170.f);
        OutProfile.TakeoffClearanceHeight = 530.f;
        OutProfile.WingSpanSweepRadius = 360.f;
        OutProfile.LandingSearchDistance = 1250.f;
        OutProfile.MaxLandingSlopeAngle = 38.f;
        OutProfile.FollowerMeshNames = {
            TEXT("Chest-Local"), TEXT("Claw-Local"), TEXT("Head-Local"), TEXT("Jaw-Local"),
            TEXT("LB_Claw-Local"), TEXT("LB_Foot-Local"), TEXT("LB_Leg1-Local"), TEXT("LB_Leg2-Local"),
            TEXT("LB_Thigh-Local"), TEXT("LF_Claw-Local"), TEXT("LF_Foot-Local"), TEXT("LF_Leg-Local"),
            TEXT("LF_Thigh-Local"), TEXT("L_Wing1-Local"), TEXT("L_Wing2-Local"), TEXT("L_Wing3-Local"),
            TEXT("Neck-Local"), TEXT("RB_Foot-Local"), TEXT("RB_Leg1-Local"), TEXT("RB_Leg2-Local"),
            TEXT("RB_Thigh-Local"), TEXT("RF_Claw-Local"), TEXT("RF_Foot-Local"), TEXT("RF_Leg-Local"),
            TEXT("RF_Thigh-Local"), TEXT("R_Wing1-Local"), TEXT("R_Wing2-Local"), TEXT("R_Wing3-Local"),
            TEXT("Tail_1-Local"), TEXT("Tail_2-Local"), TEXT("Tail_3-Local"), TEXT("Tail_4-Local"),
            TEXT("ear_L-Local"), TEXT("ear_R-Local"), TEXT("horn1_ear1-2-Local"), TEXT("horn1_ear1-3-Local"),
            TEXT("horn2_ear1-2-Local"), TEXT("horn2_ear1-3-Local")
        };
        return true;
    }

    return false;
}
