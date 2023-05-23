#pragma once

#include "Engine/DataAsset.h"
#include "Utility/ISAGameplayTags.h"
#include "ISASettings.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ISA_API UISASettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float TestValue{50.f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISA", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float WalkSpeed{175.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISA", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float RunSpeed{375.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISA", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float SprintSpeed{650.0f};
	
	static constexpr auto HasInputBrakingFrictionFactor{ 0.5f };
	static constexpr auto NoInputBrakingFrictionFactor{ 3.0f };

public:
	float GetSpeedForGait(const FGameplayTag& Gait) const;
};

inline float UISASettings::GetSpeedForGait(const FGameplayTag& Gait) const
{
	if (Gait == ISAGaitTags::Walking)
	{
		return WalkSpeed;
	}

	if (Gait == ISAGaitTags::Running)
	{
		return RunSpeed;
	}

	if (Gait == ISAGaitTags::Sprinting)
	{
		return SprintSpeed;
	}

	return 0.0f;
}