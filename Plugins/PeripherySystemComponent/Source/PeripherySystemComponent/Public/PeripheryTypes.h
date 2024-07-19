#pragma once


#include "CoreMinimal.h"
#include "PeripheryTypes.generated.h"


/**
 *	The periphery type based on the player and the periphery object (this helps with highlighting and other things)
 */
UENUM(BlueprintType)
enum class EPeripheryType : uint8
{
	EPT_None		 		UMETA(DisplayName = "None"),
	EPT_Passive    			UMETA(DisplayName = "Passive"),
	EPT_Enemy    			UMETA(DisplayName = "Enemy"),
	EPT_Ally		    	UMETA(DisplayName = "Ally"),
	EPT_Object    			UMETA(DisplayName = "Object")
};


/**
 *	Where should the periphery logic be handled? Server or client, or both?
 */
UENUM(BlueprintType)
enum class EHandlePeripheryLogic : uint8
{
	EP_None		 		    UMETA(DisplayName = "None"),
	EP_ServerAndClient    	UMETA(DisplayName = "ServerAndClient"),
	EP_Server		    	UMETA(DisplayName = "Server"),
	EP_Client    			UMETA(DisplayName = "Client"),
};
