// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EToolType.generated.h"

/**
 * 玩家当前持有的工具。
 * Step 5 引入背包后，此枚举会被 UItemDataAsset + EItemCategory::Tool 取代；
 * 在那之前作为 IToolHolder::GetCurrentTool 的返回值，承担"当前工具"这件事。
 */
UENUM(BlueprintType)
enum class EToolType : uint8
{
	None         UMETA(DisplayName = "None"),
	Hoe          UMETA(DisplayName = "Hoe"),
	WateringCan  UMETA(DisplayName = "Watering Can"),
	Seed         UMETA(DisplayName = "Seed"),
};
