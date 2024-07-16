// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPeripheryComponent.h"

#include "PeripheryObjectInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY(PeripheryLog)


UPlayerPeripheryComponent::UPlayerPeripheryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Component logic
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
	
	// Periphery components
	PeripheryRadius = CreateDefaultSubobject<USphereComponent>(TEXT("Periphery Radius"));
	// PeripheryRadius->SetupAttachment(GetOwner()->GetRootComponent());
	PeripheryRadius->InitSphereRadius(1245.0f);
	PeripheryRadius->SetHiddenInGame(true);
	PeripheryRadius->SetCastHiddenShadow(false);

	PeripheryRadius->SetGenerateOverlapEvents(true);
	PeripheryRadius->SetCollisionObjectType(PeripheryRadiusChannel);
	PeripheryRadius->SetCollisionResponseToAllChannels(ECR_Ignore);
	PeripheryRadius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PeripheryRadius->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	PeripheryRadius->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	PeripheryRadius->SetCollisionResponseToChannel(PeripheryRadiusChannel, ECollisionResponse::ECR_Overlap);

	PeripheryCone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Periphery Cone"));
	// PeripheryCone->SetupAttachment(GetOwner()->GetRootComponent());
	PeripheryCone->SetHiddenInGame(true);
	PeripheryCone->SetCastHiddenShadow(false);
	
	PeripheryCone->SetGenerateOverlapEvents(true);
	PeripheryCone->SetCollisionObjectType(PeripheryConeChannel);
	PeripheryCone->SetCollisionResponseToAllChannels(ECR_Ignore);
	PeripheryCone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PeripheryCone->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	PeripheryCone->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	PeripheryCone->SetCollisionResponseToChannel(PeripheryConeChannel, ECollisionResponse::ECR_Overlap);

	ItemDetection = CreateDefaultSubobject<USphereComponent>(TEXT("Item Detection"));
	// ItemDetection->SetupAttachment(GetOwner()->GetRootComponent());
	ItemDetection->SetHiddenInGame(true);
	ItemDetection->SetCastHiddenShadow(false);

	ItemDetection->SetGenerateOverlapEvents(true);
	ItemDetection->SetCollisionObjectType(ItemDetectionChannel);
	ItemDetection->SetCollisionResponseToChannels(ECR_Ignore);
	ItemDetection->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ItemDetection->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	ItemDetection->SetCollisionResponseToChannel(ItemDetectionChannel, ECollisionResponse::ECR_Overlap);
}


void UPlayerPeripheryComponent::InitPeripheryInformation()
{
	// Initialize the periphery
	if (PeripheryRadius)
	{
		PeripheryRadius->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheryComponent::EnterPeripheryRadius);
		PeripheryRadius->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheryComponent::ExitPeripheryRadius);
		ConfigurePeripheryCollision(PeripheryRadius, bEnablePeripheryRadius);
	}

	if (ItemDetection)
	{
		ItemDetection->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheryComponent::OnEnterItemRadius);
		ItemDetection->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheryComponent::OnExitItemRadius);
		ConfigurePeripheryCollision(ItemDetection, bEnableItemDetection);
	}

	// The cone, the cone of shame!
	if (PeripheryCone)
	{
		PeripheryCone->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheryComponent::EnterPeripheryCone);
		PeripheryCone->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheryComponent::ExitPeripheryCone);
		ConfigurePeripheryCollision(PeripheryCone, bEnablePeripheryCone);
	}
}


void UPlayerPeripheryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && IgnoreOwnerActors)
	{
		IgnoredActors.AddUnique(GetOwner());
		GetOwner()->GetAllChildActors(IgnoredActors);
	}

	InitPeripheryInformation();
}


void UPlayerPeripheryComponent::ConfigurePeripheryCollision(UPrimitiveComponent* Component, const bool bEnableCollision)
{
	if (!Component) return;
	if (bEnableCollision)
	{
		Component->SetGenerateOverlapEvents(true);
		Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		Component->SetGenerateOverlapEvents(false);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}


void UPlayerPeripheryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bEnablePeripheryTrace) HandlePeripheryLineTrace();
}




