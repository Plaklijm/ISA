#pragma once

#include "Engine/DataAsset.h"
#include "Utility/ISAGameplayTags.h"
#include "Animation/AnimMontage.h"
#include "ISASettings.generated.h"

class UMantleSettings;

//All the variables used for Sliding
USTRUCT(BlueprintType)
struct ISA_API FISASlideSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TObjectPtr<UAnimMontage> Montage{nullptr};
};

//General Settings
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISA", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float CrouchSpeed{150.0f};
	
	static constexpr auto HasInputBrakingFrictionFactor{ 0.5f };
	static constexpr auto NoInputBrakingFrictionFactor{ 3.0f };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FISASlideSettings SlideSettings;
public:
	float GetSpeedForGait(const FGameplayTag& Gait, const FGameplayTag& Stance) const;
	
};

// Helper Functions
inline float UISASettings::GetSpeedForGait(const FGameplayTag& Gait, const FGameplayTag& Stance) const
{
	if (Stance == ISAStanceTags::Standing)
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
	}
	else if (Stance == ISAStanceTags::Crouching)
	{
		return  CrouchSpeed;
	}
	
	return 0.0f;
}