// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ISA.h"
#include "Utility/ISAGameplayTags.h"

#include "ISACharacter.generated.h"


UCLASS(config=Game)
class AISACharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) class UISACharacterMovementComponent* ISACharacterMovementComponent;

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
};

