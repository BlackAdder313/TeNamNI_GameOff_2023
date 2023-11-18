// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractableObject.h"
#include "Components/BoxComponent.h"
#include "../SpyScaleGameCharacter.h"

AInteractableObject::AInteractableObject()
{
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
}

void AInteractableObject::PostActorCreated()
{
	TrySetupTriggerVolume();

	Super::PostActorCreated();
}

//void AInteractableObject::PostLoad()
//{
//	Super::PostLoad();
//	SetupTriggerVolume();
//}

void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();
	
	m_triggerActivators.Empty();
}

void AInteractableObject::TrySetupTriggerVolume()
{
	if (TriggerVolume && TriggerVolume->GetCollisionProfileName() == "OverlapAllDynamic")
	{
		return;
	}
	
	if (const auto& staticMesh = GetStaticMeshComponent()->GetStaticMesh())
	{
		TriggerVolume->SetCollisionProfileName("OverlapAllDynamic");
		
		const auto& staticMeshComponent = GetStaticMeshComponent();
		TriggerVolume->AttachToComponent(staticMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		
		const FBoxSphereBounds boundingBox = staticMesh->GetBoundingBox();
		const FVector meshCollisionExtent = staticMeshComponent->GetCollisionShape().GetExtent();

		//TriggerVolume->SetRelativeLocation(FVector(0.f, 0.f, -boundingBox.BoxExtent.Z));
		TriggerVolume->SetBoxExtent(boundingBox.BoxExtent * 1.2f, true);

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
			{
				if (ASpyScaleGameCharacter* player = Cast<ASpyScaleGameCharacter>(OtherActor))
				{
					player->RegisterInteractElement(this);
				}
				break;
			}
			case EObjectInteractionType::Proximity:
				if (!ObjectToNotify)
				{
					UE_LOG(LogTemp, Warning, TEXT("AInteractableObject::OnOverlapBegin -- No Object to notify is set"));
					return;
				}

				ObjectToNotify->OnTriggerEnabled();
				m_triggerActivators.Add(OtherActor);

				break;
			case EObjectInteractionType::Weight:
			{
				if (!OtherComp)
				{
					return;
				}

				m_triggerActivators.Add(OtherActor);

				float totalMass = 0.f;
				for (auto triggerActivator : m_triggerActivators)
				{
					totalMass += Cast<UPrimitiveComponent>(triggerActivator->GetRootComponent())->GetMass();
				}

				if (totalMass >= TriggerEnableMassThreshold)
				{
					ObjectToNotify->OnTriggerEnabled();

				}

				break;
			}
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
				if (ASpyScaleGameCharacter* player = Cast<ASpyScaleGameCharacter>(OtherActor))
				{
					player->UnregisterInteractElement(this);
				}
				break;

			case EObjectInteractionType::Proximity:
				if (m_triggerActivators.Contains(OtherActor))
				{
					m_triggerActivators.Remove(OtherActor);
					if (m_triggerActivators.IsEmpty())
					{
						ObjectToNotify->OnTriggerDisabled();
					}
				}
			
				break;
			case EObjectInteractionType::Weight:
				if (!OtherComp)
				{
					return;
				}

				if (m_triggerActivators.Contains(OtherActor))
				{
					m_triggerActivators.Remove(OtherActor);
				
					float totalMass = 0.f;
					for (auto triggerActivator : m_triggerActivators)
					{
						totalMass += Cast<UPrimitiveComponent>(triggerActivator->GetRootComponent())->GetMass();
					}

					if (totalMass < TriggerEnableMassThreshold)
					{
						ObjectToNotify->OnTriggerDisabled();
					}
				
					break;
				}

			case EObjectInteractionType::None:
				// Handled at start of function.
				// Doing this instead of using default,
				// so that if a new interaction type is added,
				// we are obliged by the compiler to take care of it
				return;
		}
	}
}