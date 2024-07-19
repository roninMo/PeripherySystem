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
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
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

	/** Periphery Values */
	bCone = false;
	bTrace = false;
	bRadius = true;
	bItemDetection = false;
	bInitPeripheryDuringBeginPlay = true;
	ActivationPhase = EHandlePeripheryLogic::EP_Server;
	
	/** Periphery Radius */
	PeripheryRadiusChannel = ECC_Pawn;
	ValidPeripheryRadiusObjects = AActor::StaticClass();
	ItemDetection->ShapeColor = FColor(116, 134, 29, 255);
	ItemDetection->SetSphereRadius(1340);
	
	/** Item Detection */
	ItemDetectionChannel = ECC_GameTraceChannel1;
	ValidItemDetectionObjects = AActor::StaticClass();
	ItemDetection->ShapeColor = FColor(150,255,108,255);
	ItemDetection->SetSphereRadius(79);
	ItemDetection->SetRelativeLocation(FVector(0, 0, -79));

	/** Periphery Cone */
	PeripheryConeChannel = ECC_Pawn;
	ValidPeripheryConeObjects = AActor::StaticClass();

	/** Periphery Trace */
	PeripheryLineTraceType = TraceTypeQuery1;
	ValidPeripheryTraceObjects = AActor::StaticClass();
	PeripheryTraceDistance = 6400;
	PeripheryTraceForwardOffset = 34.0;
	TraceShouldIgnoreOwnerActors = true;
}


void UPlayerPeripheriesComponent::InitPeripheryInformation()
{
	// Initialize the periphery
	if (PeripheryRadius && ActivatePeripheryLogic(ActivationPhase))
	{
		PeripheryRadius->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::OnEnterRadiusPeriphery);
		PeripheryRadius->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::OnExitRadiusPeriphery);
		ConfigurePeripheryCollision(PeripheryRadius, bRadius);
	}

	if (ItemDetection && ActivatePeripheryLogic(ActivationPhase))
	{
		ItemDetection->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::OnEnterItemDetection);
		ItemDetection->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::OnExitItemDetection);
		ConfigurePeripheryCollision(ItemDetection, bItemDetection);
	}

	// The cone, the cone of shame!
	if (PeripheryCone && ActivatePeripheryLogic(ActivationPhase))
	{
		PeripheryCone->OnComponentBeginOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::OnEnterConePeriphery);
		PeripheryCone->OnComponentEndOverlap.AddDynamic(this, &UPlayerPeripheriesComponent::OnExitConePeriphery);
		ConfigurePeripheryCollision(PeripheryCone, bCone);
	}
}


void UPlayerPeripheriesComponent::BeginPlay()
{
	Super::BeginPlay();
	GetCharacter();

	if (GetOwner() && TraceShouldIgnoreOwnerActors)
	{
		IgnoredActors.AddUnique(GetOwner());
		GetOwner()->GetAllChildActors(IgnoredActors);
	}

	if (bInitPeripheryDuringBeginPlay) InitPeripheryInformation();
}


void UPlayerPeripheriesComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
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
	
	if (bTrace && ActivatePeripheryLogic(ActivationPhase))
	{
		HandlePeripheryLineTrace();
	}
}




#pragma region Periphery functions
void UPlayerPeripheriesComponent::PeripheryLineTrace_Implementation(FHitResult& Result)
{
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

	AimLocation = AimLocation + (AimForwardVector * PeripheryTraceForwardOffset);
	const FVector_NetQuantize AimDirection = AimLocation + (AimForwardVector * PeripheryTraceDistance); // This calculation is an fvector from our crosshair outwards
	const FVector StartLocation = AimLocation;
	
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(), StartLocation, AimDirection, PeripheryLineTraceType, false,  IgnoredActors,
		bDrawTraceDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, Result, true, TraceColor, TraceHitColor, TraceDuration
	);
}


