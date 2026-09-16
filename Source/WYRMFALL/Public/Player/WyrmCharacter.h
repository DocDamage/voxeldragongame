#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "WyrmCharacter.generated.h"
class UAbilitySystemComponent;
class UWyrmAttributeSet;
class USpringArmComponent;
class UCameraComponent;
class UCustomizableSkeletalComponent;
class UCustomizableObject;
class UCustomizableObjectInstance;

UENUM(BlueprintType)
enum class EWyrmCameraMode : uint8 { ThirdPerson, TopDown };

// Humanoid host with Mutable visual authority.
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

    // --- Mutable Appearance Authority (WP-02) ---
    UFUNCTION(BlueprintPure, Category="Appearance")
    UCustomizableSkeletalComponent* GetCustomizableComponent() const { return CustomizableSkeletalComponent; }

    UFUNCTION(BlueprintPure, Category="Appearance")
    UCustomizableObjectInstance* GetCustomizableInstance() const { return CustomizableInstance; }

    UFUNCTION(BlueprintCallable, Category="Appearance")
    bool SetCustomizableObject(UCustomizableObject* InCustomizableObject);

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetCustomizableInstance(UCustomizableObjectInstance* InInstance);

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetColorParameter(FName ParamName, FLinearColor Color);

    UFUNCTION(BlueprintPure, Category="Appearance")
    FLinearColor GetColorParameter(FName ParamName) const;

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetOptionParameter(FName ParamName, const FString& OptionName);

    UFUNCTION(BlueprintPure, Category="Appearance")
    FString GetOptionParameter(FName ParamName) const;

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void SetFloatParameter(FName ParamName, float Value);

    UFUNCTION(BlueprintPure, Category="Appearance")
    float GetFloatParameter(FName ParamName) const;

    UFUNCTION(BlueprintCallable, Category="Appearance")
    void ApplyAppearance(bool bAsync = true);

    UFUNCTION(BlueprintCallable, Category="Appearance")
    FString CaptureAppearanceDescriptor() const;

    UFUNCTION(BlueprintCallable, Category="Appearance")
    bool RestoreAppearanceDescriptor(const FString& InDescriptor);

    // --- Equipment Attachment (CHAR-04) ---
    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool AttachEquipmentMesh(USceneComponent* ItemMesh, FName SocketName);

    UFUNCTION(BlueprintPure, Category="Equipment")
    bool IsSocketValid(FName SocketName) const;

protected:
    virtual void BeginPlay() override;
    UPROPERTY(VisibleAnywhere, Category="Combat") TObjectPtr<UAbilitySystemComponent> AbilitySystem;
    UPROPERTY(VisibleAnywhere, Category="Combat") TObjectPtr<UWyrmAttributeSet> Attributes;
    UPROPERTY(VisibleAnywhere, Category="Camera") TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, Category="Camera") TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Appearance")
    TObjectPtr<UCustomizableSkeletalComponent> CustomizableSkeletalComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
    TObjectPtr<UCustomizableObjectInstance> CustomizableInstance;

private:
    void ApplyCamera();
    UPROPERTY(VisibleAnywhere, Category="Camera") EWyrmCameraMode CameraMode = EWyrmCameraMode::ThirdPerson;
};

