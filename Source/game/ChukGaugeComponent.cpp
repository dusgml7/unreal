// 축영(逐影) - 공포 / 정신력 게이지 컴포넌트

#include "ChukGaugeComponent.h"

#include "ChukSisterBase.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Sound/SoundBase.h"

namespace
{
	// 배열에서 Index 위치 값을 안전하게 꺼낸다. 범위를 벗어나면 마지막 값(없으면 Default)
	float GetClampedArrayValue(const TArray<float>& Values, int32 Index, float Default)
	{
		if (Values.Num() == 0)
		{
			return Default;
		}
		return Values[FMath::Clamp(Index, 0, Values.Num() - 1)];
	}
}

UChukGaugeComponent::UChukGaugeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 기본 연결: 공포 → 심장 소리, 정신력 → 숨소리
	// TODO(기획 확인): 어떤 게이지에 어떤 소리를 붙일지
	HeartbeatSound.LinkedGauge = EChukGaugeType::Fear;
	BreathSound.LinkedGauge = EChukGaugeType::Sanity;
}

void UChukGaugeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (const UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			BaseWalkSpeed = Move->MaxWalkSpeed;
		}
	}

	HeartbeatAudio = CreateGaugeAudio(HeartbeatSound);
	if (HeartbeatAudio)
	{
		HeartbeatAudio->OnAudioFinished.AddDynamic(this, &UChukGaugeComponent::HandleHeartbeatFinished);
	}

	BreathAudio = CreateGaugeAudio(BreathSound);
	if (BreathAudio)
	{
		BreathAudio->OnAudioFinished.AddDynamic(this, &UChukGaugeComponent::HandleBreathFinished);
	}

	RefreshSisters();
	UpdateStages();
	ApplySpeed();
}

void UChukGaugeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HeartbeatAudio)
	{
		HeartbeatAudio->Stop();
	}
	if (BreathAudio)
	{
		BreathAudio->Stop();
	}
	Super::EndPlay(EndPlayReason);
}

void UChukGaugeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceTargetRefresh += DeltaTime;
	if (TimeSinceTargetRefresh >= TargetRefreshInterval)
	{
		RefreshSisters();
	}

	// 1) 이번 프레임에 자매를 보고 있는지 / 소리를 듣고 있는지 판정
	bool bSeeAny = false;
	bool bHearAny = false;
	for (const TWeakObjectPtr<AChukSisterBase>& WeakSister : CachedSisters)
	{
		const AChukSisterBase* Sister = WeakSister.Get();
		if (!Sister || !Sister->CanAffectPlayer())
		{
			continue;
		}
		bSeeAny = bSeeAny || CanSee(Sister);
		bHearAny = bHearAny || CanHear(Sister);
	}

	// 은신처에서 차단 중이면 판정 무시 (작업 4)
	bLookingAtSister = bSeeAny && !bSightBlocked;
	bHearingSister = bHearAny && !bSoundBlocked;

	// 2) 공포 게이지
	if (bLookingAtSister)
	{
		LookTime += DeltaTime;
		if (LookTime >= GazeTimeBeforeFearRise)
		{
			SetFearInternal(Fear + FearRisePerSecond * DeltaTime);
		}
	}
	else
	{
		LookTime = 0.f;
		if (FearRecoveryPerSecond > 0.f)
		{
			SetFearInternal(Fear - FearRecoveryPerSecond * DeltaTime);
		}
	}

	// 3) 정신력 게이지
	if (bHearingSister)
	{
		HearTime += DeltaTime;
		if (HearTime >= HearTimeBeforeSanityDrain)
		{
			SetSanityInternal(Sanity - SanityDrainPerSecond * DeltaTime);
		}
	}
	else
	{
		HearTime = 0.f;
		if (SanityRecoveryPerSecond > 0.f)
		{
			SetSanityInternal(Sanity + SanityRecoveryPerSecond * DeltaTime);
		}
	}

	if (bShowDebugText)
	{
		DrawDebugText();
	}
}

// ===================== 외부 함수 =====================

