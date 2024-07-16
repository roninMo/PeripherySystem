// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPeripheriesComponent.h"

#include "PeripheryObjectInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY(PeripheryLog)


UPlayerPeripheriesComponent::UPlayerPeripheriesComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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
	PeripheryRadius->SetOnlyOwnerSee(true);
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
	PeripheryCone->SetOnlyOwnerSee(true);
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
	ItemDetection->SetOnlyOwnerSee(true);
	ItemDetection->SetCastHiddenShadow(false);

	ItemDetection->SetGenerateOverlapEvents(true);
	ItemDetection->SetCollisionObjectType(ItemDetectionChannel);
	ItemDetection->SetCollisionResponseToChannels(ECR_Ignore);
	ItemDetection->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ItemDetection->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	ItemDetection->SetCollisionResponseToChannel(ItemDetectionChannel, ECollisionResponse::ECR_Overlap);
}


void UPlayerPeripheriesComponent::InitPeripheryInformation()
{
	// Initialize the periphery
	if (PeripheryRadius && ActivatePeripheryLogic(ActivationPhase))
	{
		PeripheryRadius->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::EnterPeripheryRadius);
		PeripheryRadius->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::ExitPeripheryRadius);
		ConfigurePeripheryCollision(PeripheryRadius, bRadius);
	}

	if (ItemDetection && ActivatePeripheryLogic(ActivationPhase))
	{
		ItemDetection->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::OnEnterItemRadius);
		ItemDetection->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::OnExitItemRadius);
		ConfigurePeripheryCollision(ItemDetection, bItemDetection);
	}

	// The cone, the cone of shame!
	if (PeripheryCone && ActivatePeripheryLogic(ActivationPhase))
	{
		PeripheryCone->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::EnterPeripheryCone);
		PeripheryCone->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::ExitPeripheryCone);
		ConfigurePeripheryCollision(PeripheryCone, bCone);
	}
}


void UPlayerPeripheriesComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && TraceShouldIgnoreOwnerActors)
	{
		IgnoredActors.AddUnique(GetOwner());
		GetOwner()->GetAllChildActors(IgnoredActors);
	}

	if (bInitPeripheryDuringBeginPlay) InitPeripheryInformation();
}


void UPlayerPeripheriesComponent::ConfigurePeripheryCollision(UPrimitiveComponent* Component, const bool bEnableCollision)
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


void UPlayerPeripheriesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bTrace && ActivatePeripheryLogic(ActivationPhase)) HandlePeripheryLineTrace();
}




#pragma region Periphery functions
void UPlayerPeripheriesComponent::HandlePeripheryLineTrace()
{
	GetCharacter();
	PreviousTracedActor = TracedActor; // Cached for on exit traces
	
	FVector_NetQuantize10 AimLocation = GetOwner()->GetActorLocation();
	FVector_NetQuantize10 AimForwardVector = GetOwner()->GetActorForwardVector();
	
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2D CrosshairScreenLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetOwner(), 0))
	{
		UGameplayStatics::DeprojectScreenToWorld(
			PlayerController,
			CrosshairScreenLocation,
			AimLocation, // Center of the screen deprojected to the world
			AimForwardVector // The forward vector (where the vector is aiming)
		);
	}
	
	const FVector_NetQuantize AimDirection = AimLocation + PeripheryTraceOffset + AimForwardVector * PeripheryTraceDistance; // This calculation is an fvector from our crosshair outwards
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
	bool bIsPeripheryObject = TracedActor && TracedActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
	bool bIsPreviousTracePeripheryObject = PreviousTracedActor && PreviousTracedActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
	
	// Only activate the enter overlap logic once (this also handles if they aren't already aiming at something, and still aren't)
	if (TracedActor == PreviousTracedActor) return;
	
	// if the player isn't already aiming at anything
	if (!PreviousTracedActor)
	{
		if (TracedActor && bIsPeripheryObject)
		{
			IPeripheryObjectInterface::Execute_OnEnterLineTracePeriphery(TracedActor, Player, FindPeripheryType(TracedActor));
			ObjectInPeripheryTrace.Broadcast(TracedActor, Player, HitResult);

			if (bDebugPeripheryLineTrace)
			{
				UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Started looking at {2}", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetNameSafe(GetOwner()), *GetNameSafe(TracedActor));
			}
		}
	}
	
	// Transition to aiming at another object, or transition out of aiming at an object
	else if (TracedActor)
	{
		if (bIsPreviousTracePeripheryObject)
		{
			IPeripheryObjectInterface::Execute_OnExitLineTracePeriphery(PreviousTracedActor, Player, FindPeripheryType(PreviousTracedActor));
			ObjectOutsideOfPeripheryTrace.Broadcast(PreviousTracedActor, Player, HitResult);
		}

		if (bIsPeripheryObject)
		{
			IPeripheryObjectInterface::Execute_OnEnterLineTracePeriphery(TracedActor, Player, FindPeripheryType(TracedActor));
			ObjectInPeripheryTrace.Broadcast(TracedActor, Player, HitResult);
		}
		
		if (bDebugPeripheryLineTrace)
		{
			UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Transitioned looking at {2} to {3}",
				*UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetNameSafe(GetOwner()), *GetNameSafe(PreviousTracedActor), *GetNameSafe(TracedActor)
			);
		}
	}
	else
	{
		if (bIsPreviousTracePeripheryObject)
		{
			IPeripheryObjectInterface::Execute_OnExitLineTracePeriphery(PreviousTracedActor, Player, FindPeripheryType(PreviousTracedActor));
			ObjectOutsideOfPeripheryTrace.Broadcast(PreviousTracedActor, Player, HitResult);
		}
		
		if (bDebugPeripheryLineTrace)
		{
			UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Stopped looking at {2}", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetNameSafe(GetOwner()), *GetNameSafe(TracedActor));
		}
	}
}


