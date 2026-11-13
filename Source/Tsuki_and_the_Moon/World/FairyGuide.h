#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FairyGuide.generated.h"

class USplineComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UCameraComponent;

/**
 * The fairy (GDD §4, opening). Plays the wake-up shot, greets Tsuki, then flies a little ahead of her
 * along LeadPath to the scissors' resting place and waits there. After the scissors are collected
 * it follows at Tsuki's shoulder.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API AFairyGuide : public AActor
{
	GENERATED_BODY()

public:
	AFairyGuide();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fairy")
	TObjectPtr<USplineComponent> LeadPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fairy")
	TObjectPtr<USceneComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fairy")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fairy")
	TObjectPtr<UPointLightComponent> Glow;

	/** Opening wake-up shot (e.g. looking down through the canopy opening). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fairy")
	TObjectPtr<UCameraComponent> OpeningCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fairy|Opening")
	bool bPlayOpening = true;

	/** Seconds on the opening shot before blending to Tsuki. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fairy|Opening", meta = (ClampMin = "0.0"))
	float OpeningHold = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fairy|Opening", meta = (ClampMin = "0.0"))
	float OpeningBlend = 2.f;

	/** Greeting lines, shown one after another once control is given. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fairy|Opening", meta = (MultiLine = true))
	TArray<FText> GreetingLines;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fairy|Opening", meta = (ClampMin = "1.0"))
	float GreetingLineDuration = 4.f;

	/** How far ahead of Tsuki (along the path) the fairy flies. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fairy|Lead", meta = (ClampMin = "0.0"))
	float LeadDistance = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fairy|Lead", meta = (ClampMin = "1.0"))
	float FlySpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fairy|Follow")
	FVector FollowOffset = FVector(-40.f, 60.f, 110.f);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	void EndOpeningShot();
	void GiveControl();
	void ShowNextGreeting();
	FVector ComputeDesiredLocation(const APawn* Tsuki) const;

	int32 NextGreeting = 0;
	float PathProgress = 0.f;
	FTimerHandle OpeningTimer;
	FTimerHandle GreetingTimer;
};
