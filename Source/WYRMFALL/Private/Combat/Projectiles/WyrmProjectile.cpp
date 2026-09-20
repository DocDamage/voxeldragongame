#include "Combat/Projectiles/WyrmProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "UObject/ConstructorHelpers.h"

AWyrmProjectile::AWyrmProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionComp->SetGenerateOverlapEvents(true);
    RootComponent = CollisionComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshFinder(TEXT("/Game/WYRMFALL/Items/Weapons/SM_Arrow"));
    if (ArrowMeshFinder.Succeeded())
    {
        MeshComp->SetStaticMesh(ArrowMeshFinder.Object);
        MeshComp->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 3500.f;
    ProjectileMovement->MaxSpeed = 3500.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.05f;

    InitialLifeSpan = 5.0f;

    SourceTeamTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Combat.Team.Player")), false);
}

void AWyrmProjectile::BeginPlay()
{
    Super::BeginPlay();

    CollisionComp->OnComponentHit.AddDynamic(this, &AWyrmProjectile::OnProjectileHit);
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AWyrmProjectile::OnProjectileOverlap);
}

void AWyrmProjectile::InitializeProjectile(AActor* InInstigator, UAbilitySystemComponent* InSourceASC, float InRawDamage, const FVector& ShootDirection, bool bInEligibleBasicWeaponHit)
{
    SourceInstigator = InInstigator;
    SourceASC = InSourceASC;
    RawDamage = InRawDamage;
    bEligibleBasicWeaponHit = bInEligibleBasicWeaponHit;

    if (CollisionComp && InInstigator)
    {
        CollisionComp->IgnoreActorWhenMoving(InInstigator, true);
    }

    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = ShootDirection.GetSafeNormal() * ProjectileMovement->InitialSpeed;
    }
}

void AWyrmProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    ProcessImpact(OtherActor);
}

void AWyrmProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ProcessImpact(OtherActor);
}

void AWyrmProjectile::ProcessImpact(AActor* HitActor)
{
    if (bHasImpacted || !HitActor || HitActor == this || HitActor == SourceInstigator.Get())
    {
        return;
    }

    bHasImpacted = true;

    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor);
    UAbilitySystemComponent* TargetASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;

    if (TargetASC && SourceASC.IsValid())
    {
        // Friendly fire protection (COM-02)
        if (SourceTeamTag.IsValid() &&
            SourceASC->HasMatchingGameplayTag(SourceTeamTag) &&
            TargetASC->HasMatchingGameplayTag(SourceTeamTag))
        {
            Destroy();
            return;
        }

        if (bEligibleBasicWeaponHit)
        {
            UWyrmMeleeAttackAbility::ApplyEligibleWeaponDamageEffect(SourceASC.Get(), TargetASC, RawDamage);
        }
        else
        {
            UWyrmMeleeAttackAbility::ApplyDamageEffect(SourceASC.Get(), TargetASC, RawDamage);
        }
    }

    Destroy();
}
