#pragma once

#include "CoreMinimal.h"
#include "World/TsukiMechanism.h"
#include "TsukiGate.generated.h"

class UStaticMeshComponent;

/** A gate whose door slides by OpenOffset while active. Used in Area 1 and the canopy shortcut. */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiGate : public ATsukiMechanism
{
	GENERATED_BODY()

public:
	ATsukiGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<UStaticMeshComponent> Frame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<UStaticMeshComponent> Door;

	/** Door offset when open, relative to its closed position. Default sinks it into the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate")
	FVector OpenOffset = FVector(0.f, 0.f, -420.f);

	/** cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate", meta = (ClampMin = "1.0"))
	float OpenSpeed = 260.f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void HandleActivated() override { bOpen = true; }
	virtual void HandleDeactivated() override { bOpen = false; }

private:
	FVector ClosedLocation = FVector::ZeroVector;
	bool bOpen = false;
};
