// 축영(逐影) - 공포 / 정신력 게이지 컴포넌트

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "ChukGaugeComponent.generated.h"

class AChukSisterBase;
class UAudioComponent;
class USoundBase;

/** 게이지 종류. 사운드를 어떤 게이지에 연결할지 고를 때 사용한다. */
UENUM(BlueprintType)
enum class EChukGaugeType : uint8
{
	None	UMETA(DisplayName = "없음 (재생 안 함)"),
	Fear	UMETA(DisplayName = "공포"),
	Sanity	UMETA(DisplayName = "정신력")
};

/**
 * 게이지 사운드 한 개(심장 소리 또는 숨소리)의 설정.
 * 단계별 배열의 0번 = 단계 0(정상), 1번 = 1단계, 2번 = 2단계, 3번 = 3단계(최악).
 * 볼륨이 0인 단계에서는 소리가 멈춘다.
 */
USTRUCT(BlueprintType)
struct FChukGaugeSoundSettings
{
	GENERATED_BODY()

	/** 재생할 소리. 반복(Looping) 설정이 없어도 컴포넌트가 끝날 때마다 다시 재생한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> Sound = nullptr;

	/** 이 소리를 어떤 게이지의 단계에 맞춰 바꿀지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	EChukGaugeType LinkedGauge = EChukGaugeType::None;

	/** 소리가 나기 시작하는 게이지 값. 공포는 이 값 "이상", 정신력은 이 값 "이하"일 때만 소리가 난다.
	 *  그 범위 안에서 볼륨·빠르기는 아래 단계별 값을 따른다. -1 = 기준값 없이 단계 볼륨만 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float StartValue = -1.f;

	/** 단계별 볼륨 배율 (0 = 재생 안 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TArray<float> VolumeByStage = { 0.f, 0.4f, 0.7f, 1.0f };

	/** 단계별 재생 빠르기(피치) 배율. 1.0 = 원래 빠르기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TArray<float> PitchByStage = { 1.0f, 1.0f, 1.2f, 1.5f };
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChukGaugeValueChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChukGaugeStageChanged, int32, Stage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChukGaugeEvent);

/**
 * 공포 / 정신력 게이지 컴포넌트
 *
 * ■ 공포 게이지 (0 ~ 100, 높을수록 나쁨)
 *   - 자매를 "직시"하면 오른다. 직시 = 카메라 시야 각도 안 + 사이에 막는 물체 없음.
 *   - 100이 되면 OnFearMaxed 이벤트 (게임 오버 연출은 블루프린트에서).
 * ■ 정신력 게이지 (100 ~ 0, 낮을수록 나쁨)
 *   - 자매 소리가 들리는 거리(청취 반경) 안에 있으면 내려간다.
 *   - 0이 되면 OnSanityDepleted 이벤트, 이동 속도 디버프가 최대가 된다.
 * ■ 단계 (0 = 정상, 1~3 = 디버프 단계)
 *   - 공포: FearStageThresholds 값 "이상"이 될 때마다 한 단계씩 올라감 (기본 33 / 66 / 100)
 *   - 정신력: SanityStageThresholds 값 "이하"가 될 때마다 한 단계씩 올라감 (기본 66 / 33 / 0)
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. 플레이어 캐릭터 블루프린트를 열고 왼쪽 Components 패널의 [+ Add] → "Chuk Gauge" 검색해서 추가.
 *   2. 컴포넌트를 선택하고 Details 패널에서 수치(Chuk|... 카테고리)와 사운드를 지정.
 *   3. Details 패널 맨 아래 Events 에서 [+] 를 누르면 On Fear Stage Changed 등 이벤트 노드가 생긴다.
 *      → 여기에 화면 효과, 게임 오버 연출 등을 연결한다.
 *   4. 다른 블루프린트에서 값을 바꿀 때는 이 컴포넌트를 가져와서 Add Fear / Add Sanity 노드를 호출한다.
 *
 * ■ 자매 찾기
 *   - 레벨에 있는 AChukSisterBase(와 그 자식 블루프린트)를 자동으로 찾는다.
 *   - 청취 반경은 각 자매의 Hearing Radius 값을 쓴다.
 *   - 자매의 Affects Player Gauges 가 꺼져 있거나 숨김(Hidden) 상태면 무시한다.
 */
UCLASS(ClassGroup = (Chuk), meta = (BlueprintSpawnableComponent, DisplayName = "Chuk Gauge"))
class GAME_API UChukGaugeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChukGaugeComponent();

	// ===================== 공포 게이지 설정 =====================