void UPlayerPeripheriesComponent::EnterPeripheryRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetCharacter() || !OtherActor) return;
	if (OtherActor == Player) return;

	
	const bool bPeripheryInterface = OtherActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
	if (bPeripheryInterface)
	{
		IPeripheryObjectInterface::Execute_OnEnterRadiusPeriphery(OtherActor, Player, FindPeripheryType(OtherActor)); // Object logic
		ObjectInPlayerRadius.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex, bFromSweep, SweepResult); // Player logic
	}
	
	if (bDebugPeripheryRadius)
	{
		if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: Entering Radius Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		else UE_LOGFMT(PeripheryLog, Verbose, "{0}: Entering Radius Periphery, {1} overlapped with {2} (non periphery object)", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
	}
}


void UPlayerPeripheriesComponent::ExitPeripheryRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetCharacter()) return;
	if (OtherActor == Player) return;

	const bool bPeripheryInterface = OtherActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
	if (bPeripheryInterface)
	{
		IPeripheryObjectInterface::Execute_OnExitRadiusPeriphery(OtherActor, Player, FindPeripheryType(OtherActor)); // Object logic
		ObjectOutsideOfPlayerRadius.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex); // Player logic
	}
	
	if (bDebugPeripheryRadius)
	{
		if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: Exiting Radius Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		UE_LOGFMT(PeripheryLog, Verbose, "{0}: Exiting Radius Periphery, {1} overlapped with {2} (non periphery object)", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
	}
}


void UPlayerPeripheriesComponent::EnterPeripheryCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetCharacter()) return;
	if (OtherActor == Player) return;

	const bool bPeripheryInterface = OtherActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
	if (bPeripheryInterface)
	{
		IPeripheryObjectInterface::Execute_OnEnterConePeriphery(OtherActor, Player, FindPeripheryType(OtherActor)); // Object logic
		ObjectInPeripheryCone.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex, bFromSweep, SweepResult); // Player logic
	}
	
	if (bDebugPeripheryCone)
	{
		if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: Entering Cone Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		else UE_LOGFMT(PeripheryLog, Verbose, "{0}: Entering Cone Periphery, {1} overlapped with {2} (non periphery object)", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
	}
}


void UPlayerPeripheriesComponent::ExitPeripheryCone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetCharacter()) return;
	if (OtherActor == Player) return;

	const bool bPeripheryInterface = OtherActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
	if (bPeripheryInterface)
	{
		IPeripheryObjectInterface::Execute_OnExitConePeriphery(OtherActor, Player, FindPeripheryType(OtherActor)); // Object logic
		ObjectOutsideOfPeripheryCone.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex); // Player logic
	}
	
	if (bDebugPeripheryCone)
	{
		if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: Exiting Cone Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		else UE_LOGFMT(PeripheryLog, Verbose, "{0}: Exiting Cone Periphery, {1} overlapped with {2} (non periphery object)", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
	}
}


void UPlayerPeripheriesComponent::OnEnterItemRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == Player) return;
	OnItemOverlapBegin.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}


void UPlayerPeripheriesComponent::OnExitItemRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == Player) return;
	OnItemOverlapEnd.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex);
}
#pragma endregion




EPeripheryType UPlayerPeripheriesComponent::FindPeripheryType(TScriptInterface<IPeripheryObjectInterface> PeripheryObject) const
{
	// Override this logic to determine the periphery type of an object within the player's periphery
	return EPeripheryType::EPT_None;
}


bool UPlayerPeripheriesComponent::GetCharacter()
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

bool UPlayerPeripheriesComponent::ActivatePeripheryLogic(const EHandlePeripheryLogic HandlePeripheryLogic) const
{
	if (EHandlePeripheryLogic::EP_ServerAndClient == HandlePeripheryLogic) return true;
	if (EHandlePeripheryLogic::EP_Server == HandlePeripheryLogic && ROLE_Authority == GetOwner()->GetLocalRole()) return true;
	if (EHandlePeripheryLogic::EP_Client == HandlePeripheryLogic && ROLE_AutonomousProxy == GetOwner()->GetLocalRole()) return true;
	return false;
}


TScriptInterface<IPeripheryObjectInterface> UPlayerPeripheriesComponent::GetTracedObject() const
{
	return TracedActor;
}

USphereComponent* UPlayerPeripheriesComponent::GetPeripheryRadius()
{
	return PeripheryRadius;
}

UStaticMeshComponent* UPlayerPeripheriesComponent::GetPeripheryCone()
{
	return PeripheryCone;
}

USphereComponent* UPlayerPeripheriesComponent::GetItemDetection()
{
	return ItemDetection;
}