void UChukGaugeComponent::AddFear(float Amount)
{
	SetFearInternal(Fear + Amount);
}

void UChukGaugeComponent::AddSanity(float Amount)
{
	SetSanityInternal(Sanity + Amount);
}

float UChukGaugeComponent::GetSpeedMultiplier() const
{
	return GetClampedArrayValue(SanitySpeedMultipliers, SanityStage, 1.f);
}

// ===================== 자매 판정 =====================

void UChukGaugeComponent::RefreshSisters()
{
	TimeSinceTargetRefresh = 0.f;
	CachedSisters.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	for (TActorIterator<AChukSisterBase> It(World); It; ++It)
	{
		if (*It != GetOwner())
		{
			CachedSisters.Add(*It);
		}
	}
}

bool UChukGaugeComponent::CanSee(const AChukSisterBase* Target) const
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Target)
	{
		return false;
	}

	// 눈(카메라) 위치와 바라보는 방향. 플레이어라면 카메라 기준
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

	const FVector TargetLocation = Target->GetActorLocation();
	const FVector ToTarget = TargetLocation - EyeLocation;
	const float Distance = ToTarget.Size();
	if (Distance > GazeMaxDistance || Distance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// 시야 각도 안에 있는지
	const float CosAngle = FVector::DotProduct(EyeRotation.Vector(), ToTarget / Distance);
	if (CosAngle < FMath::Cos(FMath::DegreesToRadians(GazeHalfAngleDegrees)))
	{
		return false;
	}

	// 사이에 막는 물체가 있는지 (라인 트레이스)
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ChukGaze), true, Owner);
	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(Hit, EyeLocation, TargetLocation, GazeTraceChannel, Params);
	return !bHit || Hit.GetActor() == Target;
}

bool UChukGaugeComponent::CanHear(const AChukSisterBase* Sister) const
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Sister)
	{
		return false;
	}
	// 자매의 청취 반경 안에 있으면 "듣는 중"
	// TODO(기획 확인): 자매 소리가 쉬는 사이(재생 간격)에도 듣는 것으로 볼지. 지금은 거리만으로 판정
	return FVector::DistSquared(Owner->GetActorLocation(), Sister->GetActorLocation()) <= FMath::Square(Sister->HearingRadius);
}

// ===================== 값 / 단계 처리 =====================

void UChukGaugeComponent::SetFearInternal(float NewFear)
{
	NewFear = FMath::Clamp(NewFear, 0.f, MaxGaugeValue);
	if (FMath::IsNearlyEqual(NewFear, Fear))
	{
		return;
	}
	Fear = NewFear;
	OnFearChanged.Broadcast(Fear);

	if (Fear >= MaxGaugeValue)
	{
		if (!bFearMaxedFired)
		{
			bFearMaxedFired = true;
			OnFearMaxed.Broadcast();
		}
	}
	else
	{
		bFearMaxedFired = false;
	}

	UpdateStages();
}

void UChukGaugeComponent::SetSanityInternal(float NewSanity)
{
	NewSanity = FMath::Clamp(NewSanity, 0.f, MaxGaugeValue);
	if (FMath::IsNearlyEqual(NewSanity, Sanity))
	{
		return;
	}
	Sanity = NewSanity;
	OnSanityChanged.Broadcast(Sanity);

	if (Sanity <= 0.f)
	{
		if (!bSanityDepletedFired)
		{
			bSanityDepletedFired = true;
			OnSanityDepleted.Broadcast();
		}
	}
	else
	{
		bSanityDepletedFired = false;
	}

	UpdateStages();
}

