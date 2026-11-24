// 축영(逐影) - 최종 추격전 직전 세이브 데이터

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ChukItemTypes.h"
#include "ChukRelicSubsystem.h"
#include "ChukSaveGame.generated.h"

/**
 * 세이브 포인트에 저장되는 데이터 묶음.
 * 직접 쓸 일은 거의 없고, Chuk Save Subsystem 의 Save To Save Point / Load From Save Point 가 알아서 채우고 읽는다.
 */
UCLASS()
class GAME_API UChukSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** 유품 상태 (꽃신, 노리개, 댕기, 비녀 순서) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	TArray<EChukRelicState> RelicStates;

	/** 인벤토리 (아이템별 남은 횟수) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	TArray<FChukInventorySlot> InventorySlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	float Fear = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	float Sanity = 100.f;

	/** 등불 남은 시간(초) = 등불 밝기 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	float LanternRemainingTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	FTransform PlayerTransform;

	/** 플레이어가 바라보던 방향 (카메라) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	FRotator PlayerControlRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	int32 RemainingHides = 0;

	/** 밤 진행 시간 (아침이 오기까지). 시간 시스템이 아직 없어 값만 저장 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	float NightTimeElapsed = 0.f;
};
