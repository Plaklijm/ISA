// Copyright Epic Games, Inc. All Rights Reserved.

#include "ISACharacterBase.h"

#include "CanvasItem.h"
#include "ISACharacterMovementComponent.h"
#include "Utility/ISASettings.h"
#include "Utility/MantleSettings.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Canvas.h"
#include "Interactibles/InteractableBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


// AISACharacter

AISACharacterBase::AISACharacterBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UISACharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	// Set default CMC 
	ISACharacterMovementComponent = Cast<UISACharacterMovementComponent>(GetCharacterMovement());
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator( 0.0f, 350.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.25f;
	GetCharacterMovement()->MaxWalkSpeed = 175.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}


void AISACharacterBase::BeginPlay()
{
	ensure(IsValid(GeneralSettings));
	ensure(IsValid(MantleSettings));
	// Call the base class  
	Super::BeginPlay();

	//Checks for the interactables and listen to the Broadcast	
	TArray<AActor*> FoundInteractables;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableBase::StaticClass(), FoundInteractables);

	for (int i = 0; i < FoundInteractables.Num(); i++)
	{
		AInteractableBase* TempInteractable = Cast<AInteractableBase>(FoundInteractables[i]);
		TempInteractable->InteractFunction.AddDynamic(this, &AISACharacterBase::Interact);
	}
	
	ApplyDesiredStance();

	ISACharacterMovementComponent->SetStance(Stance);

	RefreshGait();

	SetForceGait(true, false);
}

void AISACharacterBase::SetForceGait(bool bWalk_Run, bool bRunSprint)
{
	bForceWalkRun = bWalk_Run;
	bForceRunSprint = bRunSprint;
}

void AISACharacterBase::Jump()
{
	if (Stance == ISAStanceTags::Standing && !LocomotionAction.IsValid()
		&& LocomotionMode == ISALocomotionModeTags::Grounded)
	{
		Super::Jump();
	}
}

void AISACharacterBase::StopJumping()
{
	Super::StopJumping();
}

bool AISACharacterBase::CanCrouch() const
{
	return bIsCrouched || Super::CanCrouch();
}

void AISACharacterBase::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(ISAStanceTags::Crouching);
}

void AISACharacterBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(ISAStanceTags::Standing);
}

bool AISACharacterBase::CanSprint() const
{
	if (!GetISACharacterMovement()->bHasInput || Stance != ISAStanceTags::Standing)
	{
		return false;
	}

	if (GetISACharacterMovement()->bCanSprint)
	{
		return true;
	}
	
	return  false;
}

void AISACharacterBase::SetDebugCommand()
{
	//Inserts the showdebug Command into the cmd, executed when the debug button gets pressed
	GetNetOwningPlayer()->ConsoleCommand("showdebug");
}

void AISACharacterBase::PreRegisterAllComponents()
{
	Stance = DesiredStance;
	Gait = DesiredGait;
	
	Super::PreRegisterAllComponents();
}

