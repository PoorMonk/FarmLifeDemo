// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class USphereComponent;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractTargetChanged, AActor*, OldTarget, AActor*, NewTarget);

/**
 * 玩家交互组件（Overlap + 打分方案 / 星露谷风格）。
 *
 * 设计：
 * - 在 Owner 上自动创建 USphereComponent，使用自定义 Interact 通道做 Overlap。
 * - 实现 IInteractable 接口的对象进入 / 离开范围时，加入 / 移出候选集合。
 * - 定时（默认 0.1s）对候选集合打分，分数最高的成为当前交互目标。
 * - 打分综合考虑「玩家朝向夹角」和「距离」，玩家背后的对象直接淘汰。
 *
 * 不依赖摄像机方向，玩家走到对象旁边自然交互，无需精确瞄准。
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FARMLIFEDEMO_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Overlap 检测半径。建议 150~250cm。 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float DetectionRadius = 200.f;

    /** 朝向打分权重。越大越倾向于正前方的对象。 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Scoring")
    float ForwardWeight = 1.0f;

    /** 距离打分权重。越大越倾向于近的对象。 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Scoring")
    float DistanceWeight = 0.3f;

    /**
     * 朝向 Dot 低于此值的候选直接淘汰。
     * -0.2 ≈ 约 100° 可视范围；0 = 严格 180° 前方。
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Scoring")
    float MinForwardDot = -0.2f;

    /** 打分 / 更新当前目标的频率（秒）。0.1s 足够流畅又比 Tick 便宜。 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float UpdateInterval = 0.1f;

    /** 交互总开关。对话、菜单期间用 SetInteractionEnabled(false) 临时关闭。 */
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    bool bInteractionEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
    bool bDrawDebugInfo = false;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    TObjectPtr<AActor> CurrentInteractableActor;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractTargetChanged OnInteractTargetChanged;

public:
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void TryInteract();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    FText GetCurrentInteractText() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool HasInteractTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractionEnabled(bool bEnabled);

protected:
    /** 自动创建的玩家检测球。 */
    UPROPERTY(VisibleAnywhere, Category = "Interaction")
    TObjectPtr<USphereComponent> DetectionSphere;

    /** 当前所有重叠的、实现了 IInteractable 的候选 Actor。 */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> Candidates;

    FTimerHandle UpdateTimerHandle;

    UFUNCTION()
    void OnSphereBeginOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    /** 在候选集合中打分并刷新当前最佳目标。 */
    void UpdateBestCandidate();

    /** 对单个候选打分。返回 -FLT_MAX 表示淘汰。 */
    float ScoreCandidate(const AActor* Candidate) const;

    /** 切换当前目标，发出广播。 */
    void SetCurrentInteractableActor(AActor* NewTarget);
};
