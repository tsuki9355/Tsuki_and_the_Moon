#pragma once

#include "CoreMinimal.h"
#include "World/ToolPickup.h"
#include "TsukiChest.generated.h"

/**
 * A treasure chest: the Moonthread Hook chest, the Moonlit Bomb chest and the Moon Fragment chests.
 * Interact to open; the lid swings up and the contents are granted with the item-get moment.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiChest : public AToolPickup
{
	GENERATED_BODY()

public:
	ATsukiChest();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest")
	TObjectPtr<UStaticMeshComponent> Lid;

	/** Lid pitch when open (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
	float OpenAngle = -110.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest", meta = (ClampMin = "1.0"))
	float OpenSpeed = 220.f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void ShowCollected(bool bInstant) override;

private:
	float LidPitch = 0.f;
	bool bOpening = false;
};
