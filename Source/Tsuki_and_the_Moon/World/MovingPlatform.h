#pragma once

#include "CoreMinimal.h"
#include "Properties/PropertyActor.h"
#include "MovingPlatform.generated.h"

/**
 * A platform that shuttles between its start and start + EndOffset (GDD §6 Area 3, "the restless platform").
 * Slowness slows it through the holder's time dilation, so no extra code is needed for the puzzle.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API AMovingPlatform : public APropertyActor
{
	GENERATED_BODY()

public:
	AMovingPlatform();

	/** Far end, relative to the placed location. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform", meta = (MakeEditWidget = true))
	FVector EndOffset = FVector(1200.f, 0.f, 0.f);

	/** Travel speed before Slowness (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform", meta = (ClampMin = "1.0"))
	float Speed = 800.f;

	/** Pause at each end (seconds, before Slowness). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform", meta = (ClampMin = "0.0"))
	float PauseTime = 0.4f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	FVector Start = FVector::ZeroVector;
	float Alpha = 0.f;
	float Direction = 1.f;
	float PauseLeft = 0.f;
};