FCollisionQueryParams AISACharacterBase::GetIgnoreCharacterParams() const
{
	// Ignore character when raycasting
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

void AISACharacterBase::Tick(float DeltaTime)
{
	RefreshLocomotion(DeltaTime);

	RefreshGait();

	//CalculateWallHeight();

	//VaultJumpHelperFunction();
	
	Super::Tick(DeltaTime);
}

void AISACharacterBase::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	//Checks if the player is on the ground or in the air, set the locomotionmode accordingly
	switch (GetCharacterMovement()->MovementMode)
	{
		case MOVE_Walking:
			SetLocomotionMode(ISALocomotionModeTags::Grounded);
			break;

		case MOVE_Falling:
			SetLocomotionMode(ISALocomotionModeTags::InAir);
			break;
		
		default:
			SetLocomotionMode(FGameplayTag::EmptyTag);
			break;

	}
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void AISACharacterBase::RefreshLocomotion(const float DeltaTime)
{
	GetISACharacterMovement()->Speed = UE_REAL_TO_FLOAT(GetISACharacterMovement()->Velocity.Size2D());
}

void AISACharacterBase::MantleTrace()
{
	if (GetISACharacterMovement()->IsMovingOnGround())
	{
		for (int i = 0; i < 3; i++)
		{
			FVector StartLoc = FVector{GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + i * MantleSettings->TraceForwardStart};
			FVector ForwardVector = GetActorForwardVector() * MantleSettings->ForwardTraceLength;
			FVector EndLoc = StartLoc + ForwardVector;

			TArray<AActor*> IngoreActors;
			IngoreActors.Add(this);
			
			FHitResult HitResult;
			
			if (UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), StartLoc, EndLoc, 5, MantleSettings->ObjectTypes,
				false, IngoreActors, EDrawDebugTrace::Type::ForDuration, HitResult, true))
			{
				MantleSettings->VaultStartPos = FVector{GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - ISACharacterMovementComponent->CapHH()};
				
				for (int f = 0; f < 6; f++)
				{
					FVector _ForwardVector = GetActorForwardVector() * (f * 50);
					FVector _StartLoc = FVector{HitResult.Location.X, HitResult.Location.Y, HitResult.Location.Z + 100} + _ForwardVector;
					FVector _EndLoc = _StartLoc - FVector{0,0,100};
			
					FHitResult _HitResult;
			
					if (UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), _StartLoc, _EndLoc, 5, MantleSettings->ObjectTypes,
						false, IngoreActors, EDrawDebugTrace::Type::ForDuration, _HitResult, true))
					{
						if (!_HitResult.bStartPenetrating)
						{
							MantleSettings->VaultMidPos = FVector{_HitResult.Location.X, _HitResult.Location.Y,
								_HitResult.Location.Z - ISACharacterMovementComponent->CapHH() / 2};
						}
						else
						{
							MantleSettings->bCanWarp = false;
							MantleSettings->VaultEndPos = {0,0,2000};
							break;
						}
					}
					else if (UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), _HitResult.TraceStart + GetActorForwardVector() * 80,
						(_HitResult.TraceStart + GetActorForwardVector() * 80) - FVector{0,0,1000}, MantleSettings->ObjectTypes,
						false, IngoreActors, EDrawDebugTrace::Type::ForDuration, _HitResult, true))
					{
						MantleSettings->VaultEndPos = _HitResult.Location;
						break;
					}
				}
				MantleSettings->bCanMantle = true;
				
				SetupMantle();
			}
			MantleSettings->bCanMantle = false;
		}
	}
	else
	{
		MantleSettings->bCanMantle = false;
	}
}

void AISACharacterBase::SetupMantle_Implementation()
{
}

void AISACharacterBase::SetLocomotionMode(const FGameplayTag& NewLocomotionMode)
{
	//checks if the new mode is not the old one
	if (LocomotionMode != NewLocomotionMode)
	{
		const auto PreviousLocomotionMode{ LocomotionMode };

		//apply locomotionmode
		LocomotionMode = NewLocomotionMode;

		NotifyLocomotionModeChanged({ PreviousLocomotionMode });
	}

}

void AISACharacterBase::NotifyLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode)
{
	ApplyDesiredStance();

	if (LocomotionMode == ISALocomotionModeTags::Grounded && PreviousLocomotionMode == ISALocomotionModeTags::InAir)
	{
		/*UE_LOG(LogTemp, Error, TEXT("%f"), ISACharacterMovementComponent->Velocity.Z)
		if (GetISACharacterMovement()->Velocity.Z <= -Settings->TestValue)
		{
			UE_LOG(LogTemp, Warning, TEXT("StartRolling"));
			//StartRolling
			//Fix Velocity.Z (cant get value out of it for some reason)

		}*/
		
			ISACharacterMovementComponent->BrakingFrictionFactor = ISACharacterMovementComponent->bHasInput 
																									? GeneralSettings->HasInputBrakingFrictionFactor 
																									: GeneralSettings->NoInputBrakingFrictionFactor;

			GetWorldTimerManager().SetTimer(BrakingFrictionFactorResetTimer,
				FTimerDelegate::CreateWeakLambda(this, [this]
					{ GetISACharacterMovement()->BrakingFrictionFactor = 1.0f; }), 0.5f, false);
		
	}
}

void AISACharacterBase::SetDesiredStance(const FGameplayTag& NewDesiredStance)
{
	//Sets the stance the player wants to be in. This essentially queues the stance for it to be applied
	if (DesiredStance != NewDesiredStance)
	{
		DesiredStance = NewDesiredStance;

		ApplyDesiredStance();
	}
}

