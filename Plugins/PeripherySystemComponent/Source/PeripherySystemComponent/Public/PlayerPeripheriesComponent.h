#pragma once


#include "CoreMinimal.h"
#include "PeripheryTypes.h"
#include "Components/ActorComponent.h" 
#include "PlayerPeripheriesComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PeripheryLog, Log, All);


/** Periphery delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FObjectInRadiusDelegate, AActor*, Actor, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FObjectOutsideOfRadiusDelegate, AActor*, Actor, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FObjectInPeripheryConeDelegate, AActor*, Actor, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FObjectOutsideOfPeripheryConeDelegate, AActor*, Actor, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FObjectInPeripheryTraceDelegate, AActor*, Actor, ACharacter*, Insigator, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FObjectOutsideOfPeripheryTraceDelegate, AActor*, Actor, ACharacter*, Insigator, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnItemOverlapBeginDelegate, AActor*, Item, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnItemOverlapEndDelegate, AActor*, Item, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex);


class USphereComponent;
class IPeripheryObjectInterface;


/**
 * Class for handling interaction with certain objects within the player's periphery. This lets you do things like keep track of targets within the player's radius, highlight objects the player finds, and plenty of other things. \n\n
 * Just adjust the kinds of periphery you want to use, their detection with, and check that you run the InitPeripheryInformation() (bInitPeripheryDuringBeginPlay) function and you're good
 * 
 * @note There's also a periphery interface for objects having their own logic when they're within the player's periphery
 * @remark Check the plugin's example code or the docs for it's features and how to configure things \n\n
 */
