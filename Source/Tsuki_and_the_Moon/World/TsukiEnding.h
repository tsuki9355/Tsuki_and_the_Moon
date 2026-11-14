#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TsukiEnding.generated.h"

class UBoxComponent;
class UCameraComponent;
class USoundBase;

/**
 * End of the slice (GDD §8). Tsuki drops through the summit opening into this trigger; the camera
 * cuts to a glimpse of the next area, the fairy says her lines, and the slice ends.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiEnding : public AActor
{
	GENERATED_BODY()

public:
	ATsukiEnding();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ending")
	TObjectPtr<UBoxComponent> Trigger;

	/** Frames the glimpse of what lies beyond. Place it in the level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ending")
	TObjectPtr<UCameraComponent> GlimpseCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending", meta = (MultiLine = true))
	TArray<FText> Lines;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending", meta = (ClampMin = "1.0"))
	float LineDuration = 5.f;

	/** Shown last and left on screen. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending")
	FText EndCard = NSLOCTEXT("Tsuki", "EndCard", "- To be continued -");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending")
	TObjectPtr<USoundBase> EndingMusic;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ending")
	void OnEndingStarted();

protected:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void NextLine();

	int32 LineIndex = 0;
	bool bStarted = false;
	FTimerHandle LineTimer;
};
