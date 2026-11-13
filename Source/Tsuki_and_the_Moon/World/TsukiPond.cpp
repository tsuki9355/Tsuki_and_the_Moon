#include "World/TsukiPond.h"
#include "Player/TsukiCharacter.h"
#include "Properties/PropertyHolderComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

ATsukiPond::ATsukiPond()
{
	PrimaryActorTick.bCanEverTick = true;

	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Volume->SetupAttachment(RootComponent);
	Volume->SetBoxExtent(FVector(500.f, 500.f, 250.f));
	Volume->SetRelativeLocation(FVector(0.f, 0.f, -250.f));
	Volume->SetCollisionProfileName(TEXT("Trigger"));

	Surface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Surface"));
	Surface->SetupAttachment(RootComponent);
	Surface->SetMobility(EComponentMobility::Movable);
	Surface->SetCollisionProfileName(TEXT("NoCollision"));
	Surface->SetCastShadow(false);
}

void ATsukiPond::BeginPlay()
{
	Super::BeginPlay();
	BaseSurfaceZ = Surface->GetComponentLocation().Z;
	SurfaceRestRelZ = Surface->GetRelativeLocation().Z;
}

float ATsukiPond::FindFloorZ(const UPrimitiveComponent* Root, float FromZ) const
{
	// Highest support under the center or any (slightly inset) corner, so an object
	// only drops once it is fully off a ledge.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WaterFloor), false, Root->GetOwner());
	const FVector C = Root->Bounds.Origin;
	const FVector E = Root->Bounds.BoxExtent * 0.85f;
	const FVector2D Points[] = { {C.X, C.Y}, {C.X + E.X, C.Y + E.Y}, {C.X + E.X, C.Y - E.Y}, {C.X - E.X, C.Y + E.Y}, {C.X - E.X, C.Y - E.Y} };
	float Best = -BIG_NUMBER;
	for (const FVector2D& P : Points)
	{
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, FVector(P.X, P.Y, FromZ), FVector(P.X, P.Y, FromZ - 5000.f), ECC_Visibility, Params))
		{
			Best = FMath::Max(Best, Hit.ImpactPoint.Z);
		}
	}
	return Best > -BIG_NUMBER ? Best : FromZ;
}

void ATsukiPond::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Raise / lower the surface.
	const float TargetOffset = bLowered ? -LowerDistance : 0.f;
	if (!FMath::IsNearlyEqual(SurfaceOffset, TargetOffset, 0.1f))
	{
		SurfaceOffset = FMath::FInterpConstantTo(SurfaceOffset, TargetOffset, DeltaSeconds, LowerSpeed);
		FVector Rel = Surface->GetRelativeLocation();
		Rel.Z = SurfaceRestRelZ + SurfaceOffset;
		Surface->SetRelativeLocation(Rel);
	}
	const float SurfaceZ = GetSurfaceZ();

	OverlapCache.Reset();
	Volume->GetOverlappingActors(OverlapCache);
	for (AActor* Actor : OverlapCache)
	{
		if (ATsukiCharacter* Tsuki = Cast<ATsukiCharacter>(Actor))
		{
			Tsuki->UpdateWater(SurfaceZ);
			continue;
		}

		UPropertyHolderComponent* Holder = Actor ? Actor->FindComponentByClass<UPropertyHolderComponent>() : nullptr;
		UPrimitiveComponent* Root = Actor ? Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
		if (!Holder || !Root || Root->Mobility != EComponentMobility::Movable || Root->GetAttachParent())
		{
			continue; // Static scenery, or an object Tsuki is carrying.
		}

		const FBoxSphereBounds Bounds = Root->Bounds;
		const float Bottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
		const float FloorZ = FindFloorZ(Root, Bottom + 5.f);
		// Rest 1 cm above the floor so push/pull sweeps don't start in contact with it.
		const float RestZ = FloorZ + 1.f;
		const float TargetBottom = Holder->FloatsInWater() ? FMath::Max(SurfaceZ + FloatClearance, RestZ) : RestZ;
		if (!FMath::IsNearlyEqual(Bottom, TargetBottom, 0.5f))
		{
			const float Speed = TargetBottom > Bottom ? RiseSpeed : SinkSpeed;
			const float NewBottom = FMath::FInterpConstantTo(Bottom, TargetBottom, DeltaSeconds, Speed);
			Root->SetWorldLocation(Root->GetComponentLocation() + FVector(0.f, 0.f, NewBottom - Bottom));
		}
	}
}
