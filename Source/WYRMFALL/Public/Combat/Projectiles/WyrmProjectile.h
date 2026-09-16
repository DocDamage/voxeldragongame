#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "WyrmProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UAbilitySystemComponent;

// Authoritative projectile actor for ranged weapons and abilities.
UCLASS()
class WYRMFALL_API AWyrmProjectile : public AActor
{
    GENERATED_BODY()
public:
    AWyrmProjectile();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USphereComponent> CollisionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> MeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float RawDamage = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    FGameplayTag SourceTeamTag;

    UFUNCTION(BlueprintCallable, Category="Combat")
    void InitializeProjectile(AActor* InInstigator, UAbilitySystemComponent* InSourceASC, float InRawDamage, const FVector& ShootDirection);

    UFUNCTION()
    void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
    virtual void BeginPlay() override;
    void ProcessImpact(AActor* HitActor);

private:
    UPROPERTY()
    TWeakObjectPtr<AActor> SourceInstigator;

    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

    bool bHasImpacted = false;
};
