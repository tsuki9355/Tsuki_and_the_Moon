#include "Player/GrabComponent.h"
#include "Properties/PropertyHolderComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

UGrabComponent::UGrabComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // After the character has moved this frame.
}

ACharacter* UGrabComponent::GetCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

UPropertyHolderComponent* UGrabComponent::FindGrabbable(FVector& OutNormal) const
{
	const ACharacter* Character = GetCharacter();
	if (!Character)
	{
		return nullptr;
	}
	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const FVector Forward = Character->GetActorForwardVector() * (Capsule->GetScaledCapsuleRadius() + GrabReach);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GrabTrace), false, Character);
	if (const UPropertyHolderComponent* Held = Carried.Get())
	{
		Params.AddIgnoredActor(Held->GetOwner());
	}

	// Chest height for blocks, then knee height so small objects like the pebble can be found.
	const float Heights[] = { 0.f, -(Capsule->GetScaledCapsuleHalfHeight() - 25.f) };
	for (const float Z : Heights)
	{
		const FVector Start = Character->GetActorLocation() + FVector(0.f, 0.f, Z);
		FHitResult Hit;
		if (!GetWorld()->SweepSingleByChannel(Hit, Start, Start + Forward, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(20.f), Params))
		{
			continue;
		}
		UPropertyHolderComponent* Holder = Hit.GetActor() ? Hit.GetActor()->FindComponentByClass<UPropertyHolderComponent>() : nullptr;
		if (Holder && (Holder->bPushable || Holder->bCarryable))
		{
			OutNormal = Hit.ImpactNormal;
			return Holder;
		}
	}
	return nullptr;
}

EGrabPrompt UGrabComponent::GetPrompt() const
{
	if (IsCarrying())
	{
		return EGrabPrompt::PutDown;
	}
	if (IsGrabbing())
	{
		return EGrabPrompt::Pushing;
	}
	FVector Normal;
	const UPropertyHolderComponent* Holder = FindGrabbable(Normal);
	if (!Holder)
	{
		return EGrabPrompt::None;
	}
	const float Weight = Holder->GetEffectiveWeight();
	if (Holder->bCarryable)
	{
		return Weight <= MaxCarryWeight ? EGrabPrompt::PickUp : EGrabPrompt::TooHeavy;
	}
	return Weight <= MaxGrabWeight ? EGrabPrompt::Grab : EGrabPrompt::TooHeavy;
}

bool UGrabComponent::TryGrab()
{
	ACharacter* Character = GetCharacter();
	if (!Character)
	{
		return false;
	}
	if (IsCarrying())
	{
		return PutDown();
	}
	if (IsGrabbing() || !Character->GetCharacterMovement()->IsMovingOnGround())
	{
		return false;
	}
	FVector Normal;
	UPropertyHolderComponent* Holder = FindGrabbable(Normal);
	if (!Holder)
	{
		return false;
	}
	return Holder->bCarryable ? StartCarry(Holder) : StartPush(Holder, Normal);
}

bool UGrabComponent::StartPush(UPropertyHolderComponent* Holder, const FVector& Normal)
{
	ACharacter* Character = GetCharacter();
	if (Holder->GetEffectiveWeight() > MaxGrabWeight)
	{
		OnGrabRefused.Broadcast(Holder);
		return false;
	}

	// Face the grabbed face squarely and lock movement to that axis.
	FVector Axis = -Normal;
	Axis.Z = 0.f;
	if (!Axis.Normalize())
	{
		Axis = Character->GetActorForwardVector();
	}
	GrabAxis = Axis;
	Character->SetActorRotation(GrabAxis.Rotation());

	UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	SavedMaxWalkSpeed = Move->MaxWalkSpeed;
	bSavedOrientToMovement = Move->bOrientRotationToMovement;
	Move->MaxWalkSpeed = PushSpeed;
	Move->bOrientRotationToMovement = false;

	// Let Tsuki and the object pass through each other; the object is moved by sweep in Tick.
	AActor* Object = Holder->GetOwner();
	Character->GetCapsuleComponent()->IgnoreActorWhenMoving(Object, true);
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Object->GetRootComponent()))
	{
		Root->IgnoreActorWhenMoving(Character, true);
	}

	Grabbed = Holder;
	LastOwnerLocation = Character->GetActorLocation();
	OnGrabStarted.Broadcast(Holder);
	return true;
}

