// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SpyScaleGameCharacter.generated.h"

namespace ObjectTypeGameplayTags
{
	extern const FName MoveableObjectTypeGameplayTag;
}

class AInteractableObject;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class ASSGButton;
class ASSGInteractable;

struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EObjectAdjustmentMode
{
	Position,
	Yaw,
	Pitch,
};

UCLASS(config=Game)
class ASpyScaleGameCharacter : public ACharacter
{
public:
	GENERATED_BODY()

	ASpyScaleGameCharacter();

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Interact")
	void RegisterButton(ASSGButton* button, FVector buttonLocation);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Interact")
	void ClearButton();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	// Called when player is trying to interact with this object
	UFUNCTION(BlueprintImplementableEvent, Category = "Actor BP Extensions")
	void Tick_BP(float DeltaTime);
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for looking input */
	void Interact(const FInputActionValue& Value);

	/** Called for pulling / pushing an object */
	void ToggleWatch(const FInputActionValue& Value);

	/** Called for scaling an object */
	void ScaleHeldObject(const FInputActionValue& Value);

	/** Called for pulling / pushing an object */
	void AdjustHeldObject(const FInputActionValue& Value);
	
	void ChangeAdjustmentMode(const FInputActionValue& Value, EObjectAdjustmentMode NewMode);

	void InteractionTraceUpdate(float DeltaTime);
	void WatchUpdate(float DeltaTime);

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

protected:
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleWatchAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveHeldObjectAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ScaleHeldObjectAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SelectPositionMode = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SelectYawMode = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SelectPitchMode = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float MaxHoldingDistance = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float MinHoldingDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float MovingHoldingElementSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float HoldInterpolationSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float ObjectRotationSpeed = 20.f;

	UPROPERTY(BlueprintReadOnly)
	EObjectAdjustmentMode AdjustmentMode = EObjectAdjustmentMode::Position;

	UPROPERTY(BlueprintReadOnly)
	FVector WatchEndPointLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector LaserEndPointLocation;

	UPROPERTY(BlueprintReadOnly)
	FHitResult LaserHitResult;

	/* Physics Handle */
	class UPhysicsHandleComponent* PhysicsHandleComponent = nullptr;

	bool bIsWatchActive = false;

	struct FInteractionTraceOutput
	{
		FHitResult HitResult;
		TWeakObjectPtr<ASSGInteractable> Interactable;
	};

	struct FButtonAttributes
	{
		FVector ButtonMeshWorldLocation;
		TWeakObjectPtr<ASSGButton> Button;

		void Reset()
		{
			ButtonMeshWorldLocation = FVector::ZeroVector;
			Button.Reset();
		}
	};

	FInteractionTraceOutput TraceOutput;
	FButtonAttributes ButtonAttributes;
	TWeakObjectPtr<ASSGInteractable> HeldObject;
	float CurrentHoldingDistance = 0.f;
};