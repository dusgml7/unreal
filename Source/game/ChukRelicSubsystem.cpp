// 축영(逐影) - 유품 진행 관리 (월드 서브시스템)

#include "ChukRelicSubsystem.h"

#include "ChukHideComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

bool UChukRelicSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// 실제 게임·PIE 에서만 생성 (에디터 미리보기 창 등에서는 만들지 않음)
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UChukRelicSubsystem::FindRelic(EChukRelicType Relic)
{
	const int32 Index = static_cast<int32>(Relic);
	if (!ensure(Index >= 0 && Index < RelicCount) || States[Index] != EChukRelicState::NotFound)
	{
		return false;
	}

	States[Index] = EChukRelicState::Found;
	OnRelicStateChanged.Broadcast(Relic, States[Index]);

	ResetPlayerHideCount();

	const int32 Level = GetDifficultyLevel();
	OnDifficultyIncreased.Broadcast(Level);

	if (GetFoundCount() >= RelicCount)
	{
		OnAllRelicsFound.Broadcast();
	}
	return true;
}

bool UChukRelicSubsystem::BurnRelic(EChukRelicType Relic)
{
	const int32 Index = static_cast<int32>(Relic);
	if (!ensure(Index >= 0 && Index < RelicCount) || States[Index] != EChukRelicState::Found)
	{
		return false;
	}

	States[Index] = EChukRelicState::Burned;
	OnRelicStateChanged.Broadcast(Relic, States[Index]);

	if (GetBurnedCount() >= RelicCount)
	{
		OnAllRelicsBurned.Broadcast();
	}
	return true;
}

EChukRelicState UChukRelicSubsystem::GetRelicState(EChukRelicType Relic) const
{
	const int32 Index = static_cast<int32>(Relic);
	return (Index >= 0 && Index < RelicCount) ? States[Index] : EChukRelicState::NotFound;
}

int32 UChukRelicSubsystem::GetFoundCount() const
{
	int32 Count = 0;
	for (const EChukRelicState State : States)
	{
		if (State != EChukRelicState::NotFound)
		{
			++Count;
		}
	}
	return Count;
}

int32 UChukRelicSubsystem::GetBurnedCount() const
{
	int32 Count = 0;
	for (const EChukRelicState State : States)
	{
		if (State == EChukRelicState::Burned)
		{
			++Count;
		}
	}
	return Count;
}

void UChukRelicSubsystem::SetRelicState(EChukRelicType Relic, EChukRelicState NewState)
{
	const int32 Index = static_cast<int32>(Relic);
	if (Index < 0 || Index >= RelicCount || States[Index] == NewState)
	{
		return;
	}
	States[Index] = NewState;
	OnRelicStateChanged.Broadcast(Relic, NewState);
}

void UChukRelicSubsystem::ResetPlayerHideCount()
{
	if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UChukHideComponent* Hide = Player->FindComponentByClass<UChukHideComponent>())
		{
			Hide->ResetHideCount();
		}
	}
}
