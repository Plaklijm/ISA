#include "Utility/ISAGameplayTags.h"

namespace ISALocomotionModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Grounded, FName{ TEXTVIEW("ISA.LocomotionMode.Grounded") })
	UE_DEFINE_GAMEPLAY_TAG(InAir, FName{ TEXTVIEW("ISA.LocomotionMode.InAir") })
}

namespace ISAStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Standing, FName{ TEXTVIEW("ISA.Stance.Standing") })
	UE_DEFINE_GAMEPLAY_TAG(Crouching, FName{ TEXTVIEW("ISA.Stance.Crouching") })
}

namespace ISAGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Walking, FName{ TEXTVIEW("ISA.Gait.Walking") })
	UE_DEFINE_GAMEPLAY_TAG(Running, FName{ TEXTVIEW("ISA.Gait.Running") })
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, FName{ TEXTVIEW("ISA.Gait.Sprinting") })
}

namespace ISALocomotionActionTags
{
	UE_DEFINE_GAMEPLAY_TAG(Rolling, FName{ TEXTVIEW("ISA.LocomotionAction.Rolling") })
	UE_DEFINE_GAMEPLAY_TAG(Mantling, FName{ TEXTVIEW("ISA.LocomotionAction.Mantling") })
	//Maybe implemented later (Dont forget to change in .ini file)
	//UE_DEFINE_GAMEPLAY_TAG(GettingUp, FName{TEXTVIEW("ISA.LocomotionAction.GettingUp")})
}