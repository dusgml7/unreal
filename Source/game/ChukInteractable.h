// 축영(逐影) - 상호작용 인터페이스 (C++ 용)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ChukInteractable.generated.h"

UINTERFACE(BlueprintType, Blueprintable)
class UChukInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 플레이어가 상호작용 키로 사용할 수 있는 물체 (아이템, 은신처, 유품 등)
 *
 * ※ 팀원이 만든 블루프린트 인터페이스 BPI_Interact 와는 별개다 (이름이 겹치지 않게 따로 만듦).
 *
 * ■ 블루프린트에서 쓰는 방법
 *   - 이 인터페이스를 C++ 에서 이미 구현한 액터(AChukItemPickup, AChukHideSpot 등)는 따로 할 일 없음.
 *   - 일반 블루프린트 액터에 붙이고 싶으면: Class Settings → Interfaces → Add → "Chuk Interactable"
 *     → My Blueprint 패널의 Interfaces 에서 Interact 이벤트를 구현.
 *   - 플레이어 쪽에서는 Chuk Interact 컴포넌트의 Try Interact 를 호출하면 바라보는 물체의 Interact 가 실행된다.
 */
class GAME_API IChukInteractable
{
	GENERATED_BODY()

public:
	/** 상호작용 실행. InteractingActor = 상호작용한 플레이어 캐릭터 */
	// (상호작용을 막아야 하는 경우 — 예: 숨기 횟수 0 — 는 각 액터의 Interact 안에서 처리한다)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chuk|Interact")
	void Interact(AActor* InteractingActor);
};
