// 축영(逐影) - 보는 순간 발동하는 물체 (쥐 사체, 제웅)

#include "ChukSightHazard.h"

#include "ChukInventoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

AChukSightHazard::AChukSightHazard()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // 매 프레임 검사할 필요는 없음
	bDestroyOnPickup = false;
}

void AChukSightHazard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bTriggered)
	{
		return;
	}

	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	APawn* Viewer = PC ? PC->GetPawn() : nullptr;
	if (!Viewer || !IsSeenBy(Viewer))
	{
		SeenTime = 0.f;
		return;
	}

	SeenTime += DeltaSeconds;
	if (SeenTime < SightTimeRequired)
	{
		return;
	}

	UChukInventoryComponent* Inventory = Viewer->FindComponentByClass<UChukInventoryComponent>();
	if (!Inventory || !Inventory->AcquireItem(ItemId))
	{
		return;
	}

	bTriggered = true;
	SetActorTickEnabled(false);
	OnSeenByPlayer(Viewer);
	if (bDestroyAfterSeen)
	{
		Destroy();
	}
}

bool AChukSightHazard::IsSeenBy(const APawn* Viewer) const
{
	const APlayerController* PC = Cast<APlayerController>(Viewer->GetController());
	if (!PC)
	{
		return false;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	PC->GetPlayerViewPoint(EyeLocation, EyeRotation);

	// 물체 모양의 가운데를 본다 (기준점이 바닥에 있으면 바닥에 가려진 것으로 잘못 판정되므로)
	FVector Origin, Extent;
	GetActorBounds(true, Origin, Extent);
	const FVector Target = Extent.IsNearlyZero() ? GetActorLocation() : Origin;

	// 거리는 카메라가 아니라 캐릭터 기준 (3인칭 카메라는 캐릭터 뒤 몇 m 에 있으므로)
	const float Distance = FVector::Dist(Target, Viewer->GetActorLocation());
	if (Distance > SightMaxDistance)
	{
		return false;
	}

	// 화면(카메라)에 걸려 있는지 — 카메라 뒤쪽이면 투영 실패
	FVector2D ScreenPos;
	int32 ViewX = 0, ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	if (ViewX <= 0 || ViewY <= 0 || !PC->ProjectWorldLocationToScreen(Target, ScreenPos))
	{
		return false;
	}
	const float MarginX = ViewX * ScreenEdgeMargin;
	const float MarginY = ViewY * ScreenEdgeMargin;
	if (ScreenPos.X < MarginX || ScreenPos.X > ViewX - MarginX || ScreenPos.Y < MarginY || ScreenPos.Y > ViewY - MarginY)
	{
		return false;
	}

	// 사이에 벽이 있는지 (이 물체 자신에 맞거나 아무것도 안 맞으면 보이는 것)
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ChukSightHazard), false, Viewer);
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, Target, SightTraceChannel, Params))
	{
		return Hit.GetActor() == this;
	}
	return true;
}
