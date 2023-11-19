// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpyScaleGameCharacter.h"
#include "BlueprintClasses\InteractableObject.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputActionValue.h"
#include "Logging/LogMacros.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "SSGInteractable.h"
#include "SSGButton.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

namespace Internal
{
	const FVector MinScale = FVector(0.5f);
	const FVector MaxScale = FVector(0.5f);
}

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
	m_handleComp = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandler"));
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

	m_movableObject.Reset();
	m_isHoldingObject = false;
	m_isWatchActivated = false;
	m_currentHoldingDistance = 0.f;

	ResetMoveObjectAttributes();
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
		EnhancedInputComponent->BindAction(MoveHeldObjectAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::MoveHeldObject);
		EnhancedInputComponent->BindAction(ScaleHeldObjectAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::ScaleHeldObject);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::Interact);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASpyScaleGameCharacter::UpdateMoveObjectAttributes(const FVector minScale, const FVector maxScale)
{
	MinObjectScale = minScale;
	MaxObjectScale = maxScale;
}

void ASpyScaleGameCharacter::ResetMoveObjectAttributes()
{
	MinObjectScale = Internal::MinScale;
	MaxObjectScale = Internal::MaxScale;
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
	if (m_interactableObject.IsValid())
	{
		m_interactableObject.Get()->OnInteract();
	}
}

void ASpyScaleGameCharacter::ToggleWatch(const FInputActionValue& Value)
{
	// hi-jacking this function...
	if (ASSGButton* Button = TraceOutuput.Button.Get())
	{
		Button->PressButton();
		return;
	}


	m_isWatchActivated = !m_isWatchActivated;
	
	if (m_isHoldingObject)
	{
		m_handleComp->ReleaseComponent();
		m_movableObject.Reset();
		m_movableActor.Reset();
		
		ResetMoveObjectAttributes();
		m_isHoldingObject = false;
		
		return;

		// Kostas - self note: This should also be called if we worked with events
		//OnMoveObjectAttributesChanged.RemoveAll(this);
	}
}

void ASpyScaleGameCharacter::HoldObject()
{
	if (m_movableObject.IsValid())
	{
		auto movableObject = m_movableObject.Get();
		m_handleComp->GrabComponentAtLocationWithRotation(movableObject,
			FName(),
			m_movableObject->GetComponentLocation(),
			FRotator());

		m_isHoldingObject = true;

		if (m_movableActor.IsValid())
		{
			auto movableActor = m_movableActor.Get();
			movableActor->OnInteract();
		}

		// Kostas - self note: Evaluate if it'd be helpful to register to an event like that from the movable / interactable object
		// or keeping it like that - coupled with direct functions - is good enough
		// movableActor->OnMoveObjectAttributesChanged.AddRaw(this, &ASpyScaleGameCharacter::UpdateMoveObjectAttributes);
	}
}

void ASpyScaleGameCharacter::MoveHeldObject(const FInputActionValue& Value)
{
	if (m_isHoldingObject)
	{
		float moveDirection = Value.Get<FVector>().X;
		m_currentHoldingDistance = FMath::Clamp(m_currentHoldingDistance + moveDirection * MovingHoldingElementSpeed,
												MinHoldingDistance,
												MaxHoldingDistance);
	}
}

void ASpyScaleGameCharacter::ScaleHeldObject(const FInputActionValue& Value)
{
	float ScaleDirection = Value.Get<FVector>().X;

	if (m_movableObject.IsValid())
	{
		auto heldObjectComponent = m_movableObject.Get();
		auto currentScale = heldObjectComponent->GetComponentScale();
		
		FVector newScale = ClampVector(currentScale + ScaleDirection * ScalingHoldingElementSpeed,
									   MinObjectScale,
									   MaxObjectScale);

		heldObjectComponent->SetWorldScale3D(newScale);
	}

	if (ASSGInteractable* Interactable = TraceOutuput.Interactable.Get())
	{
		Interactable->AdjustScale(ScaleDirection);
	}
}

