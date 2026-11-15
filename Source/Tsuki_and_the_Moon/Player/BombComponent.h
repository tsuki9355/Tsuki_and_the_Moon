#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BombComponent.generated.h"

class ATsukiBomb;

/**
 * Moonlit Bombs (GDD §7): unlimited, one at a time. Press Bomb to take one out and hold it overhead
 * (the fuse is already running), press again to toss it a short way in front.
 */
UCLASS(ClassGroup = (Tsuki), meta = (BlueprintSpawnableComponent))
class TSUKI_AND_THE_MOON_API UBombComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBombComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	TSubclassOf<ATsukiBomb> BombClass;

	/** Toss speed forward and up (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	float TossForward = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	float TossUp = 320.f;

	/** Seconds into the throw animation when the bomb leaves her hands. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	float ThrowReleaseDelay = 0.22f;

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void SetBombsEnabled(bool bEnabled) { bBombsEnabled = bEnabled; }

	UFUNCTION(BlueprintPure, Category = "Bomb")
	bool AreBombsEnabled() const { return bBombsEnabled; }

	UFUNCTION(BlueprintPure, Category = "Bomb")
	bool IsHoldingBomb() const;

	/** Take out a bomb, or toss the one being held. */
	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void UseBomb();

	/** Set the held bomb down just in front of Tsuki (fuse keeps running). */
	UFUNCTION(BlueprintCallable, Category = "Bomb")
	bool PlaceBomb();

private:
	TWeakObjectPtr<ATsukiBomb> Held;
	TWeakObjectPtr<ATsukiBomb> Active;
	bool bBombsEnabled = false;
};
