#include "World/Checkpoint.h"
#include "Player/TsukiCharacter.h"
#include "Core/TsukiProgressSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

ACheckpoint::ACheckpoint()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetBoxExtent(FVector(100.f, 300.f, 200.f));
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = Trigger;

	SpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(RootComponent);
	SpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, -100.f));

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::HandleOverlap);
}

void ACheckpoint::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!Cast<ATsukiCharacter>(OtherActor))
	{
		return;
	}
	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		FTransform Spawn = SpawnPoint->GetComponentTransform();
		Spawn.SetScale3D(FVector::OneVector);
		Progress->SetRespawnTransform(Spawn);
		Progress->MarkSolved(MarkSolvedOnEnter);
	}
}

ATsukiFallVolume::ATsukiFallVolume()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetBoxExtent(FVector(1000.f, 1000.f, 100.f));
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = Trigger;
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ATsukiFallVolume::HandleOverlap);
}

void ATsukiFallVolume::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (ATsukiCharacter* Tsuki = Cast<ATsukiCharacter>(OtherActor))
	{
		Tsuki->Respawn();
	}
}