#pragma region Periphery functions
void UPlayerPeripheryComponent::HandlePeripheryLineTrace()
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
	
	// Only activate the enter overlap logic once (this also handles if they aren't already aiming at something, and still aren't)
	if (TracedActor.GetInterface() == PreviousTracedActor.GetInterface()) return;
	
	// if the player isn't already aiming at anything
	if (!PreviousTracedActor.GetInterface())
	{
		if (TracedActor.GetInterface())
		{
			TracedActor->OnEnterLineTracePeriphery(Player, FindPeripheryType(TracedActor));
			ObjectInPeripheryTrace.Broadcast(TracedActor);

			if (bDebugPeripheryLineTrace)
			{
				UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Started looking at {2}", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetName(), *GetNameSafe(TracedActor.GetObject()));
			}
		}
		
		return;
	}

	// Transition to aiming at another object, or transition out of aiming at an object
	if (TracedActor.GetInterface())
	{
		TracedActor->OnEnterLineTracePeriphery(Player, FindPeripheryType(TracedActor));
		ObjectInPeripheryTrace.Broadcast(TracedActor);
		PreviousTracedActor->OnExitLineTracePeriphery(Player, FindPeripheryType(PreviousTracedActor));
		ObjectOutsideOfPeripheryTrace.Broadcast(PreviousTracedActor);
		
		if (bDebugPeripheryLineTrace)
		{
			UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Transitioned looking at {2} to {3}",
				*UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetName(), *GetNameSafe(PreviousTracedActor.GetObject()), *GetNameSafe(TracedActor.GetObject())
			);
		}
		
		return;
	}
	else
	{
		PreviousTracedActor->OnExitLineTracePeriphery(Player, FindPeripheryType(PreviousTracedActor));
		ObjectOutsideOfPeripheryTrace.Broadcast(PreviousTracedActor);
		
		if (bDebugPeripheryLineTrace)
		{
			UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Stopped looking at {2}", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetName(), *GetNameSafe(TracedActor.GetObject()));
		}

		return;
	}
}


void UPlayerPeripheryComponent::EnterPeripheryRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetCharacter()) return;
	if (OtherActor == Player) return;

	IPeripheryObjectInterface* PeripheryObject = Cast<IPeripheryObjectInterface>(OtherActor);
	if (PeripheryObject)
	{
		PeripheryObject->OnEnterRadiusPeriphery(Player, FindPeripheryType(OtherActor)); // Object logic
		ObjectInPlayerRadius.Broadcast(OtherActor); // Player logic
	}
	
	if (bDebugPeripheryRadius)
	{
		UE_LOGFMT(PeripheryLog, Log, "{0}: Entering Radius Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetName(), *GetNameSafe(OtherActor));
	}
}


void UPlayerPeripheryComponent::ExitPeripheryRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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


void UPlayerPeripheryComponent::EnterPeripheryCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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


void UPlayerPeripheryComponent::ExitPeripheryCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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


void UPlayerPeripheryComponent::OnEnterItemRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Add logic here
}


void UPlayerPeripheryComponent::OnExitItemRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Add logic here
}
#pragma endregion




EPeripheryType UPlayerPeripheryComponent::FindPeripheryType(TScriptInterface<IPeripheryObjectInterface> PeripheryObject) const
{
	// Override this logic to determine the periphery type of an object within the player's periphery
	return EPeripheryType::EP_None;
}


bool UPlayerPeripheryComponent::GetCharacter()
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


TScriptInterface<IPeripheryObjectInterface> UPlayerPeripheryComponent::GetTracedObject() const
{
	return TracedActor;
}

USphereComponent* UPlayerPeripheryComponent::GetPeripheryRadius()
{
	return PeripheryRadius;
}

UStaticMeshComponent* UPlayerPeripheryComponent::GetPeripheryCone()
{
	return PeripheryCone;
}

USphereComponent* UPlayerPeripheryComponent::GetItemDetection()
{
	return ItemDetection;
}
