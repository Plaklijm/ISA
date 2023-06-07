// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISACharacterBase.h"
#include "UObject/Interface.h"
#include "ISAInteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UISAInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ISA_API IISAInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:
	
	virtual void OnInteracted(AISACharacterBase* Player);
};
