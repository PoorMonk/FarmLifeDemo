#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Plot/EPlotState.h"
#include "FarmPlotActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * 农田地块。
 * - 状态机：Raw → Tilled → Seeded → Mature → Tilled。
 * - 通过 IInteractable 接收玩家交互；从 Interactor 上的 IToolHolder 读取当前工具决定转移。
 * - 订阅 UTimeSubsystem::OnNewDay,跨天推进生长 + 重置 bWatered。
 * - 视觉用单个 DMI 的 BaseColor 表达,不引入子 Mesh。Step 4 接入 UCropDataAsset 后整体替换。
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

	void UpdateVisual();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|Components")
	TObjectPtr<UStaticMeshComponent> PlotMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|State")
	EPlotState State = EPlotState::Raw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|State")
	bool bWatered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plot|State")
	int32 GrowthProgress = 0;

	/** Step 4 替换为 UCropDataAsset 引用。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plot|Config")
	FName CropId = NAME_None;

	/** Step 4 由 CropData 提供。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plot|Config", meta = (ClampMin = "1"))
	int32 GrowthDaysRequired = 3;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
};