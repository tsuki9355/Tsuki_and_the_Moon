#include "Core/TsukiGameMode.h"
#include "Core/TsukiProgressSubsystem.h"
#include "Player/TsukiCharacter.h"
#include "UI/TsukiHUD.h"
#include "Kismet/GameplayStatics.h"

ATsukiGameMode::ATsukiGameMode()
{
	DefaultPawnClass = ATsukiCharacter::StaticClass();
	HUDClass = ATsukiHUD::StaticClass();
}

void ATsukiGameMode::TsukiNewGame()
{
	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		Progress->ResetProgress();
	}
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
