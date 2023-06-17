// Fill out your copyright notice in the Description page of Project Settings.


#include "ISACharacterMovementComponent.h"

#include "VectorUtil.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"


UISACharacterMovementComponent::UISACharacterMovementComponent()
{
	//Init so player can crouch
	NavAgentProps.bCanCrouch = true;
}

#pragma region CMC

void UISACharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	//Sets character variable
	ISACharacterBase = Cast<AISACharacterBase>(GetOwner());
}

// Getters / Helpers
bool UISACharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UISACharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

float UISACharacterMovementComponent::GetMaxSpeed() const
{
	//returns max speed depending on MovementMode
	if (IsMovementMode(MOVE_Walking) && bWantsToSprint && !IsCrouching()) return MaxWalkSpeed;

	if (MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return MaxSlideSpeed;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
			return -1.f;
	}
}

float UISACharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	//returns max braking depending on MovementMode
	if (MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return BrakingDecelerationSliding;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
			return -1.f;
	}
}
	#pragma region Movement Pipeline
	void UISACharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
	{
		// Slide
		if (MovementMode == MOVE_Walking && bWantsToCrouch)
		{
			if (CanSlide())
			{
				SetMovementMode(MOVE_Custom, CMOVE_Slide);
			}
		}
		else if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
		{
			SetMovementMode(MOVE_Walking);
		}

		Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
	}

	void UISACharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
	{
		Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

		if (!HasAnimRootMotion() && bHadAnimRootMotion && IsMovementMode(MOVE_Flying))
		{
			UE_LOG(LogTemp, Warning, TEXT("Ending Anim Root Motion"))
				SetMovementMode(MOVE_Walking);
		}

		bHadAnimRootMotion = HasAnimRootMotion();
	}

	void UISACharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
	{
		//Physics update on custom MovementModes
		Super::PhysCustom(deltaTime, Iterations);

		switch (CustomMovementMode)
		{
		case CMOVE_Slide:
			PhysSlide(deltaTime, Iterations);
			break;
		default:
			UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		}
	}

	void UISACharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
	{
		Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

		SetupInputDirection(GetCurrentAcceleration() / GetMaxAcceleration());
		
		bPrevWantsToCrouch = bWantsToCrouch;
	}

	// Movement Event
	void UISACharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
	{
		Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

		if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide) ExitSlide();

		if (IsCustomMovementMode(CMOVE_Slide)) EnterSlide(PreviousMovementMode, static_cast<ECustomMovementMode>(PreviousCustomMode));

		if (IsFalling())
		{
			bOrientRotationToMovement = true;
		}
	}
	#pragma endregion

#pragma endregion

#pragma region Slide

void UISACharacterMovementComponent::EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{
	//Gets called when player wants to enter a slide
	bWantsToCrouch = true;
	bOrientRotationToMovement = false;
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;
	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, NULL);
}
void UISACharacterMovementComponent::ExitSlide()
{
	//GEngine->AddOnScreenDebugMessage(1, 7.5f, FColor::Red, TEXT("Exit Slide"), true);
	bWantsToCrouch = false;
	bOrientRotationToMovement = true;
}

bool UISACharacterMovementComponent::CanSlide() const
{
	if (Stance == ISAStanceTags::Crouching)
	{
		//Checks if the player can Slide
		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector End = Start + CapHH() * 2.5f * FVector::DownVector;
		FName ProfileName = TEXT("BlockAll");
		bool bValidSurface = GetWorld()->LineTraceTestByProfile(Start, End, ProfileName, ISACharacterBase->GetIgnoreCharacterParams());
		bool bEnoughSpeed = Velocity.SizeSquared() > pow(MinSlideSpeed, 2);
		return bValidSurface && bEnoughSpeed;
	}
	return false;
}

void UISACharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	
	if (!CanSlide())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Cant slide"));
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("EnterWhile"));
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * SlideGravityForce * deltaTime;
		
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector().GetSafeNormal2D());

		// Apply acceleration
		CalcVelocity(timeTick, GroundFriction * SlideFrictionFactor, false, GetMaxBrakingDeceleration());
		
		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();
		
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("bFloorWalkable: %s"), bFloorWalkable ? TEXT("true") : TEXT("false")));

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Move ALong floor"));
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if ( IsFalling() )
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("StartNewPhys"));
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("CurrentFloor"));
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("FindFloor"));
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}


		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f,0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}
		}
		
		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround() && bFloorWalkable)
		{
			// Make velocity reflect actual move
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}
	
	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}


#pragma endregion

#pragma region Helpers
float UISACharacterMovementComponent::CapR() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UISACharacterMovementComponent::CapHH() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void UISACharacterMovementComponent::SetupInputDirection(FVector NewInputDirection)
{
	bHasInput = NewInputDirection.GetSafeNormal().SizeSquared() > UE_KINDA_SMALL_NUMBER;
}

void UISACharacterMovementComponent::SetStance(const FGameplayTag& NewStance)
{
	if (Stance != NewStance)
	{
		Stance = NewStance;

		RefreshMaxWalkSpeed();
	}
}

void UISACharacterMovementComponent::SetMaxAllowedGait(const FGameplayTag& NewMaxAllowedGait)
{
	if (MaxAllowedGait != NewMaxAllowedGait)
	{
		MaxAllowedGait = NewMaxAllowedGait;

		RefreshMaxWalkSpeed();
	}
}

void UISACharacterMovementComponent::RefreshMaxWalkSpeed()
{
	MaxWalkSpeed = Settings->GetSpeedForGait(MaxAllowedGait, Stance);
	MaxWalkSpeedCrouched = MaxWalkSpeed;
}

#pragma endregion

#pragma region Interface

bool UISACharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}
bool UISACharacterMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}
#pragma endregion