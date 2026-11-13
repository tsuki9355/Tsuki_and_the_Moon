#include "World/TsukiChest.h"
#include "Components/StaticMeshComponent.h"

ATsukiChest::ATsukiChest()
{
	bAnimateIdle = false;
	PromptText = NSLOCTEXT("Tsuki", "OpenChest", "Open");

	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	Lid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lid"));
	Lid->SetupAttachment(Mesh);
	Lid->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Lid->SetMobility(EComponentMobility::Movable);
}

void ATsukiChest::ShowCollected(bool bInstant)
{
	bOpening = true;
	if (bInstant)
	{
		LidPitch = OpenAngle;
		FRotator Rot = Lid->GetRelativeRotation();
		Rot.Pitch = LidPitch;
		Lid->SetRelativeRotation(Rot);
		return;
	}
	SetActorTickEnabled(true);
}

void ATsukiChest::Tick(float DeltaSeconds)
{
	AActor::Tick(DeltaSeconds); // Skip the pickup bob.
	if (!bOpening)
	{
		return;
	}
	LidPitch = FMath::FInterpConstantTo(LidPitch, OpenAngle, DeltaSeconds, OpenSpeed);
	FRotator Rot = Lid->GetRelativeRotation();
	Rot.Pitch = LidPitch;
	Lid->SetRelativeRotation(Rot);
	if (FMath::IsNearlyEqual(LidPitch, OpenAngle, 0.1f))
	{
		SetActorTickEnabled(false);
	}
}
