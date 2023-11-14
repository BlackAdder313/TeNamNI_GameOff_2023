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
	if (OtherActor && (OtherActor != this)) {
		UE_LOG(LogTemp, Warning, TEXT("OnOverlapBegin"));
	}
}

void AInteractableObject::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
									   AActor* OtherActor,
									   UPrimitiveComponent* OtherComp,
									   int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this)) {
		UE_LOG(LogTemp, Warning, TEXT("OnOverlapEnd"));

	}
}