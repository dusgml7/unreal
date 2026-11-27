// 축영(逐影) - 인벤토리 / 등불 컴포넌트

#include "ChukInventoryComponent.h"

#include "ChukGaugeComponent.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"

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

	if (!SetHeldItem(StartingHeldItem) && Slots.Num() > 0)
	{
		SetHeldItem(Slots[0].ItemId);
	}

	LanternRemainingTime = bLanternLitAtStart ? LanternDuration : 0.f;
}

void UChukInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 기획 확정: 등불은 손에 들고 있을 때만 기름(시간)이 줄어든다. 안 들면 멈춤
	if (LanternRemainingTime > 0.f && IsHoldingLantern())
	{
		LanternRemainingTime -= DeltaTime;
		if (LanternRemainingTime <= 0.f)
		{
			LanternRemainingTime = 0.f;
			OnLanternExtinguished.Broadcast();
		}
	}

	if (bShowDebugText && GEngine)
	{
		FString Text = FString::Printf(TEXT("[Inventory] Held: %s   Lantern: %.0fs (%s)\n"), *HeldItemId.ToString(), LanternRemainingTime,
			IsHoldingLantern() ? TEXT("light ON") : TEXT("light OFF"));
		for (const FChukInventorySlot& S : Slots)
		{
			const int32 Remaining = GetRemainingUses(S.ItemId);
			Text += FString::Printf(TEXT("%s%s: %s   "), S.ItemId == HeldItemId ? TEXT(">") : TEXT(""),
				*S.ItemId.ToString(), Remaining < 0 ? TEXT("inf") : *FString::FromInt(Remaining));
		}
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 0.f, FColor::Cyan, Text);
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

	// 기획 확정: 등불은 들고 R 을 눌러도 아무 일 없음 (들고 있기만 하면 빛남)
	if (Data->Effect == EChukItemEffect::Lantern)
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

	// 일기: 아이템 창에서 사용하면 다시 읽기
	if (Data->Effect == EChukItemEffect::Diary)
	{
		OnDiaryRead.Broadcast(ItemId);
	}
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

	// 일기: 인벤토리에 넣고, 설정에 따라 주운 순간 바로 읽기
	if (Data->Effect == EChukItemEffect::Diary && bReadDiaryOnPickup)
	{
		OnDiaryRead.Broadcast(ItemId);
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

// ===================== 손에 든 아이템 =====================

bool UChukInventoryComponent::SetHeldItem(FName ItemId)
{
	if (!HasItem(ItemId))
	{
		return false;
	}
	if (HeldItemId != ItemId)
	{
		HeldItemId = ItemId;
		OnHeldItemChanged.Broadcast(HeldItemId);
	}
	return true;
}

bool UChukInventoryComponent::SelectHeldItemByIndex(int32 Index)
{
	return Slots.IsValidIndex(Index) && SetHeldItem(Slots[Index].ItemId);
}

void UChukInventoryComponent::CycleHeldItem(int32 Direction)
{
	if (Slots.Num() == 0 || Direction == 0)
	{
		return;
	}
	// TODO(기획 확인): 다 쓴(횟수 0) 아이템도 휠로 고를 수 있는지 (지금은 고를 수 있고, 사용만 안 됨)
	const int32 Current = Slots.IndexOfByPredicate([this](const FChukInventorySlot& S) { return S.ItemId == HeldItemId; });
	const int32 Step = Direction > 0 ? 1 : -1;
	const int32 Next = Current == INDEX_NONE ? 0 : (Current + Step + Slots.Num()) % Slots.Num();
	SetHeldItem(Slots[Next].ItemId);
}

bool UChukInventoryComponent::UseHeldItem()
{
	return !HeldItemId.IsNone() && UseItem(HeldItemId);
}

// ===================== 등불 =====================

float UChukInventoryComponent::GetLanternIntensity01() const
{
	// TODO(기획 확인): 밝기가 줄어드는 방식 (지금은 시간에 비례해 일정하게 줄어듦)
	return LanternDuration > 0.f ? FMath::Clamp(LanternRemainingTime / LanternDuration, 0.f, 1.f) : 0.f;
}

bool UChukInventoryComponent::IsHoldingLantern() const
{
	const FChukItemData* Data = FindItemData(HeldItemId);
	return Data && Data->Effect == EChukItemEffect::Lantern;
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
