// 축영(逐影) - 플레이어 숨기 상태 / 숨기 횟수 컴포넌트

#include "ChukHideComponent.h"

#include "ChukGaugeComponent.h"
#include "ChukHideSpot.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UChukHideComponent::UChukHideComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UChukHideComponent::BeginPlay()
{
	Super::BeginPlay();
	RemainingHides = MaxHideCount;
	OnHideCountChanged.Broadcast(RemainingHides);
}

void UChukHideComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CurrentSpot = nullptr;
	Super::EndPlay(EndPlayReason);
}

bool UChukHideComponent::EnterHideSpot(AChukHideSpot* Spot)
{
	if (!Spot || IsHiding())
	{
		return false;
	}

	if (RemainingHides <= 0)
	{
		OnHideDenied.Broadcast();
		if (bBlockWhenNoHidesLeft)
		{
			return false;
		}
	}
	else
	{
		--RemainingHides;
		OnHideCountChanged.Broadcast(RemainingHides);
	}

	CurrentSpot = Spot;
	ApplyBlocking(Spot->BlocksSight(), Spot->BlocksSound());

	if (bMoveCharacterIntoSpot)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			// 장롱·항아리 안 메시와 겹쳐도 밀려나지 않도록 숨는 동안 충돌을 끈다
			Character->SetActorEnableCollision(false);
			Character->SetActorLocation(Spot->GetHideLocation(), false, nullptr, ETeleportType::TeleportPhysics);
			if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
			{
				Move->StopMovementImmediately();
				Move->DisableMovement();
			}
		}
	}

	Spot->NotifyPlayerEntered(GetOwner());
	OnHideStarted.Broadcast(Spot);
	return true;
}

void UChukHideComponent::ExitHideSpot()
{
	AChukHideSpot* Spot = CurrentSpot;
	if (!Spot)
	{
		return;
	}
	CurrentSpot = nullptr;
	ApplyBlocking(false, false);

	if (bMoveCharacterIntoSpot)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			Character->SetActorLocation(Spot->GetExitLocation(), false, nullptr, ETeleportType::TeleportPhysics);
			Character->SetActorEnableCollision(true);
			if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
			{
				Move->SetMovementMode(MOVE_Walking);
			}
		}
	}

	Spot->NotifyPlayerExited(GetOwner());
	OnHideEnded.Broadcast(Spot);
}

void UChukHideComponent::ResetHideCount()
{
	SetRemainingHides(MaxHideCount);
}

void UChukHideComponent::SetRemainingHides(int32 NewRemaining)
{
	RemainingHides = FMath::Clamp(NewRemaining, 0, MaxHideCount);
	OnHideCountChanged.Broadcast(RemainingHides);
}

void UChukHideComponent::ApplyBlocking(bool bSight, bool bSound)
{
	if (UChukGaugeComponent* Gauge = GetOwner() ? GetOwner()->FindComponentByClass<UChukGaugeComponent>() : nullptr)
	{
		Gauge->SetSightBlocked(bSight);
		Gauge->SetSoundBlocked(bSound);
	}
}
