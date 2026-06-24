#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CropDataAsset.generated.h"

class UStaticMesh;

/**
 * 作物配置数据。
 * 通过 UPrimaryDataAsset 在 Content/Data/Crops/ 下管理,新增作物零 C++ 改动。
 * 本轮只放跟"长大"相关的字段;SeedItem / HarvestItem / SellPrice 在对应 Step 接入。
 */
UCLASS(BlueprintType)
class FARMLIFEDEMO_API UCropDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 唯一标识,用作 PrimaryAssetId 的 PrimaryAssetName。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crop")
	FName CropId = NAME_None;

	/** UI 显示用名字。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crop")
	FText DisplayName;

	/** 从播种到成熟所需天数。地块的成熟阈值从这里读。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crop", meta = (ClampMin = "1"))
	int32 GrowthDaysRequired = 3;

	/**
	 * 阶段 Mesh,从早到晚。
	 * Seeded 状态按 GrowthProgress 比例线性映射到数组下标;Mature 强制取最后一项。
	 * 空数组合法 —— 地块仅不显示作物 Mesh,逻辑不受影响。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crop")
	TArray<TObjectPtr<UStaticMesh>> GrowthStageMeshes;

	//~ UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("Crop"), CropId);
	}
	//~ End UPrimaryDataAsset interface
};