void UChukGaugeComponent::UpdateStages()
{
	// 공포: 경계값 "이상"인 개수 = 단계
	int32 NewFearStage = 0;
	for (const float Threshold : FearStageThresholds)
	{
		if (Fear >= Threshold)
		{
			++NewFearStage;
		}
	}

	// 정신력: 경계값 "이하"인 개수 = 단계
	int32 NewSanityStage = 0;
	for (const float Threshold : SanityStageThresholds)
	{
		if (Sanity <= Threshold)
		{
			++NewSanityStage;
		}
	}

	const bool bFearChanged = NewFearStage != FearStage;
	const bool bSanityChanged = NewSanityStage != SanityStage;
	FearStage = NewFearStage;
	SanityStage = NewSanityStage;

	// 사운드는 단계가 안 바뀌어도 매번 맞춰 둔다 (BeginPlay 첫 적용 포함)
	UpdateGaugeSound(HeartbeatAudio, HeartbeatSound);
	UpdateGaugeSound(BreathAudio, BreathSound);

	if (bSanityChanged)
	{
		ApplySpeed();
		OnSanityStageChanged.Broadcast(SanityStage);
	}
	if (bFearChanged)
	{
		OnFearStageChanged.Broadcast(FearStage);
	}
}

void UChukGaugeComponent::ApplySpeed()
{
	if (!bApplySpeedToCharacter || BaseWalkSpeed < 0.f)
	{
		return;
	}
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			Move->MaxWalkSpeed = BaseWalkSpeed * GetSpeedMultiplier();
		}
	}
}

// ===================== 사운드 =====================

int32 UChukGaugeComponent::GetStageOf(EChukGaugeType Gauge) const
{
	switch (Gauge)
	{
	case EChukGaugeType::Fear:		return FearStage;
	case EChukGaugeType::Sanity:	return SanityStage;
	default:						return 0;
	}
}

UAudioComponent* UChukGaugeComponent::CreateGaugeAudio(const FChukGaugeSoundSettings& Settings)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Settings.Sound)
	{
		return nullptr;
	}

	UAudioComponent* Audio = NewObject<UAudioComponent>(Owner);
	Audio->bAutoActivate = false;
	Audio->bAutoDestroy = false;
	Audio->bAllowSpatialization = false; // 플레이어 자신의 소리이므로 2D로 재생
	Audio->SetSound(Settings.Sound);
	if (USceneComponent* Root = Owner->GetRootComponent())
	{
		Audio->SetupAttachment(Root);
	}
	Audio->RegisterComponent();
	return Audio;
}

void UChukGaugeComponent::UpdateGaugeSound(UAudioComponent* Audio, const FChukGaugeSoundSettings& Settings)
{
	if (!Audio)
	{
		return;
	}

	const int32 Stage = GetStageOf(Settings.LinkedGauge);
	const float Volume = Settings.LinkedGauge == EChukGaugeType::None ? 0.f : GetClampedArrayValue(Settings.VolumeByStage, Stage, 0.f);
	const float Pitch = GetClampedArrayValue(Settings.PitchByStage, Stage, 1.f);

	if (Volume <= 0.f)
	{
		if (Audio->IsPlaying())
		{
			Audio->Stop();
		}
		return;
	}

	Audio->SetVolumeMultiplier(Volume);
	Audio->SetPitchMultiplier(Pitch);
	if (!Audio->IsPlaying())
	{
		Audio->Play();
	}
}

void UChukGaugeComponent::HandleHeartbeatFinished()
{
	// 반복 설정이 없는 소리도 계속 이어서 재생 (볼륨 0 단계면 UpdateGaugeSound 가 멈춤)
	UpdateGaugeSound(HeartbeatAudio, HeartbeatSound);
}

void UChukGaugeComponent::HandleBreathFinished()
{
	UpdateGaugeSound(BreathAudio, BreathSound);
}

// ===================== 디버그 =====================

void UChukGaugeComponent::DrawDebugText() const
{
	if (!GEngine)
	{
		return;
	}
	const FString Text = FString::Printf(
		TEXT("[Chuk Gauge] Fear %.1f (Stage %d)%s | Sanity %.1f (Stage %d)%s | Speed x%.2f | SightBlocked %d SoundBlocked %d"),
		Fear, FearStage, bLookingAtSister ? TEXT(" LOOKING") : TEXT(""),
		Sanity, SanityStage, bHearingSister ? TEXT(" HEARING") : TEXT(""),
		GetSpeedMultiplier(), bSightBlocked ? 1 : 0, bSoundBlocked ? 1 : 0);
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 0.f, FColor::Yellow, Text);
}
