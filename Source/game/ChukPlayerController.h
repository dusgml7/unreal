// 축영(逐影) - 플레이어 컨트롤러 (HUD 생성 + 아이템·달리기 키)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "ChukPlayerController.generated.h"

class UChukHUDWidget;
class UChukInventoryComponent;
class UChukGaugeComponent;

/**
 * 게임 시작 시 HUD 위젯을 만들어 화면에 띄우고, 조종하는 캐릭터와 연결하는 플레이어 컨트롤러.
 * 아이템 키와 달리기 키도 여기서 연결한다 (캐릭터 블루프린트에서 따로 연결할 필요 없음).
 *
 * ■ 기본 키 (Class Defaults 의 Chuk|Keys 에서 변경)
 *   - R            : 손에 든 아이템 사용 (Chuk Inventory → Use Held Item)
 *   - 마우스 휠     : 손에 든 아이템 바꾸기 (위 = 이전, 아래 = 다음)
 *   - 숫자 1~6     : 아이템 창 N번째 칸을 손에 들기
 *   - 왼쪽 Shift   : 누르는 동안 달리기 (Chuk Gauge → Set Running)
 *   - 왼쪽 Ctrl    : 웅크리기 (Toggle Crouch 를 끄면 누르는 동안, 켜면 한 번 누를 때마다 켜고 끄기)
 *                    → 애니메이션 블루프린트에서 캐릭터의 Is Crouched 로 웅크리기 동작 연결 필요
 *
 * ■ 사용법
 *   1. 게임 모드 블루프린트의 Player Controller Class 를 ChukPlayerController 로 지정
 *      (HUD 를 띄우려면 자식 블루프린트 BP_ChukPlayerController 를 만들어 HUD Widget Class 에 UI_HUD 지정 후 그것을 사용)
 *   2. 플레이어 캐릭터에 Chuk Inventory / Chuk Gauge 컴포넌트가 붙어 있어야 키가 동작한다.
 *   3. 캐릭터 블루프린트에 이미 같은 키(예: IA_Run 의 Shift)를 쓰는 로직이 있으면 둘이 같이 동작하므로 한쪽을 끊는다.
 */
UCLASS(Blueprintable)
class GAME_API AChukPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 화면에 띄울 HUD 위젯 블루프린트 (UI_HUD) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI")
	TSubclassOf<UChukHUDWidget> HUDWidgetClass;

	/** 손에 든 아이템 사용 키 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chuk|Keys")
	FKey UseItemKey = EKeys::R;

	/** 다음 아이템 키 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chuk|Keys")
	FKey NextItemKey = EKeys::MouseScrollDown;

	/** 이전 아이템 키 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chuk|Keys")
	FKey PreviousItemKey = EKeys::MouseScrollUp;

	/** 아이템 창 칸을 바로 고르는 키. 첫 번째 = 1번 칸 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chuk|Keys")
	TArray<FKey> ItemSlotKeys = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six };

	/** 달리기 키 (누르는 동안 달림) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chuk|Keys")
	FKey RunKey = EKeys::LeftShift;

	/** 웅크리기 키 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chuk|Keys")
	FKey CrouchKey = EKeys::LeftControl;

	/** 끄면 누르고 있는 동안만 웅크림, 켜면 누를 때마다 웅크리기 ↔ 일어서기 */
	// TODO(기획 확인): 웅크리기를 누르는 동안만 할지, 눌러서 켜고 끌지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chuk|Keys")
	bool bToggleCrouch = false;

	/** 만들어진 HUD 위젯 */
	UFUNCTION(BlueprintPure, Category = "Chuk|UI")
	UChukHUDWidget* GetHUDWidget() const { return HUDWidget; }

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	TObjectPtr<UChukHUDWidget> HUDWidget;

	UChukInventoryComponent* GetPawnInventory() const;
	UChukGaugeComponent* GetPawnGauge() const;

	void BindChukKey(const FKey& Key, EInputEvent Event, TFunction<void()> Action);

	/** bCrouch = true 면 웅크리기, false 면 일어서기 (숨어 있는 중에는 무시) */
	void SetPawnCrouched(bool bCrouch);
};
