#include "Player/HookComponent.h"
#include "World/HookTarget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UHookComponent::UHookComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UHookComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* Owner = GetOwner())
	{
		Thread = NewObject<UStaticMeshComponent>(Owner, TEXT("HookThread"));
		Thread->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
		Thread->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Thread->SetCastShadow(false);
		Thread->SetUsingAbsoluteLocation(true);
		Thread->SetUsingAbsoluteRotation(true);
		Thread->SetUsingAbsoluteScale(true);
		Thread->SetupAttachment(Owner->GetRootComponent());
		Thread->RegisterComponent();
		Thread->SetVisibility(false);
	}
}

bool UHookComponent::GetAimRay(FVector& OutStart, FVector& OutDir) const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const AController* Controller = Character ? Character->GetController() : nullptr;
	if (!Controller)
	{
		return false;
	}
	FVector ViewLoc;
	FRotator ViewRot;
	Controller->GetPlayerViewPoint(ViewLoc, ViewRot);
	OutDir = ViewRot.Vector();
	// Start level with Tsuki along the view ray so nothing behind her is picked.
	const float Ahead = FMath::Max(0.f, FVector::DotProduct(Character->GetActorLocation() - ViewLoc, OutDir));
	OutStart = ViewLoc + OutDir * Ahead;
	return true;
}

AHookTarget* UHookComponent::FindTarget() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	FVector RayStart, RayDir;
	if (!Character || !GetAimRay(RayStart, RayDir))
	{
		return nullptr;
	}
	const FVector From = Character->GetActorLocation();

	// Precise: the crosshair ray must pass within AimTolerance of the marker.
	AHookTarget* Best = nullptr;
	float BestAlong = TNumericLimits<float>::Max();
	for (TActorIterator<AHookTarget> It(GetWorld()); It; ++It)
	{
		AHookTarget* Target = *It;
		if (!Target->bHookEnabled)
		{
			continue;
		}
		const FVector Anchor = Target->GetAnchorLocation();
		if (FVector::DistSquared(From, Anchor) > FMath::Square(Range))
		{
			continue;
		}
		const float Along = FVector::DotProduct(Anchor - RayStart, RayDir);
		if (Along <= 0.f || Along >= BestAlong)
		{
			continue;
		}
		const FVector Closest = RayStart + RayDir * Along;
		if (FVector::DistSquared(Closest, Anchor) > FMath::Square(AimTolerance))
		{
			continue;
		}
		FCollisionQueryParams Params(SCENE_QUERY_STAT(HookSight), false, Character);
		Params.AddIgnoredActor(Target);
		const FVector Dir = (Anchor - From).GetSafeNormal();
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, From, Anchor - Dir * 40.f, ECC_Visibility, Params))
		{
			continue;
		}
		Best = Target;
		BestAlong = Along;
	}
	return Best;
}

void UHookComponent::UpdateAim()
{
	AHookTarget* NewTarget = (bHookEnabled && bAiming && State == EState::Idle) ? FindTarget() : nullptr;
	if (NewTarget != AimedTarget.Get())
	{
		if (AHookTarget* Old = AimedTarget.Get())
		{
			Old->SetAimed(false);
		}
		if (NewTarget)
		{
			NewTarget->SetAimed(true);
		}
		AimedTarget = NewTarget;
	}
}

void UHookComponent::StartAiming()
{
	if (bHookEnabled && State == EState::Idle)
	{
		bAiming = true;
	}
}

void UHookComponent::ReleaseAim()
{
	if (!bAiming)
	{
		return;
	}
	UpdateAim();
	Fire();
	bAiming = false;
	UpdateAim();
}

bool UHookComponent::Fire()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!bHookEnabled || State != EState::Idle || !Character)
	{
		return false;
	}
	FVector RayStart, RayDir;
	if (!GetAimRay(RayStart, RayDir))
	{
		return false;
	}

	// Face the shot.
	FVector Flat = RayDir;
	Flat.Z = 0.f;
	if (!Flat.IsNearlyZero())
	{
		Character->SetActorRotation(Flat.Rotation());
	}
	TipLocation = Character->GetActorLocation();
	StateTime = 0.f;
	Thread->SetVisibility(true);
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	if (AHookTarget* Target = AimedTarget.Get())
	{
		ActiveTarget = Target;
		Target->SetAimed(false);
		AimedTarget = nullptr;
		State = EState::Firing;
		return true;
	}

	// Miss: fly straight along the crosshair to whatever it hits (or full range), then reel back.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(HookWhiff), false, Character);
	FHitResult Hit;
	const FVector End = RayStart + RayDir * Range;
	WhiffEnd = GetWorld()->LineTraceSingleByChannel(Hit, RayStart, End, ECC_Visibility, Params) ? Hit.ImpactPoint : End;
	ActiveTarget = nullptr;
	State = EState::Whiff;
	return false;
}

