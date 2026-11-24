// 축영(逐影) - 유품 창 위젯 (부모 클래스, 화면 오른쪽 사이드)

#include "ChukRelicPanelWidget.h"

#include "Components/Image.h"
#include "Engine/World.h"

void UChukRelicPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UChukRelicSubsystem* Relics = GetWorld() ? GetWorld()->GetSubsystem<UChukRelicSubsystem>() : nullptr)
	{
		Relics->OnRelicStateChanged.AddUniqueDynamic(this, &UChukRelicPanelWidget::HandleRelicStateChanged);
	}
	RefreshAll();
}

void UChukRelicPanelWidget::NativeDestruct()
{
	if (UChukRelicSubsystem* Relics = GetWorld() ? GetWorld()->GetSubsystem<UChukRelicSubsystem>() : nullptr)
	{
		Relics->OnRelicStateChanged.RemoveDynamic(this, &UChukRelicPanelWidget::HandleRelicStateChanged);
	}
	Super::NativeDestruct();
}

void UChukRelicPanelWidget::RefreshAll()
{
	const UChukRelicSubsystem* Relics = GetWorld() ? GetWorld()->GetSubsystem<UChukRelicSubsystem>() : nullptr;
	for (int32 i = 0; i < UChukRelicSubsystem::RelicCount; ++i)
	{
		const EChukRelicType Relic = static_cast<EChukRelicType>(i);
		HandleRelicStateChanged(Relic, Relics ? Relics->GetRelicState(Relic) : EChukRelicState::NotFound);
	}
}

UImage* UChukRelicPanelWidget::GetImageFor(EChukRelicType Relic) const
{
	switch (Relic)
	{
	case EChukRelicType::Kkotsin:	return KkotsinImage;
	case EChukRelicType::Norigae:	return NorigaeImage;
	case EChukRelicType::Daenggi:	return DaenggiImage;
	case EChukRelicType::Binyeo:	return BinyeoImage;
	default:						return nullptr;
	}
}

void UChukRelicPanelWidget::HandleRelicStateChanged(EChukRelicType Relic, EChukRelicState NewState)
{
	if (UImage* Image = GetImageFor(Relic))
	{
		float Opacity = NotFoundOpacity;
		switch (NewState)
		{
		case EChukRelicState::Found:	Opacity = FoundOpacity;		break;
		case EChukRelicState::Burned:	Opacity = BurnedOpacity;	break;
		default:						Opacity = NotFoundOpacity;	break;
		}
		Image->SetRenderOpacity(Opacity);
	}
	OnRelicDisplayChanged(Relic, NewState);
}
