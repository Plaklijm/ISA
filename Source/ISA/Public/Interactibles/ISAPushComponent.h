// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISAPushableBase.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "ISAPushComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ISA_API UISAPushComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float PushSpeed{60.f};
	
	UPROPERTY(EditAnywhere)
	float PushRange{120.f};
	
private:
	UPROPERTY(VisibleAnywhere)
	AISAPushableBase* CurrentPushable{};

public:
	// Sets default values for this component's properties
	UISAPushComponent();

	void BeginPush(AISAPushableBase* Pushable);
	void EndPush();
	void PushObject(FVector2D MovementVector);

	UFUNCTION(BlueprintCallable)
	bool IsPushingObject() const;
	UFUNCTION(BlueprintCallable)
	float GetPushableHeight() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
