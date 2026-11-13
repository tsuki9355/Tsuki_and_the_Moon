#include "World/FairyHintVolume.h"
#include "Player/TsukiCharacter.h"
#include "Core/TsukiProgressSubsystem.h"
#include "UI/TsukiHUD.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"

AFairyHintVolume::AFairyHintVolume()
{
	Area = CreateDefaultSubobject<UBoxComponent>(TEXT("Area"));
	Area->SetBoxExtent(FVector(600.f, 600.f, 300.f));
	Area->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = Area;
	Area->OnComponentBeginOverlap.AddDynamic(this, &AFairyHintVolume::HandleBegin);
	Area->OnComponentEndOverlap.AddDynamic(this, &AFairyHintVolume::HandleEnd);
}

bool AFairyHintVolume::IsActiveForHints() const
{
	const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this);
	if (!Progress)
	{
		return false;
	}
	if (bRequireScissors && !Progress->HasTool(ETsukiTool::Scissors))
	{
		return false;
	}
	return !Progress->IsSolved(PuzzleId);
}

void AFairyHintVolume::ShowLine(const FText& Line)
{
	if (ATsukiHUD* HUD = ATsukiHUD::Get(this))
	{
		if (!Line.IsEmpty())
		{
			HUD->ShowFairyLine(Line, LineDuration);
		}
	}
}

void AFairyHintVolume::HandleBegin(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	ATsukiCharacter* Tsuki = Cast<ATsukiCharacter>(OtherActor);
	if (!Tsuki)
	{
		return;
	}
	Tsuki->SetActiveHintVolume(this, true);
	if (Tier == 0 && IsActiveForHints())
	{
		Tier = 1;
		ShowLine(NudgeLine);
	}
	if (Tier < 2)
	{
		GetWorldTimerManager().SetTimer(Tier2Timer, this, &AFairyHintVolume::Tier2Timeout, Tier2Delay, false);
	}
}

void AFairyHintVolume::HandleEnd(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	if (ATsukiCharacter* Tsuki = Cast<ATsukiCharacter>(OtherActor))
	{
		Tsuki->SetActiveHintVolume(this, false);
		GetWorldTimerManager().ClearTimer(Tier2Timer);
	}
}

void AFairyHintVolume::Tier2Timeout()
{
	if (IsActiveForHints())
	{
		Tier = 2;
		ShowLine(DirectLine);
	}
}

void AFairyHintVolume::RequestHint()
{
	if (!IsActiveForHints())
	{
		return;
	}
	// First ask gives the nudge if it was missed, later asks give the direct clue.
	if (Tier == 0)
	{
		Tier = 1;
		ShowLine(NudgeLine);
	}
	else
	{
		Tier = 2;
		ShowLine(DirectLine);
	}
}
