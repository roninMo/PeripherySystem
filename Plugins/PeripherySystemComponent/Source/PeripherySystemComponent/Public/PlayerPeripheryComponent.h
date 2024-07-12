// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PeripheryTypes.h"
#include "Components/ActorComponent.h"
#include "PlayerPeripheryComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PeripheryLog, Log, All);

class USphereComponent;
class IPeripheryObjectInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInRadius, AActor*, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfRadius, AActor*, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInPeripheryCone, AActor*, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfPeripheryCone, AActor*, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInPeripheryTrace, AActor*, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfPeripheryTrace, AActor*, PeripheryObject);


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
	// Periphery components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery")
	TObjectPtr<USphereComponent> PeripheryRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery")
	TObjectPtr<UStaticMeshComponent> PeripheryCone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Detection")
	TObjectPtr<USphereComponent> ItemDetection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") FVector PeripheryConeToFirstPersonLocation = FVector(340.0f, 0.0f, 64.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") float PeripheryTraceDistance = 6400;
	UPROPERTY(BlueprintReadWrite) TArray<AActor*> IgnoredActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") TEnumAsByte<ETraceTypeQuery> PeripheryLineTraceType = TraceTypeQuery1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") TEnumAsByte<ECollisionChannel> PeripheryRadiusChannel = ECC_Pawn;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Detection") TEnumAsByte<ECollisionChannel> ItemDetectionChannel = ECC_GameTraceChannel1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool IgnoreOwnerActors = true;

	
	/** A reference to the currently traced object of the character */
	UPROPERTY(BlueprintReadWrite) TObjectPtr<AActor> TracedActor;
	UPROPERTY(BlueprintReadWrite) TObjectPtr<AActor> PreviousTracedActor;
	IPeripheryObjectInterface* ActiveTraceObject; // Internal object reference for the periphery interface
	UPROPERTY(BlueprintReadWrite) ACharacter* Player;

	// UPROPERTY(BlueprintReadWrite) TScriptInterface<IPeripheryObjectInterface> PeripheryTraceInformation; // TODO: Eventually use script interfaces, we already have a good thing in place for handling logic for different objects, so this isn't necessary

	/** A list of the interactive objects the player can interact with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<AActor*> InteractiveObjectsInRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<AActor> ActiveInteractiveObject;

	/** Other */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug") bool bDebugPeripheryRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug") bool bDebugItemDetection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug") bool bDebugPeripheryCone;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|LineTrace") bool bDebugPeripheryLineTrace;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|LineTrace") FColor TraceColor = FColor::Silver;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|LineTrace") FColor TraceHitColor = FColor::Emerald;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|LineTrace") float TraceDuration = 0.1;
	
	
public:	
	UPlayerPeripheryComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * InitCharacterInformation -> Initializes the periphery for the player
	 * @note this needs to be called once the character has been initialized
	 */
	UFUNCTION(BlueprintCallable) virtual void InitPeripheryInformation();
	
	/**
	  * This logic is executed for characters with a periphery component if the object overlaps with detection components
	  *	This is useful for a number of things. Showing and updating ui states, keeping track of enemies using radar, etc.
	  *	Information that is not persistent and the player should not keep track of, but needs to be updated if the player interacts or comes within "range" of a object, this is where this might come in handy  
	 */
	/** Radius delegates */
	UPROPERTY(BlueprintAssignable) FObjectInRadius ObjectInPlayerRadius;
	UPROPERTY(BlueprintAssignable) FObjectOutsideOfRadius ObjectOutsideOfPlayerRadius;
	
	/** Periphery Cone delegates */
	UPROPERTY(BlueprintAssignable) FObjectInPeripheryCone ObjectInPeripheryCone;
	UPROPERTY(BlueprintAssignable) FObjectOutsideOfPeripheryCone ObjectOutsideOfPeripheryCone;
	
	/** Periphery Trace delegates */
	UPROPERTY(BlueprintAssignable) FObjectInPeripheryTrace ObjectInPeripheryTrace;
	UPROPERTY(BlueprintAssignable) FObjectOutsideOfPeripheryTrace ObjectOutsideOfPeripheryTrace;

	
protected:
	virtual void BeginPlay() override;
	
	/** Add collision events for the locally controlled player */
	virtual void ConfigurePeripheryCollision(UPrimitiveComponent* Component);

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
	/** Helper function for determining the type of overlay that should be used */
	UFUNCTION() virtual EPeripheryType FindPeripheryType(AActor* OtherActor) const;
	virtual bool GetCharacter(); 


public:
	UFUNCTION(BlueprintCallable) virtual AActor* GetTracedObject() const;
	UFUNCTION(BlueprintCallable) virtual USphereComponent* GetPeripheryRadius();
	UFUNCTION(BlueprintCallable) virtual UStaticMeshComponent* GetPeripheryCone();
	UFUNCTION(BlueprintCallable) virtual USphereComponent* GetItemDetection();

	
};
