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

	/** Looping background music, started when play begins. Example: CUE_Field_Loop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Music")
	TObjectPtr<USoundBase> BackgroundMusic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Music", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MusicVolume = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Music", meta = (ClampMin = "0.0"))
	float MusicFadeInTime = 2.f;

	/** Fades the background music out (e.g. before the ending theme). Usage: GameMode->StopMusic(1.5f). */
	UFUNCTION(BlueprintCallable, Category = "Tsuki|Music")
	void StopMusic(float FadeOutTime = 1.5f);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> MusicComponent;
};
