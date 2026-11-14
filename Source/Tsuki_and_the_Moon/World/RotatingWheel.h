#pragma once

#include "CoreMinimal.h"
#include "Properties/PropertyActor.h"
#include "RotatingWheel.generated.h"

class UStaticMesh;

/**
 * The rapidly rotating wheel (GDD §6 Area 3). The hub (Mesh) spins about its local Y axis and carries
 * level gondolas around it, so Tsuki can ride one upward once Slowness makes it safe.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ARotatingWheel : public APropertyActor
{
	GENERATED_BODY()

public:
	ARotatingWheel();

	/** Degrees per second before Slowness. Positive turns bottom -> +X side -> top -> -X side. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	float DegreesPerSecond = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel", meta = (ClampMin = "50.0"))
	float Radius = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel", meta = (ClampMin = "1", ClampMax = "12"))
	int32 GondolaCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	TObjectPtr<UStaticMesh> GondolaMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	FVector GondolaScale = FVector(2.4f, 2.4f, 0.3f);

	/** Horizontal / vertical launch speed when Tsuki lands on a gondola before the wheel is slowed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	float KnockbackSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	float KnockbackUp = 450.f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	void PlaceGondolas();
	void KnockOffRiders();

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Gondolas;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Spokes;

	float Angle = 0.f;
};
