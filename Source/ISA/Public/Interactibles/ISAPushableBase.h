// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactibles/ISAInteractableInterface.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ISAPushableBase.generated.h"

UCLASS()
class ISA_API AISAPushableBase : public AActor, public	IISAInteractableInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Box;

protected:
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Meta = (MakeEditWidget = true))
	TArray<FTransform> PushTransforms;
	
public:
	// Sets default values for this actor's properties
	AISAPushableBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void OnInteracted(AISACharacterBase* Player) override;

private:
	void HandleInteraction(AISACharacterBase* Player);

	int FindClosestPushTransfromIndex(FVector2d CharacterLoc, float PushRange);
};
