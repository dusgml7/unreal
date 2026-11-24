// 축영(逐影) - 플레이어 상호작용 컴포넌트

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "ChukInteractComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChukInteractEvent, AActor*, TargetActor);

/**
 * 플레이어가 바라보는 물체와 상호작용하는 컴포넌트
 *
 * 카메라 정면으로 짧은 트레이스를 쏴서, 맞은 액터가 Chuk Interactable 인터페이스를 가지고 있으면
 * 그 액터의 Interact 를 실행한다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. 플레이어 캐릭터 블루프린트에 [+ Add] → "Chuk Interact" 추가
 *   2. 이벤트 그래프에서 상호작용 키 입력(예: E 키 또는 Enhanced Input 의 Interact 액션) 이벤트에
 *      ChukInteract 컴포넌트의 "Try Interact" 노드를 연결
 *   3. "E: 줍기" 같은 안내 문구는 Get Interact Target 을 Tick 에서 확인해 표시하면 된다.
 *
 * ■ 기존 BPI_Interact 와 함께 쓰기
 *   기존 캐릭터가 이미 BPI_Interact 로 상호작용하고 있다면, 같은 키 입력에서 Try Interact 도 함께 호출하면 된다.
 *   (Try Interact 는 Chuk Interactable 이 없는 물체에는 아무 일도 하지 않음)
 */
UCLASS(ClassGroup = (Chuk), meta = (BlueprintSpawnableComponent, DisplayName = "Chuk Interact"))
class GAME_API UChukInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChukInteractComponent();

	/** 상호작용 가능한 거리(cm) */
	// TODO(기획 확인): 상호작용 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Interact", meta = (ClampMin = "0"))
	float InteractDistance = 250.f;

	/** 트레이스 두께(cm). 클수록 정확히 조준하지 않아도 잡힘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Interact", meta = (ClampMin = "0"))
	float TraceRadius = 15.f;

	/** 트레이스 충돌 채널 (보통 Visibility) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Interact")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** 상호작용에 성공했을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Interact")
	FChukInteractEvent OnInteracted;

	/** 지금 바라보고 있는 상호작용 대상. 없으면 None */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Interact")
	AActor* GetInteractTarget() const;

	/** 바라보는 대상과 상호작용. 성공하면 true. 은신처에 숨어 있는 중이면 은신처에서 나온다 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Interact")
	bool TryInteract();
};
