// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISA.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ISACharacterBase.h"
#include "Utility/ISAGameplayTags.h"
#include "Utility/ISASettings.h"
#include "ISACharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Slide			UMETA(DisplayName = "Slide"),
	CMOVE_MAX			UMETA(Hidden),
};


UCLASS()
class ISA_API UISACharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()


#pragma region Parameters
public:
		UPROPERTY(EditDefaultsOnly) float Speed = 0;

private:
		// Parameters

	#pragma region Slide
		UPROPERTY(EditDefaultsOnly) float MinSlideSpeed = 200.f;
		UPROPERTY(EditDefaultsOnly) float MaxSlideSpeed = 500.f;
		UPROPERTY(EditDefaultsOnly) float SlideEnterImpulse = 200.f;
		UPROPERTY(EditDefaultsOnly) float SlideGravityForce = 5000.f;
		UPROPERTY(EditDefaultsOnly) float SlideFrictionFactor = .2f;
		UPROPERTY(EditDefaultsOnly) float BrakingDecelerationSliding = 2500.f;
	#pragma endregion	
		// Transient
		UPROPERTY(Transient) AISACharacterBase* ISACharacterBase;

	#pragma region Flags
public:
		bool bHasInput{false};
		bool bCanSprint{false};
private:
		bool bWantsToSprint;

		bool bHadAnimRootMotion;
		bool bPrevWantsToCrouch;
	#pragma endregion

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag MaxAllowedGait{ISAGaitTags::Walking};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Stance{ISAStanceTags::Standing};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|ISA Character")
	TObjectPtr<UISASettings> Settings;
	
	
#pragma endregion

public:
	UISACharacterMovementComponent();
	// Actor Component
protected: 
	virtual void InitializeComponent() override;
	// Character Movement Component
public:
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

protected:
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	// Slide
private:
	void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	void ExitSlide();
	bool CanSlide() const;
	void PhysSlide(float deltaTime, int32 Iterations);

	// Helpers
public:
	float CapR() const;
	float CapHH() const;
	
private:
	void SetupInputDirection(FVector NewInputDirection);

public:
	void SetStance(const FGameplayTag& NewStance);

	void SetMaxAllowedGait(const FGameplayTag& NewMaxAllowedGait);

private:
	void RefreshMaxWalkSpeed();

	void RefreshGaitSettings();
	
	// Interface
public:

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;
};
