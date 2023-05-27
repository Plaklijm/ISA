// Fill out your copyright notice in the Description page of Project Settings.

#include "ISAAnimation.h"

#include "ISACharacterBase.h"

void UISAAnimation::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ISACharacter = Cast<AISACharacterBase>(GetOwningActor());
}

void UISAAnimation::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UISAAnimation::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	LocomotionMode = ISACharacter->GetLocomotionMode();
	Stance = ISACharacter->GetStance();
	Gait = ISACharacter->GetGait();

	if (LocomotionAction != ISACharacter->GetLocomotionAction())
	{
		LocomotionAction = ISACharacter->GetLocomotionAction();
	}
}
