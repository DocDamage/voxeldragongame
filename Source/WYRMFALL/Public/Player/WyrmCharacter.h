#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "WyrmCharacter.generated.h"
class UAbilitySystemComponent;
class UWyrmAttributeSet;
class USpringArmComponent;
class UCameraComponent;

UENUM(BlueprintType)
enum class EWyrmCameraMode : uint8 { ThirdPerson, TopDown };

// Diagnostic humanoid host. Mesh deliberately unassigned until real Mutable integration.
UCLASS()
class WYRMFALL_API AWyrmCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AWyrmCharacter();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintCallable, Category="Camera") void ToggleCamera();
    UFUNCTION(BlueprintPure, Category="Camera") EWyrmCameraMode GetCameraMode() const { return CameraMode; }
    UFUNCTION(BlueprintPure, Category="Combat") UWyrmAttributeSet* GetAttributes() const { return Attributes; }
protected:
    virtual void BeginPlay() override;
    UPROPERTY(VisibleAnywhere, Category="Combat") TObjectPtr<UAbilitySystemComponent> AbilitySystem;
    UPROPERTY(VisibleAnywhere, Category="Combat") TObjectPtr<UWyrmAttributeSet> Attributes;
    UPROPERTY(VisibleAnywhere, Category="Camera") TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, Category="Camera") TObjectPtr<UCameraComponent> Camera;
private:
    void ApplyCamera();
    UPROPERTY(VisibleAnywhere, Category="Camera") EWyrmCameraMode CameraMode = EWyrmCameraMode::ThirdPerson;
};
