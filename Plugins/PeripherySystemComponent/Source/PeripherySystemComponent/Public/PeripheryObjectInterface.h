// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PeripheryTypes.h"
#include "UObject/Interface.h"
#include "PeripheryObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, BlueprintType)
class UPeripheryObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
  * This logic is executed for objects that need to do something when the player interacts with them.
  * This is useful for updating information safely without any extra complications
 */
class PERIPHERYSYSTEMCOMPONENT_API IPeripheryObjectInterface
{
	GENERATED_BODY()

	
public:
	/** Logic when a character registers it within it's periphery */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Radius", DisplayName = "(Periphery Object) Within Player Radius Periphery") 
	void WithinPlayerRadiusPeriphery(AActor* SourceCharacter, EPeripheryType PeripheryType);
	virtual void WithinPlayerRadiusPeriphery_Implementation(AActor* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character unregisters it within it's periphery */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Radius", DisplayName = "(Periphery Object) Outside Of Player Radius Periphery") 
	void OutsideOfPlayerRadiusPeriphery(AActor* SourceCharacter, EPeripheryType PeripheryType);
	virtual void OutsideOfPlayerRadiusPeriphery_Implementation(AActor* SourceCharacter, EPeripheryType PeripheryType);
	
	/** Logic when a character's periphery cone registers the object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Cone", DisplayName = "(Periphery Object) Within Player Cone Periphery") 
	void WithinPlayerConePeriphery(AActor* SourceCharacter, EPeripheryType PeripheryType);
	virtual void WithinPlayer_Implementation(AActor* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character's periphery cone unregisters the object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Cone", DisplayName = "(Periphery Object) Outside Of Cone Periphery") 
	void OutsideOfConePeriphery(AActor* SourceCharacter, EPeripheryType PeripheryType);
	virtual void OutsideOfConePeriphery_Implementation(AActor* SourceCharacter, EPeripheryType PeripheryType);
	
	/** Logic when a character's trace registers the object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Trace", DisplayName = "(Periphery Object) Within Player Trace Periphery") 
	void WithinPlayerTracePeriphery(AActor* SourceCharacter, EPeripheryType PeripheryType);
	virtual void WithinPlayerTracePeriphery_Implementation(AActor* SourceCharacter, EPeripheryType PeripheryType);

	/** Logic when a character's trace unregisters the object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Trace", DisplayName = "(Periphery Object) Outside Of Player Trace Periphery") 
	void OutsideOfPlayerTracePeriphery(AActor* SourceCharacter, EPeripheryType PeripheryType);
	virtual void OutsideOfPlayerTracePeriphery_Implementation(AActor* SourceCharacter, EPeripheryType PeripheryType);

	
};
