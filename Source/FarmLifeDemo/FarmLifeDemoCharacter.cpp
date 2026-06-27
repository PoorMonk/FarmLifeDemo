// Copyright Epic Games, Inc. All Rights Reserved.

#include "FarmLifeDemoCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Public/Interaction/InteractionComponent.h"
#include "Time/TimeSubsystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Animation/AnimMontage.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AFarmLifeDemoCharacter

AFarmLifeDemoCharacter::AFarmLifeDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	
	ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
	ToolMesh->SetupAttachment(GetMesh(), TEXT("hand_r_weapon_socket"));
	ToolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ToolMesh->SetGenerateOverlapEvents(false);
	ToolMesh->SetVisibility(false);  // 默认 None,不显示
}

void AFarmLifeDemoCharacter::Interact()
{
	if (!PlayToolUseMontage())
	{
		if (InteractionComponent)
		{
			InteractionComponent->TryInteract();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFarmLifeDemoCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AFarmLifeDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFarmLifeDemoCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFarmLifeDemoCharacter::Look);
		
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFarmLifeDemoCharacter::Interact);
		}
		if (SelectHoeAction)
		{
			EnhancedInputComponent->BindAction(SelectHoeAction, ETriggerEvent::Started,this, &AFarmLifeDemoCharacter::SelectHoe);
		}
		if (SelectWateringCanAction)
		{
			EnhancedInputComponent->BindAction(SelectWateringCanAction, ETriggerEvent::Started,this, &AFarmLifeDemoCharacter::SelectWateringCan);
		}
		if (SelectSeedAction)
		{
			EnhancedInputComponent->BindAction(SelectSeedAction, ETriggerEvent::Started,this, &AFarmLifeDemoCharacter::SelectSeed);
		}
		if (SelectNoneAction)
		{
			EnhancedInputComponent->BindAction(SelectNoneAction, ETriggerEvent::Started,this, &AFarmLifeDemoCharacter::SelectNone);
		}
		if (DebugAdvanceDayAction)
		{
			EnhancedInputComponent->BindAction(DebugAdvanceDayAction, ETriggerEvent::Started,this, &AFarmLifeDemoCharacter::DebugAdvanceDay);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AFarmLifeDemoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFarmLifeDemoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFarmLifeDemoCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateToolMesh();
}

namespace
{
	const TCHAR* ToolTypeToString(EToolType Tool)
	{
		switch (Tool)
		{
		case EToolType::Hoe:         return TEXT("Hoe");
		case EToolType::WateringCan: return TEXT("Watering Can");
		case EToolType::Seed:        return TEXT("Seed");
		case EToolType::None:
		default:                     return TEXT("None");
		}
	}
}

void AFarmLifeDemoCharacter::ApplyTool(EToolType NewTool)
{
	CurrentTool = NewTool;

	const TCHAR* ToolName = ToolTypeToString(NewTool);

	UE_LOG(LogTemp, Log, TEXT("Tool: %s"), ToolName);

	if (GEngine)
	{
		// 用 UniqueID 作为 Key,后续切换会原地刷新而不是堆叠
		GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()),2.0f, FColor::Yellow, FString::Printf(TEXT("Tool: %s"), ToolName));
	}
	
	UpdateToolMesh();
}

void AFarmLifeDemoCharacter::DebugAdvanceDay()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSS = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSS->AdvanceDay();
		}
	}
}

void AFarmLifeDemoCharacter::UpdateToolMesh()
{
	if (!ToolMesh) return;

	UStaticMesh* MeshToShow = nullptr;
	if (const TObjectPtr<UStaticMesh>* Found = ToolMeshMap.Find(CurrentTool))
	{
		MeshToShow = *Found;
	}

	ToolMesh->SetStaticMesh(MeshToShow);
	ToolMesh->SetVisibility(MeshToShow != nullptr);
}

bool AFarmLifeDemoCharacter::PlayToolUseMontage()
{
	const TObjectPtr<UAnimMontage>* Found = ToolMontageMap.Find(CurrentTool);
	if (!Found || !*Found)
	{
		return false;
	}
	PlayAnimMontage(*Found);
	return true;
}

void AFarmLifeDemoCharacter::OnToolHitNotify()
{
	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
}