// 축영(逐影) - 인벤토리 / 등불 컴포넌트

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChukItemTypes.h"
#include "ChukInventoryComponent.generated.h"

class UDataTable;
class UChukGaugeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChukItemUsesChanged, FName, ItemId, int32, Remaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChukItemEvent, FName, ItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChukLanternEvent);

/**
 * 인벤토리 + 등불 컴포넌트
 *
 * ■ 아이템
 *   - 아이템 정보(이름, 아이콘, 효과, 횟수)는 Item Data Table 에서 읽는다. 아이템 ID = DataTable 의 Row Name.
 *   - 게임 시작 시 Starting Items(기본: Bell, Fan, Talisman, Lantern)를 최대 횟수로 받는다.
 *   - Use Item(ItemId): 남은 횟수 1 감소 + 게이지에 효과 적용 + On Item Uses Changed 이벤트.
 *     남은 횟수가 0이 되면 "비활성" (Is Item Usable = false). 목록에서 사라지지는 않음 (UI에서 흐리게 표시).
 *   - Acquire Item(ItemId): 아이템 획득.
 *       · 쥐 사체·제웅처럼 "줍는 즉시 적용"(bApplyOnPickup) 아이템은 인벤토리에 넣지 않고 바로 정신력 악화.
 *       · 일기(Effect = Diary)는 인벤토리에 넣지 않고 On Diary Read 이벤트만 발생 (내용 UI는 범위 밖).
 *       · 이미 가진 아이템이면 남은 횟수에 최대 횟수만큼 더함.
 *
 * ■ 등불
 *   - Lantern Duration(초) 동안 밝기가 1 → 0 으로 줄어든다. 0이 되면 꺼지고 On Lantern Extinguished.
 *   - 등잔(Effect = RelightLantern)을 사용하면 다시 최대 밝기 + On Lantern Relit.
 *   - Get Lantern Intensity 01 (0~1) 값을 블루프린트에서 라이트 컴포넌트의 Intensity 에 곱해서 쓴다.
 *
 * ■ 블루프린트에서 쓰는 방법
 *   1. 플레이어 캐릭터 블루프린트에 [+ Add] → "Chuk Inventory" 추가 (Chuk Gauge 도 같은 캐릭터에 있어야 효과가 적용됨)
 *   2. Details 의 Item Data Table 에 DT_ChukItems 지정
 *   3. 아이템 사용 키 입력 → Use Item 노드 (Item Id 에 Fan / Bell / Talisman / OilLamp 입력)
 *   4. 부채가 망가지는 / 방울이 사라지는 외형 변화: Events 의 On Item Uses Changed 에서 Item Id 와 Remaining 으로 처리
 *   5. 등불 라이트: 이벤트 그래프 Tick 에서 PointLight 의 Set Intensity = (최대 밝기) × Get Lantern Intensity 01
 */
