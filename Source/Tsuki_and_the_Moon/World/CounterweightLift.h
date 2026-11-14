#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CounterweightLift.generated.h"

class UStaticMeshComponent;
class APropertyActor;

/**
 * Pulley lift and counterweight (GDD §6 Area 3, "the hanging counterweight").
 * While the counterweight is heavier than BalanceWeight it hangs low and the lift sits at the top;
 * make it light (Lightness) and it rises while the lift comes down. Cut Lightness off again while
 * riding and the counterweight drops, carrying the lift and Tsuki back up.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ACounterweightLift : public AActor
{
	GENERATED_BODY()

public:
	ACounterweightLift();

	/** The platform Tsuki rides. Place the actor at the lift's TOP position. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lift")
	TObjectPtr<UStaticMeshComponent> Lift;

	/** The counterweight (a Lightness receiver). Place it at its LOW position. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lift")
	TObjectPtr<APropertyActor> Counterweight;

	/** Vertical distance both parts travel (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lift", meta = (ClampMin = "0.0"))
	float TravelHeight = 600.f;

	/** Counterweight lighter than this lets the lift down. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lift", meta = (ClampMin = "0.0"))
	float BalanceWeight = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lift", meta = (ClampMin = "1.0"))
	float Speed = 220.f;

	UFUNCTION(BlueprintPure, Category = "Lift")
	bool IsLiftDown() const;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	FVector LiftTop = FVector::ZeroVector;
	FVector WeightLow = FVector::ZeroVector;
	float Progress = 0.f; // 0 = lift at top, 1 = lift at bottom.
};
