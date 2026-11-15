#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TsukiGameMode.generated.h"

/** Uses ATsukiHUD; BP_TsukiGameMode sets the pawn to BP_Tsuki. */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATsukiGameMode();

	/** Console: "TsukiNewGame" erases the save and restarts the slice from the opening. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tsuki")
	void TsukiNewGame();
};
