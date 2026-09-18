#include "Dragon/WyrmVerdanceBossArena.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "Region/WyrmRegion01Interactable.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "Combat/WyrmEnemyCharacter.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Player/WyrmCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AWyrmVerdanceBossArena::AWyrmVerdanceBossArena()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    ArenaVolume = CreateDefaultSubobject<USphereComponent>(TEXT("ArenaVolume"));
    ArenaVolume->SetupAttachment(SceneRoot);
    ArenaVolume->InitSphereRadius(ArenaRadius);
    ArenaVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void AWyrmVerdanceBossArena::BeginPlay()
{
    Super::BeginPlay();

    SetActorLocation(ArenaCenter);
    if (ArenaVolume)
    {
        ArenaVolume->SetSphereRadius(ArenaRadius);
    }

    // Locate or link VerdanceBoss
    if (!VerdanceBoss)
    {
        for (TActorIterator<AWyrmDragonCharacter> It(GetWorld()); It; ++It)
        {
            if (It->DragonId == FName(TEXT("Verdance")))
            {
                VerdanceBoss = *It;
                break;
            }
        }
    }

    if (VerdanceBoss)
    {
        VerdanceBoss->OnDragonDefeated.AddDynamic(this, &AWyrmVerdanceBossArena::HandleDragonDefeated);
    }

    // Locate CentralClaimConsole
    if (!CentralClaimConsole)
    {
        for (TActorIterator<AWyrmRegion01Interactable> It(GetWorld()); It; ++It)
        {
            if (It->InteractableType == EWyrmRegion01InteractableType::CentralClaim)
            {
                CentralClaimConsole = *It;
                break;
            }
        }
    }

    SyncArenaStateWithLedger();
}

void AWyrmVerdanceBossArena::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (ArenaState == EWyrmBossArenaState::Inactive)
    {
        if (IsPlayerInsideArena() && VerdanceBoss && VerdanceBoss->GetDragonRole() == EWyrmDragonRole::HostileBoss)
        {
            StartBossCombat();
        }
    }
    else if (ArenaState == EWyrmBossArenaState::BossCombat)
    {
        if (IsAuxiliaryInterferenceActive())
        {
            InterferenceTimer += DeltaSeconds;
            if (InterferenceTimer >= InterferencePulseInterval)
            {
                EmitInterferencePulse();
                InterferenceTimer = 0.f;
            }
        }
    }
}

AWyrmCharacter* AWyrmVerdanceBossArena::GetPlayerCharacter() const
{
    if (UWorld* World = GetWorld())
    {
        return Cast<AWyrmCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    }
    return nullptr;
}

bool AWyrmVerdanceBossArena::IsPlayerInsideArena() const
{
    AWyrmCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        return false;
    }
    const float DistSq = FVector::DistSquared(ArenaCenter, Player->GetActorLocation());
    return DistSq <= FMath::Square(ArenaRadius);
}

bool AWyrmVerdanceBossArena::IsAuxiliaryInterferenceActive() const
{
    if (ArenaState != EWyrmBossArenaState::BossCombat)
    {
        return false;
    }

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
            {
                if (Region01->HasFact(FName(TEXT("quarry.aux_disabled"))))
                {
                    // Disabled via Sella route: removes the interference pattern
                    return false;
                }
            }
        }
    }
    return true;
}

void AWyrmVerdanceBossArena::EmitInterferencePulse()
{
    TotalInterferencePulsesEmitted++;

    AWyrmCharacter* Player = GetPlayerCharacter();
    if (Player && IsPlayerInsideArena())
    {
        UAbilitySystemComponent* SourceASC = VerdanceBoss ? VerdanceBoss->GetAbilitySystemComponent() : nullptr;
        UAbilitySystemComponent* TargetASC = Player->GetAbilitySystemComponent();

        if (SourceASC && TargetASC)
        {
            UWyrmMeleeAttackAbility::ApplyDamageEffect(SourceASC, TargetASC, InterferencePulseDamage);
        }
    }
}

void AWyrmVerdanceBossArena::StartBossCombat()
{
    ArenaState = EWyrmBossArenaState::BossCombat;
    InterferenceTimer = 0.f;
    if (VerdanceBoss)
    {
        AWyrmCharacter* Player = GetPlayerCharacter();
        VerdanceBoss->SetDragonRole(EWyrmDragonRole::HostileBoss);
    }
}

void AWyrmVerdanceBossArena::HandleDragonDefeated(AWyrmDragonCharacter* DefeatedDragon)
{
    ArenaState = EWyrmBossArenaState::LivingDefeat;
    InterferenceTimer = 0.f;

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
            {
                Region01->RecordVerdanceDefeatedAlive();
            }
        }
    }
}

