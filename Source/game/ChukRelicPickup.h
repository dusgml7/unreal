// 축영(逐影) - 유품 액터 (레벨에 배치)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChukInteractable.h"
#include "ChukRelicSubsystem.h"
#include "ChukRelicPickup.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;

/**
 * 레벨에 놓는 유품 (꽃신 / 노리개 / 댕기 / 비녀)
 *
 * 플레이어가 E(Try Interact)로 상호작용하면 유품 서브시스템의 Find Relic 을 호출하고 사라진다.
 * → 숨기 횟수 초기화, 난이도 +1, 4개 모두 찾으면 최종 추격전 이벤트가 자동으로 이어진다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. Content Browser 우클릭 → Blueprint Class → All Classes 에서 "ChukRelicPickup" 선택
 *      → 이름 예: BP_Relic_Kkotsin, BP_Relic_Norigae, BP_Relic_Daenggi, BP_Relic_Binyeo
 *   2. Mesh 에 유품 메시 지정, Class Defaults → Chuk|Relic 의 Relic Type 선택
 *   3. 줍는 소리·연출은 Event On Relic Found 에서 연결
 */
UCLASS(Blueprintable)
class GAME_API AChukRelicPickup : public AActor, public IChukInteractable
{
	GENERATED_BODY()

public:
	AChukRelicPickup();

	/** 이 액터가 어떤 유품인지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Relic")
	EChukRelicType RelicType = EChukRelicType::Kkotsin;

	/** 상호작용 판정용 캡슐. Shape 에서 Capsule Half Height(세로), Capsule Radius(가로) 조절 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Relic")
	TObjectPtr<UCapsuleComponent> InteractCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Relic")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** 유품을 찾았을 때 (사운드·연출용). 액터가 사라지기 직전에 호출됨 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Relic")
	void OnRelicFound(AActor* FoundBy);

	// IChukInteractable
	virtual void Interact_Implementation(AActor* InteractingActor) override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleRelicStateChanged(EChukRelicType Relic, EChukRelicState NewState);
};
