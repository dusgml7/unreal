// 축영(逐影) - E 키로 열고 닫는 가구 (문·서랍·상자 등)

#include "ChukAnimatedProp.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/GameplayStatics.h"

AChukAnimatedProp::AChukAnimatedProp()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);
	// E 키 판정(Visibility 트레이스)에 걸리도록 충돌을 켠다 (Physics Asset 모양 사용)
	SkeletalMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
}

void AChukAnimatedProp::BeginPlay()
{
	Super::BeginPlay();

	// 시작 상태를 소리 없이 맞춘다: 열린 채 시작하면 열기 애니메이션의 마지막 자세로
	bOpen = bStartOpen;
	if (bOpen && OpenAnimation && SkeletalMesh->GetSkeletalMeshAsset())
	{
		SkeletalMesh->PlayAnimation(OpenAnimation, false);
		if (const UAnimSequenceBase* Seq = Cast<UAnimSequenceBase>(OpenAnimation))
		{
			SkeletalMesh->SetPosition(Seq->GetPlayLength(), false);
		}
	}
}

void AChukAnimatedProp::SetOpen(bool bNewOpen)
{
	if (bOpen == bNewOpen)
	{
		return;
	}
	bOpen = bNewOpen;

	UAnimationAsset* Anim = bOpen ? OpenAnimation.Get() : CloseAnimation.Get();
	if (Anim && SkeletalMesh->GetSkeletalMeshAsset())
	{
		SkeletalMesh->PlayAnimation(Anim, false); // 끝나면 마지막 자세로 멈춤
	}

	if (USoundBase* Sound = bOpen ? OpenSound.Get() : CloseSound.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}

	if (bOpen)
	{
		OnOpened();
	}
	else
	{
		OnClosed();
	}
}

void AChukAnimatedProp::Interact_Implementation(AActor* InteractingActor)
{
	if (bCanInteract)
	{
		SetOpen(!bOpen);
	}
}
