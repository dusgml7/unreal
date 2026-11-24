// 축영(逐影) - 아이템 랜덤 생성 지점 / 생성기

#include "ChukItemSpawnPoint.h"

#include "ChukItemPickup.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// ===================== AChukItemSpawnPoint =====================

AChukItemSpawnPoint::AChukItemSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA
	Sprite = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (Sprite)
	{
		Sprite->SetupAttachment(RootComponent);
	}
	Arrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (Arrow)
	{
		Arrow->SetupAttachment(RootComponent);
	}
#endif
}

bool AChukItemSpawnPoint::AllowsClass(TSubclassOf<AChukItemPickup> PickupClass) const
{
	if (AllowedPickupClasses.Num() == 0)
	{
		return true;
	}
	for (const TSubclassOf<AChukItemPickup>& Allowed : AllowedPickupClasses)
	{
		if (Allowed && PickupClass && PickupClass->IsChildOf(Allowed))
		{
			return true;
		}
	}
	return false;
}

// ===================== AChukItemSpawner =====================

AChukItemSpawner::AChukItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA
	Sprite = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (Sprite)
	{
		Sprite->SetupAttachment(RootComponent);
	}
#endif
}

void AChukItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (bSpawnOnBeginPlay)
	{
		SpawnItems();
	}
}

TArray<AChukItemPickup*> AChukItemSpawner::SpawnItems()
{
	TArray<AChukItemPickup*> Spawned;
	UWorld* World = GetWorld();
	if (!World)
	{
		return Spawned;
	}

	// 사용 가능한 지점 모으고 섞기
	TArray<AChukItemSpawnPoint*> FreePoints;
	for (TActorIterator<AChukItemSpawnPoint> It(World); It; ++It)
	{
		FreePoints.Add(*It);
	}
	for (int32 i = FreePoints.Num() - 1; i > 0; --i)
	{
		FreePoints.Swap(i, FMath::RandRange(0, i));
	}

	for (const FChukSpawnEntry& Entry : SpawnEntries)
	{
		if (!Entry.PickupClass)
		{
			continue;
		}
		for (int32 n = 0; n < Entry.Count; ++n)
		{
			const int32 PointIndex = FreePoints.IndexOfByPredicate(
				[&Entry](const AChukItemSpawnPoint* P) { return P->AllowsClass(Entry.PickupClass); });
			if (PointIndex == INDEX_NONE)
			{
				UE_LOG(LogTemp, Warning, TEXT("[ChukItemSpawner] Not enough spawn points for %s (%d/%d spawned)"),
					*GetNameSafe(Entry.PickupClass), n, Entry.Count);
				break;
			}

			AChukItemSpawnPoint* Point = FreePoints[PointIndex];
			FreePoints.RemoveAt(PointIndex);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AChukItemPickup* Pickup = World->SpawnActor<AChukItemPickup>(Entry.PickupClass, Point->GetActorTransform(), Params))
			{
				Spawned.Add(Pickup);
			}
		}
	}
	return Spawned;
}
