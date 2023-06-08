// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactibles/ISAInteractableInterface.h"
#include "ISADoorBase.generated.h"

class UBoxComponent;
UCLASS()
class ISA_API AISADoorBase : public AActor, public IISAInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AISADoorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Meta = (MakeEditWidget = true))
	FTransform WarpTransform;
	
	UPROPERTY(BlueprintReadOnly)
	UBoxComponent* BoxComp;

public:

	UFUNCTION(BlueprintCallable)
	virtual void OnInteracted(AISACharacterBase* Player) override;

	UFUNCTION(BlueprintNativeEvent)
	void BPInteracted();
};
