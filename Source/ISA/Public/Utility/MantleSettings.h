// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MantleSettings.generated.h"

//All the variables for Mantling
UENUM(BlueprintType)
enum class EISAMantleType
{
	NoMantle,
	MantleLow,
	MantleHigh
};

UCLASS(Blueprintable, BlueprintType)
class ISA_API UMantleSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantle Settings");
	EISAMantleType MantleType{EISAMantleType::NoMantle};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantle Settings");
	FVector VaultStartPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantle Settings");
	FVector VaultMidPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mantle Settings");
	FVector VaultEndPos;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Settings");
	bool bCanWarp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mantle Settings");
	bool bCanMantle{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mantle Settings");
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float ForwardTraceLength{180.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float TraceForwardStart{30.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float TraceRadius{5.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TObjectPtr<UAnimMontage> Montage{nullptr};
};
