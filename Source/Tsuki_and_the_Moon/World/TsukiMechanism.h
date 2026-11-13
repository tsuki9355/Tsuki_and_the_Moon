#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TsukiMechanism.generated.h"

class UCameraComponent;
class USoundBase;

/**
 * Something a pressure plate drives: gate, pond sluice, hidden path. On its first activation it can
 * play a short reveal (GDD §4): the camera cuts to RevealCamera, the reveal sting plays, the mechanism
 * moves, and control returns after RevealDuration.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiMechanism : public AActor
{
	GENERATED_BODY()

public:
	ATsukiMechanism();

	/** Position this in the level to frame the result of the reveal. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mechanism")
	TObjectPtr<UCameraComponent> RevealCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanism|Reveal")
	bool bPlayRevealOnFirstActivate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanism|Reveal", meta = (ClampMin = "0.5"))
	float RevealDuration = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanism|Reveal", meta = (ClampMin = "0.0"))
	float RevealBlendTime = 0.35f;

	/** Short puzzle-solved sting played with the reveal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanism|Reveal")
	TObjectPtr<USoundBase> RevealSound;

	UFUNCTION(BlueprintCallable, Category = "Mechanism")
	void SetMechanismActive(bool bActive);

	UFUNCTION(BlueprintPure, Category = "Mechanism")
	bool IsMechanismActive() const { return bIsActive; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Mechanism")
	void OnMechanismActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Mechanism")
	void OnMechanismDeactivated();

protected:
	/** Subclasses start moving here. */
	virtual void HandleActivated() {}
	virtual void HandleDeactivated() {}

private:
	void BeginReveal();
	void FinishReveal();
	void ApplyActivated();

	bool bIsActive = false;
	bool bRevealPlayed = false;
	FTimerHandle RevealTimer;
	FTimerHandle ActivateTimer;
};
