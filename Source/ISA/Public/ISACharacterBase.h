// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "DrawDebugHelpers.h"
#include "Utility/ISAGameplayTags.h"
#include "Utility/ISASettings.h"
#include "ISA.h"

#include "ISACharacterBase.generated.h"

UCLASS(config=Game)
class AISACharacterBase : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) 
	class UISACharacterMovementComponent* ISACharacterMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|ISA Character")
	TObjectPtr<UISASettings> GeneralSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|ISA Character")
	TObjectPtr<UMantleSettings> MantleSettings;

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

public:
	bool bPressedISAJump;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force ISA Player")
	bool bForceWalkRun;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force ISA Player")
	bool bForceRunSprint;

public:
	AISACharacterBase(const FObjectInitializer& ObjectInitializer);

public:
	virtual void Jump() override;
	virtual void StopJumping() override;

	virtual bool CanCrouch() const override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	bool CanSprint() const;
	
	void SetDebugCommand();

	virtual void PreRegisterAllComponents() override;
protected:
	// To add mapping context
	virtual void BeginPlay() override;

	void SetForceGait(bool bWalk_Run, bool bRunSprint);

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

public:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode = 0) override;

private:
	void RefreshLocomotion(const float DeltaTime);

protected:
	void MantleTrace();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetupMantle();
	
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

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
	void OnGaitChanged(const FGameplayTag& PreviousGaitTag);

//Locomotion Action
public:
	UFUNCTION(BlueprintCallable, Category = "Als Character")
	void SetLocomotionAction(const FGameplayTag& NewLocomotionAction);

	const FGameplayTag& GetLocomotionAction() const;

	void NotifyLocomotionActionChanged(const FGameplayTag& PreviousLocomotionAction);

//Locomotion Actions
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
	UAnimMontage* SelectRollMontage();
protected:
	void TryStartSliding();
	bool IsAllowedToSlide(const UAnimMontage* Montage) const;
	void StartSliding();
	void StartSlidingImplementation(UAnimMontage* Montage);

#pragma endregion
	
//Debug functions
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	
	void DisplayDebugStateInfo(const UCanvas* Canvas, const float Scale, const float HorizontalLocation, float& VerticalLocation) const;
	
	static FName GetSimpleTagName(const FGameplayTag& Tag);
};

#pragma region TagGettersImplementation
inline const FGameplayTag& AISACharacterBase::GetDesiredStance() const { return DesiredStance; }

inline const FGameplayTag& AISACharacterBase::GetDesiredGait() const { return DesiredGait; }

inline const FGameplayTag& AISACharacterBase::GetLocomotionMode() const { return LocomotionMode; }

inline const FGameplayTag& AISACharacterBase::GetLocomotionAction() const { return LocomotionAction; }

inline const FGameplayTag& AISACharacterBase::GetStance() const { return Stance; }

inline const FGameplayTag& AISACharacterBase::GetGait() const { return Gait; }
#pragma endregion