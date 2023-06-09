// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactibles/ISAPushableBase.h"

#include "ISACharacterBase.h"
#include "GameFramework/Character.h"
#include "VectorTypes.h"
#include "DrawDebugHelpers.h"
#include "ISACharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interactibles/ISAPushComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AISAPushableBase::AISAPushableBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box"));
	SetRootComponent(Box);
	
}

// Called when the game starts or when spawned
void AISAPushableBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AISAPushableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AISAPushableBase::OnInteracted(AISACharacterBase* Player)
{
	IISAInteractableInterface::OnInteracted(Player);

	HandleInteraction(Player);
}

void AISAPushableBase::HandleInteraction(AISACharacterBase* Player)
{
	// Checks if pushcomponent is attached to the player
	UISAPushComponent* PushComp = Player->GetComponentByClass<UISAPushComponent>();
	if (Player && PushComp)
	{
		int Index = FindClosestPushTransfromIndex(FVector2d(Player->GetActorLocation()), PushComp->PushRange);
		if (Index >= 0)
		{
			const float HalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			const float Radius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
			FVector Center = (PushTransforms[Index] * GetActorTransform()).GetLocation();
			Center.Z += HalfHeight;

			FTransform CurrentCharacterTransform = FTransform(PushTransforms[Index].GetRotation(), Center, Player->GetActorScale3D());
			FVector Start = CurrentCharacterTransform.GetLocation() + FVector(0,0, 70);
			FVector End = CurrentCharacterTransform.GetLocation() - FVector(0,0,100);
			FHitResult HitResult;
			TArray<AActor*> IgnoreActors;
			IgnoreActors.Add(this);

			// trace for the pushable box
			UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), Start, End, Radius, HalfHeight, UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false, IgnoreActors, EDrawDebugTrace::Type::None, HitResult, true, FColor::Red, FColor::Green, 5);

			if (!HitResult.bStartPenetrating && Player->GetISACharacterMovement()->GetWalkableFloorZ() < HitResult.ImpactNormal.Z)
			{
				if (!UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetActorLocation(), CurrentCharacterTransform.GetLocation(), UEngineTypes::ConvertToTraceType(ECC_Visibility),
						false, TArray<AActor*>(), EDrawDebugTrace::Type::None, HitResult, true))
				{
					// begins the push in the component
					Player->SetActorTransform(CurrentCharacterTransform);
					PushComp->BeginPush(this);
				}
			}
		}
	}
}

int AISAPushableBase::FindClosestPushTransfromIndex(FVector2d CharacterLoc, float PushRange)
{
	// Checks for the closest transform attached to the pushable. Determines which direction the player pushes the box
	int ClosestTransformIndex = -1;
	float ClosestDistanceSq{};
	
	for (int i = 0; i < PushTransforms.Num(); i++)
	{
		const auto CurrentTransformIndex = i;
		FVector2d TempLoc{(GetActorTransform().TransformPosition(PushTransforms[i].GetLocation()))};
		const float CurrentDistanceSq = UE::Geometry::DistanceSquared(TempLoc, CharacterLoc);
		if (CurrentDistanceSq < FMath::Square(PushRange))
		{
			if (CurrentDistanceSq < ClosestDistanceSq || ClosestTransformIndex < 0)
			{
				ClosestTransformIndex = CurrentTransformIndex;
				ClosestDistanceSq = CurrentDistanceSq;
			}
		}
	}
	return ClosestTransformIndex;
}
