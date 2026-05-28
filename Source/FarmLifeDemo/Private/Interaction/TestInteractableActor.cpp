// Fill out your copyright notice in the Description page of Project Settings.

#include "FarmLifeDemo/Public/Interaction/TestInteractableActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

ATestInteractableActor::ATestInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}

	// 碰撞配置：
	// - ObjectType 设为自定义 Interact 通道，让玩家身上的检测球能识别它。
	// - 对其他通道默认 Block（保持作为正常物理障碍）。
	// - 开启 Overlap 事件，配合 InteractionComponent 的 Sphere。
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_GameTraceChannel1); // Interact
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetGenerateOverlapEvents(true);
}

FText ATestInteractableActor::GetInteractText_Implementation() const
{
	return InteractText;
}

void ATestInteractableActor::Interact_Implementation(AActor* Interactor)
{
	const FString InteractorName = IsValid(Interactor)
		? Interactor->GetName()
		: TEXT("None");

	const FString Message = FString::Printf(
		TEXT("Interact Success: %s | Interactor: %s"),
		*DebugName,
		*InteractorName);

	UE_LOG(LogTemp, Log, TEXT("%s"), *Message);

	if (GEngine)
	{
		// 用 UniqueID 作为稳定 Key，同一对象重复交互会替换上一条而不是堆叠。
		GEngine->AddOnScreenDebugMessage(
			static_cast<int32>(GetUniqueID()),
			2.f,
			FColor::Green,
			Message);
	}
}