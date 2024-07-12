// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PeripheryTypes.h"
#include "UObject/Interface.h"
#include "PeripheryObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Periphery|Radius") 
	void OnEnterRadiusPeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);
	virtual void OnEnterRadiusPeriphery_Implementation(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character unregisters it within it's periphery */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Periphery|Radius") 
	void OnExitRadiusPeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	virtual void OnExitRadiusPeriphery_Implementation(ACharacter* SourceCharacter, EPeripheryType PeripheryType);
	
	/** Logic when a character's periphery cone registers the object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Periphery|Cone") 
	void OnEnterConePeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);
	virtual void OnEnterConePeriphery_Implementation(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character's periphery cone unregisters the object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Periphery|Cone") 
	void OnExitConePeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	virtual void OnExitConePeriphery_Implementation(ACharacter* SourceCharacter, EPeripheryType PeripheryType);
	
	/** Logic when a character's trace registers the object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Periphery|Trace") 
	void OnEnterLineTracePeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);
	virtual void OnEnterLineTracePeriphery_Implementation(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character's trace unregisters the object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Periphery|Trace") 
	void OnExitLineTracePeriphery(ACharacter* SourceCharacter, EPeripheryType PeripheryType);
	virtual void OnExitLineTracePeriphery_Implementation(ACharacter* SourceCharacter, EPeripheryType PeripheryType);

	
};
