// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PeripheryTypes.h"
#include "Components/ActorComponent.h"
#include "PlayerPeripheryComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PeripheryLog, Log, All);

class USphereComponent;
class IPeripheryObjectInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInRadiusDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfRadiusDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInPeripheryConeDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfPeripheryConeDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInPeripheryTraceDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfPeripheryTraceDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);


/**
 * Class for handling interaction with certain objects within the player's periphery.
 * @note there's logic for handling what happens when a player finds an object, and you're able to customize the behavior for different objects
 * @ref You still need to handle the majority of the logic for building up the components, it only takes a moment to setup the configuration
 *		- Configure the components to link to the character, InitPeripheryInformation, Blueprint delegates, and periphery functions customization
 */
UCLASS( ClassGroup=(Periphery), meta=(BlueprintSpawnableComponent) )
class PERIPHERYSYSTEMCOMPONENT_API UPlayerPeripheryComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bEnablePeripheryRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bEnablePeripheryTrace;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bEnableItemDetection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bEnablePeripheryCone;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius", meta = (EditCondition = "bEnablePeripheryRadius", EditConditionHides))
	TObjectPtr<USphereComponent>		PeripheryRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection", meta = (EditCondition = "bEnableItemDetection", EditConditionHides))
	TObjectPtr<USphereComponent>		ItemDetection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone", meta = (EditCondition = "bEnablePeripheryCone", EditConditionHides))
	TObjectPtr<UStaticMeshComponent>	PeripheryCone;


	/** Periphery Radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius", meta = (EditCondition = "bEnablePeripheryRadius", EditConditionHides))
	TEnumAsByte<ECollisionChannel> PeripheryRadiusChannel = ECC_Pawn;

	/** Item Detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection", meta = (EditCondition = "bEnableItemDetection", EditConditionHides))
	TEnumAsByte<ECollisionChannel> ItemDetectionChannel = ECC_GameTraceChannel1;

	/** Periphery Cone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone", meta = (EditCondition = "bEnablePeripheryCone", EditConditionHides))
	TEnumAsByte<ECollisionChannel> PeripheryConeChannel = ECC_Pawn;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone", meta = (EditCondition = "bEnablePeripheryCone", EditConditionHides))
	FVector PeripheryConeToFirstPersonLocation = FVector(340.0f, 0.0f, 64.0f);

	/** Periphery Trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bEnablePeripheryTrace", EditConditionHides)) TEnumAsByte<ETraceTypeQuery> PeripheryLineTraceType = TraceTypeQuery1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bEnablePeripheryTrace", EditConditionHides)) float PeripheryTraceDistance = 6400;
	UPROPERTY(BlueprintReadWrite) TScriptInterface<IPeripheryObjectInterface> TracedActor;
	UPROPERTY(BlueprintReadWrite) TScriptInterface<IPeripheryObjectInterface> PreviousTracedActor;
	
	/** Other */
	UPROPERTY(BlueprintReadWrite) TArray<AActor*> IgnoredActors;
	UPROPERTY(BlueprintReadWrite) ACharacter* Player;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Utility", meta = (EditCondition = "bEnablePeripheryRadius || bEnablePeripheryTrace || bEnableItemDetection || bEnablePeripheryCone", EditConditionHides))
	bool IgnoreOwnerActors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius|Debug", meta = (EditCondition = "bEnablePeripheryRadius", EditConditionHides)) bool bDebugPeripheryRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection|Debug", meta = (EditCondition = "bEnableItemDetection", EditConditionHides)) bool bDebugItemDetection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone|Debug", meta = (EditCondition = "bEnablePeripheryCone", EditConditionHides)) bool bDebugPeripheryCone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bEnablePeripheryTrace", EditConditionHides)) bool bDebugPeripheryLineTrace;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bEnablePeripheryTrace", EditConditionHides)) FColor TraceColor = FColor::Silver;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bEnablePeripheryTrace", EditConditionHides)) FColor TraceHitColor = FColor::Emerald;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bEnablePeripheryTrace", EditConditionHides)) float TraceDuration = 0.1;
	
	
	// UPROPERTY(BlueprintReadWrite) TScriptInterface<IPeripheryObjectInterface> PeripheryTraceInformation; // TODO: Eventually use script interfaces, we already have a good thing in place for handling logic for different objects, so this isn't necessary
	
	
