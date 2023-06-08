// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactibles/ISAPushComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UISAPushComponent::UISAPushComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UISAPushComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetCharacter();
	SetComponentTickEnabled(false);
}


// Called every frame
void UISAPushComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	CurrentPushable->AddActorWorldOffset(GetOwner()->GetActorForwardVector() * PushSpeed * DeltaTime);
	
}

void UISAPushComponent::BeginPush(AISAPushableBase* Pushable)
{
	if (!IsValid(CurrentPushable))
	{
		CurrentPushable = Pushable;
		const FAttachmentTransformRules Rules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
		Player->AttachToActor(CurrentPushable, Rules);
		Player->GetCharacterMovement()->SetPlaneConstraintEnabled(true);
		Player->GetCharacterMovement()->SetPlaneConstraintNormal(Player->GetActorRightVector());
		Player->GetCharacterMovement()->bOrientRotationToMovement = false;
		SetComponentTickEnabled(true);
	}
}

void UISAPushComponent::EndPush()
{
	CurrentPushable = {};
	const FDetachmentTransformRules Rules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);
	Player->DetachFromActor(Rules);
	Player->GetCharacterMovement()->SetPlaneConstraintEnabled(false);
	Player->GetCharacterMovement()->bOrientRotationToMovement = true;
	SetComponentTickEnabled(false);
}

bool UISAPushComponent::IsPushingObject() const
{
	return IsValid(CurrentPushable);
}

float UISAPushComponent::GetPushableHeight() const
{
	if (CurrentPushable)
	{
		FVector Max;
		FVector Min;
		CurrentPushable->Box->GetLocalBounds(Min, Max);
		float ObjectTop = Max.Z - Min.Z;
		ObjectTop += CurrentPushable->GetActorLocation().Z;
		const float CharacterFeet = Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2;
		
		return ObjectTop - CharacterFeet;
	}

	return 0;
}


