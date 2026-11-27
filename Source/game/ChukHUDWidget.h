// 축영(逐影) - 메인 HUD 위젯 (부모 클래스)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChukHUDWidget.generated.h"

class APawn;
class UChukGaugeWidget;
class UChukHideComponent;
class UChukItemBarWidget;
class UChukRelicPanelWidget;
class UTextBlock;

/**
 * 게이지 창 / 아이템 창 / 유품 창을 모은 메인 HUD.
 * AChukPlayerController 가 게임 시작 시 만들어 화면에 띄우고, 플레이어 캐릭터의 컴포넌트와 자동 연결한다.
 *
 * ■ 위젯 블루프린트 만드는 법 (예: UI_HUD)
 *   1. Widget Blueprint 생성 → 부모 클래스 "ChukHUDWidget"
 *   2. Designer 에 Canvas Panel 을 놓고, Palette 의 User Created 에서 아래 위젯을 끌어다 놓은 뒤 이름 지정
 *        UI_게이지     → 이름 "ui_gauge"
 *        UI_아이템창   → 이름 "ui_itembar"
 *        UI_유품창 → 이름 "ui_relic" (앵커: 오른쪽 가운데)
 *      (셋 다 선택 사항 — 없는 것은 건너뜀)
 *   3. 남은 숨기 횟수를 보여 주려면 Text 를 놓고 이름 "ui_hidecount" (선택)
 */
UCLASS(Abstract)
class GAME_API UChukHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidgetOptional))
	TObjectPtr<UChukGaugeWidget> ui_gauge;

	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidgetOptional))
	TObjectPtr<UChukItemBarWidget> ui_itembar;

	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidgetOptional))
	TObjectPtr<UChukRelicPanelWidget> ui_relic;

	/** 남은 숨기 횟수 글자 (선택) */
	// TODO(기획 확인): UI 에 남은 숨기 횟수를 표시하는지
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ui_hidecount;

	/** 플레이어 캐릭터의 게이지·인벤토리·숨기 컴포넌트와 연결 (플레이어 컨트롤러가 자동 호출) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void BindToPawn(APawn* Pawn);

	/** 연결이 끝났을 때 (블루프린트에서 추가 연결용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|UI")
	void OnBoundToPawn(APawn* Pawn);

private:
	TWeakObjectPtr<UChukHideComponent> BoundHide;

	UFUNCTION()
	void HandleHideCountChanged(int32 Remaining);
};
