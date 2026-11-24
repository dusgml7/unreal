// 축영(逐影) - 플레이어 컨트롤러 (HUD 생성)

#include "ChukPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "ChukHUDWidget.h"

void AChukPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController() || !HUDWidgetClass)
	{
		return;
	}

	HUDWidget = CreateWidget<UChukHUDWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
		HUDWidget->BindToPawn(GetPawn());
	}
}

void AChukPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// BeginPlay 이후에 캐릭터를 조종하게 되는 경우 다시 연결
	if (HUDWidget)
	{
		HUDWidget->BindToPawn(InPawn);
	}
}
