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
	void MoveHeldObject(const FInputActionValue& Value);
	
	void InteractionTraceUpdate(float DeltaTime);
	void WatchUpdate(float DeltaTime);

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

private:
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleWatchAction;

	// todo: current not being used? do we need this?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveHeldObjectAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ScaleHeldObjectAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float MaxHoldingDistance = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float MinHoldingDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float MovingHoldingElementSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float HoldInterpolationSpeed = 20.f;

	/* Physics Handle */
	class UPhysicsHandleComponent* PhysicsHandleComponent = nullptr;

	bool bIsWatchActive = false;

	struct FInteractionTraceOutput
	{
		FHitResult HitResult;
		TWeakObjectPtr<ASSGInteractable> Interactable;
		TWeakObjectPtr<ASSGButton> Button;
	};

	FInteractionTraceOutput TraceOutuput;
	TWeakObjectPtr<ASSGInteractable> HeldObject;
	float CurrentHoldingDistance = 0.f;
};