// 축영(逐影) - 세이브 포인트 트리거 박스

#include "ChukSavePointTrigger.h"

#include "ChukRelicSubsystem.h"
#include "ChukSaveSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

AChukSavePointTrigger::AChukSavePointTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->InitBoxExtent(FVector(100.f, 100.f, 100.f));
	Box->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = Box;
}

void AChukSavePointTrigger::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &AChukSavePointTrigger::HandleBeginOverlap);
}

void AChukSavePointTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled() || (bTriggerOnce && bAlreadyTriggered))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bRequireAllRelicsFound)
	{
		const UChukRelicSubsystem* Relics = World->GetSubsystem<UChukRelicSubsystem>();
		if (!Relics || Relics->GetFoundCount() < UChukRelicSubsystem::RelicCount)
		{
			return;
		}
	}

	UChukSaveSubsystem* SaveSystem = World->GetSubsystem<UChukSaveSubsystem>();
	if (SaveSystem && SaveSystem->SaveToSavePoint())
	{
		bAlreadyTriggered = true;
		OnSaved();
	}
}
