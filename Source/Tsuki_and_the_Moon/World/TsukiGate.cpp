#include "World/TsukiGate.h"
#include "Components/StaticMeshComponent.h"

ATsukiGate::ATsukiGate()
{
	PrimaryActorTick.bCanEverTick = true;

	Frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Frame"));
	Frame->SetupAttachment(RootComponent);

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(RootComponent);
	Door->SetMobility(EComponentMobility::Movable);
}

void ATsukiGate::BeginPlay()
{
	Super::BeginPlay();
	ClosedLocation = Door->GetRelativeLocation();
}

void ATsukiGate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const FVector Target = bOpen ? ClosedLocation + OpenOffset : ClosedLocation;
	const FVector Current = Door->GetRelativeLocation();
	if (!Current.Equals(Target, 0.1f))
	{
		Door->SetRelativeLocation(FMath::VInterpConstantTo(Current, Target, DeltaSeconds, OpenSpeed));
	}
}
