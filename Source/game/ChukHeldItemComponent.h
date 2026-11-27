// 축영(逐影) - 손에 든 아이템 3D 모델 표시 컴포넌트

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "ChukHeldItemComponent.generated.h"

class UChukInventoryComponent;

/**
 * 손에 든 아이템의 3D 모델을 보여 주는 컴포넌트.
 * Chuk Inventory 에서 손에 든 아이템이 바뀔 때마다, 데이터 테이블(DT_ChukItems)의
 * Held Mesh / Held Offset 값으로 자동으로 모델과 위치를 바꾼다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. BP_MyCharacter 의 Components 에서 Mesh (CharacterMesh0) 를 선택한 상태로 + Add → "Chuk Held Item" 추가
 *      (Mesh 아래에 붙어야 손을 따라 움직임)
 *   2. 추가한 컴포넌트를 선택 → Details 의 Sockets → Parent Socket 에서 오른손 뼈(예: hand_r, mixamorig:RightHand) 선택
 *   3. DT_ChukItems 에서 아이템마다 아래 둘 중 하나 지정, Held Offset 으로 손 안 위치·회전·크기 조절
 *      - Held Mesh        : Static Mesh 하나로 된 모델
 *      - Held Actor Class : 여러 부품으로 조립된 블루프린트 (예: BP_Bell). 지정하면 Held Mesh 보다 우선
 *      (둘 다 비워 둔 아이템은 손에 아무것도 안 보임)
 *   4. 등불 빛(player_lantern_light)을 이 컴포넌트 아래로 끌어다 놓으면 등불 모델을 따라 빛이 움직인다.
 *
 * 충돌은 꺼져 있다 (손에 든 물건이 벽이나 캐릭터에 걸리지 않게).
 *
 * TODO(기획 확인): 아이템을 들고 있을 때의 팔 동작(들고 있는 자세 애니메이션) — 지금은 걷기 동작 그대로라 손에 붙어서 따라다니기만 함.
 */
UCLASS(ClassGroup = (Chuk), meta = (BlueprintSpawnableComponent))
class GAME_API UChukHeldItemComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UChukHeldItemComponent();

	/** 지금 보여 주고 있는 아이템 ID */
	UFUNCTION(BlueprintPure, Category = "Chuk|Held")
	FName GetShownItemId() const { return ShownItemId; }

	/** 손에 든 아이템 모델을 다시 불러온다 (보통은 자동으로 되므로 부를 필요 없음) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Held")
	void RefreshHeldItem();

	/** 손에 든 모델이 바뀐 뒤 호출 (아이템 꺼내는 소리·연출 연결용). ItemId 가 None 이면 빈손 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Held")
	void OnHeldMeshChanged(FName ItemId);

	/** 손에 붙어 있는 블루프린트 액터 (Held Actor Class 를 쓰는 아이템일 때만, 아니면 None) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Held")
	AActor* GetHeldActor() const { return HeldActor; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<UChukInventoryComponent> Inventory;

	UPROPERTY()
	TObjectPtr<AActor> HeldActor;

	/** 아이템별로 한 번 만든 블루프린트를 보관 (다른 아이템으로 바꾸면 숨겼다가, 돌아오면 다시 보여 줌 → 떨어진 방울 등 상태 유지) */
	UPROPERTY()
	TMap<FName, TObjectPtr<AActor>> HeldActorCache;

	/** 지금 손의 블루프린트를 숨긴다 (지우지 않음) */
	void HideHeldActor();

	/** 보관해 둔 블루프린트를 전부 지운다 (게임 종료 시) */
	void DestroyAllHeldActors();

	FName ShownItemId;

	UFUNCTION()
	void HandleHeldItemChanged(FName ItemId);

	/** 아이템 사용 시 → DT 의 Held Use Event Name 이벤트를 손의 블루프린트에서 실행 */
	UFUNCTION()
	void HandleItemUsed(FName ItemId);
};