void UPlayerPeripheriesComponent::HandlePeripheryLineTrace_Implementation()
{
	GetCharacter();

	FHitResult TraceResult;
	PeripheryLineTrace(TraceResult);

	// Periphery logic
	TracedActor = TraceResult.GetActor();
	const bool bIsTraceValidPeripheryObject = IsValidTracedObject(TracedActor, TraceResult);
	
	// Only activate the enter overlap logic once (this also handles if they aren't already aiming at something, and still aren't)
	if (TracedActor == PreviousTracedActor) return;
	
	// if the player isn't already aiming at anything
	if (!PreviousTracedActor)
	{
		if (TracedActor && bIsTraceValidPeripheryObject)
		{
			// If this is a periphery object with custom logic, activate the functions
			const bool bPeripheryInterface = TracedActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
			if (bPeripheryInterface) IPeripheryObjectInterface::Execute_WithinPlayerTracePeriphery(TracedActor, Player, FindPeripheryType(TracedActor));

			// Periphery Trace delegates
			ObjectInPeripheryTrace.Broadcast(TracedActor, Player, TraceResult);

			if (bDebugPeripheryTrace)
			{
				if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Started looking at {2}(PeripheryInt)", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetNameSafe(GetOwner()), *GetNameSafe(TracedActor));
				else UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Started looking at {2}", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetNameSafe(GetOwner()), *GetNameSafe(TracedActor));
			}
		}
	}
	
	// Transition to aiming at another object, or transition out of aiming at an object
	else if (TracedActor)
	{
		const bool bPeripheryInterface = TracedActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
		const bool bPreviousActorPeripheryInterface = PreviousTracedActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
		
		if (bIsPreviousTraceValidPeripheryObject)
		{
			// If this is a periphery object with custom logic, activate the functions
			if (bPreviousActorPeripheryInterface) IPeripheryObjectInterface::Execute_OutsideOfPlayerTracePeriphery(PreviousTracedActor, Player, FindPeripheryType(PreviousTracedActor));

			// Periphery Trace delegates
			ObjectOutsideOfPeripheryTrace.Broadcast(PreviousTracedActor, Player, TraceResult);
		} 

		if (bIsTraceValidPeripheryObject)
		{
			// If this is a periphery object with custom logic, activate the functions
			if (bPeripheryInterface) IPeripheryObjectInterface::Execute_WithinPlayerTracePeriphery(TracedActor, Player, FindPeripheryType(TracedActor));

			// Periphery Trace delegates
			ObjectInPeripheryTrace.Broadcast(TracedActor, Player, TraceResult);
		}
		
		if (bDebugPeripheryTrace)
		{
			UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Transitioned looking at {2}{3} to {4}{5}", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetNameSafe(GetOwner()),
				*GetNameSafe(PreviousTracedActor),
				bPreviousActorPeripheryInterface ? "(PeripheryInt)" : "",
				*GetNameSafe(TracedActor),
				bPeripheryInterface ? "(PeripheryInt)" : ""
			);
		}
	}
	else
	{
		const bool bPreviousActorPeripheryInterface = PreviousTracedActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
		if (bIsPreviousTraceValidPeripheryObject)
		{
			// If this is a periphery object with custom logic, activate the functions
			if (bPreviousActorPeripheryInterface) IPeripheryObjectInterface::Execute_OutsideOfPlayerTracePeriphery(PreviousTracedActor, Player, FindPeripheryType(PreviousTracedActor));
			
			// Periphery Trace delegates
			ObjectOutsideOfPeripheryTrace.Broadcast(PreviousTracedActor, Player, TraceResult);
		}
		
		if (bDebugPeripheryTrace)
		{
			if (bPreviousActorPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Stopped looking at {2}(PeripheryInt)", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetNameSafe(GetOwner()), *GetNameSafe(PreviousTracedActor));
			else UE_LOGFMT(PeripheryLog, Log, "{0}: {1} Stopped looking at {2}", *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *GetNameSafe(GetOwner()), *GetNameSafe(PreviousTracedActor));
		}
	}
	
	bIsPreviousTraceValidPeripheryObject = bIsTraceValidPeripheryObject;
	PreviousTracedActor = TracedActor; // Cached for on exit traces
}


void UPlayerPeripheriesComponent::OnEnterRadiusPeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetCharacter() || !OtherActor) return;
	if (OtherActor == Player) return;

	if (IsValidObjectInRadius(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult))
	{
		// If this is a periphery object with custom logic, activate the functions
		const bool bPeripheryInterface = OtherActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
		if (bPeripheryInterface) IPeripheryObjectInterface::Execute_WithinPlayerRadiusPeriphery(OtherActor, Player, FindPeripheryType(OtherActor));
		
		// Player logic
		ObjectInPlayerRadius.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
		
		if (bDebugPeripheryRadius)
		{
			if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: Entering Radius Periphery, {1} overlapped with {2}(PeripheryInt)", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
			else UE_LOGFMT(PeripheryLog, Verbose, "{0}: Entering Radius Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		}
	}
}


void UPlayerPeripheriesComponent::OnExitRadiusPeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetCharacter() || !OtherActor) return;
	if (OtherActor == Player) return;

	if (IsValidObjectInRadius(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex))
	{
		// If this is a periphery object with custom logic, activate the functions
		const bool bPeripheryInterface = OtherActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
		if (bPeripheryInterface) IPeripheryObjectInterface::Execute_OutsideOfPlayerRadiusPeriphery(OtherActor, Player, FindPeripheryType(OtherActor));
		
		// Player logic
		ObjectOutsideOfPlayerRadius.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex);
		
		if (bDebugPeripheryRadius)
		{
			if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: Exiting Radius Periphery, {1} overlapped with {2}(PeripheryInt)", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
			UE_LOGFMT(PeripheryLog, Verbose, "{0}: Exiting Radius Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		}
	}
}


