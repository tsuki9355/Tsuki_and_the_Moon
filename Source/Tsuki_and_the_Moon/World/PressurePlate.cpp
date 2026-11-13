#include "World/PressurePlate.h"
#include "World/TsukiMechanism.h"
#include "Properties/PropertyHolderComponent.h"
#include "Core/TsukiProgressSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

APressurePlate::APressurePlate()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	PlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
	PlateMesh->SetupAttachment(RootComponent);
	PlateMesh->SetMobility(EComponentMobility::Movable);

	// The plate sits nearly flush with the ground; no collision so pushed objects slide on and off it.
	BaseMesh->SetCollisionProfileName(TEXT("NoCollision"));
	PlateMesh->SetCollisionProfileName(TEXT("NoCollision"));

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(RootComponent);
	Trigger->SetBoxExtent(FVector(60.f, 60.f, 40.f));
	Trigger->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trigger->SetGenerateOverlapEvents(true);
}

void APressurePlate::BeginPlay()
{
	Super::BeginPlay();
	PlateRestZ = PlateMesh->GetRelativeLocation().Z;

	const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this);
	if (bLatch && Progress && Progress->IsSolved(PuzzleId))
	{
		SetActivated(true, true);
	}
}

float APressurePlate::GetCurrentWeight() const
{
	OverlapCache.Reset();
	Trigger->GetOverlappingActors(OverlapCache);
	float Total = 0.f;
	for (const AActor* Actor : OverlapCache)
	{
		if (const UPropertyHolderComponent* Holder = Actor ? Actor->FindComponentByClass<UPropertyHolderComponent>() : nullptr)
		{
			Total += Holder->GetEffectiveWeight();
		}
	}
	return Total;
}

void APressurePlate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Weight = GetCurrentWeight();
	const bool bShould = Weight >= ActivationWeight || (bLatch && bActivated);
	if (bShould != bActivated)
	{
		SetActivated(bShould, false);
	}

	const float Depth = bActivated ? DepressDepth : (Weight > 0.f ? DepressDepth * PartialDepressFraction : 0.f);
	FVector Loc = PlateMesh->GetRelativeLocation();
	const float TargetZ = PlateRestZ - Depth;
	if (!FMath::IsNearlyEqual(Loc.Z, TargetZ, 0.01f))
	{
		Loc.Z = FMath::FInterpConstantTo(Loc.Z, TargetZ, DeltaSeconds, DepressSpeed);
		PlateMesh->SetRelativeLocation(Loc);
	}
}

void APressurePlate::SetActivated(bool bNew, bool bSilent)
{
	bActivated = bNew;
	if (!bSilent)
	{
		if (USoundBase* Sound = bNew ? PressSound : ReleaseSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}
	}
	if (bNew)
	{
		if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
		{
			Progress->MarkSolved(PuzzleId);
		}
	}
	for (ATsukiMechanism* Target : Targets)
	{
		if (Target)
		{
			Target->SetMechanismActive(bNew);
		}
	}
	if (bNew)
	{
		OnActivated.Broadcast();
	}
	else
	{
		OnDeactivated.Broadcast();
	}
}
