// 축영(逐影) - 아이템 슬롯 / 아이템 창 위젯 (부모 클래스)

#include "ChukItemWidgets.h"

#include "ChukInventoryComponent.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

// ===================== UChukItemSlotWidget =====================

void UChukItemSlotWidget::SetupSlot(FName InItemId, const FChukItemData& Data)
{
	ItemId = InItemId;
	MaxUsesPerItem = Data.MaxUses;
	if (item_icon && Data.Icon)
	{
		item_icon->SetBrushFromTexture(Data.Icon);
	}
	SetToolTipText(Data.DisplayName);
}

void UChukItemSlotWidget::UpdateSlot(int32 Remaining, bool bActive)
{
	SetRenderOpacity(bActive ? ActiveOpacity : InactiveOpacity);

	if (item_count)
	{
		// 1회용 아이템(등잔)은 남은 횟수 = 가진 개수. 1개일 때는 숫자 숨김
		const bool bHide = bHideCountWhenOne && MaxUsesPerItem == 1 && Remaining == 1;
		item_count->SetText(Remaining < 0 ? UnlimitedText : (bHide ? FText::GetEmpty() : FText::AsNumber(Remaining)));
	}
	OnSlotStateChanged(bActive, Remaining);
}

void UChukItemSlotWidget::SetHeld(bool bHeld)
{
	if (item_held)
	{
		item_held->SetVisibility(bHeld ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
	OnHeldStateChanged(bHeld);
}

// ===================== UChukItemBarWidget =====================

void UChukItemBarWidget::BindToInventory(UChukInventoryComponent* Inventory)
{
	if (UChukInventoryComponent* Old = BoundInventory.Get())
	{
		Old->OnItemAdded.RemoveDynamic(this, &UChukItemBarWidget::HandleItemAdded);
		Old->OnItemUsesChanged.RemoveDynamic(this, &UChukItemBarWidget::HandleItemUsesChanged);
		Old->OnHeldItemChanged.RemoveDynamic(this, &UChukItemBarWidget::HandleHeldItemChanged);
	}

	BoundInventory = Inventory;
	SlotWidgets.Reset();
	if (item_slots)
	{
		item_slots->ClearChildren();
	}
	if (!Inventory)
	{
		return;
	}

	Inventory->OnItemAdded.AddDynamic(this, &UChukItemBarWidget::HandleItemAdded);
	Inventory->OnItemUsesChanged.AddDynamic(this, &UChukItemBarWidget::HandleItemUsesChanged);
	Inventory->OnHeldItemChanged.AddDynamic(this, &UChukItemBarWidget::HandleHeldItemChanged);

	// 인벤토리에 들어온 순서대로 칸 생성
	for (const FChukInventorySlot& OwnedSlot : Inventory->GetSlots())
	{
		RefreshSlot(OwnedSlot.ItemId);
	}
	HandleHeldItemChanged(Inventory->GetHeldItem());
}

UChukItemSlotWidget* UChukItemBarWidget::FindOrCreateSlot(FName ItemId)
{
	if (TObjectPtr<UChukItemSlotWidget>* Found = SlotWidgets.Find(ItemId))
	{
		return *Found;
	}

	UChukInventoryComponent* Inventory = BoundInventory.Get();
	FChukItemData Data;
	if (!SlotWidgetClass || !item_slots || !Inventory || !Inventory->GetItemData(ItemId, Data))
	{
		return nullptr;
	}
	// 줍는 즉시 적용 아이템(쥐 사체, 제웅)은 인벤토리에 남지 않으므로 칸을 만들지 않음
	if (Data.bApplyOnPickup)
	{
		return nullptr;
	}

	UChukItemSlotWidget* NewSlot = CreateWidget<UChukItemSlotWidget>(this, SlotWidgetClass);
	if (!NewSlot)
	{
		return nullptr;
	}
	NewSlot->SetupSlot(ItemId, Data);
	NewSlot->SetHeld(Inventory->GetHeldItem() == ItemId);
	item_slots->AddChild(NewSlot);
	SlotWidgets.Add(ItemId, NewSlot);
	return NewSlot;
}

void UChukItemBarWidget::RefreshSlot(FName ItemId)
{
	UChukInventoryComponent* Inventory = BoundInventory.Get();
	UChukItemSlotWidget* SlotWidget = FindOrCreateSlot(ItemId);
	if (!Inventory || !SlotWidget)
	{
		return;
	}
	const int32 Remaining = Inventory->HasItem(ItemId) ? Inventory->GetRemainingUses(ItemId) : 0;
	SlotWidget->UpdateSlot(Remaining, Inventory->IsItemUsable(ItemId));
}

void UChukItemBarWidget::HandleItemAdded(FName ItemId)
{
	RefreshSlot(ItemId);
}

void UChukItemBarWidget::HandleItemUsesChanged(FName ItemId, int32 Remaining)
{
	RefreshSlot(ItemId);
}

void UChukItemBarWidget::HandleHeldItemChanged(FName ItemId)
{
	for (const TPair<FName, TObjectPtr<UChukItemSlotWidget>>& Pair : SlotWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->SetHeld(Pair.Key == ItemId);
		}
	}
}
