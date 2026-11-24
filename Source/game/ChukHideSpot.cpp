// 축영(逐影) - 은신처 액터

#include "ChukHideSpot.h"

#include "ChukHideComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AChukHideSpot::AChukHideSpot()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	HidePoint = CreateDefaultSubobject<UArrowComponent>(TEXT("HidePoint"));
	HidePoint->SetupAttachment(RootComponent);
	HidePoint->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	HidePoint->ArrowColor = FColor::Green;

	ExitPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitPoint"));
	ExitPoint->SetupAttachment(RootComponent);
	ExitPoint->SetRelativeLocation(FVector(120.f, 0.f, 90.f));
	ExitPoint->ArrowColor = FColor::Blue;
}

void AChukHideSpot::GetTypeDefaults(EChukHideSpotType Type, bool& bOutBlocksSight, bool& bOutBlocksSound)
{
	// 기획서 7번 표
	// TODO(기획 확인): 병풍 뒤·장롱·마루 밑의 시야 차단 여부가 기획서에 없음 → 기본값 X(차단 안 함)
	switch (Type)
	{
	case EChukHideSpotType::FoldingScreen:	bOutBlocksSight = false;	bOutBlocksSound = false;	break;
	case EChukHideSpotType::Wardrobe:		bOutBlocksSight = false;	bOutBlocksSound = true;		break;
	case EChukHideSpotType::UnderFloor:		bOutBlocksSight = false;	bOutBlocksSound = false;	break;
	case EChukHideSpotType::RiceChest:		bOutBlocksSight = true;		bOutBlocksSound = true;		break;
	case EChukHideSpotType::Jar:			bOutBlocksSight = true;		bOutBlocksSound = true;		break;
	default:								bOutBlocksSight = false;	bOutBlocksSound = false;	break;
	}
}

bool AChukHideSpot::BlocksSight() const
{
	if (bOverrideTypeDefaults)
	{
		return bCustomBlocksSight;
	}
	bool bSight, bSound;
	GetTypeDefaults(HideSpotType, bSight, bSound);
	return bSight;
}

bool AChukHideSpot::BlocksSound() const
{
	if (bOverrideTypeDefaults)
	{
		return bCustomBlocksSound;
	}
	bool bSight, bSound;
	GetTypeDefaults(HideSpotType, bSight, bSound);
	return bSound;
}

FVector AChukHideSpot::GetHideLocation() const
{
	return HidePoint->GetComponentLocation();
}

FVector AChukHideSpot::GetExitLocation() const
{
	return ExitPoint->GetComponentLocation();
}

void AChukHideSpot::Interact_Implementation(AActor* InteractingActor)
{
	UChukHideComponent* Hide = InteractingActor ? InteractingActor->FindComponentByClass<UChukHideComponent>() : nullptr;
	if (!Hide)
	{
		return;
	}

	if (Hide->GetCurrentHideSpot() == this)
	{
		Hide->ExitHideSpot();
	}
	else
	{
		Hide->EnterHideSpot(this);
	}
}
