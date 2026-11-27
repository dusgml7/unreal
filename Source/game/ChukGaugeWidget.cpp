// 축영(逐影) - 게이지 창 위젯 (부모 클래스)

#include "ChukGaugeWidget.h"

#include "ChukGaugeComponent.h"
#include "Components/ProgressBar.h"

void UChukGaugeWidget::BindToGauge(UChukGaugeComponent* Gauge)
{
	if (UChukGaugeComponent* Old = BoundGauge.Get())
	{
		Old->OnFearChanged.RemoveDynamic(this, &UChukGaugeWidget::HandleFearChanged);
		Old->OnSanityChanged.RemoveDynamic(this, &UChukGaugeWidget::HandleSanityChanged);
		Old->OnFearStageChanged.RemoveDynamic(this, &UChukGaugeWidget::HandleFearStageChanged);
		Old->OnSanityStageChanged.RemoveDynamic(this, &UChukGaugeWidget::HandleSanityStageChanged);
	}

	BoundGauge = Gauge;
	if (!Gauge)
	{
		return;
	}

	Gauge->OnFearChanged.AddDynamic(this, &UChukGaugeWidget::HandleFearChanged);
	Gauge->OnSanityChanged.AddDynamic(this, &UChukGaugeWidget::HandleSanityChanged);
	Gauge->OnFearStageChanged.AddDynamic(this, &UChukGaugeWidget::HandleFearStageChanged);
	Gauge->OnSanityStageChanged.AddDynamic(this, &UChukGaugeWidget::HandleSanityStageChanged);

	// 현재 값으로 한 번 갱신
	HandleFearChanged(Gauge->GetFear());
	HandleSanityChanged(Gauge->GetSanity());
	HandleFearStageChanged(Gauge->GetFearStage());
	HandleSanityStageChanged(Gauge->GetSanityStage());
}

void UChukGaugeWidget::HandleFearChanged(float NewValue)
{
	if (gauge_fear)
	{
		gauge_fear->SetPercent(NewValue / UChukGaugeComponent::MaxGaugeValue);
	}
}

void UChukGaugeWidget::HandleSanityChanged(float NewValue)
{
	if (gauge_mentality)
	{
		gauge_mentality->SetPercent(NewValue / UChukGaugeComponent::MaxGaugeValue);
	}
}

void UChukGaugeWidget::HandleFearStageChanged(int32 Stage)
{
	OnFearStageDisplay(Stage);
}

void UChukGaugeWidget::HandleSanityStageChanged(int32 Stage)
{
	OnSanityStageDisplay(Stage);
}
