// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PeripheryTypes.h"
#include "UObject/Interface.h"
#include "PeripheryObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(NotBlueprintable)
class UPeripheryObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
  * This logic is executed for objects with a periphery component if the object overlaps with detection components
  * This is useful for updating information safely without any extra complications
 */
class PERIPHERYSYSTEMCOMPONENT_API IPeripheryObjectInterface
{
	GENERATED_BODY()


public:
	/** Logic when a character registers it within it's periphery */
	UFUNCTION(BlueprintCallable) virtual void OnEnterRadiusPeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character unregisters it within it's periphery */
	UFUNCTION(BlueprintCallable) virtual void OnExitRadiusPeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);
	
	/** Logic when a character's periphery cone registers the object */
	UFUNCTION(BlueprintCallable) virtual void OnEnterConePeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character's periphery cone unregisters the object */
	UFUNCTION(BlueprintCallable) virtual void OnExitConePeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);
	
	/** Logic when a character's trace registers the object */
	UFUNCTION(BlueprintCallable) virtual void OnEnterLineTracePeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character's trace unregisters the object */
	UFUNCTION(BlueprintCallable) virtual void OnExitLineTracePeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	
};
