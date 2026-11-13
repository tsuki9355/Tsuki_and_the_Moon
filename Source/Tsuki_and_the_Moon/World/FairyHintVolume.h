#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FairyHintVolume.generated.h"

class UBoxComponent;

/**
 * Layered fairy hints around one puzzle (GDD §4). Entering shows a thematic nudge; the direct
 * clue comes when the player asks (Hint button) or stays stuck for Tier2Delay. Silent once solved.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API AFairyHintVolume : public AActor
{
	GENERATED_BODY()

public:
	AFairyHintVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hint")
	TObjectPtr<UBoxComponent> Area;

	/** Hints stop once this puzzle is solved. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hint")
	FName PuzzleId;

	/** Only hint once Tsuki has the scissors. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hint")
	bool bRequireScissors = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hint", meta = (MultiLine = true))
	FText NudgeLine;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hint", meta = (MultiLine = true))
	FText DirectLine;

	/** Seconds inside, unsolved, before the direct clue is offered on its own. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hint", meta = (ClampMin = "5.0"))
	float Tier2Delay = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hint", meta = (ClampMin = "1.0"))
	float LineDuration = 6.f;

	/** Player asked the fairy for help. */
	void RequestHint();

protected:
	UFUNCTION()
	void HandleBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool IsActiveForHints() const;
	void ShowLine(const FText& Line);
	void Tier2Timeout();

	int32 Tier = 0;
	FTimerHandle Tier2Timer;
};
