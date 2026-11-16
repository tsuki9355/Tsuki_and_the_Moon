#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TsukiTitleCamera.generated.h"

class UCameraComponent;
class USplineComponent;

/**
 * Title-screen camera: glides slowly along Path (a closed loop by default) while looking at FocusPoint,
 * with a gentle float. ATsukiTitleGameMode uses the first one in the level as the view.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiTitleCamera : public AActor
{
	GENERATED_BODY()

public:
	ATsukiTitleCamera();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Title")
	TObjectPtr<USplineComponent> Path;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Title")
	TObjectPtr<UCameraComponent> Camera;

	/** Glide speed along the path, cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title", meta = (ClampMin = "0.0"))
	float Speed = 90.f;

	/** Where the camera looks, relative to the actor. Drag the diamond widget in the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title", meta = (MakeEditWidget = true))
	FVector FocusPoint = FVector(0.f, 0.f, 150.f);

	/** Vertical float amplitude (cm) and period (s), for a slow hand-held drift. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title", meta = (ClampMin = "0.0"))
	float FloatHeight = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title", meta = (ClampMin = "1.0"))
	float FloatPeriod = 9.f;

	/**
	 * Build Path as an ellipse around the actor from the values below (easiest to tune).
	 * Turn off to hand-edit the spline points instead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title|Orbit")
	bool bUseOrbit = true;

	/** Orbit half-width along X and Y, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title|Orbit", meta = (EditCondition = "bUseOrbit"))
	FVector2D OrbitRadius = FVector2D(1500.f, 1500.f);

	/** Orbit height above the actor, cm, and how much it rises and dips twice per lap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title|Orbit", meta = (EditCondition = "bUseOrbit"))
	float OrbitHeight = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title|Orbit", meta = (EditCondition = "bUseOrbit"))
	float OrbitHeightWave = 0.f;

	/** Start position along the path, 0..1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StartFraction = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void PlaceCamera(float Time);
	void BuildOrbit();

	float Distance = 0.f;
	float Direction = 1.f;
};
