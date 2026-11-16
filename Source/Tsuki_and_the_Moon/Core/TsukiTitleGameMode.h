#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TsukiTitleGameMode.generated.h"

class UAudioComponent;
class USoundBase;
class UTsukiTitleWidget;

/**
 * Title screen (L_Title). No pawn is spawned: the view is an ATsukiTitleCamera panning over the forest,
 * the title music loops, and UTsukiTitleWidget shows "Press any button", then Start / Continue / Credits / Quit.
 * Start and Continue fade to black and open GameMap. BP_TsukiTitleGameMode holds the tuning.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiTitleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATsukiTitleGameMode();

	/** Map opened by Start and Continue. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Title")
	TSoftObjectPtr<UWorld> GameMap;

	/** Widget class for the menu (a Blueprint child can restyle it). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Title")
	TSubclassOf<UTsukiTitleWidget> TitleWidgetClass;

	/** Text shown on the Credits panel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Title", meta = (MultiLine = true))
	FText CreditsText;

	/** Looping title music. Example: CUE_Menu_Loop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Music")
	TObjectPtr<USoundBase> TitleMusic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Music", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MusicVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Music", meta = (ClampMin = "0.0"))
	float MusicFadeInTime = 3.f;

	/** Optional UI sounds: moving the selection and pressing a button. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Music")
	TObjectPtr<USoundBase> SelectSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Music")
	TObjectPtr<USoundBase> ConfirmSound;

	/** Fade from black when the title appears. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Title", meta = (ClampMin = "0.0"))
	float FadeInTime = 2.5f;

	/** Fade to black (picture and music) before the game map opens. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Title", meta = (ClampMin = "0.1"))
	float FadeOutTime = 1.5f;

	/** True when a save exists, so Continue is offered. */
	UFUNCTION(BlueprintPure, Category = "Tsuki|Title")
	bool CanContinue() const;

	/** Erases any save and starts the slice from the opening. */
	UFUNCTION(BlueprintCallable, Category = "Tsuki|Title")
	void StartNewGame();

	/** Loads the save and resumes at the last checkpoint. */
	UFUNCTION(BlueprintCallable, Category = "Tsuki|Title")
	void ContinueGame();

	UFUNCTION(BlueprintCallable, Category = "Tsuki|Title")
	void QuitGame();

	/** Plays SelectSound or ConfirmSound if set. */
	void PlayUISound(bool bConfirm) const;

	bool IsLeaving() const { return bLeaving; }

protected:
	virtual void BeginPlay() override;

	/** The title has no player pawn. */
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override { return false; }

private:
	void LeaveToGame();
	void OpenGameMap();

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> MusicComponent;

	UPROPERTY(Transient)
	TObjectPtr<UTsukiTitleWidget> TitleWidget;

	FTimerHandle LeaveTimer;
	bool bLeaving = false;
};
