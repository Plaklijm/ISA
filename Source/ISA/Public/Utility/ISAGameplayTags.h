#pragma once

#include "NativeGameplayTags.h"

//Declare all the Gameplaytags needed in the project
//Init happens in .CPP file
namespace ISALocomotionModeTags
{
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Grounded)
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InAir)
}

namespace ISAStanceTags
{
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Standing)
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crouching)
}

namespace ISAGaitTags
{
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Walking)
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Running)
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprinting)
}

namespace ISALocomotionActionTags
{
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mantling)
	ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sliding);
	//Maybe implemented later
	//ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GettingUp)
	//ISA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rolling)
}