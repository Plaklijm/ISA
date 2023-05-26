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
		EnhancedInput->BindAction(ForceModeAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnForceMode);
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

void AISAPlayerCharacter::Input_OnForceMode(const FInputActionValue& ActionValue)
{
	
	if (!ActionValue.Get<bool>())
	{
		SetForceGait(true, false);
	}
	else if (ActionValue.Get<bool>())
	{
		SetForceGait(false, true);
	}
}

void AISAPlayerCharacter::Input_OnSprint(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), ActionValue.Get<bool>() ? TEXT("True") : TEXT("False"));
	if (bForceWalkRun)
	{
		SetDesiredGait(ActionValue.Get<bool>() ? ISAGaitTags::Running : ISAGaitTags::Walking);
	}
	else if (bForceRunSprint)
	{
		SetDesiredGait(ActionValue.Get<bool>() ? ISAGaitTags::Sprinting : ISAGaitTags::Running);
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
		//TryStartSliding();
	}
	else if (GetDesiredStance() == ISAStanceTags::Crouching)
	{
		SetDesiredStance(ISAStanceTags::Standing);
	}
}

