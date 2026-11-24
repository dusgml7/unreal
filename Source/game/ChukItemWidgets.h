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
 * ■ 위젯 블루프린트 만드는 법 (예: WBP_ItemSlot)
 *   1. Widget Blueprint 생성 → 부모 클래스 "ChukItemSlotWidget"
 *   2. Designer 에 Image 를 놓고 이름 "IconImage" (필수)
 *   3. Text 를 놓고 이름 "UsesText" (선택 — 남은 횟수 표시)
 *   4. 비활성일 때 투명도는 Class Defaults 의 Inactive Opacity 로 조절
 *   5. 다른 표현(회색 처리 등)이 필요하면 Event On Slot State Changed 구현
 */
UCLASS(Abstract)
class GAME_API UChukItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 아이콘 (이름 "IconImage" 필수) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	/** 남은 횟수 글자 (이름 "UsesText", 없어도 됨) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> UsesText;

	/** 비활성(못 가짐 / 다 씀) 상태의 투명도 */
	// TODO(기획 확인): 비활성 표시 방식 (피그마)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI", meta = (ClampMin = "0", ClampMax = "1"))
	float InactiveOpacity = 0.35f;

	/** 활성 상태의 투명도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI", meta = (ClampMin = "0", ClampMax = "1"))
	float ActiveOpacity = 1.f;

	/** 횟수 제한 없는 아이템(등불 등)에 표시할 글자. 비워 두면 표시 안 함 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI")
	FText UnlimitedText;

	/** 이 칸의 아이템 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI")
	FName ItemId;

	/** 아이템 정보로 칸 초기화 (아이템 창이 자동 호출) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void SetupSlot(FName InItemId, const FChukItemData& Data);

	/** 상태 갱신. Remaining = -1 이면 무제한 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|UI")
	void UpdateSlot(int32 Remaining, bool bActive);

	/** 상태가 바뀔 때 (추가 연출용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|UI")
	void OnSlotStateChanged(bool bActive, int32 Remaining);
};

/**
 * 아이템 창 (여러 칸을 담는 바). 인벤토리 이벤트를 받아 자동 갱신.
 *
 * ■ 위젯 블루프린트 만드는 법 (예: WBP_ItemBar)
 *   1. Widget Blueprint 생성 → 부모 클래스 "ChukItemBarWidget"
 *   2. Designer 에 Horizontal Box(또는 Vertical Box / Wrap Box)를 놓고 이름 "SlotContainer" (필수)
 *   3. Class Defaults 의 Slot Widget Class 에 WBP_ItemSlot 지정
 *   4. Fixed Item Order: 처음부터 보여 줄 칸 순서 (가지지 않은 아이템은 비활성으로 표시)
 */
UCLASS(Abstract)
class GAME_API UChukItemBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 슬롯을 담을 패널 (이름 "SlotContainer" 필수) */
	UPROPERTY(BlueprintReadOnly, Category = "Chuk|UI", meta = (BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;

	/** 칸으로 쓸 위젯 블루프린트 (WBP_ItemSlot) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI")
	TSubclassOf<UChukItemSlotWidget> SlotWidgetClass;

	/** 처음부터 이 순서로 칸을 만든다. 여기에 없는 아이템은 얻을 때 뒤에 추가 */
	// TODO(기획 확인): 아이템 창에 보일 아이템과 순서 (피그마)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI")
	TArray<FName> FixedItemOrder = { TEXT("Fan"), TEXT("Bell"), TEXT("Talisman"), TEXT("Lantern"), TEXT("OilLamp") };

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
};
