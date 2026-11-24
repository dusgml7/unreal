// 축영(逐影) - 플레이어 숨기 상태 / 숨기 횟수 컴포넌트

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChukHideComponent.generated.h"

class AChukHideSpot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChukHideCountChanged, int32, Remaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChukHideSpotEvent, AChukHideSpot*, HideSpot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChukHideDenied);

/**
 * 플레이어 쪽 숨기 관리 컴포넌트
 *
 * - 유품 하나를 찾기 전까지 최대 5번 숨을 수 있다 (숨으러 들어갈 때 1회 차감).
 *   유품을 찾으면 Reset Hide Count 로 다시 5번 (작업 5에서 자동 호출).
 * - 숨는 동안 은신처의 차단 설정을 Chuk Gauge 에 전달한다.
 *     · 시야 차단 은신처 → 공포가 오르지 않음
 *     · 소리 차단 은신처 → 정신력이 내려가지 않음
 * - 숨는 동안 캐릭터는 은신처의 Hide Point 로 옮겨지고 이동이 막힌다. 나올 때 Exit Point 로 옮겨진다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. 플레이어 캐릭터 블루프린트에 [+ Add] → "Chuk Hide" 추가 (Chuk Gauge 와 같은 캐릭터)
 *   2. 은신처에 들어가기/나오기는 Chuk Interact 의 Try Interact (E 키) 로 자동 처리된다.
 *   3. 카메라 연출·숨소리 등은 Events 의 On Hide Started / On Hide Ended 에서 연결.
 *   4. 남은 횟수 UI 는 On Hide Count Changed 로 갱신.
 */
UCLASS(ClassGroup = (Chuk), meta = (BlueprintSpawnableComponent, DisplayName = "Chuk Hide"))
class GAME_API UChukHideComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChukHideComponent();

	/** 유품 하나를 찾기 전까지 숨을 수 있는 최대 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Hide", meta = (ClampMin = "0"))
	int32 MaxHideCount = 5;

	/** 켜면 횟수를 다 썼을 때 숨기가 막힌다. 끄면 숨을 수 있고 이벤트만 발생 */
	// TODO(기획 확인): 5번을 다 쓰면 은신처 상호작용이 막히는지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Hide")
	bool bBlockWhenNoHidesLeft = true;

	/** 켜면 숨을 때 캐릭터를 은신처 안(Hide Point)으로 옮기고 이동을 막는다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Hide")
	bool bMoveCharacterIntoSpot = true;

	/** 남은 숨기 횟수가 바뀔 때 (UI 용) */
	// TODO(기획 확인): UI 에 남은 횟수를 표시하는지
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukHideCountChanged OnHideCountChanged;

	/** 숨기 시작 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukHideSpotEvent OnHideStarted;

	/** 숨기 끝 (은신처에서 나옴) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukHideSpotEvent OnHideEnded;

	/** 남은 횟수가 0이라 숨기가 막혔을 때 ("더 이상 숨을 수 없다" 안내 등) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukHideDenied OnHideDenied;

	/** 은신처에 들어간다. 성공하면 true */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Hide")
	bool EnterHideSpot(AChukHideSpot* Spot);

	/** 은신처에서 나온다 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Hide")
	void ExitHideSpot();

	UFUNCTION(BlueprintPure, Category = "Chuk|Hide")
	bool IsHiding() const { return CurrentSpot != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Chuk|Hide")
	AChukHideSpot* GetCurrentHideSpot() const { return CurrentSpot; }

	UFUNCTION(BlueprintPure, Category = "Chuk|Hide")
	int32 GetRemainingHides() const { return RemainingHides; }

	/** 숨기 횟수를 최대로 되돌린다 (유품 발견 시) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Hide")
	void ResetHideCount();

	/** 남은 숨기 횟수를 직접 지정 (세이브 불러오기 등) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Hide")
	void SetRemainingHides(int32 NewRemaining);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<AChukHideSpot> CurrentSpot;

	int32 RemainingHides = 0;

	void ApplyBlocking(bool bSight, bool bSound);
};
