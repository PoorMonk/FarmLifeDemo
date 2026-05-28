// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "TestInteractableActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class FARMLIFEDEMO_API ATestInteractableActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestInteractableActor();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** 交互提示文本。不含按键，按键由 HUD 拼接。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractText = FText::FromString(TEXT("Interact"));

	/** 调试名，用于区分场景中放置的多个测试方块。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FString DebugName = TEXT("Test Interactable");

	// IInteractable
	virtual FText GetInteractText_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
};
