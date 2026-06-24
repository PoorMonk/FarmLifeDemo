#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Plot/EPlotState.h"
#include "FarmPlotActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UCropDataAsset;

/**
 * 农田地块。
 * - 状态机:Raw → Tilled → Seeded → Mature → Tilled。
 * - 通过 IInteractable 接收玩家交互;从 Interactor 上的 IToolHolder 读工具 + 当前种子。
 * - 订阅 UTimeSubsystem::OnNewDay,跨天推进生长 + 重置 bWatered。
 * - 视觉拆为两件:
 *   - PlotMesh + DMI 表达土地状态(Raw / Tilled±Watered)
 *   - CropMesh 从 UCropDataAsset.GrowthStageMeshes 切阶段
 */
UCLASS()
class FARMLIFEDEMO_API AFarmPlotActor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AFarmPlotActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// === IInteractable ===
	virtual FText GetInteractText_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION()
	void HandleNewDay(int32 NewDayNumber);

	/** 一次刷新土地 + 作物两个子可视组件。 */
	void UpdateVisual();
	void UpdateSoilVisual();
	void UpdateCropVisual();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|Components")
	TObjectPtr<UStaticMeshComponent> PlotMesh;

	/** 新增:作物阶段 Mesh。挂在 SceneRoot 而不是 PlotMesh,避免被土地组件 Z=0.1 压扁。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|Components")
	TObjectPtr<UStaticMeshComponent> CropMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|State")
	EPlotState State = EPlotState::Raw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|State")
	bool bWatered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|State")
	int32 GrowthProgress = 0;

	/** 新增:当前种植作物配置。Tilled + Seed 时由 Interactor 写入。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|State")
	TObjectPtr<UCropDataAsset> Crop;

	// 删除:FName CropId / int32 GrowthDaysRequired —— 都搬进 UCropDataAsset

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
};