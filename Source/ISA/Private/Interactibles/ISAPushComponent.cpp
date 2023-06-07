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

	SetComponentTickEnabled(false);
}


// Called every frame
void UISAPushComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ACharacter* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetCharacter();
	CurrentPushable->AddActorWorldOffset(GetOwner()->GetActorForwardVector() * PushSpeed * DeltaTime);
	
}

void UISAPushComponent::BeginPush(AISAPushableBase* Pushable)
{
	if (!IsValid(CurrentPushable))
	{
		ACharacter* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetCharacter();
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
	ACharacter* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetCharacter();
	const FDetachmentTransformRules Rules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);
	Player->DetachFromActor(Rules);
	Player->GetCharacterMovement()->SetPlaneConstraintEnabled(false);
	Player->GetCharacterMovement()->bOrientRotationToMovement = true;
	SetComponentTickEnabled(false);
}

void UISAPushComponent::PushObject(FVector2D MovementVector)
{
	ACharacter* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetCharacter();
	UE_LOG(LogTemp, Warning, TEXT("%f, %f"), MovementVector.X, MovementVector.Y);
	UE_LOG(LogTemp, Warning, TEXT("AdjustedSpeed: %s"), *(Player->GetActorRightVector() * PushSpeed * GetWorld()->GetDeltaSeconds()).ToString());
	UE_LOG(LogTemp, Warning, TEXT("PlayerForward Vector: %s"), *Player->GetActorForwardVector().ToString())

	CurrentPushable->AddActorWorldOffset(FVector{MovementVector.Y, MovementVector.X, 0}, true);
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
		const ACharacter* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetCharacter();
		const float CharacterFeet = Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2;


		return ObjectTop - CharacterFeet;
	}

	return 0;
}


