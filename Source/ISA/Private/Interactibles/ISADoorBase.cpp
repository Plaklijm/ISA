// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactibles/ISADoorBase.h"

#include "Components/BoxComponent.h"

// Sets default values
AISADoorBase::AISADoorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("BoxComp"));
	BoxComp->InitBoxExtent(FVector{150,20,125});
	BoxComp->bHiddenInGame = false;
	BoxComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	BoxComp->SetCollisionResponseToAllChannels(ECR_Block);
	SetRootComponent(BoxComp);
	
}

// Called when the game starts or when spawned
void AISADoorBase::BeginPlay()
{
	Super::BeginPlay();
	
}


void AISADoorBase::OnInteracted(AISACharacterBase* Player)
{
	IISAInteractableInterface::OnInteracted(Player);
	BPInteracted();
	Player->Interact(WarpTransform + GetActorTransform());
}

void AISADoorBase::BPInteracted_Implementation()
{
}