UCLASS(ClassGroup = (Chuk), meta = (BlueprintSpawnableComponent, DisplayName = "Chuk Inventory"))
class GAME_API UChukInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChukInventoryComponent();

	// ===================== 설정 =====================

	/** 아이템 데이터 테이블 (Row Type = ChukItemData) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Inventory")
	TObjectPtr<UDataTable> ItemDataTable = nullptr;

	/** 게임 시작 시 가지고 있는 아이템 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Inventory")
	TArray<FName> StartingItems = { TEXT("Bell"), TEXT("Fan"), TEXT("Talisman"), TEXT("Lantern") };

	/** 효과가 필요 없는 상태에서도 아이템을 쓸 수 있는지 (예: 공포 0일 때 부채 사용). 끄면 그런 경우 사용이 막힘 */
	// TODO(기획 확인): 게이지가 이미 가득/비어 있을 때 회복 아이템을 쓸 수 있는지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Inventory")
	bool bAllowUseWhenNoEffect = true;

	/** 등불이 최대 밝기에서 완전히 꺼질 때까지 걸리는 시간(초) */
	// TODO(기획 확인): 등불 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Lantern", meta = (ClampMin = "1"))
	float LanternDuration = 300.f;

	/** 게임 시작 시 등불이 켜져 있는지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chuk|Lantern")
	bool bLanternLitAtStart = true;

	// ===================== 이벤트 =====================

	/** 아이템이 인벤토리에 새로 들어왔을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukItemEvent OnItemAdded;

	/** 아이템 남은 횟수가 바뀔 때 (사용, 추가 획득). 외형 변화·UI 갱신은 여기서 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukItemUsesChanged OnItemUsesChanged;

	/** 아이템을 사용했을 때 (사용 소리·이펙트 연결용) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukItemEvent OnItemUsed;

	/** 쥐 사체·제웅처럼 줍는 즉시 적용되는 아이템을 주웠을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukItemEvent OnPickupEffectApplied;

	/** 일기를 읽었을 때 (내용 UI 는 범위 밖) */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukItemEvent OnDiaryRead;

	/** 등불이 완전히 꺼졌을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukLanternEvent OnLanternExtinguished;

	/** 등잔으로 등불을 다시 밝혔을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Chuk|Events")
	FChukLanternEvent OnLanternRelit;

	// ===================== 아이템 함수 =====================

	/** 아이템 사용. 성공하면 true */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Inventory")
	bool UseItem(FName ItemId);

	/** 아이템 획득 (줍기). 성공하면 true */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Inventory")
	bool AcquireItem(FName ItemId);

	/** 아이템을 가지고 있는지 (횟수 0 이어도 목록에 있으면 true) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Inventory")
	bool HasItem(FName ItemId) const;

	/** 지금 사용할 수 있는지 (가지고 있고 + 남은 횟수 > 0 또는 무제한). UI 활성/비활성 표시에 사용 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Inventory")
	bool IsItemUsable(FName ItemId) const;

	/** 남은 횟수. 없으면 0, 무제한 아이템은 -1 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Inventory")
	int32 GetRemainingUses(FName ItemId) const;

	/** DataTable 에서 아이템 정보 가져오기. 없으면 false */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Inventory")
	bool GetItemData(FName ItemId, FChukItemData& OutData) const;

	/** 아이템 남은 횟수를 직접 지정 (세이브 불러오기 등). 없는 아이템이면 새로 추가 */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Inventory")
	void SetRemainingUses(FName ItemId, int32 Remaining);

	/** 인벤토리 전체 목록 (UI 용, 들어온 순서대로) */
	UFUNCTION(BlueprintPure, Category = "Chuk|Inventory")
	const TArray<FChukInventorySlot>& GetSlots() const { return Slots; }

	// ===================== 등불 함수 =====================

	/** 등불 밝기 0~1. 라이트 Intensity 에 곱해서 쓴다 */
	UFUNCTION(BlueprintPure, Category = "Chuk|Lantern")
	float GetLanternIntensity01() const;

	UFUNCTION(BlueprintPure, Category = "Chuk|Lantern")
	bool IsLanternLit() const { return LanternRemainingTime > 0.f; }

	/** 등불을 최대 밝기로 다시 켠다 (등잔 사용 시 자동 호출) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Lantern")
	void RelightLantern();

	/** 등불 남은 시간(초)을 직접 지정 (세이브 불러오기 등) */
	UFUNCTION(BlueprintCallable, Category = "Chuk|Lantern")
	void SetLanternRemainingTime(float Seconds);

	UFUNCTION(BlueprintPure, Category = "Chuk|Lantern")
	float GetLanternRemainingTime() const { return LanternRemainingTime; }

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TArray<FChukInventorySlot> Slots;

	float LanternRemainingTime = 0.f;

	const FChukItemData* FindItemData(FName ItemId) const;
	FChukInventorySlot* FindSlot(FName ItemId);
	const FChukInventorySlot* FindSlot(FName ItemId) const;
	UChukGaugeComponent* GetGauge() const;

	/** 효과를 적용. 효과가 의미 없는 상황이면(공포 0인데 부채 등) false */
	bool ApplyEffect(const FChukItemData& Data, bool bCheckOnly);
};
