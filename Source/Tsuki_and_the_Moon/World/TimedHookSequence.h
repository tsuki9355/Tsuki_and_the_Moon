#pragma once

#include "CoreMinimal.h"
#include "World/TsukiMechanism.h"
#include "TimedHookSequence.generated.h"

class AHookTarget;

/**
 * The giant-tree climb (GDD §6 Area 3, beat 6). When its plate is pressed, the hook targets light up
 * one after another, stay on for ActiveDuration, blink as a warning, then switch off. Pressing the
 * plate again restarts the sequence, so a fall costs only a retry.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATimedHookSequence : public ATsukiMechanism
{
	GENERATED_BODY()

public:
	ATimedHookSequence();

	/** In climbing order. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence")
	TArray<TObjectPtr<AHookTarget>> Targets;

	/** Seconds between each target lighting up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence", meta = (ClampMin = "0.0"))
	float StaggerDelay = 0.35f;

	/** Seconds all targets stay usable (generous). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence", meta = (ClampMin = "1.0"))
	float ActiveDuration = 25.f;

	/** Final seconds during which markers blink. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence", meta = (ClampMin = "0.0"))
	float WarningTime = 5.f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void HandleActivated() override;
	virtual void HandleDeactivated() override {} // Stepping off the plate does not cancel the climb.

private:
	void SetAll(bool bEnabled);

	float Elapsed = 0.f;
	bool bRunning = false;
};
