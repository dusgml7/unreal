// 축영(逐影) - 인벤토리 / 등불 컴포넌트

#include "ChukInventoryComponent.h"

#include "ChukGaugeComponent.h"
#include "Engine/DataTable.h"

UChukInventoryComponent::UChukInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UChukInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChukInventory] Item Data Table is not set on %s"), *GetNameSafe(GetOwner()));
	}

	for (const FName& ItemId : StartingItems)
	{
		AcquireItem(ItemId);
	}

	LanternRemainingTime = bLanternLitAtStart ? LanternDuration : 0.f;
}

void UChukInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (LanternRemainingTime > 0.f)
	{
		LanternRemainingTime -= DeltaTime;
		if (LanternRemainingTime <= 0.f)
		{
			LanternRemainingTime = 0.f;
			OnLanternExtinguished.Broadcast();
		}
	}
}

// ===================== 아이템 =====================

bool UChukInventoryComponent::UseItem(FName ItemId)
{
	const FChukItemData* Data = FindItemData(ItemId);
	FChukInventorySlot* Slot = FindSlot(ItemId);
	if (!Data || !Slot)
	{
		return false;
	}

	const bool bUnlimited = Data->MaxUses <= 0;
	if (!bUnlimited && Slot->RemainingUses <= 0)
	{
		return false;
	}

	// 효과가 의미 없는 상황인지 먼저 확인
	if (!bAllowUseWhenNoEffect && !ApplyEffect(*Data, true))
	{
		return false;
	}

	ApplyEffect(*Data, false);

	if (!bUnlimited)
	{
		--Slot->RemainingUses;
		OnItemUsesChanged.Broadcast(ItemId, Slot->RemainingUses);
	}
	OnItemUsed.Broadcast(ItemId);
	return true;
}

bool UChukInventoryComponent::AcquireItem(FName ItemId)
{
	const FChukItemData* Data = FindItemData(ItemId);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChukInventory] Unknown item id: %s"), *ItemId.ToString());
		return false;
	}

	// 줍는 즉시 적용 (쥐 사체, 제웅)
	if (Data->bApplyOnPickup)
	{
		ApplyEffect(*Data, false);
		OnPickupEffectApplied.Broadcast(ItemId);
		return true;
	}

	// 일기: 인벤토리에 넣지 않음
	if (Data->Effect == EChukItemEffect::Diary)
	{
		OnDiaryRead.Broadcast(ItemId);
		return true;
	}

	if (FChukInventorySlot* Existing = FindSlot(ItemId))
	{
		// 이미 있으면 횟수 추가 (무제한 아이템은 변화 없음)
		if (Data->MaxUses > 0)
		{
			Existing->RemainingUses += Data->MaxUses;
			OnItemUsesChanged.Broadcast(ItemId, Existing->RemainingUses);
		}
		return true;
	}

	FChukInventorySlot NewSlot;
	NewSlot.ItemId = ItemId;
	NewSlot.RemainingUses = Data->MaxUses;
	Slots.Add(NewSlot);
	OnItemAdded.Broadcast(ItemId);
	OnItemUsesChanged.Broadcast(ItemId, NewSlot.RemainingUses);
	return true;
}

bool UChukInventoryComponent::HasItem(FName ItemId) const
{
	return FindSlot(ItemId) != nullptr;
}

bool UChukInventoryComponent::IsItemUsable(FName ItemId) const
{
	const FChukItemData* Data = FindItemData(ItemId);
	const FChukInventorySlot* Slot = FindSlot(ItemId);
	if (!Data || !Slot)
	{
		return false;
	}
	return Data->MaxUses <= 0 || Slot->RemainingUses > 0;
}

int32 UChukInventoryComponent::GetRemainingUses(FName ItemId) const
{
	const FChukItemData* Data = FindItemData(ItemId);
	const FChukInventorySlot* Slot = FindSlot(ItemId);
	if (!Data || !Slot)
	{
		return 0;
	}
	return Data->MaxUses <= 0 ? -1 : Slot->RemainingUses;
}

void UChukInventoryComponent::SetRemainingUses(FName ItemId, int32 Remaining)
{
	if (!FindItemData(ItemId))
	{
		return;
	}
	Remaining = FMath::Max(0, Remaining);

	if (FChukInventorySlot* Slot = FindSlot(ItemId))
	{
		Slot->RemainingUses = Remaining;
	}
	else
	{
		FChukInventorySlot NewSlot;
		NewSlot.ItemId = ItemId;
		NewSlot.RemainingUses = Remaining;
		Slots.Add(NewSlot);
		OnItemAdded.Broadcast(ItemId);
	}
	OnItemUsesChanged.Broadcast(ItemId, Remaining);
}

bool UChukInventoryComponent::GetItemData(FName ItemId, FChukItemData& OutData) const
{
	if (const FChukItemData* Data = FindItemData(ItemId))
	{
		OutData = *Data;
		return true;
	}
	return false;
}

// ===================== 등불 =====================

float UChukInventoryComponent::GetLanternIntensity01() const
{
	// TODO(기획 확인): 밝기가 줄어드는 방식 (지금은 시간에 비례해 일정하게 줄어듦)
	return LanternDuration > 0.f ? FMath::Clamp(LanternRemainingTime / LanternDuration, 0.f, 1.f) : 0.f;
}

void UChukInventoryComponent::RelightLantern()
{
	LanternRemainingTime = LanternDuration;
	OnLanternRelit.Broadcast();
}

void UChukInventoryComponent::SetLanternRemainingTime(float Seconds)
{
	LanternRemainingTime = FMath::Clamp(Seconds, 0.f, LanternDuration);
}

// ===================== 내부 =====================

const FChukItemData* UChukInventoryComponent::FindItemData(FName ItemId) const
{
	if (!ItemDataTable || ItemId.IsNone())
	{
		return nullptr;
	}
	return ItemDataTable->FindRow<FChukItemData>(ItemId, TEXT("ChukInventory"), false);
}

FChukInventorySlot* UChukInventoryComponent::FindSlot(FName ItemId)
{
	return Slots.FindByPredicate([ItemId](const FChukInventorySlot& S) { return S.ItemId == ItemId; });
}

const FChukInventorySlot* UChukInventoryComponent::FindSlot(FName ItemId) const
{
	return Slots.FindByPredicate([ItemId](const FChukInventorySlot& S) { return S.ItemId == ItemId; });
}

UChukGaugeComponent* UChukInventoryComponent::GetGauge() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UChukGaugeComponent>() : nullptr;
}

bool UChukInventoryComponent::ApplyEffect(const FChukItemData& Data, bool bCheckOnly)
{
	UChukGaugeComponent* Gauge = GetGauge();

	switch (Data.Effect)
	{
	case EChukItemEffect::RecoverFear:
		if (!Gauge || Gauge->GetFear() <= 0.f) return false;
		if (!bCheckOnly) Gauge->AddFear(-Data.EffectAmount);
		return true;

	case EChukItemEffect::RecoverSanity:
		if (!Gauge || Gauge->GetSanity() >= UChukGaugeComponent::MaxGaugeValue) return false;
		if (!bCheckOnly) Gauge->AddSanity(Data.EffectAmount);
		return true;

	case EChukItemEffect::WorsenSanity:
		if (!Gauge) return false;
		if (!bCheckOnly) Gauge->AddSanity(-Data.EffectAmount);
		return true;

	case EChukItemEffect::RelightLantern:
		if (!bCheckOnly) RelightLantern();
		return true;

	default:
		// 등불·일기·효과 없음: 게이지 변화 없음
		return true;
	}
}
