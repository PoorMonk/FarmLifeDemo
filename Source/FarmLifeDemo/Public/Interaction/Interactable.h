// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class AActor;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FARMLIFEDEMO_API IInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * 当前是否允许交互。
	 * 例如：地块未开垦时不响应水壶、出售箱夜间不可交互。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FarmLife|Interaction")
	bool CanInteract(AActor* Interactor) const;

	/**
	 * 交互提示文本（不含按键，按键由 HUD 拼接）。
	 * 例如："开垦土地" / "查看出售箱"。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FarmLife|Interaction")
	FText GetInteractText() const;

	/**
	 * 执行交互。Interactor 通常是玩家角色。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FarmLife|Interaction")
	void Interact(AActor* Interactor);
};