void UPlayerPeripheriesComponent::OnEnterConePeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetCharacter() || !OtherActor) return;
	if (OtherActor == Player) return;

	if (IsValidObjectInCone(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult))
	{
		// If this is a periphery object with custom logic, activate the functions
		const bool bPeripheryInterface = OtherActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
		if (bPeripheryInterface) IPeripheryObjectInterface::Execute_WithinPlayerConePeriphery(OtherActor, Player, FindPeripheryType(OtherActor));
		
		// Player logic
		ObjectInPeripheryCone.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
		if (bDebugPeripheryCone)
		{
			if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: Entering Cone Periphery, {1} overlapped with {2}(PeripheryInt)", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
			else UE_LOGFMT(PeripheryLog, Verbose, "{0}: Entering Cone Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		}
	}
}


void UPlayerPeripheriesComponent::OnExitConePeriphery(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetCharacter() || !OtherActor) return;
	if (OtherActor == Player) return;

	if (IsValidObjectInCone(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex))
	{
		// If this is a periphery object with custom logic, activate the functions
		const bool bPeripheryInterface = OtherActor->GetClass()->ImplementsInterface(UPeripheryObjectInterface::StaticClass());
		if (bPeripheryInterface) IPeripheryObjectInterface::Execute_OutsideOfConePeriphery(OtherActor, Player, FindPeripheryType(OtherActor)); 
		
		// Player logic
		ObjectOutsideOfPeripheryCone.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex);

		if (bDebugPeripheryCone)
		{
			if (bPeripheryInterface) UE_LOGFMT(PeripheryLog, Log, "{0}: Exiting Cone Periphery, {1} overlapped with {2}(PeripheryInt)", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
			else UE_LOGFMT(PeripheryLog, Verbose, "{0}: Exiting Cone Periphery, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		}
	}
}


void UPlayerPeripheriesComponent::OnEnterItemDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetCharacter() || !OtherActor) return;
	if (OtherActor == Player) return;

	if (IsValidItemDetected(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult))
	{
		// Player logic
		OnItemOverlapBegin.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
		
		if (bDebugItemDetection)
		{
			UE_LOGFMT(PeripheryLog, Log, "{0}: Item detected, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		}
	}
}


void UPlayerPeripheriesComponent::OnExitItemDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetCharacter() || !OtherActor) return;
	if (OtherActor == Player) return;

	if (IsValidItemDetected(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex))
	{
		// Player logic
		OnItemOverlapEnd.Broadcast(OtherActor, OverlappedComponent, OtherComp, OtherBodyIndex);
		
		if (bDebugItemDetection)
		{
			UE_LOGFMT(PeripheryLog, Log, "{0}: Item undetected, {1} overlapped with {2}", *UEnum::GetValueAsString(Player->GetLocalRole()), *GetNameSafe(Player), *GetNameSafe(OtherActor));
		}
	}
}


bool UPlayerPeripheriesComponent::IsValidObjectInRadius_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return false;
	return OtherActor->GetClass()->IsChildOf(ValidPeripheryRadiusObjects);
}

bool UPlayerPeripheriesComponent::IsValidTracedObject_Implementation(AActor* OtherActor, const FHitResult& HitResult)
{
	if (!OtherActor) return false;
	return OtherActor->GetClass()->IsChildOf(ValidPeripheryTraceObjects);
}

bool UPlayerPeripheriesComponent::IsValidObjectInCone_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return false;
	return OtherActor->GetClass()->IsChildOf(ValidPeripheryRadiusObjects);
}

bool UPlayerPeripheriesComponent::IsValidItemDetected_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return false;
	return OtherActor->GetClass()->IsChildOf(ValidPeripheryRadiusObjects);
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
		UE_LOGFMT(PeripheryLog, Error, "{0}:{1}() ->  Error while trying to find the owner of the periphery component", *FString(__FUNCTION__), *GetName());
		return false;
	}

	return true;
	
}

bool UPlayerPeripheriesComponent::ActivatePeripheryLogic(const EHandlePeripheryLogic HandlePeripheryLogic) const
{
	if (EHandlePeripheryLogic::EP_ServerAndClient == HandlePeripheryLogic) return true;
	if (EHandlePeripheryLogic::EP_Server == HandlePeripheryLogic && ROLE_Authority == GetOwner()->GetLocalRole()) return true;
	if (EHandlePeripheryLogic::EP_Client == HandlePeripheryLogic && ROLE_AutonomousProxy == GetOwner()->GetLocalRole()) return true;
	if (EHandlePeripheryLogic::EP_Client == HandlePeripheryLogic && ROLE_Authority == GetOwner()->GetLocalRole() && Player && Player->IsLocallyControlled()) return true;
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

bool UPlayerPeripheriesComponent::IsPlayingInEditor(UObject* WorldContextObject) const
{
	return WorldContextObject->GetWorld()->IsEditorWorld();
}

bool UPlayerPeripheriesComponent::IsPlayingInGame(UObject* WorldContextObject) const
{
	return WorldContextObject->GetWorld()->IsGameWorld();
}
