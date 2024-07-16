#pragma once


#include "CoreMinimal.h"
#include "PeripheryTypes.h"
#include "Components/ActorComponent.h"
#include "PlayerPeripheriesComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PeripheryLog, Log, All);


class USphereComponent;
class IPeripheryObjectInterface;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInRadiusDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfRadiusDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInPeripheryConeDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfPeripheryConeDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectInPeripheryTraceDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectOutsideOfPeripheryTraceDelegate, TScriptInterface<IPeripheryObjectInterface>, PeripheryObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemOverlapBeginDelegate, AActor*, ItemDetected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemOverlapEndDelegate, AActor*, ItemDetected);


/**
 * Class for handling interaction with certain objects within the player's periphery.
 * @note there's logic for handling what happens when a player finds an object, and you're able to customize the behavior for different objects
 * @ref You still need to handle the majority of the logic for building up the components, it only takes a moment to setup the configuration
 *		- Configure the components to link to the character, InitPeripheryInformation, Blueprint delegates, and periphery functions customization
 */
UCLASS( ClassGroup=(Periphery), meta=(BlueprintSpawnableComponent) )
class PERIPHERYSYSTEMCOMPONENT_API UPlayerPeripheriesComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bPeripheryRadius = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bPeripheryTrace = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bItemDetection = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bPeripheryCone = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery") bool bInitPeripheryDuringBeginPlay = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius", meta = (EditCondition = "bPeripheryRadius", EditConditionHides))
	TObjectPtr<USphereComponent>		PeripheryRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection", meta = (EditCondition = "bItemDetection", EditConditionHides))
	TObjectPtr<USphereComponent>		ItemDetection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone", meta = (EditCondition = "bPeripheryCone", EditConditionHides))
	TObjectPtr<UStaticMeshComponent>	PeripheryCone;


	/** Periphery Radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius", meta = (EditCondition = "bPeripheryRadius", EditConditionHides))
	TEnumAsByte<ECollisionChannel> PeripheryRadiusChannel = ECC_Pawn;

	/** Item Detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection", meta = (EditCondition = "bItemDetection", EditConditionHides))
	TEnumAsByte<ECollisionChannel> ItemDetectionChannel = ECC_GameTraceChannel1;

	/** Periphery Cone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone", meta = (EditCondition = "bPeripheryCone", EditConditionHides))
	TEnumAsByte<ECollisionChannel> PeripheryConeChannel = ECC_Pawn;

	/** Periphery Trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bPeripheryTrace", EditConditionHides)) TEnumAsByte<ETraceTypeQuery> PeripheryLineTraceType = TraceTypeQuery1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bPeripheryTrace", EditConditionHides)) float PeripheryTraceDistance = 6400;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace", meta = (EditCondition = "bPeripheryTrace", EditConditionHides)) FVector PeripheryTraceOffset = FVector(0.0f, 0.0f, 34.0f);
	UPROPERTY(BlueprintReadWrite) TObjectPtr<AActor> TracedActor;
	UPROPERTY(BlueprintReadWrite) TObjectPtr<AActor> PreviousTracedActor;
	
	/** Other */
	UPROPERTY(BlueprintReadWrite) TArray<AActor*> IgnoredActors;
	UPROPERTY(BlueprintReadWrite) ACharacter* Player;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Other", meta = (EditCondition = "bPeripheryRadius || bPeripheryTrace || bItemDetection || bPeripheryCone", EditConditionHides)) bool TraceShouldIgnoreOwnerActors = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Other", meta = (EditCondition = "bPeripheryRadius || bPeripheryTrace || bItemDetection || bPeripheryCone", EditConditionHides)) EHandlePeripheryLogic ActivationPhase = EHandlePeripheryLogic::EP_ServerAndClient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Radius|Debug", meta = (EditCondition = "bPeripheryRadius", EditConditionHides)) bool bDebugPeripheryRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Item Detection|Debug", meta = (EditCondition = "bItemDetection", EditConditionHides)) bool bDebugItemDetection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Cone|Debug", meta = (EditCondition = "bPeripheryCone", EditConditionHides)) bool bDebugPeripheryCone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bPeripheryTrace", EditConditionHides)) bool bDebugPeripheryLineTrace;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bPeripheryTrace", EditConditionHides)) FColor TraceColor = FColor::Silver;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bPeripheryTrace", EditConditionHides)) FColor TraceHitColor = FColor::Emerald;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periphery|Trace|Debug", meta = (EditCondition = "bPeripheryTrace", EditConditionHides)) float TraceDuration = 0.1;

	
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
	UFUNCTION(BlueprintCallable) virtual bool ActivatePeripheryLogic(const EHandlePeripheryLogic HandlePeripheryLogic) const;
	UFUNCTION(BlueprintCallable) virtual TScriptInterface<IPeripheryObjectInterface> GetTracedObject() const;
	UFUNCTION(BlueprintCallable) virtual USphereComponent* GetPeripheryRadius();
	UFUNCTION(BlueprintCallable) virtual UStaticMeshComponent* GetPeripheryCone();
	UFUNCTION(BlueprintCallable) virtual USphereComponent* GetItemDetection();

	
};