	/** 자매를 직시하는 동안 1초마다 오르는 공포 양 */
	// TODO(기획 확인): 공포 상승 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Fear", meta = (ClampMin = "0"))
	float FearRisePerSecond = 10.f;

	/** 등불이 다 꺼졌을 때 자매를 볼 때 공포 상승 속도 배율 (Chuk Inventory 의 등불 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Fear", meta = (ClampMin = "1"))
	float DarkFearRiseMultiplier = 1.5f;

	/** 직시를 시작하고 이 시간(초)이 지난 뒤부터 공포가 오른다. 0이면 보자마자 오름 */
	// TODO(기획 확인): "일정 시간 직시" 의 일정 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Fear", meta = (ClampMin = "0"))
	float GazeTimeBeforeFearRise = 0.f;

	/** 자매를 보지 않을 때 1초마다 저절로 내려가는 공포 양. 0이면 저절로 내려가지 않음 */
	// TODO(기획 확인): 공포 게이지가 시간이 지나면 저절로 회복되는지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Fear", meta = (ClampMin = "0"))
	float FearRecoveryPerSecond = 0.f;

	/** 직시 판정 각도. 화면 정중앙에서 이 각도(도) 안에 자매가 있으면 "보고 있다"로 판정 */
	// TODO(기획 확인): 직시 판정 각도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Fear", meta = (ClampMin = "1", ClampMax = "90"))
	float GazeHalfAngleDegrees = 30.f;

	/** 이 거리(cm)보다 먼 자매는 직시 판정을 하지 않음 */
	// TODO(기획 확인): 직시 판정 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Fear", meta = (ClampMin = "0"))
	float GazeMaxDistance = 3000.f;

	/** 시야를 막는 물체를 검사할 때 쓰는 충돌 채널 (보통 Visibility 그대로 두면 됨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Fear")
	TEnumAsByte<ECollisionChannel> GazeTraceChannel = ECC_Visibility;

	/** 공포 단계 경계값. 공포가 이 값 "이상"이면 그 단계. 기본 33 / 66 / 100 → 1 / 2 / 3단계 */
	// TODO(기획 확인): 공포 단계 경계값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Fear")
	TArray<float> FearStageThresholds = { 33.f, 66.f, 100.f };

	// ===================== 정신력 게이지 설정 =====================

	/** 자매 소리를 듣는 동안 1초마다 내려가는 정신력 양 */
	// TODO(기획 확인): 정신력 감소 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sanity", meta = (ClampMin = "0"))
	float SanityDrainPerSecond = 5.f;

	/** 등불이 다 꺼졌을 때 자매 소리를 들을 때 정신력 감소 속도 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sanity", meta = (ClampMin = "1"))
	float DarkSanityDrainMultiplier = 1.5f;

	// ===================== 어둠 =====================
	// 등불이 다 꺼져 있으면 자매가 없어도 Darkness Tick Interval 초마다 정신력 -Loss, 공포 +Gain.
	// 등잔으로 다시 켜면 멈추고, 시간은 처음부터 다시 센다. 은신처에 숨어 있는 동안은 적용되지 않는다.

	/** 어둠 속에서 몇 초마다 게이지가 변하는지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Darkness", meta = (ClampMin = "0.1"))
	float DarknessTickInterval = 3.5f;

	/** 한 번에 줄어드는 정신력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Darkness", meta = (ClampMin = "0"))
	float DarknessSanityLoss = 1.f;

	/** 한 번에 오르는 공포 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Darkness", meta = (ClampMin = "0"))
	float DarknessFearGain = 1.f;

	/** 소리를 듣기 시작하고 이 시간(초)이 지난 뒤부터 정신력이 내려간다. 0이면 바로 내려감 */
	// TODO(기획 확인): "일정 시간 들으면" 의 일정 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sanity", meta = (ClampMin = "0"))
	float HearTimeBeforeSanityDrain = 0.f;

	/** 소리가 안 들릴 때 1초마다 저절로 회복되는 정신력 양. 0이면 저절로 회복되지 않음 */
	// TODO(기획 확인): 정신력 게이지가 시간이 지나면 저절로 회복되는지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sanity", meta = (ClampMin = "0"))
	float SanityRecoveryPerSecond = 0.f;

	// 청취 반경은 자매마다 다르게 줄 수 있도록 AChukSisterBase 의 Hearing Radius 에서 지정한다.

	/** 정신력 단계 경계값. 정신력이 이 값 "이하"이면 그 단계. 기본 66 / 33 / 0 → 1 / 2 / 3단계 */
	// TODO(기획 확인): 정신력 단계 경계값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sanity")
	TArray<float> SanityStageThresholds = { 66.f, 33.f, 0.f };

