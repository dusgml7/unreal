// 축영(逐影) - 세이브 포인트 트리거 박스

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChukSavePointTrigger.generated.h"

class UBoxComponent;

/**
 * 플레이어가 이 박스에 들어오면 세이브 포인트에 자동 저장한다 (최종 추격전 시작 지점 앞 등).
 *
 * ■ 사용법
 *   1. Place Actors 에서 "Chuk Save Point Trigger" 검색 → 레벨에 놓기
 *   2. Details 의 Box 크기(Box Extent)를 통로 크기에 맞게 조절
 *   3. 저장 안내 연출은 블루프린트 자식 클래스의 Event On Saved 또는
 *      Chuk Save Subsystem 의 On Save Point Saved 에서 연결
 */
UCLASS(Blueprintable)
class GAME_API AChukSavePointTrigger : public AActor
{
	GENERATED_BODY()

public:
	AChukSavePointTrigger();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Save")
	TObjectPtr<UBoxComponent> Box;

	/** 켜면 한 번만 저장 (다시 들어가도 저장 안 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Save")
	bool bTriggerOnce = true;

	/** 켜면 유품을 4개 모두 찾은 뒤에만 저장 */
	// TODO(기획 확인): 세이브 트리거를 유품 4개 발견 후에만 동작하게 할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Save")
	bool bRequireAllRelicsFound = true;

	/** 저장했을 때 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Save")
	void OnSaved();

protected:
	virtual void BeginPlay() override;

private:
	bool bAlreadyTriggered = false;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
