#include "World/MovingPlatform.h"
#include "Properties/PropertyHolderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics; // Move before characters standing on it.
}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();
	Start = GetActorLocation();
}

void AMovingPlatform::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); // DeltaSeconds already includes Slowness (CustomTimeDilation).
	if (PauseLeft > 0.f)
	{
		PauseLeft -= DeltaSeconds;
		return;
	}
	const float Length = FMath::Max(EndOffset.Size(), 1.f);
	Alpha += Direction * (Speed * DeltaSeconds / Length);
	if (Alpha >= 1.f || Alpha <= 0.f)
	{
		Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
		Direction = -Direction;
		PauseLeft = PauseTime;
	}
	SetActorLocation(Start + EndOffset * Alpha);

	// Too fast to ride: without Slowness the platform slides out from under Tsuki instead of carrying her.
	// Clearing her movement base after each platform move means the movement component never applies
	// the platform's motion to her; she keeps her spot and drops once the platform has passed.
	if (Holder && Holder->GetTimeScale() >= 1.f)
	{
		for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
		{
			UCharacterMovementComponent* Move = It->GetCharacterMovement();
			if (Move && Move->IsMovingOnGround() && Move->CurrentFloor.HitResult.GetComponent() == Mesh)
			{
				Move->SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
			}
		}
	}
}
