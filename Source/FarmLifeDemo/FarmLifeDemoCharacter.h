// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Tools/EToolType.h"
#include "Tools/ToolHolder.h"
#include "FarmLifeDemoCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UInteractionComponent;
class UCropDataAsset;
class UStaticMeshComponent;
class UStaticMesh;
class UAnimMontage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AFarmLifeDemoCharacter : public ACharacter, public IToolHolder
{
	GENERATED_BODY()
public:
	AFarmLifeDemoCharacter();
	
	/**
	 * 新增：交互输入回调。
	 */
	void Interact();
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	UFUNCTION(BlueprintCallable, Category = "Tools|Visual")
	void UpdateToolMesh();
	
	/** 播放对应 Montage,返回是否真的播了(没匹配到 Montage → false)。 */
	UFUNCTION(BlueprintCallable, Category = "Tools|Visual")
	bool PlayToolUseMontage();

	/** 由 UAnimNotify_ToolHit 在挥击峰值帧调用,真正触发交互。 */
	UFUNCTION(BlueprintCallable, Category = "Tools|Visual")
	void OnToolHitNotify();

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	virtual void BeginPlay() override;
	
	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void SelectHoe()         { ApplyTool(EToolType::Hoe); }
	void SelectWateringCan() { ApplyTool(EToolType::WateringCan); }
	void SelectSeed()        { ApplyTool(EToolType::Seed); }
	void SelectNone()        { ApplyTool(EToolType::None); }
	void DebugAdvanceDay();
	
private:
	void ApplyTool(EToolType NewTool);
	
public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> InteractAction;
	
	// === Tools ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tools")
	EToolType CurrentTool = EToolType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools")
	TObjectPtr<UCropDataAsset> CurrentSeed = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Tools")
	TObjectPtr<UInputAction> SelectHoeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Tools")
	TObjectPtr<UInputAction> SelectWateringCanAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Tools")
	TObjectPtr<UInputAction> SelectSeedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Tools")
	TObjectPtr<UInputAction> SelectNoneAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> DebugAdvanceDayAction;

	// === IToolHolder ===
	virtual EToolType GetCurrentTool_Implementation() const override { return CurrentTool; }
	
	virtual UCropDataAsset* GetCurrentSeed_Implementation() const override { return CurrentSeed; }
	
	/**
	 * 新增：玩家交互组件。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction)
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
	// === 工具视觉 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tools|Visual")
	TObjectPtr<UStaticMeshComponent> ToolMesh;

	/** 工具 → 手持 Mesh 的映射,蓝图里填表。Key 没匹配到时手部为空。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools|Visual")
	TMap<EToolType, TObjectPtr<UStaticMesh>> ToolMeshMap;

	/** 工具 → 使用动画 Montage 的映射,蓝图填表。未匹配时不播,沿用 Locomotion。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools|Visual")
	TMap<EToolType, TObjectPtr<UAnimMontage>> ToolMontageMap;

	
};

