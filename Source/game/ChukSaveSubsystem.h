// 축영(逐影) - 세이브 포인트 저장 / 불러오기 (월드 서브시스템)

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ChukSaveSubsystem.generated.h"

class UChukSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChukSaveEvent);

/**
 * 최종 추격전 직전 세이브 포인트 (맵마다 자동으로 하나 생김 — 배치할 필요 없음)
 *
 * ■ 저장 (Save To Save Point)
 *   - 유품 상태, 인벤토리 남은 횟수, 공포·정신력, 등불 남은 시간, 플레이어 위치, 남은 숨기 횟수를 저장.
 *   - 자동 저장 시점 (둘 다 쓸 수 있음):
 *       ① 유품 4개를 모두 찾는 순간 (Auto Save On All Relics Found = true 일 때, 기본 켜짐)
 *       ② 레벨에 놓은 AChukSavePointTrigger 박스에 플레이어가 들어갈 때
 *
 * ■ 불러오기 (Load From Save Point)
 *   - 추격전 중 게임 오버 → Load From Save Point 호출.
 *   - 현재 맵을 처음부터 다시 연 다음, 저장된 값으로 되돌린다
 *     (자매·아이템·문 등 레벨 상태가 깨끗하게 초기화되도록).
 *   - 다 되돌린 뒤 On Save Point Loaded 이벤트 → 여기서 최종 추격전을 다시 시작하는 연출을 연결.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   - 우클릭 → "Get ChukSaveSubsystem" 노드 → Save To Save Point / Load From Save Point / Has Save Point
 *   - 이벤트는 레벨 블루프린트 BeginPlay 에서 Bind Event to On Save Point Loaded 로 연결.
 */
UCLASS()
class GAME_API UChukSaveSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 유품 4개를 모두 찾는 순간 자동 저장할지 */
	// TODO(기획 확인): 세이브 시점 — 유품 4개 발견 즉시 / 세이브 트리거 박스 / 둘 다
	UPROPERTY(BlueprintReadWrite, Category = "Chuk|Save")
	bool bAutoSaveOnAllRelicsFound = true;

	/**
	 * 밤 진행 시간(초). 아침이 오면 게임이 끝나는 시간 시스템은 아직 없음 — 값만 열어 둠.
	 * 기획: 유품을 다 태웠을 때 시간을 초기화해서 추격 중 아침이 되어 끝나는 것을 막는다.
	 */
	// TODO(기획 확인): 밤 시간 시스템(제한 시간, 아침 판정, 추격전 시작 시 초기화)은 미구현. 블루프린트에서 이 값을 읽고 쓸 수 있게만 해 둠
	UPROPERTY(BlueprintReadWrite, Category = "Chuk|Save")
	float NightTimeElapsed = 0.f;

	/** 저장이 끝났을 때 ("저장되었습니다" 표시 등) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Save")
	FChukSaveEvent OnSavePointSaved;

	/** 불러오기가 끝났을 때 → 최종 추격전 다시 시작 연출 연결 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Save")
	FChukSaveEvent OnSavePointLoaded;

	/** 지금 상태를 세이브 포인트에 저장. 성공하면 true */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Save")
	bool SaveToSavePoint();

	/** 세이브 포인트에서 다시 시작 (맵을 다시 열고 저장 값으로 되돌림). 세이브가 없으면 false */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Save")
	bool LoadFromSavePoint();

	/** 세이브 포인트가 있는지 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Save")
	bool HasSavePoint() const;

	/** 세이브 포인트 삭제 (새 게임 시작 시 등) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Save")
	void DeleteSavePoint();

	/** 밤 시간을 0으로 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Save")
	void ResetNightTime() { NightTimeElapsed = 0.f; }

	static const FString SaveSlotName;
	static constexpr int32 SaveUserIndex = 0;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	UPROPERTY()
	TObjectPtr<UChukSaveGame> PendingLoad;

	UFUNCTION()
	void HandleAllRelicsFound();

	/** 맵을 다시 연 뒤 플레이어 상태를 되돌림 (다음 프레임에 실행) */
	void ApplyPendingLoadToPlayer();
};
