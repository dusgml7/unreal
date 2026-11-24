// 축영(逐影) - 플레이어 상호작용 컴포넌트

#include "ChukInteractComponent.h"

#include "ChukHideComponent.h"
#include "ChukHideSpot.h"
#include "ChukInteractable.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

UChukInteractComponent::UChukInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* UChukInteractComponent::GetInteractTarget() const
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return nullptr;
	}

	// 카메라(눈) 위치에서 바라보는 방향으로
	FVector EyeLocation;
	FRotator EyeRotation;
	const APawn* Pawn = Cast<APawn>(Owner);
	if (Pawn && Pawn->GetController())
	{
		Pawn->GetController()->GetPlayerViewPoint(EyeLocation, EyeRotation);
	}
	else
	{
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	}

	// 3인칭 카메라일 수 있으므로 캐릭터 위치에서 카메라까지 거리만큼 더 멀리 쏜다
	const float CameraOffset = FVector::Dist(EyeLocation, Owner->GetActorLocation());
	const FVector End = EyeLocation + EyeRotation.Vector() * (InteractDistance + CameraOffset);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ChukInteract), false, Owner);
	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(Hit, EyeLocation, End, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius), Params);
	if (!bHit)
	{
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !HitActor->GetClass()->ImplementsInterface(UChukInteractable::StaticClass()))
	{
		return nullptr;
	}

	// 캐릭터 기준 거리 다시 확인
	if (FVector::Dist(Owner->GetActorLocation(), Hit.ImpactPoint) > InteractDistance)
	{
		return nullptr;
	}
	return HitActor;
}

bool UChukInteractComponent::TryInteract()
{
	// 숨어 있는 중이면 바라보는 곳과 상관없이 은신처에서 나온다
	if (UChukHideComponent* Hide = GetOwner() ? GetOwner()->FindComponentByClass<UChukHideComponent>() : nullptr)
	{
		if (AChukHideSpot* Spot = Hide->GetCurrentHideSpot())
		{
			Hide->ExitHideSpot();
			OnInteracted.Broadcast(Spot);
			return true;
		}
	}

	AActor* Target = GetInteractTarget();
	if (!Target)
	{
		return false;
	}
	IChukInteractable::Execute_Interact(Target, GetOwner());
	OnInteracted.Broadcast(Target);
	return true;
}
