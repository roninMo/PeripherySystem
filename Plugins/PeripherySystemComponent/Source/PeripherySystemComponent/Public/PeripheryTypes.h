#pragma once


#include "CoreMinimal.h"
#include "PeripheryTypes.generated.h"


/**
 *	The periphery type based on the player and the periphery object (this helps with highlighting and other things)
 */
UENUM(BlueprintType)
enum class EPeripheryType : uint8
{
	EP_None		 		    UMETA(DisplayName = "None"),
	EP_Passive    			UMETA(DisplayName = "Passive"),
	EP_Enemy    			UMETA(DisplayName = "Enemy"),
	EP_Ally		    	    UMETA(DisplayName = "Ally"),
	EP_Object    			UMETA(DisplayName = "Object")
};
