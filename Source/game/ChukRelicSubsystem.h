// 축영(逐影) - 유품 진행 관리 (월드 서브시스템)

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ChukRelicSubsystem.generated.h"

/** 자매의 유품 4종 */
UENUM(BlueprintType)
enum class EChukRelicType : uint8
{
	Kkotsin		UMETA(DisplayName = "꽃신"),
	Norigae		UMETA(DisplayName = "노리개"),
	Daenggi		UMETA(DisplayName = "댕기"),
	Binyeo		UMETA(DisplayName = "비녀")
};

/** 유품 상태 */
// TODO(기획 확인): 유품 상태가 "미발견 / 발견 / 태움" 3가지인지, 2가지인지
UENUM(BlueprintType)
enum class EChukRelicState : uint8
{
	NotFound	UMETA(DisplayName = "미발견"),
	Found		UMETA(DisplayName = "발견"),
	Burned		UMETA(DisplayName = "태움")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChukRelicStateChanged, EChukRelicType, Relic, EChukRelicState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChukDifficultyIncreased, int32, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChukRelicEvent);

/**
 * 유품 진행 관리 (맵마다 자동으로 하나 생기는 서브시스템 — 레벨에 배치할 필요 없음)
 *
 * - 유품을 찾으면(Find Relic):
 *     · 상태 미발견 → 발견
 *     · 플레이어의 숨기 횟수를 최대로 초기화 (Chuk Hide)
 *     · 난이도 단계 +1 → On Difficulty Increased(Level)  (Level = 찾은 유품 수, 1~4)
 *       스포그 강화·화면 어두워짐·추적 강화 연출은 블루프린트에서 이 이벤트로 처리
 *     · 4개 모두 찾으면 On All Relics Found → 최종 추격전 시작 (세이브는 작업 6에서 연결)
 * - 유품을 태우면(Burn Relic): 발견 → 태움. 4개 모두 태우면 On All Relics Burned.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   - 아무 블루프린트(레벨 블루프린트, 캐릭터 등)에서 우클릭 → "Get ChukRelicSubsystem" 노드
 *   - 그 핀에서 "Bind Event to On Difficulty Increased" 등으로 이벤트 연결
 *     (레벨 블루프린트의 Event BeginPlay 에서 바인딩하는 것을 추천)
 *   - 유품 줍기는 AChukRelicPickup 블루프린트가 자동으로 Find Relic 을 호출한다.
 */
UCLASS()
class GAME_API UChukRelicSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 유품 상태가 바뀔 때 (UI 유품 창 갱신용) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Relic")
	FChukRelicStateChanged OnRelicStateChanged;

	/** 유품을 찾아 난이도가 올라갈 때. Level = 찾은 유품 수(1~4) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Relic")
	FChukDifficultyIncreased OnDifficultyIncreased;

	/** 유품 4개를 모두 찾았을 때 → 최종 추격전 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Relic")
	FChukRelicEvent OnAllRelicsFound;

	/** 유품 4개를 모두 태웠을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Relic")
	FChukRelicEvent OnAllRelicsBurned;

	/** 유품을 찾음. 이미 찾았거나 태운 유품이면 false */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Relic")
	bool FindRelic(EChukRelicType Relic);

	/** 유품을 태움. 발견 상태가 아니면 false */
	// TODO(기획 확인): 유품을 언제·어디서 태우는지 (찾자마자 / 특정 장소 / 최종 추격전 중)
	UFUNCTION(BlueprintCallable, Category = "Chuk|Relic")
	bool BurnRelic(EChukRelicType Relic);

	UFUNCTION(BlueprintPure, Category = "Chuk|Relic")
	EChukRelicState GetRelicState(EChukRelicType Relic) const;

	/** 찾은 유품 수 (태운 것 포함) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Relic")
	int32 GetFoundCount() const;

	/** 태운 유품 수 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Relic")
	int32 GetBurnedCount() const;

	/** 현재 난이도 단계 (0 = 시작 ~ 4 = 유품 모두 발견) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Relic")
	int32 GetDifficultyLevel() const { return GetFoundCount(); }

	/**
	 * 유품 상태를 직접 지정 (세이브 불러오기·디버그용).
	 * 숨기 횟수 초기화·난이도·전체 완료 이벤트는 발생하지 않고 On Relic State Changed 만 발생.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Relic")
	void SetRelicState(EChukRelicType Relic, EChukRelicState NewState);

	static constexpr int32 RelicCount = 4;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	EChukRelicState States[RelicCount] = { EChukRelicState::NotFound, EChukRelicState::NotFound, EChukRelicState::NotFound, EChukRelicState::NotFound };

	void ResetPlayerHideCount();
};
