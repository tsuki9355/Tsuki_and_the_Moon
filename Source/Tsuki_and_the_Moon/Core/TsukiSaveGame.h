#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Properties/MoonPropertyTypes.h"
#include "TsukiSaveGame.generated.h"

/** Slice progress written at every checkpoint and pickup (GDD §8). */
UCLASS()
class TSUKI_AND_THE_MOON_API UTsukiSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<ETsukiTool> Tools;

	UPROPERTY()
	TArray<FName> SolvedPuzzles;

	UPROPERTY()
	int32 MoonFragments = 0;

	UPROPERTY()
	FTransform RespawnTransform;

	UPROPERTY()
	bool bHasRespawn = false;
};
