// 축영(逐影) - 아이템 랜덤 생성 지점 / 생성기

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChukItemSpawnPoint.generated.h"

class AChukItemPickup;
class UArrowComponent;
class UBillboardComponent;

/**
 * 아이템이 생길 수 있는 자리 표시. 레벨에 여러 개 배치만 하면 된다 (게임 중에는 안 보임).
 * 실제 생성은 레벨에 하나 놓는 AChukItemSpawner 가 한다.
 *
 * ■ 사용법: Place Actors 패널에서 "Chuk Item Spawn Point" 검색 → 궁 안 여기저기에 끌어다 놓기.
 *   화살표 방향 = 생성될 아이템의 방향.
 */
UCLASS()
class GAME_API AChukItemSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AChukItemSpawnPoint();

	/** 이 지점에 생성될 수 있는 아이템 종류. 비워 두면 아무 아이템이나 가능 */
	// TODO(기획 확인): 위치별로 나올 수 있는 아이템을 제한할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Spawn")
	TArray<TSubclassOf<AChukItemPickup>> AllowedPickupClasses;

	bool AllowsClass(TSubclassOf<AChukItemPickup> PickupClass) const;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	TObjectPtr<UBillboardComponent> Sprite;

	UPROPERTY()
	TObjectPtr<UArrowComponent> Arrow;
#endif
};

/** 생성기에서 아이템 한 종류를 몇 개 만들지 */
USTRUCT(BlueprintType)
struct FChukSpawnEntry
{
	GENERATED_BODY()

	/** 생성할 아이템 블루프린트 (예: BP_Pickup_OilLamp) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AChukItemPickup> PickupClass;

	/** 생성 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	int32 Count = 1;
};

/**
 * 게임 시작 시 레벨의 모든 AChukItemSpawnPoint 중에서 랜덤으로 골라 아이템을 생성한다.
 * 한 지점에는 아이템 하나만 생긴다. 지점이 모자라면 생길 수 있는 만큼만 생기고 로그에 경고.
 *
 * ■ 사용법
 *   1. Place Actors 에서 "Chuk Item Spawner" 를 레벨에 하나 놓는다 (위치는 상관없음).
 *   2. Details 의 Spawn Entries 에 [+] → Pickup Class(BP_Pickup_OilLamp 등)와 Count 지정.
 */
UCLASS()
class GAME_API AChukItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	AChukItemSpawner();

	/** 생성할 아이템 목록 */
	// TODO(기획 확인): 등잔 / 쥐 사체 / 제웅 각각 몇 개 생성할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Spawn")
	TArray<FChukSpawnEntry> SpawnEntries;

	/** 켜면 게임 시작 시 자동으로 생성 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Spawn")
	bool bSpawnOnBeginPlay = true;

	/** 랜덤 생성 실행. 생성된 아이템 목록을 돌려줌 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Spawn")
	TArray<AChukItemPickup*> SpawnItems();

protected:
	virtual void BeginPlay() override;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	TObjectPtr<UBillboardComponent> Sprite;
#endif
};