	/** 정신력 단계별 이동 속도 배율. 0번 = 정상, 1~3번 = 1~3단계 */
	// TODO(기획 확인): 단계별 속도 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sanity")
	TArray<float> SanitySpeedMultipliers = { 1.0f, 0.85f, 0.7f, 0.5f };

	/** 켜 두면 이 컴포넌트가 캐릭터의 걷기 속도(Max Walk Speed)를 직접 바꾼다 (달리기 포함).
	 *  끄면 달리기·정신력 감속 모두 적용되지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sanity")
	bool bApplySpeedToCharacter = true;

	// ===================== 달리기 =====================
	// 달리기도 걷기 속도를 바꾸므로, 정신력 감속과 충돌하지 않도록 이 컴포넌트에서 함께 계산한다.
	// 최종 속도 = 원래 걷기 속도 × (달리는 중이면 Run Speed Multiplier) × 정신력 단계 배율
	// 키(기본 왼쪽 Shift)는 AChukPlayerController 가 Set Running 으로 연결한다.

	/** 달릴 때 걷기 속도의 몇 배인지 */
	// 기본 2.5 = 걷기 200 × 2.5 = 500 → BS_Locomotion 의 달리기(run) 위치와 맞춤 (중간 속도면 걷기·달리기가 섞여 버벅임)
	// TODO(기획 확인): 달리기 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Run", meta = (ClampMin = "1"))
	float RunSpeedMultiplier = 2.5f;

	/** 정신력이 이 값 미만이면 달릴 수 없다 (Shift 를 눌러도 걷기). 0 = 정신력과 상관없이 항상 달릴 수 있음 (기획 결정 2026-11-27) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Run", meta = (ClampMin = "0", ClampMax = "100"))
	float RunDisabledSanityThreshold = 0.f;
	// TODO(기획 확인): 달리기에 스태미나 제한이 있는지, 달리면 자매가 소리를 듣는지

	// ===================== 웅크리기 =====================
	// 키(기본 왼쪽 Ctrl)는 AChukPlayerController 가 캐릭터의 Crouch / Un Crouch 로 연결한다.
	// 애니메이션 블루프린트에서는 캐릭터의 "Is Crouched" 값을 읽어 웅크리기 동작으로 바꾸면 된다.

	/** 웅크려 걸을 때 걷기 속도의 몇 배인지 (정신력 감속도 같이 곱해짐) */
	// TODO(기획 확인): 웅크리기 이동 속도, 웅크리면 자매에게 덜 들키는지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Crouch", meta = (ClampMin = "0"))
	float CrouchSpeedMultiplier = 0.5f;

	// ===================== 사운드 =====================

	/** 심장 소리. 기본은 공포 게이지에 연결 */
	// TODO(기획 확인): 심장 소리를 어느 게이지에 붙일지, 단계별 볼륨·빠르기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sound")
	FChukGaugeSoundSettings HeartbeatSound;

	/** 숨소리(헉헉). 기본은 정신력 게이지에 연결 */
	// TODO(기획 확인): 숨소리를 어느 게이지에 붙일지, 단계별 볼륨·빠르기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sound")
	FChukGaugeSoundSettings BreathSound;

	// ===================== 자매 찾기 =====================

	/** 레벨에서 자매(AChukSisterBase) 목록을 다시 찾는 간격(초). 게임 중에 자매가 새로 생겨도 이 간격 안에 인식됨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Target", meta = (ClampMin = "0.1"))
	float TargetRefreshInterval = 1.f;

	// ===================== 디버그 =====================

	/** 켜면 게임 화면 왼쪽 위에 게이지 값과 단계를 글자로 표시 (테스트용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Debug")
	bool bShowDebugText = false;

	// ===================== 이벤트 =====================

	/** 공포 값이 바뀔 때마다 (UI 갱신용) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukGaugeValueChanged OnFearChanged;

	/** 정신력 값이 바뀔 때마다 (UI 갱신용) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukGaugeValueChanged OnSanityChanged;

	/** 공포 단계가 바뀔 때 (0~3). 단계별 화면 효과는 여기에 연결 */
	// TODO(기획 확인): 공포 단계별 효과(화면 효과 등)
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukGaugeStageChanged OnFearStageChanged;