void AISACharacterBase::ApplyDesiredStance()
{
	if (!LocomotionAction.IsValid())
	{
		if (LocomotionMode == ISALocomotionModeTags::Grounded)
		{
			if (DesiredStance == ISAStanceTags::Standing)
			{
				UnCrouch();
			}
			else if (DesiredStance == ISAStanceTags::Crouching)
			{
				Crouch();
			}
		}
		else if (LocomotionMode == ISALocomotionModeTags::InAir)
		{
			UnCrouch();
		}
	}
	else if (LocomotionAction == ISALocomotionActionTags::Rolling || LocomotionAction == ISALocomotionActionTags::Sliding)
	{
		Crouch();
	}
}

void AISACharacterBase::SetStance(const FGameplayTag& NewStance)
{
	ISACharacterMovementComponent->SetStance(NewStance);

	//Check if the current stance isnt the same as the new one
	if (Stance != NewStance)
	{
		//Set previous and current stance
		const auto PreviousStance{Stance};

		Stance = NewStance;
	}
}

void AISACharacterBase::SetDesiredGait(const FGameplayTag& NewDesiredGait)
{
	if (DesiredGait != NewDesiredGait)
	{
		DesiredGait = NewDesiredGait;
	}
}

void AISACharacterBase::SetGait(const FGameplayTag& NewGait)
{
	if (Gait != NewGait)
	{
		const auto PreviousGait{Gait};

		Gait = NewGait;

		OnGaitChanged(PreviousGait);
	}
}

void AISACharacterBase::OnGaitChanged_Implementation(const FGameplayTag& PreviousGaitTag) {}

void AISACharacterBase::RefreshGait()
{
	if (LocomotionMode != ISALocomotionModeTags::Grounded)
	{
		return;
	}

	const auto MaxAllowedGait{CalculateMaxAllowedGait()};

	GetISACharacterMovement()->SetMaxAllowedGait(MaxAllowedGait);

	SetGait(CalculateActualGait(MaxAllowedGait));
}

FGameplayTag AISACharacterBase::CalculateMaxAllowedGait() const
{
	//This represents the maximum gait the character is currently allowed to be in and can be determined by 
	//desired gait, stance etc (If you want to force the character to be in a Gait based on something you can do it here)
	if (bForceWalkRun)
	{
		if (DesiredGait != ISAGaitTags::Sprinting)
		{
			return DesiredGait;
		}
	}
	if (bForceRunSprint)
	{
		if (CanSprint())
		{
			return ISAGaitTags::Sprinting;
		}
		return ISAGaitTags::Running;
	}

	return ISAGaitTags::Walking;
}

FGameplayTag AISACharacterBase::CalculateActualGait(const FGameplayTag& MaxAllowedGait) const
{
	//Calculates the actual gait the player is in, this can differ from the desired or max allowed gait,
	//When sprinting to walking youll only be in the walking gait when you decelerate enough to be considerd walking
	
	if (GetISACharacterMovement()->Speed < GeneralSettings->WalkSpeed + 10.f)
	{
		return ISAGaitTags::Walking;
	}

	if (GetISACharacterMovement()->Speed < GeneralSettings->RunSpeed + 10.f || MaxAllowedGait != ISAGaitTags::Sprinting)
	{
		return ISAGaitTags::Running;
	}

	return ISAGaitTags::Sprinting;
}

void AISACharacterBase::SetLocomotionAction(const FGameplayTag& NewLocomotionAction)
{
	if (LocomotionAction != NewLocomotionAction)
	{
		const auto PreviousLocomotionAction{LocomotionAction};

		LocomotionAction = NewLocomotionAction;

		NotifyLocomotionActionChanged(PreviousLocomotionAction);
	}
}

void AISACharacterBase::NotifyLocomotionActionChanged(const FGameplayTag& PreviousLocomotionAction)
{
	ApplyDesiredStance();
}

UAnimMontage* AISACharacterBase::SelectRollMontage_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("SELECTMONTAGE"));
	return GeneralSettings->SlideSettings.Montage;
}

void AISACharacterBase::TryStartSliding()
{
	UE_LOG(LogTemp, Warning, TEXT("TryStartSliding"));
	if (LocomotionMode == ISALocomotionModeTags::Grounded)
	{
		StartSliding();
	}
}

