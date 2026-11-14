#include "World/CounterweightLift.h"
#include "Properties/PropertyActor.h"
#include "Properties/PropertyHolderComponent.h"
#include "Components/StaticMeshComponent.h"

ACounterweightLift::ACounterweightLift()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	Lift = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lift"));
	Lift->SetMobility(EComponentMobility::Movable);
	RootComponent = Lift;
}

void ACounterweightLift::BeginPlay()
{
	Super::BeginPlay();
	LiftTop = GetActorLocation();
	if (Counterweight)
	{
		WeightLow = Counterweight->GetActorLocation();
	}
}

bool ACounterweightLift::IsLiftDown() const
{
	const UPropertyHolderComponent* Holder = Counterweight ? Counterweight->Holder.Get() : nullptr;
	return Holder && Holder->GetEffectiveWeight() < BalanceWeight;
}

void ACounterweightLift::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Target = IsLiftDown() ? 1.f : 0.f;
	if (FMath::IsNearlyEqual(Progress, Target))
	{
		return;
	}
	Progress = FMath::FInterpConstantTo(Progress, Target, DeltaSeconds, Speed / FMath::Max(TravelHeight, 1.f));
	SetActorLocation(LiftTop - FVector(0.f, 0.f, TravelHeight * Progress));
	if (Counterweight)
	{
		Counterweight->SetActorLocation(WeightLow + FVector(0.f, 0.f, TravelHeight * Progress));
	}
}