UCLASS(Blueprintable, ClassGroup=(Periphery), meta = (BlueprintSpawnableComponent))
class PERIPHERYSYSTEMCOMPONENT_API UPlayerPeripheriesComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/** Whether to use the periphery cone logic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries") bool bCone;
	
	/** Whether to use the periphery trace logic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries") bool bTrace;
	
	/** Whether to use the periphery radius logic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries") bool bRadius;
	
	/** Whether to use the item detection logic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries") bool bItemDetection;

	/** Whether to initialize the periphery during BeginPlay. If this isn't set to true, you need to call InitPeripheryInformation() before any of the periphery logic initializes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries") bool bInitPeripheryDuringBeginPlay;
	
	/** The radius of the character, for things like target locking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Radius", meta = (EditCondition = "bRadius", EditConditionHides))
	TObjectPtr<USphereComponent>		PeripheryRadius;

	/** Item detection, for finding and interacting with items the player can pickup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Item Detection", meta = (EditCondition = "bItemDetection", EditConditionHides))
	TObjectPtr<USphereComponent>		ItemDetection;

	// TODO: Investigate third person physics updates to render overlaps when the character isn't moving, and bUseControllerRotation is on.
	// The camera logic isn't updating on the client/server for the character's components unless the character is actually moving, so it doesn't activate the overlap functions
	/** The periphery cone used for interacting with things that are close the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Cone", meta = (EditCondition = "bCone", EditConditionHides))
	TObjectPtr<UStaticMeshComponent>	PeripheryCone;


	/**** Periphery Radius ****/
	/** The collision channel for the periphery radius sphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Radius", meta = (EditCondition = "bRadius", EditConditionHides)) TEnumAsByte<ECollisionChannel> PeripheryRadiusChannel;

	/** A reference to the classes the periphery radius searches for. You can also override IsValidObjectInRadius() for custom logic to search for different things */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Radius", meta = (EditCondition = "bRadius", EditConditionHides)) TSubclassOf<AActor> ValidPeripheryRadiusObjects;

	/** Debug the periphery radius functions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Radius", meta = (EditCondition = "bRadius", EditConditionHides)) bool bDebugPeripheryRadius;

	
	/**** Item Detection ****/
	/** The collision channel for the item detection sphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Item Detection", meta = (EditCondition = "bItemDetection", EditConditionHides)) TEnumAsByte<ECollisionChannel> ItemDetectionChannel;
	
	/** A reference to the classes the item detection sphere searches for. You can also override IsValidItemDetected() for custom logic to search for different things */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Item Detection", meta = (EditCondition = "bItemDetection", EditConditionHides)) TSubclassOf<AActor> ValidItemDetectionObjects;
	
	/** Debug the item detection functions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Item Detection", meta = (EditCondition = "bItemDetection", EditConditionHides)) bool bDebugItemDetection;

	
	/**** Periphery Cone ****/
	/** The collision channel for the periphery cone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Cone", meta = (EditCondition = "bCone", EditConditionHides)) TEnumAsByte<ECollisionChannel> PeripheryConeChannel;
	
	/** A reference to the classes the periphery cone searches for. You can also override IsValidObjectInCone() for custom logic to search for different things */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Cone", meta = (EditCondition = "bCone", EditConditionHides)) TSubclassOf<AActor> ValidPeripheryConeObjects;

	/** Debug the periphery cone functions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Cone", meta = (EditCondition = "bCone", EditConditionHides)) bool bDebugPeripheryCone;

	
	/**** Periphery Trace ****/
	/** The object types the periphery trace searches for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) TArray<TEnumAsByte<EObjectTypeQuery>> PeripheryLineTraceObjectTypes;

	/** A reference to the classes the periphery cone searches for. You can also override IsValidTracedObject() for custom logic to search for different things */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) TSubclassOf<AActor> ValidPeripheryTraceObjects;

	/** The distance of the trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) float PeripheryTraceDistance;

	/** The offset is to help with things like third person camera adjustments so it doesn't trace over the character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) float PeripheryTraceForwardOffset;

	/** Whether the trace should ignore the owner's actors, which are captured during begin play (if this is set to true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) bool TraceShouldIgnoreOwnerActors;
	
	/** Debug the periphery trace functions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) bool bDebugPeripheryTrace;

	/** Draw the debug trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) bool bDrawTraceDebug;

	/** The color of the trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace && bDrawTraceDebug", EditConditionHides)) FColor TraceColor = FColor::Emerald;

	/** The color of the trace when it finds something */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace && bDrawTraceDebug", EditConditionHides)) FColor TraceHitColor = FColor::Emerald;

	/** The duration of the trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Trace", meta = (EditCondition = "bTrace && bDrawTraceDebug", EditConditionHides)) float TraceDuration = 0.1;
	UPROPERTY(BlueprintReadWrite, Category = "Peripheries|Trace") TObjectPtr<AActor> TracedActor;
	UPROPERTY(BlueprintReadWrite, Category = "Peripheries|Trace") TObjectPtr<AActor> PreviousTracedActor;
	UPROPERTY(BlueprintReadWrite, Category = "Peripheries|Trace") bool bIsPreviousTraceValidPeripheryObject;

	
	/**** Other ****/
	/** Does the periphery logic run on the client, server, or both? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Peripheries|Other", meta = (EditCondition = "bRadius || bTrace || bItemDetection || bCone", EditConditionHides)) EHandlePeripheryLogic ActivationPhase;
	UPROPERTY(BlueprintReadWrite, Category = "Peripheries|Other") TArray<AActor*> IgnoredActors;
	UPROPERTY(BlueprintReadWrite, Category = "Peripheries|Utilitiy") ACharacter* Player;

	
public:	
	UPlayerPeripheriesComponent(const FObjectInitializer& ObjectInitializer);

	/**
	  * This logic is executed for characters with a periphery component if the object overlaps with detection components
	  *	This is useful for a number of things. Showing and updating ui states, keeping track of enemies using radar, etc.
	  *	Information that is not persistent and the player should not keep track of, but needs to be updated if the player interacts or comes within "range" of a object, this is where this might come in handy  
	 */
	/** Radius delegates */
	UPROPERTY(BlueprintAssignable, Category = "Peripheries|Radius") FObjectInRadiusDelegate ObjectInPlayerRadius;
	UPROPERTY(BlueprintAssignable, Category = "Peripheries|Radius") FObjectOutsideOfRadiusDelegate ObjectOutsideOfPlayerRadius;
	
	/** Item Detection delegates */
	UPROPERTY(BlueprintAssignable, Category = "Peripheries|Item Detection") FOnItemOverlapBeginDelegate OnItemOverlapBegin;
	UPROPERTY(BlueprintAssignable, Category = "Peripheries|Item Detection") FOnItemOverlapEndDelegate OnItemOverlapEnd;
    	
	/** Periphery Cone delegates */
	UPROPERTY(BlueprintAssignable, Category = "Peripheries|Cone") FObjectInPeripheryConeDelegate ObjectInPeripheryCone;
	UPROPERTY(BlueprintAssignable, Category = "Peripheries|Cone") FObjectOutsideOfPeripheryConeDelegate ObjectOutsideOfPeripheryCone;
	
	/** Periphery Trace delegates */
	UPROPERTY(BlueprintAssignable, Category = "Peripheries|Trace") FObjectInPeripheryTraceDelegate ObjectInPeripheryTrace;
	UPROPERTY(BlueprintAssignable, Category = "Peripheries|Cone") FObjectOutsideOfPeripheryTraceDelegate ObjectOutsideOfPeripheryTrace;


	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Add collision events for the locally controlled player */
	UFUNCTION(BlueprintCallable, Category = "Peripheries|Utilities")
	virtual void ConfigurePeripheryCollision(UPrimitiveComponent* Component, bool bCollision);

	/** This is used for performing accurate traces for anything the player is aiming at */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
