// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactibles/InteractableInterface.h"
#include "InteractableBase.generated.h"

UCLASS()
class ISA_API AInteractableBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractableBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "interaction")
	void OnInteract(AActor* Caller);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "interaction")
	void StartFocus();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "interaction")
	void EndFocus();
};
