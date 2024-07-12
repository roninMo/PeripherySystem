// Fill out your copyright notice in the Description page of Project Settings.


#include "PeripheryComponent.h"

#include "PeripheryObjectInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY(PeripheryLog)


UPeripheryComponent::UPeripheryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Component logic
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
	
	// Periphery components
	PeripheryRadius = CreateDefaultSubobject<USphereComponent>(TEXT("Periphery Radius"));
	PeripheryRadius->SetupAttachment(GetOwner()->GetRootComponent());
	PeripheryRadius->InitSphereRadius(1250.0f);
	
	PeripheryCone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Periphery Cone"));
	PeripheryCone->SetupAttachment(GetOwner()->GetRootComponent());
	PeripheryCone->SetRelativeLocation(FVector(640.0f, 0.0f, 0.0f));
	PeripheryCone->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	PeripheryCone->SetRelativeScale3D(FVector(1.0f, 1.0f, 6.4f));
	PeripheryCone->SetHiddenInGame(true);
	PeripheryCone->SetCastHiddenShadow(false);
	
	PeripheryRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PeripheryRadius->SetCollisionObjectType(PeripheryRadiusChannel);
	PeripheryCone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PeripheryCone->SetCollisionObjectType(PeripheryRadiusChannel);
	
	// Item Detection
	ItemDetection = CreateDefaultSubobject<USphereComponent>(TEXT("Item Detection"));
	ItemDetection->SetupAttachment(GetOwner()->GetRootComponent());
	ItemDetection->InitSphereRadius(100.f);
	ItemDetection->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	ItemDetection->SetGenerateOverlapEvents(true);
	ItemDetection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ItemDetection->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemDetection->SetCollisionObjectType(ItemDetectionChannel);
	ItemDetection->SetCollisionResponseToChannel(ItemDetectionChannel, ECollisionResponse::ECR_Overlap);
	ItemDetection->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	ItemDetection->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}


void UPeripheryComponent::InitPeripheryInformation()
{
	// Initialize the periphery
	if (PeripheryRadius)
	{
		PeripheryRadius->OnComponentBeginOverlap.AddDynamic(this, &UPeripheryComponent::EnterPeripheryRadius);
		PeripheryRadius->OnComponentEndOverlap.AddDynamic(this, &UPeripheryComponent::ExitPeripheryRadius);
		ConfigurePeripheryCollision(PeripheryRadius);
	}

	// The cone, the cone of shame!
	if (PeripheryCone)
	{
		PeripheryCone->OnComponentBeginOverlap.AddDynamic(this, &UPeripheryComponent::EnterPeripheryCone);
		PeripheryCone->OnComponentEndOverlap.AddDynamic(this, &UPeripheryComponent::ExitPeripheryCone);
		ConfigurePeripheryCollision(PeripheryCone);
	}

	ItemDetection->OnComponentBeginOverlap.AddDynamic(this, &UPeripheryComponent::OnEnterItemRadius);
	ItemDetection->OnComponentEndOverlap.AddDynamic(this, &UPeripheryComponent::OnExitItemRadius);
}


void UPeripheryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		IgnoredActors.AddUnique(GetOwner());
		GetOwner()->GetAllChildActors(IgnoredActors);
	}
}


void UPeripheryComponent::ConfigurePeripheryCollision(UPrimitiveComponent* Component)
{
	if (!Component) return;
	Component->SetGenerateOverlapEvents(true);
	Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Component->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	Component->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	Component->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
}


void UPeripheryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}




