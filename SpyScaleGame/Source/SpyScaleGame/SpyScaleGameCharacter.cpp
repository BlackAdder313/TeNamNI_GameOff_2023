// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpyScaleGameCharacter.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Logging/LogMacros.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "SSGButton.h"
#include "SSGInteractable.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASpyScaleGameCharacter

ASpyScaleGameCharacter::ASpyScaleGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Set up physics handler
	PhysicsHandleComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandler"));
}

void ASpyScaleGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////// Input

void ASpyScaleGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::Look);

		// Object manipulation
		EnhancedInputComponent->BindAction(ToggleWatchAction, ETriggerEvent::Started, this, &ASpyScaleGameCharacter::ToggleWatch);
		EnhancedInputComponent->BindAction(ScaleHeldObjectAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::ScaleHeldObject);
		EnhancedInputComponent->BindAction(MoveHeldObjectAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::AdjustHeldObject);

		EnhancedInputComponent->BindAction(SelectPositionMode, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::ChangeAdjustmentMode, EObjectAdjustmentMode::Position);
		EnhancedInputComponent->BindAction(SelectYawMode, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::ChangeAdjustmentMode, EObjectAdjustmentMode::Yaw);
		EnhancedInputComponent->BindAction(SelectPitchMode, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::ChangeAdjustmentMode, EObjectAdjustmentMode::Pitch);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::Interact);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASpyScaleGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ASpyScaleGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASpyScaleGameCharacter::Interact(const FInputActionValue& Value)
{
	if (ASSGButton* Button = TraceOutuput.Button.Get())
	{
		Button->PressButton();
	}
}

void ASpyScaleGameCharacter::ToggleWatch(const FInputActionValue& Value)
{
	bIsWatchActive = !bIsWatchActive;
}

void ASpyScaleGameCharacter::ScaleHeldObject(const FInputActionValue& Value)
{
	if (ASSGInteractable* Interactable = TraceOutuput.Interactable.Get())
	{
		const float ScaleDirection = Value.Get<FVector>().X;
		Interactable->AdjustScale(ScaleDirection);
	}
}

void ASpyScaleGameCharacter::ChangeAdjustmentMode(const FInputActionValue& Value, EObjectAdjustmentMode NewMode)
{
	AdjustmentMode = NewMode;
}

void ASpyScaleGameCharacter::AdjustHeldObject(const FInputActionValue& Value)
{
	if (const ASSGInteractable* HeldObjectPtr = HeldObject.Get())
	{
		const float MoveDirection = Value.Get<FVector>().X;
		const FQuat ObjectRotation = HeldObjectPtr->GetStaticMeshComponent()->GetComponentQuat();

		switch (AdjustmentMode)
		{
			case EObjectAdjustmentMode::Position:
			{
				CurrentHoldingDistance = FMath::Clamp(CurrentHoldingDistance + MovingHoldingElementSpeed * MoveDirection,
					MinHoldingDistance,
					MaxHoldingDistance);
				break;
			}
			case EObjectAdjustmentMode::Yaw:
			{
				const FQuat NewRotation = ObjectRotation * FQuat(FVector::UpVector, FMath::DegreesToRadians(ObjectRotationSpeed * MoveDirection));
				PhysicsHandleComponent->SetTargetRotation(NewRotation.Rotator());
				break;
			}
			case EObjectAdjustmentMode::Pitch:
			{
				const FQuat NewRotation = ObjectRotation * FQuat(FVector::RightVector, FMath::DegreesToRadians(ObjectRotationSpeed * MoveDirection));
				PhysicsHandleComponent->SetTargetRotation(NewRotation.Rotator());
				break;
			}
		}
	}
}

void ASpyScaleGameCharacter::InteractionTraceUpdate(float DeltaTime)
{
	TraceOutuput = FInteractionTraceOutput();
	
	const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	const FVector TraceEnd = TraceStart + MaxHoldingDistance * FirstPersonCameraComponent->GetForwardVector();

	FCollisionQueryParams TraceParams(TEXT("InteractionTrace"), true);
	
	GetWorld()->LineTraceSingleByChannel(TraceOutuput.HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
	TraceOutuput.Interactable = Cast<ASSGInteractable>(TraceOutuput.HitResult.GetActor());
	TraceOutuput.Button = Cast<ASSGButton>(TraceOutuput.HitResult.GetActor());
}

void ASpyScaleGameCharacter::WatchUpdate(float DeltaTime)
{
	if (!bIsWatchActive)
	{
		PhysicsHandleComponent->ReleaseComponent();
		HeldObject.Reset();
		return;
	}

	if (!HeldObject.IsValid())
	{
		HeldObject = TraceOutuput.Interactable;
		CurrentHoldingDistance = (MaxHoldingDistance + MinHoldingDistance) * .5f;
		if (ASSGInteractable* HeldObjectPtr = HeldObject.Get())
		{
			PhysicsHandleComponent->GrabComponentAtLocationWithRotation(HeldObject->GetStaticMeshComponent(),
				FName(),
				HeldObject->GetActorLocation(),
				HeldObject->GetActorRotation());

			OnObjectHold_BP();
		}
	}

	if (const ASSGInteractable* HeldObjectPtr = HeldObject.Get())
	{
		const FVector Start = FirstPersonCameraComponent->GetComponentLocation();

		FVector Origin;
		FVector BoxExtent;
		HeldObjectPtr->GetActorBounds(true, Origin, BoxExtent);

		FCollisionQueryParams TraceParams(TEXT("HoldObjectTrace"), true);
		TraceParams.AddIgnoredActor(HeldObjectPtr);
		TraceParams.AddIgnoredActor(this);

		FHitResult HitResult;
		const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
		const FVector TraceEnd = TraceStart + MaxHoldingDistance * FirstPersonCameraComponent->GetForwardVector();
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, TraceParams);

		const float HeldDistance = FMath::Min(BoxExtent.GetMax() + CurrentHoldingDistance, HitResult.bBlockingHit ? HitResult.Distance : FLT_MAX);
		const FVector RequestedTargetLocation = Start + HeldDistance * FirstPersonCameraComponent->GetForwardVector();
		const FVector TargetLocation = FMath::VInterpTo(HeldObjectPtr->GetActorLocation(), RequestedTargetLocation, DeltaTime, HoldInterpolationSpeed);

		PhysicsHandleComponent->SetTargetLocation(TargetLocation);
	}
}

void ASpyScaleGameCharacter::Tick(float DeltaTime)
{
	Tick_BP(DeltaTime);
	InteractionTraceUpdate(DeltaTime);
	WatchUpdate(DeltaTime);
}