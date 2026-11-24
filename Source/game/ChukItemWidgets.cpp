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
	if (IconImage && Data.Icon)
	{
		IconImage->SetBrushFromTexture(Data.Icon);
	}
	SetToolTipText(Data.DisplayName);
}

void UChukItemSlotWidget::UpdateSlot(int32 Remaining, bool bActive)
{
	SetRenderOpacity(bActive ? ActiveOpacity : InactiveOpacity);

	if (UsesText)
	{
		UsesText->SetText(Remaining < 0 ? UnlimitedText : FText::AsNumber(Remaining));
	}
	OnSlotStateChanged(bActive, Remaining);
}

// ===================== UChukItemBarWidget =====================

void UChukItemBarWidget::BindToInventory(UChukInventoryComponent* Inventory)
{
	if (UChukInventoryComponent* Old = BoundInventory.Get())
	{
		Old->OnItemAdded.RemoveDynamic(this, &UChukItemBarWidget::HandleItemAdded);
		Old->OnItemUsesChanged.RemoveDynamic(this, &UChukItemBarWidget::HandleItemUsesChanged);
	}

	BoundInventory = Inventory;
	SlotWidgets.Reset();
	if (SlotContainer)
	{
		SlotContainer->ClearChildren();
	}
	if (!Inventory)
	{
		return;
	}

	Inventory->OnItemAdded.AddDynamic(this, &UChukItemBarWidget::HandleItemAdded);
	Inventory->OnItemUsesChanged.AddDynamic(this, &UChukItemBarWidget::HandleItemUsesChanged);

	// 고정 순서 칸 먼저, 그다음 이미 가진 아이템
	for (const FName& Id : FixedItemOrder)
	{
		RefreshSlot(Id);
	}
	for (const FChukInventorySlot& OwnedSlot : Inventory->GetSlots())
	{
		RefreshSlot(OwnedSlot.ItemId);
	}
}

UChukItemSlotWidget* UChukItemBarWidget::FindOrCreateSlot(FName ItemId)
{
	if (TObjectPtr<UChukItemSlotWidget>* Found = SlotWidgets.Find(ItemId))
	{
		return *Found;
	}

	UChukInventoryComponent* Inventory = BoundInventory.Get();
	FChukItemData Data;
	if (!SlotWidgetClass || !SlotContainer || !Inventory || !Inventory->GetItemData(ItemId, Data))
	{
		return nullptr;
	}
	// 일기·줍는 즉시 적용 아이템은 칸을 만들지 않음
	if (Data.Effect == EChukItemEffect::Diary || Data.bApplyOnPickup)
	{
		return nullptr;
	}

	UChukItemSlotWidget* NewSlot = CreateWidget<UChukItemSlotWidget>(this, SlotWidgetClass);
	if (!NewSlot)
	{
		return nullptr;
	}
	NewSlot->SetupSlot(ItemId, Data);
	SlotContainer->AddChild(NewSlot);
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
