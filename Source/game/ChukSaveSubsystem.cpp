// 축영(逐影) - 세이브 포인트 저장 / 불러오기 (월드 서브시스템)

#include "ChukSaveSubsystem.h"

#include "ChukGaugeComponent.h"
#include "ChukHideComponent.h"
#include "ChukHideSpot.h"
#include "ChukInventoryComponent.h"
#include "ChukRelicSubsystem.h"
#include "ChukSaveGame.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

const FString UChukSaveSubsystem::SaveSlotName = TEXT("ChukSavePoint");

namespace
{
	// 맵을 다시 열 때 붙이는 옵션. 새 맵에서 이 옵션을 보고 세이브를 적용한다
	const TCHAR* LoadOption = TEXT("ChukLoadSave");
}

bool UChukSaveSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UChukSaveSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UChukRelicSubsystem* Relics = InWorld.GetSubsystem<UChukRelicSubsystem>();
	if (Relics)
	{
		Relics->OnAllRelicsFound.AddDynamic(this, &UChukSaveSubsystem::HandleAllRelicsFound);
	}

	// "세이브에서 다시 시작" 으로 열린 맵이면 저장 값 적용
	if (!InWorld.URL.HasOption(LoadOption))
	{
		return;
	}
	PendingLoad = Cast<UChukSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	if (!PendingLoad)
	{
		return;
	}

	// 유품 상태는 바로 적용 (이미 찾은 유품 액터가 레벨에서 치워지도록)
	if (Relics)
	{
		for (int32 i = 0; i < PendingLoad->RelicStates.Num() && i < UChukRelicSubsystem::RelicCount; ++i)
		{
			Relics->SetRelicState(static_cast<EChukRelicType>(i), PendingLoad->RelicStates[i]);
		}
	}
	NightTimeElapsed = PendingLoad->NightTimeElapsed;

	// 플레이어 컴포넌트들의 BeginPlay(시작 아이템 지급 등)가 끝난 다음 프레임에 덮어쓴다
	InWorld.GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UChukSaveSubsystem::ApplyPendingLoadToPlayer));
}

void UChukSaveSubsystem::HandleAllRelicsFound()
{
	if (bAutoSaveOnAllRelicsFound)
	{
		SaveToSavePoint();
	}
}

bool UChukSaveSubsystem::SaveToSavePoint()
{
	UChukSaveGame* Save = Cast<UChukSaveGame>(UGameplayStatics::CreateSaveGameObject(UChukSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}

	if (const UChukRelicSubsystem* Relics = GetWorld()->GetSubsystem<UChukRelicSubsystem>())
	{
		for (int32 i = 0; i < UChukRelicSubsystem::RelicCount; ++i)
		{
			Save->RelicStates.Add(Relics->GetRelicState(static_cast<EChukRelicType>(i)));
		}
	}

	if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Save->PlayerTransform = Player->GetActorTransform();
		if (const AController* Controller = Player->GetController())
		{
			Save->PlayerControlRotation = Controller->GetControlRotation();
		}

		if (const UChukGaugeComponent* Gauge = Player->FindComponentByClass<UChukGaugeComponent>())
		{
			Save->Fear = Gauge->GetFear();
			Save->Sanity = Gauge->GetSanity();
		}
		if (const UChukInventoryComponent* Inventory = Player->FindComponentByClass<UChukInventoryComponent>())
		{
			Save->InventorySlots = Inventory->GetSlots();
			Save->LanternRemainingTime = Inventory->GetLanternRemainingTime();
		}
		if (const UChukHideComponent* Hide = Player->FindComponentByClass<UChukHideComponent>())
		{
			Save->RemainingHides = Hide->GetRemainingHides();
			// 숨어 있는 중에 저장되면 불러올 때 은신처 밖(나오는 위치)에서 시작
			if (const AChukHideSpot* Spot = Hide->GetCurrentHideSpot())
			{
				Save->PlayerTransform.SetLocation(Spot->GetExitLocation());
			}
		}
	}

	Save->NightTimeElapsed = NightTimeElapsed;

	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, SaveUserIndex);
	if (bSaved)
	{
		OnSavePointSaved.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChukSave] SaveGameToSlot failed"));
	}
	return bSaved;
}

bool UChukSaveSubsystem::LoadFromSavePoint()
{
	if (!HasSavePoint())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChukSave] No save point to load"));
		return false;
	}
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*LevelName), true, LoadOption);
	return true;
}

bool UChukSaveSubsystem::HasSavePoint() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex);
}

void UChukSaveSubsystem::DeleteSavePoint()
{
	UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveUserIndex);
}

void UChukSaveSubsystem::ApplyPendingLoadToPlayer()
{
	UChukSaveGame* Save = PendingLoad;
	PendingLoad = nullptr;
	if (!Save)
	{
		return;
	}

	if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Player->SetActorTransform(Save->PlayerTransform, false, nullptr, ETeleportType::TeleportPhysics);
		if (AController* Controller = Player->GetController())
		{
			Controller->SetControlRotation(Save->PlayerControlRotation);
		}

		if (UChukGaugeComponent* Gauge = Player->FindComponentByClass<UChukGaugeComponent>())
		{
			Gauge->SetFear(Save->Fear);
			Gauge->SetSanity(Save->Sanity);
		}
		if (UChukInventoryComponent* Inventory = Player->FindComponentByClass<UChukInventoryComponent>())
		{
			for (const FChukInventorySlot& Slot : Save->InventorySlots)
			{
				Inventory->SetRemainingUses(Slot.ItemId, Slot.RemainingUses);
			}
			Inventory->SetLanternRemainingTime(Save->LanternRemainingTime);
		}
		if (UChukHideComponent* Hide = Player->FindComponentByClass<UChukHideComponent>())
		{
			Hide->SetRemainingHides(Save->RemainingHides);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChukSave] Player pawn not found when applying save"));
	}

	OnSavePointLoaded.Broadcast();
}
