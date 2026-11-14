#include "World/TsukiBomb.h"
#include "World/CrackedSurface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ATsukiBomb::ATsukiBomb()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere.Succeeded())
	{
		Mesh->SetStaticMesh(Sphere.Object);
	}
	Mesh->SetWorldScale3D(FVector(0.35f));
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetSimulatePhysics(false);
	RootComponent = Mesh;

	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(Mesh);
	Glow->SetLightColor(FLinearColor(0.7f, 0.75f, 1.f));
	Glow->SetIntensity(600.f);
	Glow->SetAttenuationRadius(250.f);
	Glow->SetCastShadows(false);
}

void ATsukiBomb::Throw(const FVector& Velocity)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetSimulatePhysics(true);
	Mesh->SetPhysicsLinearVelocity(Velocity);
}

void ATsukiBomb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bExploded)
	{
		// Placeholder burst: a quick expanding flash, then gone.
		BurstTime += DeltaSeconds;
		Mesh->SetWorldScale3D(FVector(FMath::Lerp(0.5f, ExplosionRadius / 50.f, FMath::Clamp(BurstTime / 0.2f, 0.f, 1.f))));
		Glow->SetIntensity(FMath::Max(0.f, 40000.f * (1.f - BurstTime / 0.35f)));
		if (BurstTime > 0.35f)
		{
			Destroy();
		}
		return;
	}
	Elapsed += DeltaSeconds;
	// Fuse flash speeds up toward the end (Ocarina-style warning).
	const float Remaining = FMath::Max(FuseTime - Elapsed, 0.f);
	const float Rate = FMath::Lerp(14.f, 2.f, Remaining / FuseTime);
	Glow->SetIntensity(FMath::Sin(Elapsed * Rate * PI) > 0.f ? 2500.f : 300.f);
	if (Elapsed >= FuseTime)
	{
		Explode();
	}
}

void ATsukiBomb::Explode()
{
	bExploded = true;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Glow->SetAttenuationRadius(ExplosionRadius * 3.f);
	if (ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	}
	const FVector Center = GetActorLocation();
	for (TActorIterator<ACrackedSurface> It(GetWorld()); It; ++It)
	{
		const FBox Box = It->GetComponentsBoundingBox(true);
		if (Box.ComputeSquaredDistanceToPoint(Center) <= FMath::Square(ExplosionRadius))
		{
			It->Break();
		}
	}
}
