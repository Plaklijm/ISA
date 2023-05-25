// Fill out your copyright notice in the Description page of Project Settings.


#include "ISAPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ISACharacterMovementComponent.h"


void AISAPlayerCharacter::NotifyControllerChanged()
{
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	Super::NotifyControllerChanged();
}


// Called to bind functionality to input
void AISAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);
	
	auto* EnhancedInput{Cast<UEnhancedInputComponent>(Input)};

	if (IsValid(EnhancedInput))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnMove);
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRun);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnSprint);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnJump);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnCrouch);
		EnhancedInput->BindAction(DebugCommand, ETriggerEvent::Triggered, this, &ThisClass::SetDebugCommand);
	}
}

void AISAPlayerCharacter::Input_OnMove(const FInputActionValue& ActionValue)
{
	// input is a Vector2D
	FVector2D MovementVector = ActionValue.Get<FVector2D>();

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

void AISAPlayerCharacter::Input_OnRun(const FInputActionValue& ActionValue)
{
	if (GetDesiredGait() == ISAGaitTags::Walking)
	{
		SetDesiredGait(ISAGaitTags::Running);
	}
	else if (GetDesiredGait() == ISAGaitTags::Running)
	{
		SetDesiredGait(ISAGaitTags::Walking);
	}
}

void AISAPlayerCharacter::Input_OnSprint(const FInputActionValue& ActionValue)
{
	if (GetDesiredGait() == ISAGaitTags::Running)
	{
		SetDesiredGait(ISAGaitTags::Sprinting);
	}
	else if (GetDesiredGait() == ISAGaitTags::Sprinting)
	{
		SetDesiredGait(ISAGaitTags::Running);
	}
	GetISACharacterMovement()->bCanSprint = ActionValue.Get<bool>();
}

void AISAPlayerCharacter::Input_OnJump(const FInputActionValue& ActionValue)
{
	if (ActionValue.Get<bool>())
	{
		/*if (TryStartMantlingGrounded())
		{
			return;
		}*/

		if (GetStance() == ISAStanceTags::Crouching)
		{
			SetDesiredStance(ISAStanceTags::Standing);
			return;
		}

		Jump();
	}
	else
	{
		StopJumping();
	}
}

void AISAPlayerCharacter::Input_OnCrouch()
{
	if (GetDesiredStance() == ISAStanceTags::Standing)
	{
		SetDesiredStance(ISAStanceTags::Crouching);
		TryStartSliding();
	}
	else if (GetDesiredStance() == ISAStanceTags::Crouching)
	{
		SetDesiredStance(ISAStanceTags::Standing);
	}
}

