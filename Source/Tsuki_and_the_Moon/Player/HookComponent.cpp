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

AHookTarget* UHookComponent::FindTarget() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const AController* Controller = Character ? Character->GetController() : nullptr;
	if (!Controller)
	{
		return nullptr;
	}
	FVector ViewLoc;
	FRotator ViewRot;
	Controller->GetPlayerViewPoint(ViewLoc, ViewRot);
	const FVector ViewDir = ViewRot.Vector();
	const FVector From = Character->GetActorLocation();
	const float CosCone = FMath::Cos(FMath::DegreesToRadians(AimConeDegrees));

	AHookTarget* Best = nullptr;
	float BestDot = CosCone;
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
		const float Dot = FVector::DotProduct((Anchor - ViewLoc).GetSafeNormal(), ViewDir);
		if (Dot <= BestDot)
		{
			continue;
		}
		// Line of sight from Tsuki to just short of the anchor.
		FCollisionQueryParams Params(SCENE_QUERY_STAT(HookSight), false, Character);
		Params.AddIgnoredActor(Target);
		const FVector Dir = (Anchor - From).GetSafeNormal();
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, From, Anchor - Dir * 40.f, ECC_Visibility, Params))
		{
			continue;
		}
		Best = Target;
		BestDot = Dot;
	}
	return Best;
}

void UHookComponent::UpdateAim()
{
	AHookTarget* NewTarget = (bHookEnabled && State == EState::Idle) ? FindTarget() : nullptr;
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

bool UHookComponent::Fire()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	AHookTarget* Target = AimedTarget.Get();
	if (!bHookEnabled || State != EState::Idle || !Character || !Target)
	{
		return false;
	}
	ActiveTarget = Target;
	Target->SetAimed(false);
	AimedTarget = nullptr;
	State = EState::Firing;
	StateTime = 0.f;
	TipLocation = Character->GetActorLocation();
	Thread->SetVisibility(true);
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
	// Face the target, Ocarina-style.
	FVector Flat = Target->GetAnchorLocation() - Character->GetActorLocation();
	Flat.Z = 0.f;
	if (!Flat.IsNearlyZero())
	{
		Character->SetActorRotation(Flat.Rotation());
	}
	return true;
}

void UHookComponent::Cancel()
{
	if (State == EState::Idle)
	{
		return;
	}
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
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
	const FVector Hand = Character->GetActorLocation() + Character->GetActorForwardVector() * 30.f + FVector(0.f, 0.f, 30.f);
	const FVector Span = Tip - Hand;
	const float Length = Span.Size();
	if (Length < 1.f)
	{
		Thread->SetVisibility(false);
		return;
	}
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
	AHookTarget* Target = ActiveTarget.Get();
	if (State == EState::Idle || !Character)
	{
		return;
	}
	if (!Target)
	{
		Cancel();
		return;
	}
	StateTime += DeltaTime;
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
