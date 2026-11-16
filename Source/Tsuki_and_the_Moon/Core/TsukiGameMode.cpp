#include "Core/TsukiGameMode.h"
#include "Core/TsukiProgressSubsystem.h"
#include "Player/TsukiCharacter.h"
#include "UI/TsukiHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

ATsukiGameMode::ATsukiGameMode()
{
	DefaultPawnClass = ATsukiCharacter::StaticClass();
	HUDClass = ATsukiHUD::StaticClass();
}

void ATsukiGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (BackgroundMusic)
	{
		MusicComponent = UGameplayStatics::CreateSound2D(this, BackgroundMusic, MusicVolume, 1.f, 0.f, nullptr, false, false);
		if (MusicComponent)
		{
			MusicComponent->FadeIn(MusicFadeInTime, MusicVolume);
		}
	}
}

void ATsukiGameMode::StopMusic(float FadeOutTime)
{
	if (MusicComponent)
	{
		MusicComponent->FadeOut(FMath::Max(FadeOutTime, 0.f), 0.f);
	}
}

void ATsukiGameMode::TsukiNewGame()
{
	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		Progress->ResetProgress();
	}
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
