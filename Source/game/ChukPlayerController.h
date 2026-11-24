// 축영(逐影) - 플레이어 컨트롤러 (HUD 생성)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChukPlayerController.generated.h"

class UChukHUDWidget;

/**
 * 게임 시작 시 HUD 위젯을 만들어 화면에 띄우고, 조종하는 캐릭터와 연결하는 플레이어 컨트롤러.
 *
 * ■ 사용법
 *   1. Content Browser 우클릭 → Blueprint Class → All Classes 에서 "ChukPlayerController" 선택 → BP_ChukPlayerController
 *   2. Class Defaults 의 HUD Widget Class 에 WBP_HUD 지정
 *   3. 게임 모드 블루프린트의 Player Controller Class 를 BP_ChukPlayerController 로 지정
 *      (또는 World Settings → GameMode Override 의 Selected GameMode → Player Controller Class)
 */
UCLASS(Blueprintable)
class GAME_API AChukPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 화면에 띄울 HUD 위젯 블루프린트 (WBP_HUD) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|UI")
	TSubclassOf<UChukHUDWidget> HUDWidgetClass;

	/** 만들어진 HUD 위젯 */
	UFUNCTION(BlueprintPure, Category = "Chuk|UI")
	UChukHUDWidget* GetHUDWidget() const { return HUDWidget; }

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY()
	TObjectPtr<UChukHUDWidget> HUDWidget;
};
