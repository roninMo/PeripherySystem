#pragma once


#include "CoreMinimal.h"
#include "PeripheryTypes.h"
#include "Components/ActorComponent.h"
#include "PlayerPeripheriesComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PeripheryLog, Log, All);


class USphereComponent;
class IPeripheryObjectInterface;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FObjectInRadiusDelegate, AActor*, PeripheryObject, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FObjectOutsideOfRadiusDelegate, AActor*, PeripheryObject, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FObjectInPeripheryConeDelegate, AActor*, PeripheryObject, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FObjectOutsideOfPeripheryConeDelegate, AActor*, PeripheryObject, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FObjectInPeripheryTraceDelegate, AActor*, PeripheryObject, ACharacter*, Insigator, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FObjectOutsideOfPeripheryTraceDelegate, AActor*, PeripheryObject, ACharacter*, Insigator, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnItemOverlapBeginDelegate, AActor*, Item, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult&, SweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnItemOverlapEndDelegate, AActor*, Item, UPrimitiveComponent*, OverlappedComponent, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex);


/**
 * Class for handling interaction with certain objects within the player's periphery. \n\n
 * Just adjust the kinds of periphery you want to use, their detection, and what classes they search for, and check that they run the InitPeripheryInformation() function and you're good
 * @note There's also a periphery interface for objects having their own logic when they're within the player's periphery
 * @remarks Check the plugin's example code or the docs for it's features and how to configure things
 */
UCLASS( ClassGroup=(Periphery), meta=(BlueprintSpawnableComponent) )
class PERIPHERYSYSTEMCOMPONENT_API UPlayerPeripheriesComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bCone = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bTrace = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bRadius = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bItemDetection = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bInitPeripheryDuringBeginPlay = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius", meta = (EditCondition = "bRadius", EditConditionHides))
	TObjectPtr<USphereComponent>		PeripheryRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection", meta = (EditCondition = "bItemDetection", EditConditionHides))
	TObjectPtr<USphereComponent>		ItemDetection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone", meta = (EditCondition = "bCone", EditConditionHides))
	TObjectPtr<UStaticMeshComponent>	PeripheryCone;


	/** Periphery Radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius", meta = (EditCondition = "bRadius", EditConditionHides)) TEnumAsByte<ECollisionChannel> PeripheryRadiusChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius", meta = (EditCondition = "bRadius", EditConditionHides)) TSubclassOf<AActor> ValidPeripheryRadiusObjects;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius", meta = (EditCondition = "bRadius", EditConditionHides)) TSubclassOf<AActor> ValidPeripheryRadiusInterface;
	

	/** Item Detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection", meta = (EditCondition = "bItemDetection", EditConditionHides)) TEnumAsByte<ECollisionChannel> ItemDetectionChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection", meta = (EditCondition = "bRadius", EditConditionHides)) TSubclassOf<AActor> ValidItemDetectionObjects;

	/** Periphery Cone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone", meta = (EditCondition = "bCone", EditConditionHides)) TEnumAsByte<ECollisionChannel> PeripheryConeChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone", meta = (EditCondition = "bRadius", EditConditionHides)) TSubclassOf<AActor> ValidPeripheryConeObjects;

	/** Periphery Trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) TEnumAsByte<ETraceTypeQuery> PeripheryLineTraceType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bRadius", EditConditionHides)) TSubclassOf<AActor> ValidPeripheryTraceObjects;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) float PeripheryTraceDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bTrace", EditConditionHides)) FVector PeripheryTraceOffset;
	UPROPERTY(BlueprintReadWrite) TObjectPtr<AActor> TracedActor;
	UPROPERTY(BlueprintReadWrite) TObjectPtr<AActor> PreviousTracedActor;
	
	/** Other */
	UPROPERTY(BlueprintReadWrite) TArray<AActor*> IgnoredActors;
	UPROPERTY(BlueprintReadWrite) ACharacter* Player;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Other", meta = (EditCondition = "bTrace", EditConditionHides)) bool TraceShouldIgnoreOwnerActors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Other", meta = (EditCondition = "bRadius || bTrace || bItemDetection || bCone", EditConditionHides)) EHandlePeripheryLogic ActivationPhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius|Debug", meta = (EditCondition = "bRadius", EditConditionHides)) bool bDebugPeripheryRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection|Debug", meta = (EditCondition = "bItemDetection", EditConditionHides)) bool bDebugItemDetection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone|Debug", meta = (EditCondition = "bCone", EditConditionHides)) bool bDebugPeripheryCone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bTrace", EditConditionHides)) bool bDebugPeripheryLineTrace;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bTrace", EditConditionHides)) FColor TraceColor = FColor::Silver;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bTrace", EditConditionHides)) FColor TraceHitColor = FColor::Emerald;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bTrace", EditConditionHides)) float TraceDuration = 0.1;

	
