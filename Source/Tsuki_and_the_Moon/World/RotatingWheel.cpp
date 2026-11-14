#include "World/RotatingWheel.h"
#include "Properties/PropertyHolderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

ARotatingWheel::ARotatingWheel()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void ARotatingWheel::BeginPlay()
{
	Super::BeginPlay();
	UStaticMesh* GMesh = GondolaMesh ? GondolaMesh.Get() : LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* SpokeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));

	// One arm per gondola, running from the hub to the gondola it carries.
	for (int32 i = 0; i < GondolaCount; ++i)
	{
		UStaticMeshComponent* Spoke = NewObject<UStaticMeshComponent>(this);
		Spoke->SetStaticMesh(SpokeMesh);
		Spoke->SetMobility(EComponentMobility::Movable);
		Spoke->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Spoke->SetUsingAbsoluteLocation(true);
		Spoke->SetUsingAbsoluteRotation(true);
		Spoke->SetUsingAbsoluteScale(true);
		Spoke->SetupAttachment(Mesh);
		Spoke->RegisterComponent();
		Spoke->SetWorldScale3D(FVector(Radius / 100.f, 0.2f, 0.2f));
		Spokes.Add(Spoke);
	}

	for (int32 i = 0; i < GondolaCount; ++i)
	{
		UStaticMeshComponent* G = NewObject<UStaticMeshComponent>(this);
		G->SetStaticMesh(GMesh);
		G->SetMobility(EComponentMobility::Movable);
		G->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		G->SetUsingAbsoluteLocation(true);
		G->SetUsingAbsoluteRotation(true);
		G->SetUsingAbsoluteScale(true);
		G->SetupAttachment(Mesh);
		G->RegisterComponent();
		G->SetWorldScale3D(GondolaScale);
		G->SetWorldRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
		Gondolas.Add(G);
	}
	PlaceGondolas();
}

void ARotatingWheel::PlaceGondolas()
{
	const FVector Center = Mesh->GetComponentLocation();
	const FVector Right = FRotator(0.f, GetActorRotation().Yaw, 0.f).Vector(); // In-plane horizontal axis.
	const FVector Up = FVector::UpVector;
	const FVector Axis = FVector::CrossProduct(Right, Up); // Wheel axle, perpendicular to its plane.
	for (int32 i = 0; i < Gondolas.Num(); ++i)
	{
		// Start at the bottom (-90 deg) so a gondola is waiting at floor level.
		const float A = FMath::DegreesToRadians(Angle - 90.f + i * 360.f / Gondolas.Num());
		const FVector Dir = Right * FMath::Cos(A) + Up * FMath::Sin(A);
		Gondolas[i]->SetWorldLocation(Center + Dir * Radius);
		if (Spokes.IsValidIndex(i))
		{
			Spokes[i]->SetWorldLocationAndRotation(Center + Dir * (Radius * 0.5f), FRotationMatrix::MakeFromXZ(Dir, Axis).Rotator());
		}
	}
}

void ARotatingWheel::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); // Includes Slowness.
	const float Step = DegreesPerSecond * DeltaSeconds;
	Angle = FMath::Fmod(Angle + Step, 360.f);
	PlaceGondolas(); // Arms and gondolas; the hub (root) stays put so the actor's axes never change.
	KnockOffRiders();
}

void ARotatingWheel::KnockOffRiders()
{
	// Spinning too fast to ride: anyone landing on a gondola is thrown back out of the wheel.
	if (!Holder || Holder->GetTimeScale() < 1.f)
	{
		return;
	}
	const FVector Center = Mesh->GetComponentLocation();
	for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
	{
		const UCharacterMovementComponent* Move = It->GetCharacterMovement();
		UPrimitiveComponent* Base = Move && Move->IsMovingOnGround() ? Move->CurrentFloor.HitResult.GetComponent() : nullptr;
		if (!Base || !Gondolas.Contains(Base))
		{
			continue;
		}
		FVector Away = It->GetActorLocation() - Center;
		Away.Z = 0.f;
		if (!Away.Normalize())
		{
			Away = -It->GetActorForwardVector();
		}
		It->LaunchCharacter(Away * KnockbackSpeed + FVector(0.f, 0.f, KnockbackUp), true, true);
	}
}
