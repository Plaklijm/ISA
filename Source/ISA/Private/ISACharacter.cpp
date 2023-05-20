// Copyright Epic Games, Inc. All Rights Reserved.

#include "ISACharacter.h"

#include "ISACharacterMovementComponent.h"
#include "Utility/ISASettings.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


// AISACharacter

AISACharacter::AISACharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UISACharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	// Set default CMC 
	ISACharacterMovementComponent = Cast<UISACharacterMovementComponent>(GetCharacterMovement());
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AISACharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AISACharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AISACharacter::Move);
	}
}

void AISACharacter::Jump()
{
	bPressedISAJump = true;

	Super::Jump();

	bPressedJump = false;
}

void AISACharacter::StopJumping()
{
	bPressedISAJump = false;

	Super::StopJumping();
}

void AISACharacter::Move(const FInputActionValue& Value)
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

FCollisionQueryParams AISACharacter::GetIgnoreCharacterParams() const
{
	// Ignore character when raycasting
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

void AISACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AISACharacter::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	//Checks if the player is on the ground or in the air, set the locomotionmode accordingly
	switch (GetCharacterMovement()->MovementMode)
	{
		case MOVE_Walking:
			SetLocomotionMode(ISALocomotionModeTags::Grounded);
			break;

		case MOVE_Falling:
			SetLocomotionMode(ISALocomotionModeTags::InAir);
			break;
		
		default:
			SetLocomotionMode(FGameplayTag::EmptyTag);
			break;

	}
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void AISACharacter::SetLocomotionMode(const FGameplayTag& NewLocomotionMode)
{
	//checks if the new mode is not the old one
	if (LocomotionMode != NewLocomotionMode)
	{
		const auto PreviousLocomotionMode{ LocomotionMode };

		//apply locomotionmode
		LocomotionMode = NewLocomotionMode;

		NotifyLocomotionModeChanged({ PreviousLocomotionMode });
	}

}

void AISACharacter::NotifyLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode)
{
	ApplyDesiredStance();

	if (LocomotionMode == ISALocomotionModeTags::Grounded && PreviousLocomotionMode == ISALocomotionModeTags::InAir)
	{
		UE_LOG(LogTemp, Error, TEXT("%f"), ISACharacterMovementComponent->Velocity.Z)
		if (GetISACharacterMovement()->Velocity.Z <= -Settings->TestValue)
		{
			UE_LOG(LogTemp, Warning, TEXT("StartRolling"));
			//StartRolling
			//Fix Velocity.Z (cant get value out of it for some reason)

		}
		else
		{
			ISACharacterMovementComponent->BrakingFrictionFactor = ISACharacterMovementComponent->bHasInput 
																									? Settings->HasInputBrakingFrictionFactor 
																									: Settings->NoInputBrakingFrictionFactor;

			GetWorldTimerManager().SetTimer(BrakingFrictionFactorResetTimer,
				FTimerDelegate::CreateWeakLambda(this, [this]
					{
						GetISACharacterMovement()->BrakingFrictionFactor = 1.0f;
						UE_LOG(LogTemp, Warning, TEXT("Braking = 0"));
					}), 0.5f, false);
			UE_LOG(LogTemp, Warning, TEXT("ELSE"));
		}
	}
}

void AISACharacter::SetDesiredStance(const FGameplayTag& NewDesiredStance)
{
	//Sets the stance the player wants to be in. This essentially queues the stance for it to be applied
	if (DesiredStance != NewDesiredStance)
	{
		DesiredStance = NewDesiredStance;

		ApplyDesiredStance();
	}
	
}

void AISACharacter::ApplyDesiredStance()
{
	if (!LocomotionAction.IsValid())
	{
		if (LocomotionMode == ISALocomotionModeTags::Grounded)
		{
			if (DesiredStance == ISAStanceTags::Standing)
			{
				UnCrouch();
			}
			else if (DesiredStance == ISAStanceTags::Crouching)
			{
				Crouch();
			}
		}
		else if (LocomotionMode == ISALocomotionModeTags::InAir)
		{
			UnCrouch();
		}
	}
	else if (LocomotionAction == ISALocomotionActionTags::Rolling)
	{
		Crouch();
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *(DesiredStance.ToString()))
}

void AISACharacter::SetStance(const FGameplayTag& NewStance)
{
}

void AISACharacter::SetDesiredGait(const FGameplayTag& NewDesiredGait)
{
}

void AISACharacter::SetGait(const FGameplayTag& NewGait)
{
}

void AISACharacter::RefreshGait()
{
}

FGameplayTag AISACharacter::CalculateMaxAllowedGait() const
{
	return FGameplayTag();
}

FGameplayTag AISACharacter::CalculateActualGait(const FGameplayTag& MaxAllowedGait) const
{
	return FGameplayTag();
}

void AISACharacter::SetLocomotionAction(const FGameplayTag& NewLocomotionAction)
{
}

void AISACharacter::NotifyLocomotionActionChanged(const FGameplayTag& PreviousLocomotionAction)
{
}