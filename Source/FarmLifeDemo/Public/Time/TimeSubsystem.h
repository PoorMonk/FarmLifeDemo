#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimeSubsystem.generated.h"

/**
 * 新一天事件。NewDayNumber 是结算后的新天数（从 1 开始）。
 * 订阅者在这里做跨天结算（地块生长推进、bWatered 重置等）。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewDay, int32, NewDayNumber);

/**
 * 时间系统桩。
 * 本轮只暴露"当前天数"和"手动跨天"。
 * Step 6 接入睡觉流程后会扩展为完整的小时 / 分钟时钟，AdvanceDay 改由睡觉流程触发，
 * 但 OnNewDay 这个事件签名不会改 —— 订阅方代码沿用即可。
 */
UCLASS()
class FARMLIFEDEMO_API UTimeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnNewDay OnNewDay;

	UFUNCTION(BlueprintCallable, Category = "Time")
	void AdvanceDay();

	UFUNCTION(BlueprintCallable, Category = "Time")
	int32 GetCurrentDay() const { return CurrentDay; }

private:
	UPROPERTY()
	int32 CurrentDay = 1;
};