//----------------------------------------------------------------------------------------------------------------------//
// Periphery functions																									//
//----------------------------------------------------------------------------------------------------------------------//
protected:
	/**
	 * The trace logic for the line trace periphery. This creates a trace returns the result. \n\n
	 * This function is called during HandlePeripheryLineTrace()
	 * @remark Adjust this for handling your own logic for the trace \n
	 * @remark Use PeripheryTraceOffset for handling camera offsets and other values for the trace (so you don't have to create a custom function)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Trace") void PeripheryLineTrace(FHitResult& Result);
	virtual void PeripheryLineTrace_Implementation(FHitResult& Result);
	
	/**
	 * The overlap logic for the line trace periphery. This creates a trace that keeps track of the current item the player is aiming at. \n\n
	 * Activates delegate the delegate functions ObjectInPeripheryTrace() and ObjectOutsideOfPeripheryTrace() when a valid object is within or outside of the trace \n\n
	 * @remark Adjust this for handling your own logic for finding valid things within the player's periphery \n
	 * @remark Use PeripheryTraceOffset for handling camera offsets and other values for the trace
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Trace") void HandlePeripheryLineTrace();
	virtual void HandlePeripheryLineTrace_Implementation();
	
	/** The overlap function for items within the player's periphery radius. Adjust what items you find with IsValidObjectInRadius(), and the settings in the blueprint */
	UFUNCTION() virtual void OnEnterRadiusPeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** The overlap function for items outside of the player's periphery radius. Adjust what items you find with IsValidObjectInRadius(), and the settings in the blueprint */
	UFUNCTION() virtual void OnExitRadiusPeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	/** The overlap function for items within the player's periphery radius. Adjust what items you find with IsValidObjectInCone(), and the settings in the blueprint */
	UFUNCTION() virtual void OnEnterConePeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** The overlap function for items outside of the player's periphery radius. Adjust what items you find with IsValidObjectInCone(), and the settings in the blueprint */
	UFUNCTION() virtual void OnExitConePeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	/** The overlap function for items within the player's item detection. Adjust what items you find with IsValidItemDetected(), and the settings in the blueprint */
	UFUNCTION() virtual void OnEnterItemDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** The overlap function for items outside of the player's item detection. Adjust what items you find with IsValidItemDetected(), and the settings in the blueprint */
	UFUNCTION() virtual void OnExitItemDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	
	/**
	 * The overlap function to handle checking for valid objects within the periphery radius. Adjust this for handling your own logic for finding valid things within the player's periphery. \n\n
	 * Activates delegate the delegate functions ObjectInPlayerRadius() and ObjectOutsideOfPlayerRadius() when a valid object is within or outside of the radius \n\n
	 * @remarks Adjust this for handling your own logic for finding valid things within the player's periphery
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Radius") bool IsValidObjectInRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep = false, const FHitResult& SweepResult = FHitResult());
	virtual bool IsValidObjectInRadius_Implementation(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep = false,
		const FHitResult& SweepResult = FHitResult()
	);
	
	/**
	 * The overlap function to handle checking for valid items within the periphery trace. Adjust this for handling your own logic for finding valid things within the player's periphery. \n\n
	 * Activates delegate the delegate functions ObjectInPeripheryTrace() and ObjectOutsideOfPeripheryTrace() when a valid object is within or outside of the radius \n\n
	 * @remarks Adjust this for handling your own logic for finding valid things within the player's periphery
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Trace") bool IsValidTracedObject(AActor* OtherActor, const FHitResult& HitResult);
	virtual bool IsValidTracedObject_Implementation(AActor* OtherActor, const FHitResult& HitResult);
	
	/**
	 * The overlap function to handle checking for valid objects within the periphery cone. Adjust this for handling your own logic for finding valid things within the player's periphery. \n\n
	 * Activates delegate the delegate functions ObjectInPeripheryCone() and ObjectOutsideOfPeripheryCone() when a valid object is within or outside of the cone \n\n
	 * @remarks Adjust this for handling your own logic for finding valid things within the player's periphery
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Cone") bool IsValidObjectInCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep = false, const FHitResult& SweepResult = FHitResult());
	virtual bool IsValidObjectInCone_Implementation(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep = false,
		const FHitResult& SweepResult = FHitResult()
	);
	
	/**
	 * The overlap function to handle detecting valid items. Adjust this for handling your own logic for finding valid things for item detection. \n\n
	 * Activates delegate the delegate functions OnItemOverlapBegin() and OnItemOverlapEnd() when a valid item is within or outside of the player's item detection \n\n
	 * @remarks Adjust this for handling your own logic for finding valid items
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Item Detection") bool IsValidItemDetected(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep = false, const FHitResult& SweepResult = FHitResult());
	virtual bool IsValidItemDetected_Implementation(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep = false,
		const FHitResult& SweepResult = FHitResult()
	);

	
//----------------------------------------------------------------------------------------------//
// Other																						//
//----------------------------------------------------------------------------------------------//
protected:
	/**
	 * Configures the periphery component's logic for the player \n\n
	 * @remarks This is called automatically if bInitPeripheryDuringBeginPlay is set to true. Otherwise, this needs to be called once the character has been initialized
	 */
	UFUNCTION(BlueprintCallable, Category = "Peripheries|Utilities") virtual void InitPeripheryInformation();
	
	/** Helper function for determining the type of overlay that should be used */
	UFUNCTION() virtual EPeripheryType FindPeripheryType(TScriptInterface<IPeripheryObjectInterface> PeripheryObject) const;
	virtual bool GetCharacter(); 


public:
	/** Used for networking. Determines whether the logic should be activated based on the argument passed in and if it's the client or server character */
	UFUNCTION(BlueprintCallable, Category = "Peripheries|Utilities") virtual bool ActivatePeripheryLogic(const EHandlePeripheryLogic HandlePeripheryLogic) const;
	UFUNCTION(BlueprintCallable, Category = "Peripheries|Utilities") virtual TScriptInterface<IPeripheryObjectInterface> GetTracedObject() const;
	UFUNCTION(BlueprintCallable, Category = "Peripheries|Utilities") virtual USphereComponent* GetPeripheryRadius();
	UFUNCTION(BlueprintCallable, Category = "Peripheries|Utilities") virtual UStaticMeshComponent* GetPeripheryCone();
	UFUNCTION(BlueprintCallable, Category = "Peripheries|Utilities") virtual USphereComponent* GetItemDetection();

	/** Utility functions for checking if playing in editor */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Platform") bool IsPlayingInEditor(UObject* WorldContextObject) const;
	
	/** Utility functions for checking if playing in game */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Platform") bool IsPlayingInGame(UObject* WorldContextObject) const;

	
};
