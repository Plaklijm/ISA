// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Utility/ISAGameplayTags.h"

#include "ISAAnimation.generated.h"

class AISACharacterBase;

UCLASS()
class ISA_API UISAAnimation : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<AISACharacterBase> ISACharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
FGameplayTag LocomotionMode{ISALocomotionModeTags::Grounded};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Stance{ISAStanceTags::Standing};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Gait{ISAGaitTags::Walking};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag LocomotionAction;

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;
	

	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;
};
