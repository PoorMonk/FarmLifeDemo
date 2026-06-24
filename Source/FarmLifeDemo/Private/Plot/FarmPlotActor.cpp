#include "Plot/FarmPlotActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

#include "Crop/CropDataAsset.h"
#include "Time/TimeSubsystem.h"
#include "Tools/EToolType.h"
#include "Tools/ToolHolder.h"

DEFINE_LOG_CATEGORY_STATIC(LogFarmPlot, Log, All);

AFarmPlotActor::AFarmPlotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// --- 土地组件 ---
	PlotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlotMesh"));
	PlotMesh->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		PlotMesh->SetStaticMesh(CubeMeshFinder.Object);
	}

	PlotMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.1f));  // 压成薄方块

	PlotMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlotMesh->SetCollisionObjectType(ECC_GameTraceChannel1);
	PlotMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PlotMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	PlotMesh->SetGenerateOverlapEvents(true);

	// --- 作物组件 ---
	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CropMesh"));
	CropMesh->SetupAttachment(SceneRoot);  // 关键:挂 SceneRoot,不挂 PlotMesh
	CropMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));  // 大致在 PlotMesh 顶面上方
	CropMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CropMesh->SetGenerateOverlapEvents(false);
	CropMesh->SetVisibility(false);
}

void AFarmPlotActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSS = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSS->OnNewDay.AddDynamic(this, &AFarmPlotActor::HandleNewDay);
		}
	}

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
	// 取工具
	EToolType Tool = EToolType::None;
	if (Interactor && Interactor->Implements<UToolHolder>())
	{
		Tool = IToolHolder::Execute_GetCurrentTool(Interactor);
	}

	const EPlotState PrevState = State;
	const bool bPrevWatered = bWatered;
	const UCropDataAsset* PrevCrop = Crop;

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
			// 从 Interactor 取选中的种子(同一接口的另一个方法)
			UCropDataAsset* SeedToPlant = nullptr;
			if (Interactor && Interactor->Implements<UToolHolder>())
			{
				SeedToPlant = IToolHolder::Execute_GetCurrentSeed(Interactor);
			}
			if (!SeedToPlant)
			{
				UE_LOG(LogFarmPlot, Log, TEXT("Plot %s: tried to plant but interactor has no CurrentSeed."), *GetName());
				break;
			}
			State = EPlotState::Seeded;
			Crop = SeedToPlant;
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
		// 任意工具收获 → Tilled,清空作物相关状态
		State = EPlotState::Tilled;
		Crop = nullptr;
		GrowthProgress = 0;
		bWatered = false;
		UE_LOG(LogFarmPlot, Log, TEXT("Harvested plot %s"), *GetName());
		break;

	default:
		break;
	}

	if (State != PrevState || bWatered != bPrevWatered || Crop != PrevCrop)
	{
		UpdateVisual();
	}
}

void AFarmPlotActor::HandleNewDay(int32 NewDayNumber)
{
	if (State == EPlotState::Seeded && bWatered && Crop)
	{
		++GrowthProgress;
		if (GrowthProgress >= Crop->GrowthDaysRequired)
		{
			State = EPlotState::Mature;
		}
	}

	bWatered = false;

	UpdateVisual();
}

void AFarmPlotActor::UpdateVisual()
{
	UpdateSoilVisual();
	UpdateCropVisual();
}

void AFarmPlotActor::UpdateSoilVisual()
{
	if (!DynamicMaterial)
	{
		return;
	}

	FLinearColor Color;
	if (State == EPlotState::Raw)
	{
		Color = FLinearColor(0.60f, 0.40f, 0.20f);  // 浅棕(未开垦)
	}
	else
	{
		// Tilled / Seeded / Mature 共用土壤色,只看 bWatered
		Color = bWatered
			? FLinearColor(0.30f, 0.20f, 0.10f)     // 深棕(已浇水)
			: FLinearColor(0.45f, 0.30f, 0.15f);    // 中棕(未浇水)
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}

void AFarmPlotActor::UpdateCropVisual()
{
	if (!CropMesh)
	{
		return;
	}

	const bool bShouldShow =
		Crop && Crop->GrowthStageMeshes.Num() > 0
		&& (State == EPlotState::Seeded || State == EPlotState::Mature);

	if (!bShouldShow)
	{
		CropMesh->SetVisibility(false);
		CropMesh->SetStaticMesh(nullptr);
		return;
	}

	const int32 Num = Crop->GrowthStageMeshes.Num();
	int32 Stage = 0;
	if (State == EPlotState::Mature)
	{
		Stage = Num - 1;
	}
	else  // Seeded
	{
		const int32 Days = FMath::Max(1, Crop->GrowthDaysRequired);
		Stage = FMath::Clamp((GrowthProgress * Num) / Days, 0, Num - 1);
	}

	CropMesh->SetStaticMesh(Crop->GrowthStageMeshes[Stage]);
	CropMesh->SetVisibility(true);
}