void UHookComponent::Cancel()
{
	bAiming = false;
	if (State == EState::Idle)
	{
		return;
	}
	if (State == EState::Pulling || State == EState::Mantling)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}
	Finish();
}

void UHookComponent::Finish()
{
	State = EState::Idle;
	ActiveTarget = nullptr;
	if (Thread)
	{
		Thread->SetVisibility(false);
	}
}

void UHookComponent::UpdateThread(const FVector& Tip)
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Thread || !Character)
	{
		return;
	}
	static const FName HandBone(TEXT("RightHand"));
	const USkeletalMeshComponent* SkMesh = Character->GetMesh();
	const FVector Hand = (SkMesh && SkMesh->DoesSocketExist(HandBone))
		? SkMesh->GetSocketLocation(HandBone)
		: Character->GetActorLocation() + Character->GetActorForwardVector() * 30.f + FVector(0.f, 0.f, 30.f);
	const FVector Span = Tip - Hand;
	const float Length = Span.Size();
	if (Length < 1.f)
	{
		Thread->SetVisibility(false);
		return;
	}
	Thread->SetVisibility(true);
	// The engine cylinder is 100 cm tall along Z: stretch it between hand and tip.
	Thread->SetWorldLocation((Hand + Tip) * 0.5f);
	Thread->SetWorldRotation(FRotationMatrix::MakeFromZ(Span / Length).Rotator());
	Thread->SetWorldScale3D(FVector(0.04f, 0.04f, Length / 100.f));
}

void UHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateAim();

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (State == EState::Idle || !Character)
	{
		return;
	}
	StateTime += DeltaTime;

	// Misses: out along the crosshair, then back to the hand.
	if (State == EState::Whiff || State == EState::Retract)
	{
		const FVector Hand = Character->GetActorLocation() + Character->GetActorForwardVector() * 30.f + FVector(0.f, 0.f, 30.f);
		const FVector Goal = State == EState::Whiff ? WhiffEnd : Hand;
		TipLocation = FMath::VInterpConstantTo(TipLocation, Goal, DeltaTime, ShootSpeed);
		UpdateThread(TipLocation);
		if (TipLocation.Equals(Goal, 5.f))
		{
			if (State == EState::Whiff)
			{
				State = EState::Retract;
			}
			else
			{
				Finish();
			}
		}
		return;
	}

	AHookTarget* Target = ActiveTarget.Get();
	if (!Target)
	{
		Cancel();
		return;
	}
	UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	const float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Anchor = Target->GetAnchorLocation();

	switch (State)
	{
	case EState::Firing:
	{
		TipLocation = FMath::VInterpConstantTo(TipLocation, Anchor, DeltaTime, ShootSpeed);
		UpdateThread(TipLocation);
		if (TipLocation.Equals(Anchor, 1.f))
		{
			if (LatchSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, LatchSound, Anchor);
			}
			OnLatched.Broadcast(Target);
			State = EState::Pulling;
			StateTime = 0.f;
			StuckTime = 0.f;
			LastDistance = FVector::Dist(Character->GetActorLocation(), Anchor);
			Move->SetMovementMode(MOVE_Flying);
		}
		break;
	}
	case EState::Pulling:
	{
		UpdateThread(Anchor);
		const FVector ToAnchor = Anchor - Character->GetActorLocation();
		const float Dist = ToAnchor.Size();
		Move->Velocity = ToAnchor.GetSafeNormal() * PullSpeed;

		// Arrived (within reach of the anchor), or blocked by geometry: climb up onto the landing.
		StuckTime = (LastDistance - Dist < PullSpeed * DeltaTime * 0.2f) ? StuckTime + DeltaTime : 0.f;
		LastDistance = Dist;
		if (Dist < 90.f || StuckTime > 0.15f || StateTime > 3.f)
		{
			Move->Velocity = FVector::ZeroVector;
			Move->SetMovementMode(MOVE_Flying);
			MantleStart = Character->GetActorLocation();
			State = EState::Mantling;
			StateTime = 0.f;
			Thread->SetVisibility(false);
		}
		break;
	}
	case EState::Mantling:
	{
		const FVector End = Target->GetLandingLocation() + FVector(0.f, 0.f, HalfHeight + 2.f);
		const float Alpha = FMath::Clamp(StateTime / MantleTime, 0.f, 1.f);
		// Up first, then over: a small hop onto the ledge.
		const float Up = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);
		const FVector Pos(FMath::Lerp(MantleStart.X, End.X, Alpha), FMath::Lerp(MantleStart.Y, End.Y, Alpha), FMath::Lerp(MantleStart.Z, End.Z, Up));
		Character->SetActorLocation(Pos, false, nullptr, ETeleportType::TeleportPhysics);
		if (Alpha >= 1.f)
		{
			Move->Velocity = FVector::ZeroVector;
			Move->SetMovementMode(MOVE_Falling);
			OnArrived.Broadcast(Target);
			Finish();
		}
		break;
	}
	default:
		break;
	}
}