bool UGrabComponent::StartCarry(UPropertyHolderComponent* Holder)
{
	ACharacter* Character = GetCharacter();
	if (Holder->GetEffectiveWeight() > MaxCarryWeight)
	{
		OnGrabRefused.Broadcast(Holder);
		return false;
	}
	AActor* Object = Holder->GetOwner();
	UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Object->GetRootComponent());
	if (!Root)
	{
		return false;
	}

	CarriedSavedCollision = Root->GetCollisionEnabled();
	Root->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Holder->bHeldByPlayer = true;

	// The object stays put until her hands reach it in the pick-up animation (see TickCarry).
	Character->GetCharacterMovement()->StopMovementImmediately();
	FVector Flat = Object->GetActorLocation() - Character->GetActorLocation();
	Flat.Z = 0.f;
	if (!Flat.IsNearlyZero())
	{
		Character->SetActorRotation(Flat.Rotation());
	}

	UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	SavedMaxWalkSpeed = Move->MaxWalkSpeed;
	Move->MaxWalkSpeed = SavedMaxWalkSpeed * CarrySpeedScale;

	Carried = Holder;
	CarryPhase = ECarryPhase::Lifting;
	PhaseTime = 0.f;
	PhaseFrom = Root->GetComponentLocation();
	OnPickedUp.Broadcast(Holder);
	return true;
}

FVector UGrabComponent::GetHoldPivot(const UPrimitiveComponent* Root) const
{
	const ACharacter* Character = GetCharacter();
	const USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
	const FVector PivotToCenter = Root->Bounds.Origin - Root->GetComponentLocation();
	static const FName LeftHand(TEXT("LeftHand"));
	static const FName RightHand(TEXT("RightHand"));
	if (Mesh && Mesh->DoesSocketExist(LeftHand) && Mesh->DoesSocketExist(RightHand))
	{
		// Object centered between her hands, so it moves with every animation.
		const FVector Mid = (Mesh->GetSocketLocation(LeftHand) + Mesh->GetSocketLocation(RightHand)) * 0.5f;
		return Mid - PivotToCenter;
	}
	// Fallback (no hand bones): just above her head.
	const FBoxSphereBounds Bounds = Root->Bounds;
	const float PivotAboveBottom = Root->GetComponentLocation().Z - (Bounds.Origin.Z - Bounds.BoxExtent.Z);
	return Character->GetActorLocation()
		+ FVector(0.f, 0.f, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + CarryHeightOffset + PivotAboveBottom);
}

void UGrabComponent::TickCarry(float DeltaTime)
{
	UPropertyHolderComponent* Holder = Carried.Get();
	UPrimitiveComponent* Root = Holder ? Cast<UPrimitiveComponent>(Holder->GetOwner()->GetRootComponent()) : nullptr;
	if (!Root || CarryPhase == ECarryPhase::None)
	{
		return;
	}
	PhaseTime += DeltaTime;
	const FVector Hold = GetHoldPivot(Root);
	const float Blend = FMath::Max(HandBlendTime, 0.01f);

	switch (CarryPhase)
	{
	case ECarryPhase::Lifting:
		if (PhaseTime >= LiftGrabTime)
		{
			const float A = FMath::Clamp((PhaseTime - LiftGrabTime) / Blend, 0.f, 1.f);
			Root->SetWorldLocation(FMath::Lerp(PhaseFrom, Hold, FMath::InterpEaseInOut(0.f, 1.f, A, 2.f)));
			if (A >= 1.f)
			{
				CarryPhase = ECarryPhase::Holding;
			}
		}
		break;
	case ECarryPhase::Holding:
		Root->SetWorldLocation(Hold);
		break;
	case ECarryPhase::Lowering:
		if (PhaseTime < LowerReleaseTime)
		{
			Root->SetWorldLocation(Hold);
			PhaseFrom = Hold;
		}
		else
		{
			const float A = FMath::Clamp((PhaseTime - LowerReleaseTime) / Blend, 0.f, 1.f);
			Root->SetWorldLocation(FMath::Lerp(PhaseFrom, PutTarget, FMath::InterpEaseInOut(0.f, 1.f, A, 2.f)));
			if (A >= 1.f)
			{
				FinishPutDown();
			}
		}
		break;
	case ECarryPhase::Throwing:
		Root->SetWorldLocation(Hold);
		if (PhaseTime >= ThrowReleaseTime)
		{
			ReleaseThrow();
		}
		break;
	default:
		break;
	}
}

