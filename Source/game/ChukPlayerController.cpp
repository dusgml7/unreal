// 축영(逐影) - 플레이어 컨트롤러 (HUD 생성 + 아이템·달리기 키)

#include "ChukPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "ChukGaugeComponent.h"
#include "ChukHUDWidget.h"
#include "ChukInventoryComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

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

void AChukPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!InputComponent)
	{
		return;
	}

	BindChukKey(UseItemKey, IE_Pressed, [this]()
	{
		if (UChukInventoryComponent* Inventory = GetPawnInventory())
		{
			Inventory->UseHeldItem();
		}
	});

	BindChukKey(NextItemKey, IE_Pressed, [this]()
	{
		if (UChukInventoryComponent* Inventory = GetPawnInventory())
		{
			Inventory->CycleHeldItem(1);
		}
	});

	BindChukKey(PreviousItemKey, IE_Pressed, [this]()
	{
		if (UChukInventoryComponent* Inventory = GetPawnInventory())
		{
			Inventory->CycleHeldItem(-1);
		}
	});

	for (int32 Index = 0; Index < ItemSlotKeys.Num(); ++Index)
	{
		BindChukKey(ItemSlotKeys[Index], IE_Pressed, [this, Index]()
		{
			if (UChukInventoryComponent* Inventory = GetPawnInventory())
			{
				Inventory->SelectHeldItemByIndex(Index);
			}
		});
	}

	BindChukKey(RunKey, IE_Pressed, [this]()
	{
		if (UChukGaugeComponent* Gauge = GetPawnGauge())
		{
			Gauge->SetRunning(true);
		}
	});

	BindChukKey(RunKey, IE_Released, [this]()
	{
		if (UChukGaugeComponent* Gauge = GetPawnGauge())
		{
			Gauge->SetRunning(false);
		}
	});

	BindChukKey(CrouchKey, IE_Pressed, [this]()
	{
		const ACharacter* PawnCharacter = Cast<ACharacter>(GetPawn());
		SetPawnCrouched(bToggleCrouch ? !(PawnCharacter && PawnCharacter->bIsCrouched) : true);
	});

	BindChukKey(CrouchKey, IE_Released, [this]()
	{
		if (!bToggleCrouch)
		{
			SetPawnCrouched(false);
		}
	});
}

void AChukPlayerController::SetPawnCrouched(bool bCrouch)
{
	ACharacter* PawnCharacter = Cast<ACharacter>(GetPawn());
	UCharacterMovementComponent* Move = PawnCharacter ? PawnCharacter->GetCharacterMovement() : nullptr;
	if (!Move)
	{
		return;
	}

	if (!bCrouch)
	{
		PawnCharacter->UnCrouch();
		return;
	}

	// 숨어 있는 중(이동 꺼짐)에는 웅크리지 않음
	if (Move->MovementMode == MOVE_None)
	{
		return;
	}
	Move->GetNavAgentPropertiesRef().bCanCrouch = true; // 캐릭터 설정에서 Can Crouch 를 안 켜도 되게
	PawnCharacter->Crouch();
}

void AChukPlayerController::BindChukKey(const FKey& Key, EInputEvent Event, TFunction<void()> Action)
{
	if (!Key.IsValid())
	{
		return;
	}
	FInputKeyBinding Binding(FInputChord(Key), Event);
	Binding.bConsumeInput = false; // 캐릭터 쪽 다른 입력을 막지 않음
	Binding.KeyDelegate.GetDelegateForManualSet().BindLambda(MoveTemp(Action));
	InputComponent->KeyBindings.Add(Binding);
}

UChukInventoryComponent* AChukPlayerController::GetPawnInventory() const
{
	return GetPawn() ? GetPawn()->FindComponentByClass<UChukInventoryComponent>() : nullptr;
}

UChukGaugeComponent* AChukPlayerController::GetPawnGauge() const
{
	return GetPawn() ? GetPawn()->FindComponentByClass<UChukGaugeComponent>() : nullptr;
}
