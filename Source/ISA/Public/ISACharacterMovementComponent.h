// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISA.h"
#include "GameFramework/CharacterMovementComponent.h"
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
private:
		// Parameters
		UPROPERTY(EditDefaultsOnly) float MaxSprintSpeed = 750.f;

	#pragma region Slide
		UPROPERTY(EditDefaultsOnly) float MinSlideSpeed = 400.f;
		UPROPERTY(EditDefaultsOnly) float MaxSlideSpeed = 400.f;
		UPROPERTY(EditDefaultsOnly) float SlideEnterImpulse = 400.f;
		UPROPERTY(EditDefaultsOnly) float SlideGravityForce = 4000.f;
		UPROPERTY(EditDefaultsOnly) float SlideFrictionFactor = .06f;
		UPROPERTY(EditDefaultsOnly) float BrakingDecelerationSliding = 1000.f;
	#pragma endregion	
		// Transient
		UPROPERTY(Transient) AISACharacter* ISACharacterOwner;

	#pragma region Flags
		bool bWantsToSprint;

		bool bHadAnimRootMotion;
		bool bPrevWantsToCrouch;
	#pragma endregion

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
private:
	float CapR() const;
	float CapHH() const;

	// Interface
public:
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();

	UFUNCTION(BlueprintCallable) void CrouchPressed();

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;
};