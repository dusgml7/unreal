// 축영(逐影) - 게이지 창 위젯 (부모 클래스)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChukGaugeWidget.generated.h"

class UChukGaugeComponent;
class UProgressBar;

/**
 * 공포 / 정신력 게이지 창. 디자인은 피그마를 보고 위젯 블루프린트(WBP)에서 만든다.
 *
 * ■ 위젯 블루프린트 만드는 법
 *   1. Content Browser 우클릭 → User Interface → Widget Blueprint → 부모 클래스로 "ChukGaugeWidget" 선택
 *      (이미 만든 WBP 라면 Graph 탭 → Class Settings → Parent Class 를 ChukGaugeWidget 으로 변경)
 *   2. Designer 에서 Progress Bar 두 개를 놓고 이름을 정확히 "FearBar", "SanityBar" 로 지정 (필수)
 *   3. 단계별 색·이미지 변경은 Graph 에서 Event On Fear Stage Display / On Sanity Stage Display 구현
 *
 * 게이지 컴포넌트 연결은 HUD 가 자동으로 해 준다 (Bind To Gauge).
 */
UCLASS(Abstract)
class GAME_API UChukGaugeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 공포 게이지 바 (이름 "FearBar" 필수) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UProgressBar> FearBar;

	/** 정신력 게이지 바 (이름 "SanityBar" 필수) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UProgressBar> SanityBar;

	/** 표시할 게이지 컴포넌트 연결 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void BindToGauge(UChukGaugeComponent* Gauge);

	/** 공포 단계가 바뀔 때 (0~3). 바 색·이미지 변경 등을 여기서 */
	// TODO(기획 확인): 단계별 게이지 색·이미지 (피그마)
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|UI")
	void OnFearStageDisplay(int32 Stage);

	/** 정신력 단계가 바뀔 때 (0~3) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|UI")
	void OnSanityStageDisplay(int32 Stage);

private:
	TWeakObjectPtr<UChukGaugeComponent> BoundGauge;

	UFUNCTION()
	void HandleFearChanged(float NewValue);

	UFUNCTION()
	void HandleSanityChanged(float NewValue);

	UFUNCTION()
	void HandleFearStageChanged(int32 Stage);

	UFUNCTION()
	void HandleSanityStageChanged(int32 Stage);
};
