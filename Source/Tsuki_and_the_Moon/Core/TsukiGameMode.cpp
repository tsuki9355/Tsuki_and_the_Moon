#include "Core/TsukiGameMode.h"
#include "Player/TsukiCharacter.h"
#include "UI/TsukiHUD.h"

ATsukiGameMode::ATsukiGameMode()
{
	DefaultPawnClass = ATsukiCharacter::StaticClass();
	HUDClass = ATsukiHUD::StaticClass();
}
