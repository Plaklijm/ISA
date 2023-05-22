// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Utility/ISAGameplayTags.h"
#include "ISA.h"

#include "ISACharacter.generated.h"

UCLASS(config=Game)
class AISACharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) 
	class UISACharacterMovementComponent* ISACharacterMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|ISA Character")
	TObjectPtr<UISASettings> Settings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tags|ISA Character", Transient)
	FGameplayTag DesiredStance { ISAStanceTags::Standing };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tags|ISA Character", Transient)
	FGameplayTag DesiredGait { ISAGaitTags::Walking };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tags|ISA Character", Transient)
	FGameplayTag LocomotionMode { ISALocomotionModeTags::Grounded };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tags|ISA Character", Transient)
	FGameplayTag LocomotionAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tags|ISA Character", Transient)
	FGameplayTag Stance { ISAStanceTags::Standing };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tags|ISA Character", Transient)
	FGameplayTag Gait { ISAGaitTags::Walking };

	FTimerHandle BrakingFrictionFactorResetTimer;

private:
	//Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	//Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	//MappingContext
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	//Jump Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	//Move Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DebugCommand;

public:
	bool bPressedISAJump;

public:
	AISACharacter(const FObjectInitializer& ObjectInitializer);
	
protected:
	// Called for movement input
	void Move(const FInputActionValue& Value);

public:
	virtual void Jump() override;
	virtual void StopJumping() override;

	virtual bool CanCrouch() const override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	void SetDebugCommand();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	//Returns CameraBoom subobject
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	//Returns FollowCamera subobject
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	//Returns MovementComponent
	FORCEINLINE class UISACharacterMovementComponent* GetISACharacterMovement() const { return ISACharacterMovementComponent; }
	//Returns Ignored Character Params
	FCollisionQueryParams GetIgnoreCharacterParams() const;

	virtual void Tick(float DeltaTime) override;

private:
	//void ApplyRotationYawSpeed(float DeltaTime);

//public:
//	UFUNCTION(BlueprintCallable, Category = "ALS|Als Character")
//		void TryStartRolling(float PlayRate = 1.0f);
//
//	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
//		UAnimMontage* SelectRollMontage();
//
//	bool IsRollingAllowedToStart(const UAnimMontage* Montage) const;
//
//private:
//	void StartRolling(float PlayRate, float TargetYawAngle);
//
//
//	UFUNCTION(NetMulticast, Reliable)
//		void MulticastStartRolling(UAnimMontage* Montage, float PlayRate, float StartYawAngle, float TargetYawAngle);
//
//	void StartRollingImplementation(UAnimMontage* Montage, float PlayRate, float StartYawAngle, float TargetYawAngle);
//
//	void RefreshRolling(float DeltaTime);
//
//	void RefreshRollingPhysics(float DeltaTime);
//
//public:
//	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
//		bool IsMantlingAllowedToStart() const;
//
//	UFUNCTION(BlueprintCallable, Category = "ALS|Als Character")
//		bool TryStartMantlingGrounded();
//
//private:
//	bool TryStartMantlingInAir();
//
//	bool TryStartMantling(const FAlsMantlingTraceSettings& TraceSettings);
//
//	UFUNCTION(NetMulticast, Reliable)
//		void MulticastStartMantling(const FAlsMantlingParameters& Parameters);
//
//	void StartMantlingImplementation(const FAlsMantlingParameters& Parameters);
//
//protected:
//	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
//		UAlsMantlingSettings* SelectMantlingSettings(EAlsMantlingType MantlingType);
//
//	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
//		void OnMantlingStarted(const FAlsMantlingParameters& Parameters);
//
//private:
//	void RefreshMantling();
//
//	void StopMantling();
//
//protected:
//	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
//		void OnMantlingEnded();

public:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode = 0) override;

#pragma region GameplayTags
//Locomotion Mode
private:
	void SetLocomotionMode(const FGameplayTag& NewLocomotionMode);

	void NotifyLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);

public:
	const FGameplayTag& GetLocomotionMode() const;

//Desired Stance
public:
	UFUNCTION(BlueprintCallable, Category = "ISA Character", Meta = (AutoCreateRefTerm = "NewDesiredStance"))
	void SetDesiredStance(const FGameplayTag& NewDesiredStance);

	const FGameplayTag& GetDesiredStance() const;

protected:
	virtual void ApplyDesiredStance();

//Stance
private:
	void SetStance(const FGameplayTag& NewStance);

public:
	const FGameplayTag& GetStance() const;

//Desired Gait
public:
	void SetDesiredGait(const FGameplayTag& NewDesiredGait);

	const FGameplayTag& GetDesiredGait() const;

//Gait
private:
	void SetGait(const FGameplayTag& NewGait);

	void RefreshGait();

	FGameplayTag CalculateMaxAllowedGait() const;

	FGameplayTag CalculateActualGait(const FGameplayTag& MaxAllowedGait) const;

public:
	const FGameplayTag& GetGait() const;

//Locomotion Action
public:
	void SetLocomotionAction(const FGameplayTag& NewLocomotionAction);

	const FGameplayTag& GetLocomotionAction() const;

	void NotifyLocomotionActionChanged(const FGameplayTag& PreviousLocomotionAction);

#pragma endregion
	
//Debug functions
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	
	void DisplayDebugStateInfo(const UCanvas* Canvas, const float Scale, const float HorizontalLocation, float& VerticalLocation) const;
	
	static FName GetSimpleTagName(const FGameplayTag& Tag);
};

#pragma region TagGettersImplementation
inline const FGameplayTag& AISACharacter::GetDesiredStance() const { return DesiredStance; }

inline const FGameplayTag& AISACharacter::GetDesiredGait() const { return DesiredGait; }

inline const FGameplayTag& AISACharacter::GetLocomotionMode() const { return LocomotionMode; }

inline const FGameplayTag& AISACharacter::GetLocomotionAction() const { return LocomotionAction; }

inline const FGameplayTag& AISACharacter::GetStance() const { return Stance; }

inline const FGameplayTag& AISACharacter::GetGait() const { return Gait; }
#pragma endregion