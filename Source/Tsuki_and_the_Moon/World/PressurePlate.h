#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PressurePlate.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class USoundBase;
class ATsukiMechanism;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlateChanged);

/**
 * Pressure plate (GDD §3). Sums the effective weight of everything on it; at ActivationWeight it sinks
 * with a clunk and activates its Targets. Lighter loads press it only partway so the player can read
 * "almost, but not enough".
 */
UCLASS()
class TSUKI_AND_THE_MOON_API APressurePlate : public AActor
{
	GENERATED_BODY()

public:
	APressurePlate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plate")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plate")
	TObjectPtr<UStaticMeshComponent> PlateMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plate")
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate", meta = (ClampMin = "0.0"))
	float ActivationWeight = 80.f;

	/** How far the plate sinks when fully pressed (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate", meta = (ClampMin = "0.0"))
	float DepressDepth = 6.f;

	/** Fraction of DepressDepth shown for a load below the threshold. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PartialDepressFraction = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate", meta = (ClampMin = "1.0"))
	float DepressSpeed = 40.f;

	/** Stay pressed once activated (keeps solved state). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
	bool bLatch = false;

	/** Marked solved in progress on first activation; a solved latched plate starts pressed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
	FName PuzzleId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
	TArray<TObjectPtr<ATsukiMechanism>> Targets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
	TObjectPtr<USoundBase> PressSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
	TObjectPtr<USoundBase> ReleaseSound;

	UPROPERTY(BlueprintAssignable, Category = "Plate")
	FOnPlateChanged OnActivated;

	UPROPERTY(BlueprintAssignable, Category = "Plate")
	FOnPlateChanged OnDeactivated;

	UFUNCTION(BlueprintPure, Category = "Plate")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Plate")
	bool IsActivated() const { return bActivated; }

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	void SetActivated(bool bNew, bool bSilent);

	mutable TArray<AActor*> OverlapCache;
	float PlateRestZ = 0.f;
	bool bActivated = false;
};
