// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactibles/InteractableBase.h"

#include "Components/BoxComponent.h"


// Sets default values
AInteractableBase::AInteractableBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("BoxComp"));
	BoxComp->InitBoxExtent(FVector{100,100,100});
	BoxComp->bHiddenInGame = false;
	SetRootComponent(BoxComp);
}

// Called when the game starts or when spawned
void AInteractableBase::BeginPlay()
{
	Super::BeginPlay();
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AInteractableBase::OverlapStarted);
	
}

void AInteractableBase::InteractEventStarted_Implementation()
{
}

void AInteractableBase::OverlapStarted_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult)
{
	InteractFunction.Broadcast();
	InteractEventStarted();
}
