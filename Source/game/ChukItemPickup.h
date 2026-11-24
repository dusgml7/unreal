// 축영(逐影) - 줍는 아이템 액터

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChukInteractable.h"
#include "ChukItemPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * 레벨에 놓는 아이템 (등잔, 쥐 사체, 제웅, 일기 두루마리 등)
 *
 * 플레이어가 Chuk Interact 의 Try Interact 로 상호작용하면 플레이어의 Chuk Inventory 에 Acquire Item 을 호출한다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. Content Browser 우클릭 → Blueprint Class → All Classes 에서 "ChukItemPickup" 선택
 *      → 이름 예: BP_Pickup_OilLamp, BP_Pickup_DeadMouse, BP_Pickup_StrawDoll, BP_Pickup_Diary
 *   2. Mesh 컴포넌트에 스태틱 메시 지정
 *   3. Class Defaults → Chuk|Pickup 의 Item Id 에 DataTable Row Name 입력 (OilLamp / DeadMouse / StrawDoll / Diary)
 *   4. 일기 두루마리는 "생성된 곳에 계속 있음" 이므로 Destroy On Pickup 을 끈다.
 *   5. 줍는 소리·이펙트는 이벤트 그래프의 Event On Picked Up 에서 연결.
 *
 * ■ 기존 BPI_Interact 와 연결하고 싶다면
 *   자식 블루프린트의 Class Settings → Interfaces 에 BPI_Interact 추가 →
 *   그 인터페이스 이벤트에서 "Interact (Chuk Interactable)" 노드를 Self 대상으로 호출하면 된다.
 */
UCLASS(Blueprintable)
class GAME_API AChukItemPickup : public AActor, public IChukInteractable
{
	GENERATED_BODY()

public:
	AChukItemPickup();

	/** 획득할 아이템 ID (DataTable 의 Row Name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Pickup")
	FName ItemId;

	/** 켜면 주운 뒤 액터가 사라짐. 일기 두루마리처럼 그 자리에 남아야 하면 끈다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Pickup")
	bool bDestroyOnPickup = true;

	/** 상호작용 판정용 구 (트레이스가 여기에 맞으면 상호작용 대상) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Pickup")
	TObjectPtr<USphereComponent> InteractCollision;

	/** 겉모습 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Pickup")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** 주웠을 때 (사운드·이펙트 연결용). 액터가 사라지기 직전에 호출됨 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Pickup")
	void OnPickedUp(AActor* PickedUpBy);

	// IChukInteractable
	virtual void Interact_Implementation(AActor* InteractingActor) override;
};
