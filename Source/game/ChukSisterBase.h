// 축영(逐影) - 장화 / 홍련 몬스터 부모 클래스

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChukSisterBase.generated.h"

class UAudioComponent;
class USoundAttenuation;
class USoundBase;

/** 자매 종류 */
UENUM(BlueprintType)
enum class EChukSisterType : uint8
{
	Janghwa		UMETA(DisplayName = "장화"),
	Hongryeon	UMETA(DisplayName = "홍련")
};

/**
 * 장화 / 홍련이 상속하는 부모 클래스 (Character 기반)
 *
 * - 플레이어의 Chuk Gauge 컴포넌트가 레벨에 있는 이 클래스(와 자식 블루프린트)를 자동으로 찾아서
 *   직시(공포 상승) / 청취(정신력 감소) 판정을 한다. 따로 연결할 필요 없음.
 * - 청취 판정: 플레이어가 이 자매의 HearingRadius 안에 있으면 "소리를 듣는 중".
 * - 반복 소리: LoopSounds 배열에서 하나를 랜덤으로 골라 재생 → 끝나면 잠깐 쉬고 → 다음 소리.
 *   (장화: 물 발자국·물 떨어지는 소리 / 홍련: 통곡·비명)
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. Content Browser 에서 우클릭 → Blueprint Class → All Classes 에서 "ChukSisterBase" 선택
 *      → 이름 예: BP_Janghwa, BP_Hongryeon
 *   2. 열어서 Mesh 컴포넌트에 스켈레탈 메시·애니메이션 지정
 *   3. Class Defaults 의 Chuk|Sister 카테고리에서 Sister Type, Hearing Radius, Loop Sounds 지정
 *   4. 레벨에 끌어다 놓으면 끝
 *
 * AI 추적·이동은 이번 범위가 아님 (아래 TODO).
 */
UCLASS(Blueprintable)
class GAME_API AChukSisterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AChukSisterBase();

	/** 장화 / 홍련 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sister")
	EChukSisterType SisterType = EChukSisterType::Janghwa;

	/** 청취 반경(cm). 플레이어가 이 거리 안에 있으면 소리를 듣는 것으로 보고 정신력이 내려감 */
	// TODO(기획 확인): 청취 반경 (장화·홍련이 같은지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sister", meta = (ClampMin = "0"))
	float HearingRadius = 1500.f;

	/** 꺼 두면 플레이어 게이지에 영향을 주지 않음 (등장 전·연출 중 등). 액터가 숨김(Hidden) 상태여도 영향 없음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sister")
	bool bAffectsPlayerGauges = true;

	/** 반복해서 재생할 소리 목록. 매번 이 중 하나를 랜덤으로 재생 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sister|Sound")
	TArray<TObjectPtr<USoundBase>> LoopSounds;

	/** 소리가 퍼지는 거리 설정(Sound Attenuation 에셋). Falloff 거리를 Hearing Radius 와 비슷하게 맞추는 것을 추천 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sister|Sound")
	TObjectPtr<USoundAttenuation> SoundAttenuation = nullptr;

	/** 소리 하나가 끝난 뒤 다음 소리까지 쉬는 시간(초) 최소값 */
	// TODO(기획 확인): 자매 소리 재생 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sister|Sound", meta = (ClampMin = "0"))
	float SoundIntervalMin = 1.f;

	/** 소리 하나가 끝난 뒤 다음 소리까지 쉬는 시간(초) 최대값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sister|Sound", meta = (ClampMin = "0"))
	float SoundIntervalMax = 3.f;

	/** 켜 두면 게임 시작 시 자동으로 소리 재생 시작 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Sister|Sound")
	bool bAutoStartSounds = true;

	/** 반복 소리 재생 시작 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Sister")
	void StartSisterSounds();

	/** 반복 소리 재생 멈춤 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Sister")
	void StopSisterSounds();

	/** 지금 플레이어 게이지에 영향을 줄 수 있는 상태인지 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Sister")
	bool CanAffectPlayer() const;

	/** 소리 하나가 재생될 때마다 호출 (자막·이펙트 연결용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Sister")
	void OnSisterSoundPlayed(USoundBase* PlayedSound);

	// TODO: AI 추적·이동은 이번 범위가 아님. 추후 AIController / Behavior Tree 로 구현 예정.
	//       (진행도에 따른 추적 강화, 최종 추격전 동시 추격 포함)

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Chuk|Sister|Sound")
	TObjectPtr<UAudioComponent> SisterAudio;

	FTimerHandle NextSoundTimer;
	bool bSoundsRunning = false;

	void PlayNextSound();

	UFUNCTION()
	void HandleSoundFinished();
};
