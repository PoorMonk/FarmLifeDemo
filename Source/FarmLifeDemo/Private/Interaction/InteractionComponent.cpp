// Fill out your copyright notice in the Description page of Project Settings.

#include "FarmLifeDemo/Public/Interaction/InteractionComponent.h"

#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "FarmLifeDemo/Public/Interaction/Interactable.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UInteractionComponent::UInteractionComponent()
{
    // 不再每帧 Tick，改用 Timer 驱动。
    PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !IsValid(Owner->GetRootComponent()))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Interaction] Owner or RootComponent invalid."));
        return;
    }

    // 1. NewObject
    DetectionSphere = NewObject<USphereComponent>(Owner, TEXT("InteractionDetectionSphere"));

    // 2. 在 Register 之前完成所有碰撞 / 形状配置
    DetectionSphere->InitSphereRadius(DetectionRadius);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
    DetectionSphere->SetGenerateOverlapEvents(true);

    // 3. 在 Register 之前指定附着关系
    DetectionSphere->SetupAttachment(Owner->GetRootComponent());

    // 4. 绑定 Overlap 事件（也要在 Register 之前）
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UInteractionComponent::OnSphereBeginOverlap);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &UInteractionComponent::OnSphereEndOverlap);

    // 5. 注册（这一步会创建 BodyInstance、加入物理场景、应用上面所有配置）
    DetectionSphere->RegisterComponent();

    // 6. 已经在场景里的可交互物，需要主动刷新一次 Overlap，否则不会回放 BeginOverlap
    DetectionSphere->UpdateOverlaps();

    // 7. 启动定时打分
    if (UpdateInterval > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UInteractionComponent::UpdateBestCandidate,
            UpdateInterval,
            true);
    }
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    if (IsValid(DetectionSphere))
    {
        DetectionSphere->OnComponentBeginOverlap.RemoveDynamic(this, &UInteractionComponent::OnSphereBeginOverlap);
        DetectionSphere->OnComponentEndOverlap.RemoveDynamic(this, &UInteractionComponent::OnSphereEndOverlap);
    }

    Candidates.Reset();
    Super::EndPlay(EndPlayReason);
}

void UInteractionComponent::OnSphereBeginOverlap(
    UPrimitiveComponent* /*OverlappedComp*/,
    AActor* OtherActor,
    UPrimitiveComponent* /*OtherComp*/,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult& /*SweepResult*/)
{
    UE_LOG(LogTemp, Warning, TEXT("[Interaction] BeginOverlap: %s, IsInteractable=%d"), *GetNameSafe(OtherActor),
        (OtherActor && OtherActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) ? 1 : 0);
    
    if (!IsValid(OtherActor) || OtherActor == GetOwner())
    {
        return;
    }

    if (!OtherActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        return;
    }

    Candidates.AddUnique(OtherActor);
}

void UInteractionComponent::OnSphereEndOverlap(
    UPrimitiveComponent* /*OverlappedComp*/,
    AActor* OtherActor,
    UPrimitiveComponent* /*OtherComp*/,
    int32 /*OtherBodyIndex*/)
{
    if (!IsValid(OtherActor))
    {
        return;
    }

    Candidates.Remove(OtherActor);

    // 如果当前目标离开了范围，立刻刷新（不等 timer），避免 UI 提示残留。
    if (OtherActor == CurrentInteractableActor)
    {
        UpdateBestCandidate();
    }
}

void UInteractionComponent::UpdateBestCandidate()
{
    if (!bInteractionEnabled)
    {
        SetCurrentInteractableActor(nullptr);
        return;
    }

    // 清理失效候选（被 Destroy / 切关卡等情况）。
    Candidates.RemoveAll([](const TObjectPtr<AActor>& A)
    {
        return !IsValid(A.Get());
    });

    AActor* Best = nullptr;
    float BestScore = -FLT_MAX;

    for (AActor* Candidate : Candidates)
    {
        const float Score = ScoreCandidate(Candidate);
        if (Score > BestScore)
        {
            BestScore = Score;
            Best = Candidate;
        }
    }

    SetCurrentInteractableActor(Best);

    if (bDrawDebugInfo && IsValid(GetOwner()) && GetWorld())
    {
        DrawDebugSphere(
            GetWorld(),
            GetOwner()->GetActorLocation(),
            DetectionRadius,
            16,
            FColor::Silver,
            false,
            UpdateInterval);

        if (IsValid(Best))
        {
            DrawDebugLine(
                GetWorld(),
                GetOwner()->GetActorLocation(),
                Best->GetActorLocation(),
                FColor::Green,
                false,
                UpdateInterval,
                0,
                2.f);
        }
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("[Interaction] Candidates=%d, Best=%s, Score=%f"), Candidates.Num(), *GetNameSafe(Best), BestScore);
}

float UInteractionComponent::ScoreCandidate(const AActor* Candidate) const
{
    const AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !IsValid(Candidate))
    {
        return -FLT_MAX;
    }

    const FVector ToTarget = (Candidate->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
    const FVector Forward  = Owner->GetActorForwardVector().GetSafeNormal2D();

    const float Dot = FVector::DotProduct(Forward, ToTarget);
    if (Dot < MinForwardDot)
    {
        return -FLT_MAX;
    }

    const float Distance = FVector::Dist2D(Candidate->GetActorLocation(), Owner->GetActorLocation());
    const float NormalizedDistance =
        (DetectionRadius > KINDA_SMALL_NUMBER) ? (Distance / DetectionRadius) : 0.f;

    return Dot * ForwardWeight - NormalizedDistance * DistanceWeight;
}

void UInteractionComponent::SetCurrentInteractableActor(AActor* NewTarget)
{
    if (CurrentInteractableActor == NewTarget)
    {
        return;
    }

    AActor* OldTarget = CurrentInteractableActor;
    CurrentInteractableActor = NewTarget;

    OnInteractTargetChanged.Broadcast(OldTarget, CurrentInteractableActor);

    if (bDrawDebugInfo && GEngine && IsValid(CurrentInteractableActor))
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            0.2f,
            FColor::Cyan,
            GetCurrentInteractText().ToString());
    }
}

void UInteractionComponent::TryInteract()
{
    if (!bInteractionEnabled)
    {
        return;
    }

    if (!IsValid(CurrentInteractableActor))
    {
        return;
    }

    if (!CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        return;
    }
    
    IInteractable::Execute_Interact(CurrentInteractableActor, GetOwner());
}

FText UInteractionComponent::GetCurrentInteractText() const
{
    if (!IsValid(CurrentInteractableActor))
    {
        return FText::GetEmpty();
    }

    if (!CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        return FText::GetEmpty();
    }

    return IInteractable::Execute_GetInteractText(CurrentInteractableActor);
}

bool UInteractionComponent::HasInteractTarget() const
{
    return IsValid(CurrentInteractableActor);
}

void UInteractionComponent::SetInteractionEnabled(bool bEnabled)
{
    if (bInteractionEnabled == bEnabled)
    {
        return;
    }

    bInteractionEnabled = bEnabled;

    if (!bEnabled)
    {
        // 禁用时立即清空当前目标，避免 UI 残留提示。
        SetCurrentInteractableActor(nullptr);
    }
    else
    {
        // 重新启用时立刻刷新一次。
        UpdateBestCandidate();
    }
}