bool UGrabComponent::FindPutDownLocation(FVector& OutLocation) const
{
	const ACharacter* Character = GetCharacter();
	const UPropertyHolderComponent* Holder = Carried.Get();
	const UPrimitiveComponent* Root = Holder ? Cast<UPrimitiveComponent>(Holder->GetOwner()->GetRootComponent()) : nullptr;
	if (!Character || !Root)
	{
		return false;
	}
	const FBoxSphereBounds Bounds = Root->Bounds;
	const float PivotAboveBottom = Root->GetComponentLocation().Z - (Bounds.Origin.Z - Bounds.BoxExtent.Z);
	const float ObjectRadius = FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y);
	const FVector Ahead = Character->GetActorLocation()
		+ Character->GetActorForwardVector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + ObjectRadius + 15.f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PutDownTrace), false, Character);
	Params.AddIgnoredActor(Holder->GetOwner());
	FHitResult Ground;
	if (!GetWorld()->LineTraceSingleByChannel(Ground, Ahead + FVector(0.f, 0.f, 100.f), Ahead - FVector(0.f, 0.f, 300.f), ECC_Visibility, Params))
	{
		return false; // No floor in front (ledge): keep holding.
	}
	const FVector Target(Ahead.X, Ahead.Y, Ground.ImpactPoint.Z + PivotAboveBottom + 1.f);

	// Room check, slightly shrunk so resting on the floor does not count as blocked.
	const FVector Center = Target + (Bounds.Origin - Root->GetComponentLocation());
	const FCollisionShape Box = FCollisionShape::MakeBox(Bounds.BoxExtent * 0.9f);
	if (GetWorld()->OverlapBlockingTestByChannel(Center + FVector(0.f, 0.f, 2.f), FQuat::Identity, ECC_WorldDynamic, Box, Params))
	{
		return false;
	}
	OutLocation = Target;
	return true;
}

bool UGrabComponent::PutDown()
{
	UPropertyHolderComponent* Holder = Carried.Get();
	ACharacter* Character = GetCharacter();
	if (!Holder || !Character || CarryPhase != ECarryPhase::Holding)
	{
		return false;
	}
	FVector Target;
	if (!FindPutDownLocation(Target))
	{
		return false;
	}
	// Lower with the animation; FinishPutDown() runs once it reaches the ground.
	Character->GetCharacterMovement()->StopMovementImmediately();
	PutTarget = Target;
	CarryPhase = ECarryPhase::Lowering;
	PhaseTime = 0.f;
	return true;
}

void UGrabComponent::FinishPutDown()
{
	UPropertyHolderComponent* Holder = Carried.Get();
	ACharacter* Character = GetCharacter();
	if (!Holder)
	{
		CarryPhase = ECarryPhase::None;
		return;
	}
	AActor* Object = Holder->GetOwner();
	Object->SetActorLocation(PutTarget);
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Object->GetRootComponent()))
	{
		Root->SetCollisionEnabled(CarriedSavedCollision);
		Root->UpdateOverlaps();
	}
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = SavedMaxWalkSpeed;
	}
	Holder->bHeldByPlayer = false;
	Carried = nullptr;
	CarryPhase = ECarryPhase::None;
	OnPutDown.Broadcast(Holder);
}

bool UGrabComponent::Throw()
{
	if (!Carried.IsValid() || CarryPhase != ECarryPhase::Holding)
	{
		return false;
	}
	// Released partway into the throw animation (ReleaseThrow).
	CarryPhase = ECarryPhase::Throwing;
	PhaseTime = 0.f;
	return true;
}

