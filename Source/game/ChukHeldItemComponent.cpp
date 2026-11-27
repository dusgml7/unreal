// 축영(逐影) - 손에 든 아이템 3D 모델 표시 컴포넌트

#include "ChukHeldItemComponent.h"

#include "ChukInventoryComponent.h"
#include "ChukItemTypes.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UChukHeldItemComponent::UChukHeldItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);

	// 손 뼈에 붙은 크기(캐릭터 임포트 방식에 따라 100배 등)를 무시하고, Held Offset Scale 을 그대로 실제 크기로 쓴다
	SetUsingAbsoluteScale(true);
}

void UChukHeldItemComponent::BeginPlay()
{
	Super::BeginPlay();

	Inventory = GetOwner() ? GetOwner()->FindComponentByClass<UChukInventoryComponent>() : nullptr;
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChukHeldItem] Chuk Inventory component not found on %s"), *GetNameSafe(GetOwner()));
		SetStaticMesh(nullptr);
		return;
	}

	Inventory->OnHeldItemChanged.AddDynamic(this, &UChukHeldItemComponent::HandleHeldItemChanged);
	Inventory->OnItemUsed.AddDynamic(this, &UChukHeldItemComponent::HandleItemUsed);
	RefreshHeldItem();
}

void UChukHeldItemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyAllHeldActors();
	Super::EndPlay(EndPlayReason);
}

void UChukHeldItemComponent::HandleHeldItemChanged(FName /*ItemId*/)
{
	RefreshHeldItem();
}

void UChukHeldItemComponent::RefreshHeldItem()
{
	const FName HeldId = Inventory ? Inventory->GetHeldItem() : NAME_None;

	FChukItemData Data;
	const bool bHasData = !HeldId.IsNone() && Inventory->GetItemData(HeldId, Data);

	HideHeldActor();
	FTransform Offset = bHasData ? Data.HeldOffset : FTransform::Identity;

	// 손 뼈의 크기만큼 위치가 같이 커지지 않게 나눠 준다 → Location 1 = 실제 1cm
	if (USceneComponent* Parent = GetAttachParent())
	{
		const FVector ParentScale = Parent->GetSocketTransform(GetAttachSocketName()).GetScale3D();
		FVector Loc = Offset.GetLocation();
		Loc.X = FMath::IsNearlyZero(ParentScale.X) ? Loc.X : Loc.X / ParentScale.X;
		Loc.Y = FMath::IsNearlyZero(ParentScale.Y) ? Loc.Y : Loc.Y / ParentScale.Y;
		Loc.Z = FMath::IsNearlyZero(ParentScale.Z) ? Loc.Z : Loc.Z / ParentScale.Z;
		Offset.SetLocation(Loc);
	}
	SetRelativeTransform(Offset);

	if (bHasData && Data.HeldActorClass)
	{
		SetStaticMesh(nullptr);

		// 전에 만든 것이 있으면 다시 보여 준다 (떨어진 방울 같은 상태가 그대로 남음)
		TObjectPtr<AActor>* Cached = HeldActorCache.Find(HeldId);
		if (Cached && IsValid(*Cached) && (*Cached)->IsA(Data.HeldActorClass))
		{
			HeldActor = *Cached;
			HeldActor->SetActorHiddenInGame(false);
		}
		else
		{
			// 처음 들 때만 만들어서 손에 붙이고 충돌·물리를 끈다 (손에서 떨어지거나 다시 주워지지 않게)
			FActorSpawnParameters Params;
			Params.Owner = GetOwner();
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			HeldActor = GetWorld() ? GetWorld()->SpawnActor<AActor>(Data.HeldActorClass, GetComponentTransform(), Params) : nullptr;
			if (HeldActor)
			{
				// 부품마다 충돌을 끈다 (액터 전체를 끄면, 블루프린트가 나중에 떨어뜨린 부품도 바닥을 뚫고 떨어짐)
				TArray<UPrimitiveComponent*> Prims;
				HeldActor->GetComponents<UPrimitiveComponent>(Prims);
				for (UPrimitiveComponent* Prim : Prims)
				{
					Prim->SetSimulatePhysics(false);
					Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					Prim->SetGenerateOverlapEvents(false);
				}
				// 크기까지 맞춘다 → DT 의 Held Offset Scale 이 블루프린트 모델에도 적용됨
				HeldActor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
				HeldActorCache.Add(HeldId, HeldActor);
			}
		}
		// TODO(기획 확인): 방울을 다시 주워 횟수가 늘어나도 떨어진 방울은 다시 붙지 않음
	}
	else
	{
		SetStaticMesh(bHasData ? Data.HeldMesh.Get() : nullptr);
	}

	if (ShownItemId != HeldId)
	{
		ShownItemId = HeldId;
		OnHeldMeshChanged(ShownItemId);
	}
}

void UChukHeldItemComponent::HandleItemUsed(FName ItemId)
{
	if (!HeldActor || ItemId != ShownItemId || !Inventory)
	{
		return;
	}

	FChukItemData Data;
	if (!Inventory->GetItemData(ItemId, Data) || Data.HeldUseEventName.IsNone())
	{
		return;
	}

	UFunction* Func = HeldActor->FindFunction(Data.HeldUseEventName);
	if (!Func || Func->ParmsSize > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChukHeldItem] %s has no event '%s' without inputs"), *GetNameSafe(HeldActor), *Data.HeldUseEventName.ToString());
		return;
	}
	HeldActor->ProcessEvent(Func, nullptr);

}

void UChukHeldItemComponent::HideHeldActor()
{
	if (HeldActor)
	{
		HeldActor->SetActorHiddenInGame(true);
		HeldActor = nullptr;
	}
}

void UChukHeldItemComponent::DestroyAllHeldActors()
{
	for (TPair<FName, TObjectPtr<AActor>>& Pair : HeldActorCache)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}
	HeldActorCache.Empty();
	HeldActor = nullptr;
}
