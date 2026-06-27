// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyToolHit.h"
#include "Components/SkeletalMeshComponent.h"
#include "FarmLifeDemo/FarmLifeDemoCharacter.h"

void UAnimNotifyToolHit::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	if (AFarmLifeDemoCharacter* Character = Cast<AFarmLifeDemoCharacter>(Owner))
	{
		Character->OnToolHitNotify();
	}
}

FString UAnimNotifyToolHit::GetNotifyName_Implementation() const
{
	return TEXT("ToolHit");
}
