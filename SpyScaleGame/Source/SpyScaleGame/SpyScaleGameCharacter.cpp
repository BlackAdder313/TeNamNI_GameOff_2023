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
#include "Kismet/GameplayStatics.h"
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

void ASpyScaleGameCharacter::RegisterButton(ASSGButton* button, FVector buttonLocation)
{
	ButtonAttributes.Button = MakeWeakObjectPtr<ASSGButton>(button);
	ButtonAttributes.ButtonMeshWorldLocation = buttonLocation;
}

void ASpyScaleGameCharacter::ClearButton()
{
	ButtonAttributes.Reset();
}

bool ASpyScaleGameCharacter::IsInFrustum(AActor* Actor)
{
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr && LocalPlayer->ViewportClient->Viewport)
	{
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
										   LocalPlayer->ViewportClient->Viewport,
										   GetWorld()->Scene,
										   LocalPlayer->ViewportClient->EngineShowFlags).SetRealtimeUpdate(true));

		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, LocalPlayer->ViewportClient->Viewport);
		if (SceneView != nullptr)
		{
			return SceneView->ViewFrustum.IntersectSphere(
				Actor->GetActorLocation(), Actor->GetSimpleCollisionRadius());
		}
	}

	return false;
}

void ASpyScaleGameCharacter::Interact(const FInputActionValue& Value)
{
	if (!ButtonAttributes.Button.IsValid())
	{
		return;
	}
	
	FVector2D screenPosition;
	const auto playerController = GetController<const APlayerController>();
	UGameplayStatics::ProjectWorldToScreen(playerController,
										   ButtonAttributes.ButtonMeshWorldLocation,
										   screenPosition);
	int32 sizeX = 0, sizeY = 0;
	playerController->GetViewportSize(sizeX, sizeY);
	bool isWithinScreen = FMath::IsWithin(screenPosition.X, sizeX * .1f, sizeX * .9f)
						  && FMath::IsWithin(screenPosition.Y, sizeY * .1f, sizeY * .9f);

	auto buttonPtr = ButtonAttributes.Button.Get();
	if (isWithinScreen && playerController->LineOfSightTo(buttonPtr))
	{
		buttonPtr->PressButton();
	}
}

void ASpyScaleGameCharacter::ToggleWatch(const FInputActionValue& Value)
{
	bIsWatchActive = !bIsWatchActive;
}

void ASpyScaleGameCharacter::ScaleHeldObject(const FInputActionValue& Value)
{
	if (ASSGInteractable* Interactable = TraceOutput.Interactable.Get())
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
	TraceOutput = FInteractionTraceOutput();
	
	const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	const FVector TraceEnd = TraceStart + MaxHoldingDistance * FirstPersonCameraComponent->GetForwardVector();

	FCollisionQueryParams TraceParams(TEXT("InteractionTrace"), true);
	
	GetWorld()->LineTraceSingleByChannel(TraceOutput.HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
	TraceOutput.Interactable = Cast<ASSGInteractable>(TraceOutput.HitResult.GetActor());

	LaserHitResult = TraceOutput.HitResult;
	if (LaserHitResult.bBlockingHit)
	{
		LaserEndPointLocation = LaserHitResult.ImpactPoint;
	}
	else
	{
		LaserEndPointLocation = TraceEnd;
	}
}

void ASpyScaleGameCharacter::WatchUpdate(float DeltaTime)
{
	const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	const FVector TraceEnd = TraceStart + MaxHoldingDistance * FirstPersonCameraComponent->GetForwardVector();

	WatchEndPointLocation = TraceEnd;

	if (!bIsWatchActive)
	{
		PhysicsHandleComponent->ReleaseComponent();
		HeldObject.Reset();
		return;
	}

	if (!HeldObject.IsValid())
	{
		HeldObject = TraceOutput.Interactable;
		CurrentHoldingDistance = (MaxHoldingDistance + MinHoldingDistance) * .5f;
		if (ASSGInteractable* HeldObjectPtr = HeldObject.Get())
		{
			PhysicsHandleComponent->GrabComponentAtLocationWithRotation(HeldObject->GetStaticMeshComponent(),
				FName(),
				HeldObject->GetActorLocation(),
				HeldObject->GetActorRotation());
		}
	}

	if (ASSGInteractable* HeldObjectPtr = HeldObject.Get())
	{
		FVector Origin;
		FVector BoxExtent;
		HeldObjectPtr->GetActorBounds(true, Origin, BoxExtent);

		FCollisionQueryParams TraceParams(TEXT("HoldObjectTrace"), true);
		TraceParams.AddIgnoredActor(HeldObjectPtr);
		TraceParams.AddIgnoredActor(this);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, TraceParams);

		const float HeldDistance = FMath::Min(BoxExtent.GetMax() + CurrentHoldingDistance, HitResult.bBlockingHit ? HitResult.Distance : FLT_MAX);
		const FVector RequestedTargetLocation = TraceStart + HeldDistance * FirstPersonCameraComponent->GetForwardVector();
		const FVector TargetLocation = FMath::VInterpTo(HeldObjectPtr->GetActorLocation(), RequestedTargetLocation, DeltaTime, HoldInterpolationSpeed);

		PhysicsHandleComponent->SetTargetLocation(TargetLocation);

		WatchEndPointLocation = HeldObjectPtr->GetActorLocation();
	}
}

void ASpyScaleGameCharacter::Tick(float DeltaTime)
{
	Tick_BP(DeltaTime);
	InteractionTraceUpdate(DeltaTime);
	WatchUpdate(DeltaTime);
}