bool AWyrmVerdanceBossArena::CanBreakClaim() const
{
    if (ArenaState != EWyrmBossArenaState::LivingDefeat)
    {
        return false;
    }

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
            {
                return Region01->HasFact(FName(TEXT("verdance.defeated_alive"))) &&
                       !Region01->HasFact(FName(TEXT("verdance.claim_broken")));
            }
        }
    }
    return false;
}

bool AWyrmVerdanceBossArena::InteractBreakClaim(AActor* Interactor)
{
    if (!CanBreakClaim())
    {
        return false;
    }

    if (CentralClaimConsole)
    {
        if (CentralClaimConsole->Interact(Interactor))
        {
            ArenaState = EWyrmBossArenaState::ClaimBroken;
            return true;
        }
    }

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
            {
                if (Region01->BreakVerdanceClaim())
                {
                    ArenaState = EWyrmBossArenaState::ClaimBroken;
                    return true;
                }
            }
        }
    }
    return false;
}

bool AWyrmVerdanceBossArena::CanOfferVoluntaryBond(const AActor* Interactor) const
{
    if (ArenaState != EWyrmBossArenaState::ClaimBroken)
    {
        return false;
    }

    if (!VerdanceBoss)
    {
        return false;
    }

    return VerdanceBoss->CanOfferVoluntaryBond(Interactor);
}

bool AWyrmVerdanceBossArena::InteractVoluntaryBond(AWyrmCharacter* Humanoid)
{
    if (!CanOfferVoluntaryBond(Humanoid))
    {
        return false;
    }

    if (VerdanceBoss->InteractVoluntaryBond(Humanoid))
    {
        if (UWorld* World = GetWorld())
        {
            if (UGameInstance* GI = World->GetGameInstance())
            {
                if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
                {
                    Region01->RecordVerdanceBondAccepted(VerdanceBoss);
                }
            }
        }
        ArenaState = EWyrmBossArenaState::CompanionBonded;
        TriggerReliefEncounter();
        return true;
    }

    return false;
}

void AWyrmVerdanceBossArena::TriggerReliefEncounter()
{
    ArenaState = EWyrmBossArenaState::ReliefCombat;

    if (UWorld* World = GetWorld())
    {
        FVector SpawnLocA = ArenaCenter + ReliefSpawnOffset;
        FVector SpawnLocB = ArenaCenter + ReliefSpawnOffset + FVector(0.f, 200.f, 0.f);

        AWyrmEnemyCharacter* EnforcerA = AWyrmEnemyCharacter::SpawnWyrmEnemy(this, EWyrmEnemyRole::MeleeChaser, FTransform(SpawnLocA));
        AWyrmEnemyCharacter* EnforcerB = AWyrmEnemyCharacter::SpawnWyrmEnemy(this, EWyrmEnemyRole::MeleeChaser, FTransform(SpawnLocB));

        if (EnforcerA) ReliefSquad.Add(EnforcerA);
        if (EnforcerB) ReliefSquad.Add(EnforcerB);
    }
}

bool AWyrmVerdanceBossArena::ResolveReliefCombat()
{
    for (AWyrmEnemyCharacter* Enemy : ReliefSquad)
    {
        if (IsValid(Enemy))
        {
            Enemy->Destroy();
        }
    }
    ReliefSquad.Empty();

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
            {
                Region01->RecordReliefResolved();
            }
        }
    }

    ArenaState = EWyrmBossArenaState::ReliefResolved;
    return true;
}

void AWyrmVerdanceBossArena::SyncArenaStateWithLedger()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UWyrmRegion01Subsystem* Region01 = GI->GetSubsystem<UWyrmRegion01Subsystem>())
            {
                if (Region01->HasFact(FName(TEXT("relief.resolved"))))
                {
                    ArenaState = EWyrmBossArenaState::ReliefResolved;
                }
                else if (Region01->HasFact(FName(TEXT("verdance.bond_accepted"))))
                {
                    ArenaState = EWyrmBossArenaState::CompanionBonded;
                }
                else if (Region01->HasFact(FName(TEXT("verdance.claim_broken"))))
                {
                    ArenaState = EWyrmBossArenaState::ClaimBroken;
                }
                else if (Region01->HasFact(FName(TEXT("verdance.defeated_alive"))))
                {
                    ArenaState = EWyrmBossArenaState::LivingDefeat;
                }
                else
                {
                    ArenaState = EWyrmBossArenaState::Inactive;
                }
            }
        }
    }
}
