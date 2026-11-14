#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TsukiBomb.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class USoundBase;

/**
 * One Moonlit Bomb. Its fuse starts when Tsuki takes it out; it flashes faster near the end and
 * bursts, breaking any cracked surface within ExplosionRadius. It only affects cracked surfaces.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiBomb : public AActor
{
	GENERATED_BODY()

public:
	ATsukiBomb();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<UPointLightComponent> Glow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "0.5"))
	float FuseTime = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "10.0"))
	float ExplosionRadius = 280.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<USoundBase> ExplodeSound;

	/** Release from Tsuki's hands with an initial velocity (physics from here on). */
	void Throw(const FVector& Velocity);

	UFUNCTION(BlueprintPure, Category = "Bomb")
	bool HasExploded() const { return bExploded; }

	virtual void Tick(float DeltaSeconds) override;

private:
	void Explode();

	float Elapsed = 0.f;
	float BurstTime = 0.f;
	bool bExploded = false;
};
