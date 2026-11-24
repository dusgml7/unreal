// 축영(逐影) - 아이템 데이터 정의

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ChukItemTypes.generated.h"

class UTexture2D;

/** 아이템 효과 종류 */
UENUM(BlueprintType)
enum class EChukItemEffect : uint8
{
	None			UMETA(DisplayName = "효과 없음"),
	RecoverFear		UMETA(DisplayName = "공포 회복 (공포 -효과량)"),
	RecoverSanity	UMETA(DisplayName = "정신력 회복 (정신력 +효과량)"),
	WorsenSanity	UMETA(DisplayName = "정신력 악화 (정신력 -효과량)"),
	RelightLantern	UMETA(DisplayName = "등불 다시 밝히기"),
	Lantern			UMETA(DisplayName = "등불 (시간제, 사용 안 함)"),
	Diary			UMETA(DisplayName = "일기 (읽기)")
};

/**
 * 아이템 한 종류의 데이터. DataTable 의 한 줄(Row)이 된다.
 * Row Name = 아이템 ID (예: Fan, Bell, Talisman, Lantern, OilLamp, Diary, DeadMouse, StrawDoll)
 *
 * ■ DataTable 만드는 법
 *   docs/data/DT_ChukItems.csv 를 Content Browser 에 끌어다 놓고 Row Type 으로 "ChukItemData" 선택.
 */
USTRUCT(BlueprintType)
struct FChukItemData : public FTableRowBase
{
	GENERATED_BODY()

	/** 화면에 보이는 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText DisplayName;

	/** 아이템 창에 보이는 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UTexture2D> Icon = nullptr;

	/** 효과 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EChukItemEffect Effect = EChukItemEffect::None;

	/** 효과량 (게이지를 얼마나 회복/악화시키는지) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float EffectAmount = 0.f;

	/** 최대 사용 횟수. 0 = 횟수 제한 없음 (등불, 일기) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0"))
	int32 MaxUses = 1;

	/** 켜면 인벤토리에 넣지 않고 줍는 순간 바로 효과 적용 (쥐 사체, 제웅) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bApplyOnPickup = false;
};

/** 인벤토리 안의 아이템 한 칸 (게임 중 바뀌는 값) */
USTRUCT(BlueprintType)
struct FChukInventorySlot
{
	GENERATED_BODY()

	/** 아이템 ID (DataTable 의 Row Name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemId;

	/** 남은 사용 횟수. 최대 횟수가 0(무제한)인 아이템은 의미 없음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 RemainingUses = 0;
};
