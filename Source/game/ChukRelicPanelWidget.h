// 축영(逐影) - 유품 창 위젯 (부모 클래스, 화면 오른쪽 사이드)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChukRelicSubsystem.h"
#include "ChukRelicPanelWidget.generated.h"

class UImage;

/**
 * 화면 오른쪽 사이드에 붙는 유품 창. 유품 4개를 상태(미발견 / 발견 / 태움)에 따라 비활성/활성으로 표시.
 * 유품 서브시스템 이벤트를 받아 자동 갱신된다 (따로 연결할 필요 없음).
 *
 * ■ 위젯 블루프린트 만드는 법 (예: UI_유품창)
 *   1. Widget Blueprint 생성 → 부모 클래스 "ChukRelicPanelWidget"
 *   2. Designer 에 Vertical Box 를 놓고, 그 안에 Image 4개를 놓은 뒤 이름을 정확히
 *      "relic_kkotsin"(꽃신), "relic_norigae"(노리개), "relic_daenggi"(댕기), "relic_binyeo"(비녀) 로 지정 (필수)
 *   3. 각 Image 의 Brush 에 유품 아이콘 지정
 *   4. 상태별 투명도는 Class Defaults 에서, 다른 표현(태움 표시 등)은 Event On Relic Display Changed 에서
 *   5. 오른쪽 사이드 배치는 HUD 위젯(UI_HUD)에서 앵커를 오른쪽으로 잡아서 한다.
 */
UCLASS(Abstract)
class GAME_API UChukRelicPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UImage> relic_kkotsin;

	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UImage> relic_norigae;

	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UImage> relic_daenggi;

	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UImage> relic_binyeo;

	/** 미발견 유품 투명도 (비활성) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI", meta = (ClampMin = "0", ClampMax = "1"))
	float NotFoundOpacity = 0.3f;

	/** 발견한 유품 투명도 (활성) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI", meta = (ClampMin = "0", ClampMax = "1"))
	float FoundOpacity = 1.f;

	/** 태운 유품 투명도 */
	// TODO(기획 확인): 태운 유품을 UI 에서 어떻게 표시할지 (유품 상태 2가지/3가지 결정에 따라)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI", meta = (ClampMin = "0", ClampMax = "1"))
	float BurnedOpacity = 0.6f;

	/** 유품 표시가 바뀔 때 (추가 연출용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|UI")
	void OnRelicDisplayChanged(EChukRelicType Relic, EChukRelicState State);

	/** 4개 모두 현재 상태로 다시 그리기 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void RefreshAll();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UImage* GetImageFor(EChukRelicType Relic) const;

	UFUNCTION()
	void HandleRelicStateChanged(EChukRelicType Relic, EChukRelicState NewState);
};
