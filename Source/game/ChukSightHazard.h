// 축영(逐影) - 보는 순간 발동하는 물체 (쥐 사체, 제웅)

#pragma once

#include "CoreMinimal.h"
#include "ChukItemPickup.h"
#include "ChukSightHazard.generated.h"

/**
 * 플레이어가 "직접 보는 순간" 효과가 적용되는 물체. 줍지 않는다.
 * 보면 플레이어의 Chuk Inventory 에 Acquire Item(Item Id) 을 호출 → DataTable 의 효과(정신력 악화)가 바로 적용된다.
 * (DataTable 에서 DeadMouse / StrawDoll 의 Apply on Pickup 이 True 여야 인벤토리에 들어가지 않고 바로 적용됨)
 *
 * "본다" 판정: 플레이어 화면(카메라)에 걸려 있음 + Sight Max Distance 안 + 가리는 벽 없음
 *              이 상태가 Sight Time Required(기본 1.5초) 동안 끊김 없이 이어지면 발동 → 정신력 감소 + 사라짐.
 *              도중에 화면 밖으로 나가거나 가려지면 시간이 0으로 돌아감 (= 못 본 것으로 간주).
 *
 * ChukItemPickup 을 상속하므로 Chuk Item Spawner 로 무작위 배치도 된다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. Content Browser 우클릭 → Blueprint Class → All Classes 에서 "ChukSightHazard" 선택
 *      → 이름 예: BP_Hazard_DeadMouse, BP_Hazard_StrawDoll
 *   2. Mesh 에 스태틱 메시 지정 (deadMouse / strawDoll 폴더의 메시)
 *   3. Class Defaults → Chuk|Pickup 의 Item Id 에 item_deadmouse 또는 item_strawdoll 입력
 *   4. 보는 순간 소리·연출은 이벤트 그래프의 Event On Seen By Player 에서 연결
 */
UCLASS(Blueprintable)
class GAME_API AChukSightHazard : public AChukItemPickup
{
	GENERATED_BODY()

public:
	AChukSightHazard();

	/** 화면 가장자리에서 이 비율만큼은 "안 걸린 것"으로 침 (0 = 화면 끝까지 인정, 0.1 = 가장자리 10% 제외) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sight", meta = (ClampMin = "0", ClampMax = "0.45"))
	float ScreenEdgeMargin = 0.f;

	/** 캐릭터와 이 거리(cm) 안에서 봐야 발동 — 가까이서 봤을 때만 (기획: 가까이). 카메라가 아니라 캐릭터 기준 */
	// TODO(기획 확인): 정확한 발동 거리 (지금 3m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sight", meta = (ClampMin = "0"))
	float SightMaxDistance = 300.f;

	/** 화면에 이 시간(초) 동안 계속 머물러야 발동 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sight", meta = (ClampMin = "0"))
	float SightTimeRequired = 1.0f;

	/** 벽 등에 가려졌는지 검사할 채널 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sight")
	TEnumAsByte<ECollisionChannel> SightTraceChannel = ECC_Visibility;

	/** 켜면 발동 후 사라짐. 끄면 그 자리에 남고 다시는 발동하지 않음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sight")
	bool bDestroyAfterSeen = true;

	/** 이미 발동했는지 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Sight")
	bool HasTriggered() const { return bTriggered; }

	/** 플레이어가 봐서 발동했을 때 (소리·연출 연결용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Sight")
	void OnSeenByPlayer(AActor* Viewer);

	// 줍지 않으므로 E 상호작용은 아무 일도 하지 않는다
	virtual void Interact_Implementation(AActor* InteractingActor) override {}

	virtual void Tick(float DeltaSeconds) override;

private:
	bool bTriggered = false;
	float SeenTime = 0.f;

	bool IsSeenBy(const APawn* Viewer) const;
};