	/** 정신력 단계가 바뀔 때 (0~3) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukGaugeStageChanged OnSanityStageChanged;

	/** 공포가 100에 도달했을 때 (환청 → 연못 자살 = 게임 오버 연출) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukGaugeEvent OnFearMaxed;

	/** 정신력이 0에 도달했을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukGaugeEvent OnSanityDepleted;

	// ===================== 함수 =====================

	/** 공포를 더한다. 양수 = 공포 상승(나빠짐), 음수 = 공포 회복 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Gauge")
	void AddFear(float Amount);

	/** 정신력을 더한다. 양수 = 정신력 회복, 음수 = 정신력 악화 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Gauge")
	void AddSanity(float Amount);

	/** 공포 값을 직접 지정 (세이브 불러오기 등). 0~100 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Gauge")
	void SetFear(float NewFear) { SetFearInternal(NewFear); }

	/** 정신력 값을 직접 지정 (세이브 불러오기 등). 0~100 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Gauge")
	void SetSanity(float NewSanity) { SetSanityInternal(NewSanity); }

	UFUNCTION(BlueprintPure, Category = "Chuk|Gauge")
	float GetFear() const { return Fear; }

	UFUNCTION(BlueprintPure, Category = "Chuk|Gauge")
	float GetSanity() const { return Sanity; }

	/** 현재 공포 단계 (0 = 정상 ~ 3 = 최악) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Gauge")
	int32 GetFearStage() const { return FearStage; }

	/** 현재 정신력 단계 (0 = 정상 ~ 3 = 최악) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Gauge")
	int32 GetSanityStage() const { return SanityStage; }

	/** 지금 적용 중인 이동 속도 배율 (정신력 단계에 따른 감속만. 달리기 제외) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Gauge")
	float GetSpeedMultiplier() const;

	/** 달리기 켜기/끄기 (Shift 누름/뗌) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Run")
	void SetRunning(bool bRun);

	/** 실제로 달리고 있는지 (Shift 를 누르고 있고 + 달릴 수 있는 상태) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Run")
	bool IsRunning() const { return bRunning && CanRun(); }

	/** 지금 정신력으로 달릴 수 있는지 (정신력 > Run Disabled Sanity Threshold). UI 에서 달리기 불가 표시용 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Run")
	bool CanRun() const { return RunDisabledSanityThreshold <= 0.f || Sanity >= RunDisabledSanityThreshold; }

	/** 등불이 다 꺼진 상태인지 (같은 캐릭터의 Chuk Inventory 기준). 꺼져 있으면 공포·정신력 변화가 빨라짐 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Gauge")
	bool IsInDarkness() const;

	/** 이번 프레임에 자매를 직시하고 있는지 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Gauge")
	bool IsLookingAtSister() const { return bLookingAtSister; }

	/** 이번 프레임에 자매 소리를 듣고 있는지 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Gauge")
	bool IsHearingSister() const { return bHearingSister; }

	/** 시야 차단 (은신처에서 사용, 작업 4). 켜져 있으면 직시해도 공포가 오르지 않음 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Gauge")
	void SetSightBlocked(bool bBlocked) { bSightBlocked = bBlocked; }

	/** 소리 차단 (은신처에서 사용, 작업 4). 켜져 있으면 소리를 들어도 정신력이 내려가지 않음 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Gauge")
	void SetSoundBlocked(bool bBlocked) { bSoundBlocked = bBlocked; }

	static constexpr float MaxGaugeValue = 100.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// 현재 값
	float Fear = 0.f;
	float Sanity = MaxGaugeValue;
	int32 FearStage = 0;
	int32 SanityStage = 0;

	bool bFearMaxedFired = false;
	bool bSanityDepletedFired = false;

	bool bLookingAtSister = false;
	bool bHearingSister = false;
	bool bSightBlocked = false;
	bool bSoundBlocked = false;

	float LookTime = 0.f;
	float HearTime = 0.f;

	// 자매 목록 캐시
	TArray<TWeakObjectPtr<AChukSisterBase>> CachedSisters;
	float TimeSinceTargetRefresh = 0.f;

	// 캐릭터 원래 걷기 속도
	float BaseWalkSpeed = -1.f;

	bool bRunning = false;

	// 어둠 속에 있은 시간 (Darkness Tick Interval 마다 초기화)
	float DarknessTime = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> HeartbeatAudio;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BreathAudio;

	void RefreshSisters();
	bool CanSee(const AChukSisterBase* Sister) const;
	bool CanHear(const AChukSisterBase* Sister) const;

	void SetFearInternal(float NewFear);
	void SetSanityInternal(float NewSanity);
	void UpdateStages();
	void ApplySpeed();

	int32 GetStageOf(EChukGaugeType Gauge) const;
	UAudioComponent* CreateGaugeAudio(const FChukGaugeSoundSettings& Settings);
	void UpdateGaugeSound(UAudioComponent* Audio, const FChukGaugeSoundSettings& Settings);

	UFUNCTION()
	void HandleHeartbeatFinished();

	UFUNCTION()
	void HandleBreathFinished();

	void DrawDebugText() const;
};
