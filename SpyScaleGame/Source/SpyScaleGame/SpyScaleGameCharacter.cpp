// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpyScaleGameCharacter.h"
#include "SpyScaleGameProjectile.h"
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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASpyScaleGameCharacter

ASpyScaleGameCharacter::ASpyScaleGameCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
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
	m_isHoldingObject = false;
	m_currentHoldingDistance = 0.f;
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
		EnhancedInputComponent->BindAction(HoldObjectAction, ETriggerEvent::Started, this, &ASpyScaleGameCharacter::HoldObject);
		EnhancedInputComponent->BindAction(MoveHeldObjectAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::MoveHeldObject);
		EnhancedInputComponent->BindAction(ScaleHeldObjectAction, ETriggerEvent::Triggered, this, &ASpyScaleGameCharacter::ScaleHeldObject);
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

void ASpyScaleGameCharacter::HoldObject(const FInputActionValue& Value)
{
	if (m_isHoldingObject)
	{
		m_handleComp->ReleaseComponent();
		m_isHoldingObject = false;
		return;
	}

	FHitResult hitResult;
	FCollisionQueryParams traceParams(TEXT("SpyPlayer_Hold_Object"), true);
	FVector rayStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector rayEnd = rayStart + FirstPersonCameraComponent->GetComponentRotation().Vector() * MaxHoldingDistance;
	bool bSuccess = GetWorld()->LineTraceSingleByChannel(hitResult, rayStart, rayEnd, ECC_Visibility, traceParams);
	if (bSuccess)
	{
		m_heldObjectComponent = hitResult.GetComponent();
		m_handleComp->GrabComponentAtLocationWithRotation(m_heldObjectComponent.Get(),
														  FName(),
														  m_heldObjectComponent.Get()->GetComponentLocation(),
														  FRotator());
		m_currentHoldingDistance = FMath::Max(MinHoldingDistance, hitResult.Distance);
		m_isHoldingObject = true;
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
	if (m_isHoldingObject && m_heldObjectComponent.IsValid())
	{
		float scaleDirection = Value.Get<FVector>().X;
		auto heldObjectComponent = m_heldObjectComponent.Get();
		auto currentScale = heldObjectComponent->GetComponentScale();
		
		FVector newScale = ClampVector(currentScale + scaleDirection * ScalingHoldingElementSpeed,
									   MinObjectScale,
									   MaxObjectScale);

		m_heldObjectComponent.Get()->SetWorldScale3D(newScale);
	}
}

void ASpyScaleGameCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool ASpyScaleGameCharacter::GetHasRifle()
{
	return bHasRifle;
}

void ASpyScaleGameCharacter::Tick(float DeltaTime)
{
	if (m_isHoldingObject)
	{
		FVector rayStart = FirstPersonCameraComponent->GetComponentLocation();
		FVector rayEnd = rayStart
						 + FirstPersonCameraComponent->GetComponentRotation().Vector() * m_currentHoldingDistance;
		m_handleComp->SetTargetLocation(rayEnd);
	}
}