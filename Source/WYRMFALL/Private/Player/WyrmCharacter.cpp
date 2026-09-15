#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/WyrmAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"

AWyrmCharacter::AWyrmCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 450.f; // development-only tuning
    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    Attributes = CreateDefaultSubobject<UWyrmAttributeSet>(TEXT("Attributes"));
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->bDoCollisionTest = true;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    ApplyCamera();
}
void AWyrmCharacter::BeginPlay()
{
    Super::BeginPlay();
    AbilitySystem->InitAbilityActorInfo(this, this);
    // Unpossessing this body must not reset GAS state when dragon control is added.
    ApplyCamera();
}
UAbilitySystemComponent* AWyrmCharacter::GetAbilitySystemComponent() const { return AbilitySystem; }
void AWyrmCharacter::ToggleCamera()
{
    CameraMode = CameraMode == EWyrmCameraMode::ThirdPerson ? EWyrmCameraMode::TopDown : EWyrmCameraMode::ThirdPerson;
    ApplyCamera();
}
void AWyrmCharacter::ApplyCamera()
{
    const bool bTop = CameraMode == EWyrmCameraMode::TopDown;
    CameraBoom->TargetArmLength = bTop ? 1100.f : 450.f;
    CameraBoom->bUsePawnControlRotation = !bTop;
    CameraBoom->SetWorldRotation(bTop ? FRotator(-60.f, 0.f, 0.f) : FRotator(-18.f, 0.f, 0.f));
    Camera->FieldOfView = bTop ? 55.f : 80.f;
}
void AWyrmCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
#if !UE_BUILD_SHIPPING
    // Wireframe diagnostic, not replacement character artwork.
    if (!GetMesh()->GetSkeletalMeshAsset())
    {
        DrawDebugCapsule(GetWorld(), GetActorLocation(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
            GetCapsuleComponent()->GetScaledCapsuleRadius(), GetActorQuat(), FColor::Cyan, false, -1.f, 0, 2.f);
    }
#endif
}
