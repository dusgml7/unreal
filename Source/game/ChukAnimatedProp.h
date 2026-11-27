// 축영(逐影) - E 키로 열고 닫는 가구 (문·서랍·상자 등)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChukInteractable.h"
#include "ChukAnimatedProp.generated.h"

class UAnimationAsset;
class USkeletalMeshComponent;
class USoundBase;

/**
 * E(Try Interact)를 누를 때마다 열림 ↔ 닫힘 애니메이션과 소리를 재생하는 가구.
 * 에셋 팩(KTInteractionPropVol2)의 SK_ 모델 + ANIM_..._Open / ANIM_..._Close + Sound_... 를 그대로 쓴다.
 *
 * ※ 에셋 팩에 들어 있는 BP_Closet_1 같은 블루프린트는 자체적으로 키 입력을 받기 때문에
 *   우리 E 키와 겹친다. 그 블루프린트는 쓰지 말고, 이 클래스로 새로 만든다.
 * ※ 숨을 수 있는 가구(장롱 등)는 이 클래스가 아니라 ChukHideSpot 으로 만든다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. Content Browser 우클릭 → Blueprint Class → All Classes 에서 "ChukAnimatedProp" 선택
 *      → 이름 예: prop_drawer, prop_box
 *   2. SkeletalMesh 컴포넌트에 모델 지정 (예: SK_BesideTable)
 *   3. Class Defaults → Chuk|Prop 에서 Open / Close Animation, Open / Close Sound 지정
 *   4. 처음부터 열려 있어야 하면 Start Open 켜기
 *   5. 추가 연출(아이템 나타나기 등)은 Event On Opened / On Closed 에서 연결
 */
UCLASS(Blueprintable)
class GAME_API AChukAnimatedProp : public AActor, public IChukInteractable
{
	GENERATED_BODY()

public:
	AChukAnimatedProp();

	/** 문·서랍이 움직이는 모델. 충돌(Physics Asset)이 있어야 E 키로 상호작용 가능 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Prop")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	/** 열 때 재생할 애니메이션 (예: ANIM_BesideTable_Open) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Prop")
	TObjectPtr<UAnimationAsset> OpenAnimation = nullptr;

	/** 닫을 때 재생할 애니메이션 (예: ANIM_BesideTable_Close) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Prop")
	TObjectPtr<UAnimationAsset> CloseAnimation = nullptr;

	/** 열 때 소리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Prop")
	TObjectPtr<USoundBase> OpenSound = nullptr;

	/** 닫을 때 소리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Prop")
	TObjectPtr<USoundBase> CloseSound = nullptr;

	/** 켜면 게임 시작 때 열린 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Prop")
	bool bStartOpen = false;

	/** 끄면 E 를 눌러도 반응하지 않음 (잠긴 문 등) */
	// TODO(기획 확인): 잠긴 가구·문이 있는지, 여는 조건
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Prop")
	bool bCanInteract = true;

	/** 지금 열려 있는지 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Prop")
	bool IsOpen() const { return bOpen; }

	/** 열기 / 닫기 (블루프린트나 다른 장치에서 직접 부를 때) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Prop")
	void SetOpen(bool bNewOpen);

	/** 열렸을 때 (추가 연출용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Prop")
	void OnOpened();

	/** 닫혔을 때 (추가 연출용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Prop")
	void OnClosed();

	// IChukInteractable
	virtual void Interact_Implementation(AActor* InteractingActor) override;

protected:
	virtual void BeginPlay() override;

private:
	bool bOpen = false;
};
