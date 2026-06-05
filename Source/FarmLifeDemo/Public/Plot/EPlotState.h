// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EPlotState.generated.h"

/**
 * 农田地块状态机。
 * 状态转移由 AFarmPlotActor::Interact 根据 (State, Tool) 驱动；
 * Seeded → Mature 的进度推进由 UTimeSubsystem::OnNewDay 跨天触发。
 */
UENUM(BlueprintType)
enum class EPlotState : uint8
{
	Raw         UMETA(DisplayName = "Raw"),       // 普通土地
	Tilled      UMETA(DisplayName = "Tilled"),    // 已开垦
	Seeded      UMETA(DisplayName = "Seeded"),    // 已播种
	Mature      UMETA(DisplayName = "Mature"),    // 可收获
};
