// 축영(逐影) - 유품 액터 (레벨에 배치)

#include "ChukRelicPickup.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AChukRelicPickup::AChukRelicPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	// 상호작용 트레이스(Visibility)만 막고 나머지는 통과
	InteractCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractCollision"));
	InteractCollision->InitCapsuleSize(40.f, 40.f); // 반지름, 절반 높이 — Shape 의 Capsule Radius / Capsule Half Height 로 조절
	InteractCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = InteractCollision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(InteractCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AChukRelicPickup::BeginPlay()
{
	Super::BeginPlay();

	// 세이브를 불러온 경우 등, 이미 찾은 유품이면 레벨에서 치운다
	if (UChukRelicSubsystem* Relics = GetWorld() ? GetWorld()->GetSubsystem<UChukRelicSubsystem>() : nullptr)
	{
		if (Relics->GetRelicState(RelicType) != EChukRelicState::NotFound)
		{
			Destroy();
			return;
		}
		// 나중에 상태가 바뀌어도(세이브 불러오기 순서 등) 치울 수 있도록
		Relics->OnRelicStateChanged.AddDynamic(this, &AChukRelicPickup::HandleRelicStateChanged);
	}
}

void AChukRelicPickup::HandleRelicStateChanged(EChukRelicType Relic, EChukRelicState NewState)
{
	if (Relic == RelicType && NewState != EChukRelicState::NotFound && !IsActorBeingDestroyed())
	{
		Destroy();
	}
}

void AChukRelicPickup::Interact_Implementation(AActor* InteractingActor)
{
	UChukRelicSubsystem* Relics = GetWorld() ? GetWorld()->GetSubsystem<UChukRelicSubsystem>() : nullptr;
	if (!Relics || Relics->GetRelicState(RelicType) != EChukRelicState::NotFound)
	{
		return;
	}

	// 연출 이벤트를 먼저 부르고, 그다음 발견 처리 (발견 처리 중 상태 변경 이벤트로 이 액터가 치워짐)
	OnRelicFound(InteractingActor);
	Relics->FindRelic(RelicType);
	if (!IsActorBeingDestroyed())
	{
		Destroy();
	}
}
