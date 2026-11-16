#include "World/TsukiTitleCamera.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"

ATsukiTitleCamera::ATsukiTitleCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Path = CreateDefaultSubobject<USplineComponent>(TEXT("Path"));
	Path->SetupAttachment(RootComponent);
	BuildOrbit();

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetFieldOfView(70.f);
}

void ATsukiTitleCamera::BuildOrbit()
{
	constexpr int32 NumPoints = 8;
	Path->ClearSplinePoints(false);
	for (int32 i = 0; i < NumPoints; ++i)
	{
		const float A = UE_TWO_PI * i / NumPoints;
		const float Z = OrbitHeight + OrbitHeightWave * FMath::Sin(2.f * A);
		Path->AddSplinePoint(FVector(FMath::Cos(A) * OrbitRadius.X, FMath::Sin(A) * OrbitRadius.Y, Z), ESplineCoordinateSpace::Local, false);
	}
	Path->SetClosedLoop(true, false);
	Path->UpdateSpline();
}

void ATsukiTitleCamera::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (bUseOrbit)
	{
		BuildOrbit();
	}
	Path->UpdateSpline();
	Distance = Path->GetSplineLength() * StartFraction;
	PlaceCamera(0.f);
}

void ATsukiTitleCamera::BeginPlay()
{
	Super::BeginPlay();
	if (bUseOrbit)
	{
		BuildOrbit();
	}
	Path->UpdateSpline(); // Points may be authored by tools that skip the editor's spline refresh.
	Distance = Path->GetSplineLength() * StartFraction;
	PlaceCamera(0.f);
}

void ATsukiTitleCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Length = Path->GetSplineLength();
	if (Length <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	Distance += Speed * Direction * DeltaSeconds;
	if (Path->IsClosedLoop())
	{
		Distance = FMath::Fmod(Distance + Length, Length);
	}
	else if (Distance > Length || Distance < 0.f)
	{
		// Open path: glide back the other way.
		Direction = -Direction;
		Distance = FMath::Clamp(Distance, 0.f, Length);
	}
	PlaceCamera(GetWorld()->GetTimeSeconds());
}

void ATsukiTitleCamera::PlaceCamera(float Time)
{
	const FVector Float(0.f, 0.f, FMath::Sin(Time * UE_TWO_PI / FloatPeriod) * FloatHeight);
	const FVector Location = Path->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World) + Float;
	const FVector Focus = GetActorTransform().TransformPosition(FocusPoint);
	Camera->SetWorldLocationAndRotation(Location, (Focus - Location).Rotation());
}
