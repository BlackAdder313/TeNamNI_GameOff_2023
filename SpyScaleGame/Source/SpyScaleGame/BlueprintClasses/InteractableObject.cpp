// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractableObject.h"
#include "Components/BoxComponent.h"

AInteractableObject::AInteractableObject()
{
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetCollisionProfileName("OverlapAllDynamic");
}

void AInteractableObject::PostActorCreated()
{
	Super::PostActorCreated();
	SetupTriggerVolume();
}

//void AInteractableObject::PostLoad()
//{
//	Super::PostLoad();
//	SetupTriggerVolume();
//}

void AInteractableObject::BeginPlay() {}

void AInteractableObject::SetupTriggerVolume()
{
	if (const auto& staticMesh = GetStaticMeshComponent()->GetStaticMesh())
	{
		const auto& staticMeshComponent = GetStaticMeshComponent();
		TriggerVolume->AttachToComponent(staticMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		
		const FBoxSphereBounds boundingBox = staticMesh->GetBoundingBox();
		const FVector meshCollisionExtent = staticMeshComponent->GetCollisionShape().GetExtent();

		TriggerVolume->SetRelativeLocation(FVector(0.f, 0.f, -boundingBox.BoxExtent.Z));
		TriggerVolume->SetBoxExtent(boundingBox.BoxExtent * 1.1f, true);

		// Ensure delegate is bound (just once)
		TriggerVolume->OnComponentBeginOverlap.RemoveAll(this);
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AInteractableObject::OnOverlapBegin);
		TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractableObject::OnOverlapEnd);
	}
}

void AInteractableObject::OnInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("On interact"));

	// Call Blueprint behaviour
	OnInteract_BP();
}

void AInteractableObject::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
										 AActor* OtherActor,
										 UPrimitiveComponent* OtherComp,
										 int32 OtherBodyIndex,
										 bool bFromSweep,
										 const FHitResult& SweepResult)
{
	if (ObjectInteractionType == EObjectInteractionType::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("AInteractableObject::OnOverlapBegin -- Missing Interaction Type"));
		return;
	}

	if (OtherActor && (OtherActor != this)) {
		switch (ObjectInteractionType)
		{
		case EObjectInteractionType::Press:
			// TODO: Check if the entered actor is of SpyScaleGameCharacter
			// If so, register this into the Character actor, so that
			// when IA_Interact is pressed, the OnInteract function of this object will be called
			break;

		case EObjectInteractionType::Proximity:
			// TODO: Call the OnTriggerEnabled function on the ObjectToNotify
			// and keep reference of actor the enabled the trigger
			break;
		case EObjectInteractionType::Weight:
			// TODO: Get static mesh of actor and get mass
			// If mass above TriggerEnableMassThreshold,
			// Call the OnTriggerEnabled function on the ObjectToNotify
			// and keep reference of actor the enabled the trigger
		case EObjectInteractionType::None:
			// Handled at start of function.
			// Doing this instead of using default,
			// so that if a new interaction type is added,
			// we are obliged by the compiler to take care of it
			return;
		}
	}
}

void AInteractableObject::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
									   AActor* OtherActor,
									   UPrimitiveComponent* OtherComp,
									   int32 OtherBodyIndex)
{
	if (ObjectInteractionType == EObjectInteractionType::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("AInteractableObject::OnOverlapEnd -- Missing Interaction Type"));
		return;
	}

	if (OtherActor && (OtherActor != this)) {
		switch (ObjectInteractionType)
		{
		case EObjectInteractionType::Press:
			// TODO: Unregister self from SpyScaleGameCharacter,
			// if actor is the playercharacter
			break;

		case EObjectInteractionType::Proximity:
			// TODO: Call the OnTriggerDisabled function on the ObjectToNotify
			// if is the actor that enabled the trigger
			break;
		case EObjectInteractionType::Weight:
			// TODO: Call the OnTriggerDisabled function on the ObjectToNotify
			// if is the actor that enabled the trigger
		case EObjectInteractionType::None:
			// Handled at start of function.
			// Doing this instead of using default,
			// so that if a new interaction type is added,
			// we are obliged by the compiler to take care of it
			return;
		}
	}
}