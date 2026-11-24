// 축영(逐影) - 은신처 액터

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChukInteractable.h"
#include "ChukHideSpot.generated.h"

class UArrowComponent;
class UStaticMeshComponent;

/** 은신처 종류 */
UENUM(BlueprintType)
enum class EChukHideSpotType : uint8
{
	FoldingScreen	UMETA(DisplayName = "병풍 뒤"),
	Wardrobe		UMETA(DisplayName = "장롱"),
	UnderFloor		UMETA(DisplayName = "마루 밑"),
	RiceChest		UMETA(DisplayName = "뒤주"),
	Jar				UMETA(DisplayName = "항아리")
};

/**
 * 은신처 (병풍 뒤 / 장롱 / 마루 밑 / 뒤주 / 항아리)
 *
 * 종류별 기본 차단 설정 (기획서 7번 표):
 *   병풍 뒤: 시야 X, 소리 X  |  장롱: 시야 X, 소리 O  |  마루 밑: 시야 X, 소리 X
 *   뒤주:   시야 O, 소리 O  |  항아리: 시야 O, 소리 O
 *   (Override Type Defaults 를 켜면 개별 은신처마다 직접 지정 가능)
 *
 * 플레이어가 E(Try Interact)를 누르면 들어가고, 숨은 상태에서 다시 E 를 누르면 나온다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. Content Browser 우클릭 → Blueprint Class → All Classes 에서 "ChukHideSpot" 선택
 *      → 이름 예: BP_Hide_Wardrobe, BP_Hide_Jar ...
 *   2. Mesh 에 장롱·항아리 등 메시 지정 (메시 충돌이 있어야 E 키로 상호작용 가능)
 *   3. Class Defaults → Chuk|Hide 의 Hide Spot Type 선택
 *   4. 뷰포트에서 HidePoint(숨는 위치), ExitPoint(나오는 위치) 화살표를 알맞게 옮긴다.
 *   5. 문 열림 애니메이션·소리는 Event On Player Entered / On Player Exited 에서 연결.
 */
UCLASS(Blueprintable)
class GAME_API AChukHideSpot : public AActor, public IChukInteractable
{
	GENERATED_BODY()

public:
	AChukHideSpot();

	/** 은신처 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Hide")
	EChukHideSpotType HideSpotType = EChukHideSpotType::Wardrobe;

	/** 켜면 아래 Custom 값을 쓰고, 끄면 종류별 기본값을 쓴다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Hide")
	bool bOverrideTypeDefaults = false;

	/** (Override 켰을 때) 시야 차단 = 숨어 있는 동안 공포가 오르지 않음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Hide", meta = (EditCondition = "bOverrideTypeDefaults"))
	bool bCustomBlocksSight = false;

	/** (Override 켰을 때) 소리 차단 = 숨어 있는 동안 정신력이 내려가지 않음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Hide", meta = (EditCondition = "bOverrideTypeDefaults"))
	bool bCustomBlocksSound = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Hide")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** 숨었을 때 캐릭터가 놓이는 위치 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Hide")
	TObjectPtr<UArrowComponent> HidePoint;

	/** 나왔을 때 캐릭터가 놓이는 위치 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chuk|Hide")
	TObjectPtr<UArrowComponent> ExitPoint;

	/** 시야 차단 여부 (종류 기본값 또는 Custom 값) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Hide")
	bool BlocksSight() const;

	/** 소리 차단 여부 (종류 기본값 또는 Custom 값) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Hide")
	bool BlocksSound() const;

	/** 종류별 기본 차단 설정 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Hide")
	static void GetTypeDefaults(EChukHideSpotType Type, bool& bOutBlocksSight, bool& bOutBlocksSound);

	FVector GetHideLocation() const;
	FVector GetExitLocation() const;

	/** 플레이어가 들어왔을 때 (문 닫힘 연출 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Hide")
	void OnPlayerEntered(AActor* Player);

	/** 플레이어가 나갔을 때 (문 열림 연출 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chuk|Hide")
	void OnPlayerExited(AActor* Player);

	void NotifyPlayerEntered(AActor* Player) { OnPlayerEntered(Player); }
	void NotifyPlayerExited(AActor* Player) { OnPlayerExited(Player); }

	// IChukInteractable
	virtual void Interact_Implementation(AActor* InteractingActor) override;
};
