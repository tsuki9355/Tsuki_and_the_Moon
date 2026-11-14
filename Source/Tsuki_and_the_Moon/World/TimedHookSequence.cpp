#include "World/TimedHookSequence.h"
#include "World/HookTarget.h"

ATimedHookSequence::ATimedHookSequence()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATimedHookSequence::BeginPlay()
{
	Super::BeginPlay();
	SetAll(false);
}

void ATimedHookSequence::SetAll(bool bEnabled)
{
	for (AHookTarget* Target : Targets)
	{
		if (Target)
		{
			Target->SetHookEnabled(bEnabled);
		}
	}
}

void ATimedHookSequence::HandleActivated()
{
	Elapsed = 0.f;
	bRunning = true;
	SetAll(false);
}

void ATimedHookSequence::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bRunning)
	{
		return;
	}
	Elapsed += DeltaSeconds;

	const float Total = StaggerDelay * Targets.Num() + ActiveDuration;
	const bool bWarning = Elapsed > Total - WarningTime;
	const bool bBlinkOn = !bWarning || FMath::Fmod(Elapsed, 0.4f) < 0.25f;
	for (int32 i = 0; i < Targets.Num(); ++i)
	{
		AHookTarget* Target = Targets[i];
		if (!Target)
		{
			continue;
		}
		const bool bOn = Elapsed >= StaggerDelay * i && Elapsed < Total;
		if (Target->bHookEnabled != bOn)
		{
			Target->SetHookEnabled(bOn);
		}
		if (bOn)
		{
			Target->SetMarkerFlash(bBlinkOn);
		}
	}
	if (Elapsed >= Total)
	{
		bRunning = false;
		SetAll(false);
	}
}