public:	
	UPlayerPeripheriesComponent(const FObjectInitializer& ObjectInitializer);

	/**
	  * This logic is executed for characters with a periphery component if the object overlaps with detection components
	  *	This is useful for a number of things. Showing and updating ui states, keeping track of enemies using radar, etc.
	  *	Information that is not persistent and the player should not keep track of, but needs to be updated if the player interacts or comes within "range" of a object, this is where this might come in handy  
	 */
	/** Radius delegates */
	UPROPERTY(BlueprintAssignable) FObjectInRadiusDelegate ObjectInPlayerRadius;
	UPROPERTY(BlueprintAssignable) FObjectOutsideOfRadiusDelegate ObjectOutsideOfPlayerRadius;
	
	/** Item Detection delegates */
	UPROPERTY(BlueprintAssignable) FOnItemOverlapBeginDelegate OnItemOverlapBegin;
	UPROPERTY(BlueprintAssignable) FOnItemOverlapEndDelegate OnItemOverlapEnd;
    	
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
	virtual void ConfigurePeripheryCollision(UPrimitiveComponent* Component, bool bCollision);

	/** This is used for performing accurate traces for anything the player is aiming at */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
//----------------------------------------------------------------------------------------------------------------------//
// Periphery functions																									//
//----------------------------------------------------------------------------------------------------------------------//
protected:
	

	/** The overlap function for items within the player's periphery radius. Adjust what items you find with IsValidObjectInRadius(), and the settings in the blueprint */
	UFUNCTION() virtual void OnEnterRadiusPeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** The overlap function for items outside of the player's periphery radius. Adjust what items you find with IsValidObjectInRadius(), and the settings in the blueprint */
	UFUNCTION() virtual void OnExitRadiusPeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// /**
	//  * The overlap logic for the line trace periphery. This creates a trace that keeps track of the current item the player is aiming at. \n\n
	//  * Activates delegate the delegate functions ObjectInPeripheryTrace() and ObjectOutsideOfPeripheryTrace() when a valid object is within or outside of the trace
	//  * @remarks Adjust this for handling your own logic for finding valid things within the player's periphery
	//  */
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Trace") void HandlePeripheryLineTrace();
	// virtual void HandlePeripheryLineTrace_Implementation();
	//
	// /**
	//  * The overlap function to handle checking for valid items within the periphery radius. Adjust this for handling your own logic for finding valid things within the player's periphery.
	//  * Activates delegate the delegate functions ObjectInPlayerRadius() and ObjectOutsideOfPlayerRadius() when a valid object is within or outside of the radius
	//  * @remarks Adjust this for handling your own logic for finding valid things within the player's periphery
	//  */
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Radius") bool IsValidObjectInRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep = false, const FHitResult& SweepResult = FHitResult());
	// virtual bool IsValidObjectInRadius_Implementation(
	// 	UPrimitiveComponent* OverlappedComponent,
	// 	AActor* OtherActor,
	// 	UPrimitiveComponent* OtherComp,
	// 	int32 OtherBodyIndex,
	// 	bool bFromSweep = false,
	// 	const FHitResult& SweepResult = FHitResult()
	// );
	//
	//
	// /**
	//  * The overlap function to handle checking for valid items within the periphery radius. Adjust this for handling your own logic for finding valid things within the player's periphery.
	//  * Activates delegate the delegate functions ObjectInPeripheryCone() and ObjectOutsideOfPeripheryCone() when a valid object is within or outside of the cone
	//  * @remarks Adjust this for handling your own logic for finding valid things within the player's periphery
	//  */
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Cone") bool IsValidObjectInCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep = false, const FHitResult& SweepResult = FHitResult());
	// virtual bool IsValidObjectInCone_Implementation(
	// 	UPrimitiveComponent* OverlappedComponent,
	// 	AActor* OtherActor,
	// 	UPrimitiveComponent* OtherComp,
	// 	int32 OtherBodyIndex,
	// 	bool bFromSweep = false,
	// 	const FHitResult& SweepResult = FHitResult()
	// );
	//
	//
	// /**
	//  * The overlap function to handle detecting valid items. Adjust this for handling your own logic for finding valid things for item detection.
	//  * Activates delegate the delegate functions OnItemOverlapBegin() and OnItemOverlapEnd() when a valid item is within or outside of the player's item detection
	//  * @remarks Adjust this for handling your own logic for finding valid items
	//  */
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Peripheries|Item Detection") bool IsValidItemDetected(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep = false, const FHitResult& SweepResult = FHitResult());
	// virtual bool IsValidItemDetected_Implementation(
	// 	UPrimitiveComponent* OverlappedComponent,
	// 	AActor* OtherActor,
	// 	UPrimitiveComponent* OtherComp,
	// 	int32 OtherBodyIndex,
	// 	bool bFromSweep = false,
	// 	const FHitResult& SweepResult = FHitResult()
	// );
	//
	// /** The overlap function for items within the player's periphery radius. Adjust what items you find with IsValidObjectInRadius(), and the settings in the blueprint */
	// UFUNCTION() virtual void OnEnterRadiusPeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//
	// /** The overlap function for items outside of the player's periphery radius. Adjust what items you find with IsValidObjectInRadius(), and the settings in the blueprint */
	// UFUNCTION() virtual void OnExitRadiusPeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//
	// /** The overlap function for items within the player's periphery radius. Adjust what items you find with IsValidObjectInCone(), and the settings in the blueprint */
	// UFUNCTION() virtual void OnEnterConePeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//
	// /** The overlap function for items outside of the player's periphery radius. Adjust what items you find with IsValidObjectInCone(), and the settings in the blueprint */
	// UFUNCTION() virtual void OnExitConePeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//
	// /** The overlap function for items within the player's item detection. Adjust what items you find with IsValidItemDetected(), and the settings in the blueprint */
	// UFUNCTION() virtual void OnEnterItemDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//
	// /** The overlap function for items outside of the player's item detection. Adjust what items you find with IsValidItemDetected(), and the settings in the blueprint */
	// UFUNCTION() virtual void OnExitItemDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
//----------------------------------------------------------------------------------------------//
// Other																						//
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
	UFUNCTION(BlueprintCallable) virtual bool ActivatePeripheryLogic(const EHandlePeripheryLogic HandlePeripheryLogic) const;
	UFUNCTION(BlueprintCallable) virtual TScriptInterface<IPeripheryObjectInterface> GetTracedObject() const;
	UFUNCTION(BlueprintCallable) virtual USphereComponent* GetPeripheryRadius();
	UFUNCTION(BlueprintCallable) virtual UStaticMeshComponent* GetPeripheryCone();
	UFUNCTION(BlueprintCallable) virtual USphereComponent* GetItemDetection();

	
};
