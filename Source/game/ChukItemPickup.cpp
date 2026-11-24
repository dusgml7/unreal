// 축영(逐影) - 줍는 아이템 액터

#include "ChukItemPickup.h"

#include "ChukInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AChukItemPickup::AChukItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	// 상호작용 트레이스(Visibility)만 막고 나머지는 통과 → 플레이어가 걸리지 않음
	InteractCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractCollision"));
	InteractCollision->InitSphereRadius(40.f);
	InteractCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = InteractCollision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(InteractCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AChukItemPickup::Interact_Implementation(AActor* InteractingActor)
{
	UChukInventoryComponent* Inventory = InteractingActor ? InteractingActor->FindComponentByClass<UChukInventoryComponent>() : nullptr;
	if (!Inventory || !Inventory->AcquireItem(ItemId))
	{
		return;
	}

	OnPickedUp(InteractingActor);
	if (bDestroyOnPickup)
	{
		Destroy();
	}
}