public:	
	UPlayerPeripheryComponent(const FObjectInitializer& ObjectInitializer);

	/**
	  * This logic is executed for characters with a periphery component if the object overlaps with detection components
	  *	This is useful for a number of things. Showing and updating ui states, keeping track of enemies using radar, etc.
	  *	Information that is not persistent and the player should not keep track of, but needs to be updated if the player interacts or comes within "range" of a object, this is where this might come in handy  
	 */
	/** Radius delegates */
	UPROPERTY(BlueprintAssignable) FObjectInRadiusDelegate ObjectInPlayerRadius;
	UPROPERTY(BlueprintAssignable) FObjectOutsideOfRadiusDelegate ObjectOutsideOfPlayerRadius;
	
	/** Periphery Cone delegates */
	UPROPERTY(BlueprintAssignable) FObjectInPeripheryConeDelegate ObjectInPeripheryCone;
	UPROPERTY(BlueprintAssignable) FObjectOutsideOfPeripheryConeDelegate ObjectOutsideOfPeripheryCone;
	
	/** Periphery Trace delegates */
	UPROPERTY(BlueprintAssignable) FObjectInPeripheryTraceDelegate ObjectInPeripheryTrace;
	UPROPERTY(BlueprintAssignable) FObjectOutsideOfPeripheryTraceDelegate ObjectOutsideOfPeripheryTrace;

	
protected:
	virtual void BeginPlay() override;
	
	/** Add collision events for the locally controlled player */
	UFUNCTION(BlueprintCallable)
	virtual void ConfigurePeripheryCollision(UPrimitiveComponent* Component, bool bEnableCollision);

	/** This is used for performing accurate traces for anything the player is aiming at */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
//----------------------------------------------------------------------------------------------------------------------//
// Periphery functions																									//
//----------------------------------------------------------------------------------------------------------------------//
protected:
	/** The enter and exit overlap logic for the line trace periphery */
	UFUNCTION() virtual void HandlePeripheryLineTrace();
	
	/** The overlap function for entering the periphery radius */
	UFUNCTION() virtual void EnterPeripheryRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** The overlap function for entering the periphery radius */
	UFUNCTION() virtual void ExitPeripheryRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** The overlap function for entering the periphery radius */
	UFUNCTION() virtual void EnterPeripheryCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** The overlap function for entering the periphery radius */
	UFUNCTION() virtual void ExitPeripheryCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


//----------------------------------------------------------------------------------------------------------------------//
// Item detection																										//
//----------------------------------------------------------------------------------------------------------------------//
protected:
	/** Adds any items in the player's radius to the item's array, and sets the active item if it's currently null */
	UFUNCTION() virtual void OnEnterItemRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Removes items outside of the player's radius, and sets the active item to null if that's the current object */
	UFUNCTION() virtual void OnExitItemRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	
//----------------------------------------------------------------------------------------------//
// Utility																						//
//----------------------------------------------------------------------------------------------//
protected:
	/**
	 * Configures the periphery component's logic for the player
	 * @remarks this needs to be called once the character has been initialized
	 */
	UFUNCTION(BlueprintCallable) virtual void InitPeripheryInformation();
	
	/** Helper function for determining the type of overlay that should be used */
	UFUNCTION() virtual EPeripheryType FindPeripheryType(TScriptInterface<IPeripheryObjectInterface> PeripheryObject) const;
	virtual bool GetCharacter(); 


public:
	UFUNCTION(BlueprintCallable) virtual TScriptInterface<IPeripheryObjectInterface> GetTracedObject() const;
	UFUNCTION(BlueprintCallable) virtual USphereComponent* GetPeripheryRadius();
	UFUNCTION(BlueprintCallable) virtual UStaticMeshComponent* GetPeripheryCone();
	UFUNCTION(BlueprintCallable) virtual USphereComponent* GetItemDetection();

	
};
