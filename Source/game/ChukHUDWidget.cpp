// 축영(逐影) - 메인 HUD 위젯 (부모 클래스)

#include "ChukHUDWidget.h"

#include "ChukGaugeComponent.h"
#include "ChukGaugeWidget.h"
#include "ChukHideComponent.h"
#include "ChukInventoryComponent.h"
#include "ChukItemWidgets.h"
#include "ChukRelicPanelWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void UChukHUDWidget::BindToPawn(APawn* Pawn)
{
	if (ui_gauge)
	{
		ui_gauge->BindToGauge(Pawn ? Pawn->FindComponentByClass<UChukGaugeComponent>() : nullptr);
	}
	if (ui_itembar)
	{
		ui_itembar->BindToInventory(Pawn ? Pawn->FindComponentByClass<UChukInventoryComponent>() : nullptr);
	}

	if (UChukHideComponent* Old = BoundHide.Get())
	{
		Old->OnHideCountChanged.RemoveDynamic(this, &UChukHUDWidget::HandleHideCountChanged);
	}
	UChukHideComponent* Hide = Pawn ? Pawn->FindComponentByClass<UChukHideComponent>() : nullptr;
	BoundHide = Hide;
	if (Hide)
	{
		Hide->OnHideCountChanged.AddDynamic(this, &UChukHUDWidget::HandleHideCountChanged);
		HandleHideCountChanged(Hide->GetRemainingHides());
	}

	// 유품 창은 서브시스템에 스스로 연결되므로 여기서 할 일 없음
	OnBoundToPawn(Pawn);
}

void UChukHUDWidget::HandleHideCountChanged(int32 Remaining)
{
	if (ui_hidecount)
	{
		ui_hidecount->SetText(FText::AsNumber(Remaining));
	}
}