void UGrabComponent::ReleaseThrow()
{
	UPropertyHolderComponent* Holder = Carried.Get();
	ACharacter* Character = GetCharacter();
	UPrimitiveComponent* Root = Holder ? Cast<UPrimitiveComponent>(Holder->GetOwner()->GetRootComponent()) : nullptr;
	CarryPhase = ECarryPhase::None;
	if (!Root || !Character)
	{
		Carried = nullptr;
		return;
	}
	AActor* Object = Holder->GetOwner();
	Holder->bHeldByPlayer = false;
	Root->SetCollisionEnabled(CarriedSavedCollision);
	Root->IgnoreActorWhenMoving(Character, true);
	Character->GetCapsuleComponent()->IgnoreActorWhenMoving(Object, true);
	Root->SetSimulatePhysics(true);
	Root->SetPhysicsLinearVelocity(Character->GetActorForwardVector() * ThrowForward + FVector(0.f, 0.f, ThrowUp) + Character->GetVelocity() * 0.5f);
	Character->GetCharacterMovement()->MaxWalkSpeed = SavedMaxWalkSpeed;
	Carried = nullptr;
	Thrown = Root;
	ThrownTime = 0.f;
	OnPutDown.Broadcast(Holder);
}

void UGrabComponent::ForceReleaseAll()
{
	Release();
	if (UPropertyHolderComponent* Holder = Carried.Get())
	{
		// Immediate drop (respawn etc.): in front of her if there is room, else at her feet.
		FVector Target;
		if (!FindPutDownLocation(Target))
		{
			Target = GetCharacter() ? GetCharacter()->GetActorLocation() : Holder->GetOwner()->GetActorLocation();
		}
		PutTarget = Target;
		FinishPutDown();
	}
}

void UGrabComponent::Release()
{
	UPropertyHolderComponent* Holder = Grabbed.Get();
	ACharacter* Character = GetCharacter();
	if (!Holder)
	{
		Grabbed = nullptr;
		return;
	}
	Grabbed = nullptr;
	if (Character)
	{
		UCharacterMovementComponent* Move = Character->GetCharacterMovement();
		Move->MaxWalkSpeed = SavedMaxWalkSpeed > 0.f ? SavedMaxWalkSpeed : Move->MaxWalkSpeed;
		Move->bOrientRotationToMovement = bSavedOrientToMovement;
		if (Holder->GetOwner())
		{
			Character->GetCapsuleComponent()->IgnoreActorWhenMoving(Holder->GetOwner(), false);
			if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Holder->GetOwner()->GetRootComponent()))
			{
				Root->IgnoreActorWhenMoving(Character, false);
			}
		}
	}
	OnGrabReleased.Broadcast(Holder);
}

void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickCarry(DeltaTime);

	// A thrown object flies with physics, then is frozen where it comes to rest so it behaves like
	// any other placed object (plates, ponds, push/pull).
	if (UPrimitiveComponent* T = Thrown.Get())
	{
		ThrownTime += DeltaTime;
		if ((ThrownTime > 0.4f && T->GetPhysicsLinearVelocity().SizeSquared() < 30.f * 30.f) || ThrownTime > 4.f)
		{
			T->SetSimulatePhysics(false);
			T->SetWorldRotation(FRotator(0.f, T->GetComponentRotation().Yaw, 0.f));
			if (ACharacter* Character = GetCharacter())
			{
				T->IgnoreActorWhenMoving(Character, false);
				Character->GetCapsuleComponent()->IgnoreActorWhenMoving(T->GetOwner(), false);
			}
			T->UpdateOverlaps();
			Thrown = nullptr;
		}
	}

	UPropertyHolderComponent* Holder = Grabbed.Get();
	ACharacter* Character = GetCharacter();
	if (!Holder || !Character)
	{
		if (Grabbed.IsStale())
		{
			Grabbed = nullptr;
		}
		return;
	}

	// Losing Lightness mid-push (or falling) ends the grab.
	if (Holder->GetEffectiveWeight() > MaxGrabWeight || !Character->GetCharacterMovement()->IsMovingOnGround())
	{
		Release();
		OnGrabRefused.Broadcast(Holder);
		return;
	}

	const FVector Now = Character->GetActorLocation();
	const float AlongAxis = FVector::DotProduct(Now - LastOwnerLocation, GrabAxis);
	if (!FMath::IsNearlyZero(AlongAxis))
	{
		USceneComponent* Root = Holder->GetOwner()->GetRootComponent();
		FHitResult Hit;
		Root->MoveComponent(GrabAxis * AlongAxis, Root->GetComponentQuat(), true, &Hit);
		if (Hit.bBlockingHit)
		{
			// Object is stuck: hold Tsuki back by the part of the move the object could not make.
			const float Blocked = AlongAxis * (1.f - Hit.Time);
			Character->SetActorLocation(Now - GrabAxis * Blocked, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
	LastOwnerLocation = Character->GetActorLocation();
}