bool AISACharacterBase::IsAllowedToSlide(const UAnimMontage* Montage) const
{
	UE_LOG(LogTemp, Warning, TEXT("ISAllowedToSlide"));
	return !LocomotionAction.IsValid() ||
			   // ReSharper disable once CppRedundantParentheses
			   (LocomotionAction == ISALocomotionActionTags::Rolling &&
				!GetMesh()->GetAnimInstance()->Montage_IsPlaying(Montage)) ||
					LocomotionMode == ISAGaitTags::Walking;
}

void AISACharacterBase::StartSliding()
{
	UE_LOG(LogTemp, Warning, TEXT("START SLIDING"));
	auto* Montage{SelectRollMontage()};

	if (!ensure(IsValid(Montage)) || !IsAllowedToSlide(Montage))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Sliding has started"));
	StartSlidingImplementation(Montage);
}

void AISACharacterBase::StartSlidingImplementation(UAnimMontage* Montage)
{
	if (IsAllowedToSlide(Montage))// && GetMesh()->GetAnimInstance()->Montage_Play(Montage, 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("started"));
		PlayAnimMontage(Montage,1);
		SetLocomotionAction(ISALocomotionActionTags::Sliding);
	}
}

void AISACharacterBase::Interact_Implementation()
{
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, TEXT("Interacted"));
}


#pragma region Debug

void AISACharacterBase::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	//Display debug and sends the correct information to the stateinfo function
	const auto Scale{FMath::Min(Canvas->SizeX / (1280.0f * Canvas->GetDPIScale()), Canvas->SizeY / (720.0f * Canvas->GetDPIScale()))};
	DisplayDebugStateInfo(Canvas, Scale, YL, YPos);
	
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
}

void AISACharacterBase::DisplayDebugStateInfo(const UCanvas* Canvas, const float Scale, const float HorizontalLocation,
                                          float& VerticalLocation) const
{
	//Function directly copied from ALS, Gets all the state information and displays it in the debug mode in editor
	VerticalLocation += 1 * Scale;

	//Makes Text item
	FCanvasTextItem Text{
		FVector2d::ZeroVector,
		FText::GetEmpty(),
		GEngine->GetMediumFont(),
		FLinearColor::White
	};

	Text.Scale = {Scale, Scale};
	Text.EnableShadow(FLinearColor::Black);

	//init offsets
	const auto RowOffset {12 * Scale};
	const auto ColumnOffset{120.f * Scale};

	//Gets the LocomotionMode and converts it to Text
	static const auto LocomotionModeText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, LocomotionMode), false))
	};

	//Display Text
	Text.Text = LocomotionModeText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	//Display the current State of the Tag
	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(GetSimpleTagName(LocomotionMode).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	//Add Offset
	VerticalLocation += RowOffset;

	//Repeat of the previous steps
	static const auto DesiredStanceText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, DesiredStance), false))
	};

	Text.Text = DesiredStanceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(GetSimpleTagName(DesiredStance).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto StanceText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, Stance), false))
	};

	Text.Text = StanceText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(GetSimpleTagName(Stance).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto DesiredGaitText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, DesiredGait), false))
	};

	Text.Text = DesiredGaitText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(GetSimpleTagName(DesiredGait).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto GaitText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, Gait), false))
	};

	Text.Text = GaitText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(GetSimpleTagName(Gait).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	static const auto LocomotionActionText{
		FText::AsCultureInvariant(FName::NameToDisplayString(GET_MEMBER_NAME_STRING_CHECKED(ThisClass, LocomotionAction), false))
	};

	Text.Text = LocomotionActionText;
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

	Text.Text = FText::AsCultureInvariant(FName::NameToDisplayString(GetSimpleTagName(LocomotionAction).ToString(), false));
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;

	Text.Text = FText::FromString("Speed:");
	Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
	
	Text.Text = FText::FromString(GetISACharacterMovement()->Velocity.ToString());
	Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

	VerticalLocation += RowOffset;
}

FName AISACharacterBase::GetSimpleTagName(const FGameplayTag& Tag)
{
	const auto TagNode{UGameplayTagsManager::Get().FindTagNode(Tag)};

	return TagNode.IsValid() ? TagNode->GetSimpleTagName() : NAME_None;
}

#pragma endregion