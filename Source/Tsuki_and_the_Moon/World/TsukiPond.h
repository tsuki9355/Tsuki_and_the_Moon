#pragma once

#include "CoreMinimal.h"
#include "World/TsukiMechanism.h"
#include "TsukiPond.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * A pond (GDD §6 Area 2, Area 3 lily pond). Objects inside ride the surface if they float
 * (Buoyancy or natural float, and not Heaviness), otherwise they sink to the floor.
 * Tsuki wades in shallow water and swims at the surface in deep water (see ATsukiCharacter::UpdateWater).
 * As a mechanism, activating it lowers the surface by LowerDistance (the pond-lowering reveal).
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiPond : public ATsukiMechanism
{
	GENERATED_BODY()

public:
	ATsukiPond();

	/** Area the water occupies. Its top should match the surface mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water")
	TObjectPtr<UBoxComponent> Volume;

	/** Visible water surface; moves down when the pond lowers. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water")
	TObjectPtr<UStaticMeshComponent> Surface;

	/** How far the surface drops when activated (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0"))
	float LowerDistance = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "1.0"))
	float LowerSpeed = 140.f;

	/** Floating objects rest with their bottom this far above the surface (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water")
	float FloatClearance = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "1.0"))
	float RiseSpeed = 160.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "1.0"))
	float SinkSpeed = 220.f;

	UFUNCTION(BlueprintPure, Category = "Water")
	float GetSurfaceZ() const { return BaseSurfaceZ + SurfaceOffset; }

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void HandleActivated() override { bLowered = true; }
	virtual void HandleDeactivated() override { bLowered = false; }

private:
	float FindFloorZ(const UPrimitiveComponent* Root, float FromZ) const;

	TArray<AActor*> OverlapCache;
	float BaseSurfaceZ = 0.f;
	float SurfaceOffset = 0.f;
	float SurfaceRestRelZ = 0.f;
	bool bLowered = false;
};
