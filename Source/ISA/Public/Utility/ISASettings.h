#pragma once

#include "Engine/DataAsset.h"
#include "ISASettings.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ISA_API UISASettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float TestValue{50.f};


	static constexpr auto HasInputBrakingFrictionFactor{ 0.5f };
	static constexpr auto NoInputBrakingFrictionFactor{ 3.0f };
public:
	
	UISASettings();
};