#pragma region Periphery functions
void UPeripheryComponent::HandlePeripheryLineTrace()
{
	PreviousTracedActor = TracedActor; // Cached for on exit traces
	
	// Search for objects the player is aiming at
	FVector_NetQuantize10 AimLocation;
	FVector_NetQuantize10 AimForwardVector;
	
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2D CrosshairScreenLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	
	UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(GetOwner(), 0), // The local client's player controller
		CrosshairScreenLocation,
		AimLocation, // Center of the screen deprojected to the world
		AimForwardVector // The forward vector (where the vector is aiming)
	);
	
	const FVector_NetQuantize AimDirection = AimLocation + AimForwardVector * PeripheryTraceDistance; // This calculation is an fvector from our crosshair outwards
	FVector StartLocation = AimLocation;
	
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(), StartLocation, AimDirection, PeripheryLineTraceType, false,  IgnoredActors,
		bDebugPeripheryLineTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, HitResult, true, TraceColor, TraceHitColor, TraceDuration
	);
	
	if (bDebugPeripheryLineTrace && HitResult.bBlockingHit) DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 12.f, 12, FColor::Red, false, TraceDuration);
	else HitResult.ImpactPoint = AimDirection;
	

	// Periphery logic
	TracedActor = HitResult.GetActor();
	IPeripheryObjectInterface* PeripheryObject = Cast<IPeripheryObjectInterface>(TracedActor);
	
	// Only activate the enter overlap logic once (this also handles if they aren't already aiming at something, and still aren't)
	if (ActiveTraceObject == PeripheryObject) return;
	
	// if the player isn't already aiming at anything
	if (!ActiveTraceObject)
	{
		if (PeripheryObject)
		{
			ActiveTraceObject = PeripheryObject;
			ActiveTraceObject->OnEnterLineTracePeriphery(Player, FindPeripheryType(TracedActor));
			ObjectInPeripheryTrace.Broadcast(TracedActor);

			if (bDebugPeripheryLineTrace)
			{
				UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Started looking at {2}", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetName(), *GetNameSafe(TracedActor));
			}
		}
		
		return;
	}

	// Transition to aiming at another object, or transition out of aiming at an object
	if (ActiveTraceObject)
	{
		ActiveTraceObject->OnExitLineTracePeriphery(Player, FindPeripheryType(PreviousTracedActor));
		ObjectOutsideOfPeripheryTrace.Broadcast(TracedActor);
		
		if (PeripheryObject)
		{
			ActiveTraceObject = PeripheryObject;
			ActiveTraceObject->OnEnterLineTracePeriphery(Player, FindPeripheryType(TracedActor));
		}
		else
		{
			ActiveTraceObject = nullptr;
			// if (bDebugPlayerPeripheries) UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Stopped looking at {2}", *UEnum::GetValueAsString(GetLocalRole()), *GetName(), *GetNameSafe(TracedActor));
		}

		return;
	}
}


void UPeripheryComponent::EnterPeripheryRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetCharacter()) return;
	if (OtherActor == Player) return;

	IPeripheryObjectInterface* PeripheryObject = Cast<IPeripheryObjectInterface>(OtherActor);
	if (PeripheryObject)
	{
		PeripheryObject->OnEnterRadiusPeriphery(Player, FindPeripheryType(OtherActor));
		ObjectInPlayerRadius.Broadcast(OtherActor);
	}
	
	if (bDebugPeripheryRadius)
	{
		UE_LOGFMT(PeripheryLog, Log, "{0}: Entering Radius Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetName(), *GetNameSafe(OtherActor));
	}
}


void UPeripheryComponent::ExitPeripheryRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetCharacter()) return;
	if (OtherActor == Player) return;

	IPeripheryObjectInterface* PeripheryObject = Cast<IPeripheryObjectInterface>(OtherActor);
	if (PeripheryObject)
	{
		PeripheryObject->OnExitRadiusPeriphery(Player, FindPeripheryType(OtherActor));
		ObjectOutsideOfPlayerRadius.Broadcast(OtherActor);
	}
	
	if (bDebugPeripheryRadius)
	{
		UE_LOGFMT(PeripheryLog, Log, "{0}: Exiting Radius Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetName(), *GetNameSafe(OtherActor));
	}
}


void UPeripheryComponent::EnterPeripheryCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetCharacter()) return;
	if (OtherActor == Player) return;

	IPeripheryObjectInterface* PeripheryObject = Cast<IPeripheryObjectInterface>(OtherActor);
	if (PeripheryObject)
	{
		PeripheryObject->OnEnterConePeriphery(Player, FindPeripheryType(OtherActor));
		ObjectInPeripheryCone.Broadcast(OtherActor);
	}
	
	if (bDebugPeripheryCone)
	{
		UE_LOGFMT(PeripheryLog, Log, "{0}: Entering Cone Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetName(), *GetNameSafe(OtherActor));
	}
}


void UPeripheryComponent::ExitPeripheryCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetCharacter()) return;
	if (OtherActor == Player) return;

	IPeripheryObjectInterface* PeripheryObject = Cast<IPeripheryObjectInterface>(OtherActor);
	if (PeripheryObject)
	{
		PeripheryObject->OnExitConePeriphery(Player, FindPeripheryType(OtherActor));
		ObjectOutsideOfPeripheryCone.Broadcast(OtherActor);
	}
	
	if (bDebugPeripheryCone)
	{
		UE_LOGFMT(PeripheryLog, Log, "{0}: Exiting Cone Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetName(), *GetNameSafe(OtherActor));
	}
}


void UPeripheryComponent::OnEnterItemRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Add logic here
}


void UPeripheryComponent::OnExitItemRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Add logic here
}
#pragma endregion




EPeripheryType UPeripheryComponent::FindPeripheryType(AActor* OtherActor) const
{
	// Override this logic to determine the periphery type of an object within the player's periphery
	return EPeripheryType::EP_None;
}


bool UPeripheryComponent::GetCharacter()
{
	if (Player) return true;

	Player = Cast<ACharacter>(GetOwner());
	if (!Player)
	{
		UE_LOGFMT(PeripheryLog, Error, "{0}:{1} ->  Error while trying to find the owner of the periphery component", *FString(__FUNCTION__), *GetName());
		return false;
	}

	return true;
	
}


AActor* UPeripheryComponent::GetTracedObject() const
{
	return TracedActor;
}

