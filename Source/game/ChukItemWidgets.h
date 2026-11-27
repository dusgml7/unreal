// 축영(逐影) - 아이템 슬롯 / 아이템 창 위젯 (부모 클래스)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChukItemTypes.h"
#include "ChukItemWidgets.generated.h"

class UChukInventoryComponent;
class UImage;
class UPanelWidget;
class UTextBlock;

/**
 * 아이템 한 칸. 아이콘 + 남은 횟수 + 활성/비활성(흐리게) 표시.
 *
 * ■ 위젯 블루프린트 만드는 법 (예: UI_아이템칸)
 *   1. Widget Blueprint 생성 → 부모 클래스 "ChukItemSlotWidget"
 *   2. Designer 에 Image 를 놓고 이름 "item_icon" (필수)
 *   3. Text 를 놓고 이름 "item_count" (선택 — 개수/남은 횟수 표시). 칸 오른쪽 아래에 작게 배치
 *      (Anchor: 오른쪽 아래, Alignment 1,1, 글자 크기 작게)
 *      · 등잔처럼 1회용 아이템: 같은 걸 또 주우면 같은 칸에서 숫자가 늘어남. 1개일 때는 숫자 숨김 (Hide Count When One)
 *      · 부채·방울·부적처럼 여러 번 쓰는 아이템: 남은 횟수 표시
 *      · 등불·일기처럼 무제한: 표시 안 함 (Unlimited Text 가 비어 있으면)
 *   4. 비활성일 때 투명도는 Class Defaults 의 Inactive Opacity 로 조절
 *   5. 다른 표현(회색 처리 등)이 필요하면 Event On Slot State Changed 구현
 *   6. 손에 든 칸 강조: 테두리용 Image(또는 Border)를 놓고 이름 "item_held" (선택 — 손에 든 칸에서만 보임)
 *      다른 강조 방식이 필요하면 Event On Held State Changed 구현
 */
UCLASS(Abstract)
class GAME_API UChukItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 아이콘 (이름 "item_icon" 필수) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UImage> item_icon;

	/** 남은 횟수 글자 (이름 "item_count", 없어도 됨) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> item_count;

	/** 손에 든 칸에서만 보이는 강조 표시 (이름 "item_held", 없어도 됨) */
	// TODO(기획 확인): 손에 든 아이템 강조 방식 (피그마)
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> item_held;

	/** 비활성(못 가짐 / 다 씀) 상태의 투명도 */
	// TODO(기획 확인): 비활성 표시 방식 (피그마)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI", meta = (ClampMin = "0", ClampMax = "1"))
	float InactiveOpacity = 0.35f;

	/** 활성 상태의 투명도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI", meta = (ClampMin = "0", ClampMax = "1"))
	float ActiveOpacity = 1.f;

	/** 1회용 아이템(등잔 등)이 1개뿐일 때는 숫자를 숨김. 2개부터 "2", "3" … 으로 표시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI")
	bool bHideCountWhenOne = true;

	/** 횟수 제한 없는 아이템(등불 등)에 표시할 글자. 비워 두면 표시 안 함 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI")
	FText UnlimitedText;

	/** 이 칸의 아이템 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI")
	FName ItemId;

	/** 이 아이템의 1개당 최대 횟수 (DataTable 의 Max Uses) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI")
	int32 MaxUsesPerItem = 0;

	/** 아이템 정보로 칸 초기화 (아이템 창이 자동 호출) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void SetupSlot(FName InItemId, const FChukItemData& Data);

	/** 상태 갱신. Remaining = -1 이면 무제한 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void UpdateSlot(int32 Remaining, bool bActive);

	/** 손에 든 칸인지 표시 (아이템 창이 자동 호출) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void SetHeld(bool bHeld);

	/** 상태가 바뀔 때 (추가 연출용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|UI")
	void OnSlotStateChanged(bool bActive, int32 Remaining);

	/** 손에 든 칸이 되거나 풀릴 때 (추가 연출용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|UI")
	void OnHeldStateChanged(bool bHeld);
};

/**
 * 아이템 창 (여러 칸을 담는 바). 인벤토리 이벤트를 받아 자동 갱신.
 *
 * ■ 위젯 블루프린트 만드는 법 (예: UI_아이템창)
 *   1. Widget Blueprint 생성 → 부모 클래스 "ChukItemBarWidget"
 *   2. Designer 에 Horizontal Box(또는 Vertical Box / Wrap Box)를 놓고 이름 "item_slots" (필수)
 *   3. Class Defaults 의 Slot Widget Class 에 UI_아이템칸 지정
 *   칸 순서 = 인벤토리에 들어온 순서 (시작 아이템 → 주운 순서). 숫자 키 1, 2, 3… 도 이 순서를 따른다.
 *   쥐 사체·제웅처럼 즉시 적용 아이템은 칸이 없다.
 */
UCLASS(Abstract)
class GAME_API UChukItemBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 슬롯을 담을 패널 (이름 "item_slots" 필수) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UPanelWidget> item_slots;

	/** 칸으로 쓸 위젯 블루프린트 (UI_아이템칸) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI")
	TSubclassOf<UChukItemSlotWidget> SlotWidgetClass;

	/** 표시할 인벤토리 연결 (HUD 가 자동 호출) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void BindToInventory(UChukInventoryComponent* Inventory);

private:
	TWeakObjectPtr<UChukInventoryComponent> BoundInventory;

	UPROPERTY()
	TMap<FName, TObjectPtr<UChukItemSlotWidget>> SlotWidgets;

	UChukItemSlotWidget* FindOrCreateSlot(FName ItemId);
	void RefreshSlot(FName ItemId);

	UFUNCTION()
	void HandleItemAdded(FName ItemId);

	UFUNCTION()
	void HandleItemUsesChanged(FName ItemId, int32 Remaining);

	UFUNCTION()
	void HandleHeldItemChanged(FName ItemId);
};
