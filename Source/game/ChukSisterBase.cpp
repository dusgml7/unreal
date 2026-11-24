// 축영(逐影) - 장화 / 홍련 몬스터 부모 클래스

#include "ChukSisterBase.h"

#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AChukSisterBase::AChukSisterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SisterAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("SisterAudio"));
	SisterAudio->SetupAttachment(GetCapsuleComponent());
	SisterAudio->bAutoActivate = false;
}

void AChukSisterBase::BeginPlay()
{
	Super::BeginPlay();

	SisterAudio->OnAudioFinished.AddDynamic(this, &AChukSisterBase::HandleSoundFinished);
	if (SoundAttenuation)
	{
		SisterAudio->AttenuationSettings = SoundAttenuation;
	}

	if (bAutoStartSounds)
	{
		StartSisterSounds();
	}
}

void AChukSisterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSisterSounds();
	Super::EndPlay(EndPlayReason);
}

bool AChukSisterBase::CanAffectPlayer() const
{
	return bAffectsPlayerGauges && !IsHidden();
}

void AChukSisterBase::StartSisterSounds()
{
	if (bSoundsRunning)
	{
		return;
	}
	bSoundsRunning = true;
	PlayNextSound();
}

void AChukSisterBase::StopSisterSounds()
{
	bSoundsRunning = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NextSoundTimer);
	}
	if (SisterAudio)
	{
		SisterAudio->Stop();
	}
}

void AChukSisterBase::PlayNextSound()
{
	if (!bSoundsRunning || LoopSounds.Num() == 0)
	{
		return;
	}

	USoundBase* Sound = LoopSounds[FMath::RandRange(0, LoopSounds.Num() - 1)];
	if (!Sound)
	{
		return;
	}

	SisterAudio->SetSound(Sound);
	SisterAudio->Play();
	OnSisterSoundPlayed(Sound);
}

void AChukSisterBase::HandleSoundFinished()
{
	if (!bSoundsRunning)
	{
		return;
	}

	const float Delay = FMath::FRandRange(SoundIntervalMin, FMath::Max(SoundIntervalMin, SoundIntervalMax));
	if (Delay <= 0.f)
	{
		PlayNextSound();
	}
	else if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(NextSoundTimer, this, &AChukSisterBase::PlayNextSound, Delay, false);
	}
}
