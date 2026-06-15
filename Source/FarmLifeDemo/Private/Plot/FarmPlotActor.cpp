#include "Plot/FarmPlotActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

#include "Time/TimeSubsystem.h"
#include "Tools/EToolType.h"
#include "Tools/ToolHolder.h"

DEFINE_LOG_CATEGORY_STATIC(LogFarmPlot, Log, All);

AFarmPlotActor::AFarmPlotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PlotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlotMesh"));
	PlotMesh->SetupAttachment(SceneRoot);

	// 默认用引擎自带 Cube 占位,在蓝图里可以覆写
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		PlotMesh->SetStaticMesh(CubeMeshFinder.Object);
	}

	// 默认压扁成 100×100×10 的薄方块,玩家能站上去也能识别为"地"
	PlotMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.1f));

	// 碰撞:作为 Interact 通道的 Object,对该通道 Overlap,其他 Block
	PlotMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlotMesh->SetCollisionObjectType(ECC_GameTraceChannel1);
	PlotMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PlotMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	PlotMesh->SetGenerateOverlapEvents(true);
}

void AFarmPlotActor::BeginPlay()
{
	Super::BeginPlay();

	// 订阅跨天事件
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSS = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSS->OnNewDay.AddDynamic(this, &AFarmPlotActor::HandleNewDay);
		}
	}

	// 创建 DMI 并刷一次初始颜色
	if (PlotMesh)
	{
		DynamicMaterial = PlotMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	UpdateVisual();
}

void AFarmPlotActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSS = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSS->OnNewDay.RemoveDynamic(this, &AFarmPlotActor::HandleNewDay);
		}
	}

	Super::EndPlay(EndPlayReason);
}

FText AFarmPlotActor::GetInteractText_Implementation() const
{
	switch (State)
	{
	case EPlotState::Raw:    return FText::FromString(TEXT("Plot (Raw)"));
	case EPlotState::Tilled: return FText::FromString(TEXT("Plot (Tilled)"));
	case EPlotState::Seeded: return FText::FromString(TEXT("Plot (Seeded)"));
	case EPlotState::Mature: return FText::FromString(TEXT("Plot (Mature) - Harvest"));
	default:                 return FText::FromString(TEXT("Plot"));
	}
}

void AFarmPlotActor::Interact_Implementation(AActor* Interactor)
{
	// 从 Interactor 拿当前工具,默认 None
	EToolType Tool = EToolType::None;
	if (Interactor && Interactor->Implements<UToolHolder>())
	{
		Tool = IToolHolder::Execute_GetCurrentTool(Interactor);
	}

	const EPlotState PrevState = State;
	const bool bPrevWatered = bWatered;

	switch (State)
	{
	case EPlotState::Raw:
		if (Tool == EToolType::Hoe)
		{
			State = EPlotState::Tilled;
		}
		break;

	case EPlotState::Tilled:
		if (Tool == EToolType::Seed)
		{
			State = EPlotState::Seeded;
			CropId = FName("DefaultCrop");  // Step 4: 从背包当前选中种子读
			GrowthProgress = 0;
		}
		else if (Tool == EToolType::WateringCan)
		{
			bWatered = true;
		}
		break;

	case EPlotState::Seeded:
		if (Tool == EToolType::WateringCan)
		{
			bWatered = true;
		}
		break;

	case EPlotState::Mature:
		// 任意工具(含空手)都可收获 → 回到 Tilled,清空作物相关状态
		State = EPlotState::Tilled;
		CropId = NAME_None;
		GrowthProgress = 0;
		bWatered = false;
		// Step 5:把产物加入背包
		UE_LOG(LogFarmPlot, Log, TEXT("Harvested plot %s"), *GetName());
		break;

	default:
		break;
	}

	// 状态没变就不刷视觉,省一次 SetVectorParameterValue
	if (State != PrevState || bWatered != bPrevWatered)
	{
		UpdateVisual();
	}
}

void AFarmPlotActor::HandleNewDay(int32 NewDayNumber)
{
	// 已浇水的 Seeded → 推进进度,达到阈值则成熟
	if (State == EPlotState::Seeded && bWatered)
	{
		++GrowthProgress;
		if (GrowthProgress >= GrowthDaysRequired)
		{
			State = EPlotState::Mature;
		}
	}
	// 否则进度不变也不回退(无需 else 分支)

	// 跨天统一重置浇水标记
	bWatered = false;

	UpdateVisual();
}

void AFarmPlotActor::UpdateVisual()
{
	if (!DynamicMaterial)
	{
		return;
	}

	FLinearColor Color;
	switch (State)
	{
	case EPlotState::Raw:
		Color = FLinearColor(0.60f, 0.40f, 0.20f);  // 浅棕
		break;

	case EPlotState::Tilled:
		Color = bWatered
			? FLinearColor(0.30f, 0.20f, 0.10f)     // 深棕
			: FLinearColor(0.45f, 0.30f, 0.15f);    // 中棕
		break;

	case EPlotState::Seeded:
		Color = bWatered
			? FLinearColor(0.25f, 0.45f, 0.10f)     // 深棕 + 绿
			: FLinearColor(0.40f, 0.50f, 0.15f);    // 中棕 + 绿
		break;

	case EPlotState::Mature:
		Color = FLinearColor(0.80f, 0.60f, 0.10f);  // 偏黄收获色
		break;

	default:
		Color = FLinearColor::White;
		break;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}