void ASpyScaleGameCharacter::InteractionTraceUpdate(float DeltaTime)
{
	TraceOutuput = FInteractionTraceOutput();
	
	const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	const FVector TraceEnd = TraceStart + MaxHoldingDistance * FirstPersonCameraComponent->GetForwardVector();

	FCollisionQueryParams TraceParams(TEXT("SpyPlayer_Hold_Object"), true);
	
	GetWorld()->LineTraceSingleByChannel(TraceOutuput.HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
	TraceOutuput.Interactable = Cast<ASSGInteractable>(TraceOutuput.HitResult.GetActor());
	TraceOutuput.Button = Cast<ASSGButton>(TraceOutuput.HitResult.GetActor());
}

void ASpyScaleGameCharacter::WatchUpdate(float DeltaTime)
{
	if (!m_isWatchActivated)
	{
		m_handleComp->ReleaseComponent();
		HeldObject.Reset();
		return;
	}

	if (!HeldObject.IsValid())
	{
		HeldObject = TraceOutuput.Interactable;

		if (ASSGInteractable* HeldObjectPtr = HeldObject.Get())
		{
			m_handleComp->GrabComponentAtLocationWithRotation(HeldObject->GetStaticMesh(),
				FName(),
				HeldObject->GetActorLocation(),
				HeldObject->GetActorRotation());
		}
	}

	if (ASSGInteractable* HeldObjectPtr = HeldObject.Get())
	{
		const FVector Start = FirstPersonCameraComponent->GetComponentLocation();

		FVector Origin;
		FVector BoxExtent;
		HeldObjectPtr->GetActorBounds(true, Origin, BoxExtent);

		const float HeldDistance = BoxExtent.GetMax() + MinHoldingDistance;
		const FVector TargetLocation = Start + HeldDistance * FirstPersonCameraComponent->GetForwardVector();

		m_handleComp->SetTargetLocation(TargetLocation);
	}
}

void ASpyScaleGameCharacter::Tick(float DeltaTime)
{
	Tick_BP(DeltaTime);
	InteractionTraceUpdate(DeltaTime);
	WatchUpdate(DeltaTime);

	FVector rayStart = FirstPersonCameraComponent->GetComponentLocation();
	auto rayEnd = [&rayStart, direction = FirstPersonCameraComponent->GetComponentRotation().Vector()](const float distanceFactor)
				  {
				      return rayStart + direction * distanceFactor;
				  };
	
	// Update position of held object
	if (m_isHoldingObject)
	{
		m_handleComp->SetTargetLocation(rayEnd(m_currentHoldingDistance));
		return;
	}

	// Try raycast for movable
	FHitResult hitResult;
	FCollisionQueryParams traceParams(TEXT("SpyPlayer_Hold_Object"), true);
	bool bSuccess = GetWorld()->LineTraceSingleByChannel(hitResult, rayStart, rayEnd(MaxHoldingDistance), ECC_Visibility, traceParams);
	if (!bSuccess)
	{
		m_movableObject.Reset();
		m_movableActor.Reset();
		return;
	}
	
	// Check if hit object is a MovableObject
	auto movableActor = Cast<AMovableObject>(hitResult.GetActor());
	if (!movableActor)
	{
		return;
	}
	
	// If there's no existing cached movableObject or we're pointing to a different one,
	// update cache
	if (!m_movableActor.IsValid() || m_movableActor.Get() != movableActor)
	{
		m_movableActor = MakeWeakObjectPtr(movableActor);
		m_movableObject = hitResult.GetComponent();
		m_currentHoldingDistance = FMath::Max(MinHoldingDistance, hitResult.Distance);

		UpdateMoveObjectAttributes(movableActor->MinObjectScale, movableActor->MaxObjectScale);
	}

	if(m_isWatchActivated && !m_isHoldingObject)
	{
		HoldObject();
	}
}

void ASpyScaleGameCharacter::RegisterInteractElement(AInteractableObject* interactableObject)
{
	if (!m_interactableObject.IsValid() || m_interactableObject.Get() != interactableObject)
	{
		m_interactableObject = MakeWeakObjectPtr<AInteractableObject>(interactableObject);
	}
}

void ASpyScaleGameCharacter::UnregisterInteractElement(AInteractableObject* interactableObject)
{
	if (m_interactableObject.IsValid() && m_interactableObject.Get() == interactableObject)
	{
		m_interactableObject.Reset();
	}
}