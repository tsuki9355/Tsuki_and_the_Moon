#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HookTarget.generated.h"

class UStaticMeshComponent;
class UArrowComponent;

/**
 * A marked Moonthread Hook anchor (GDD §7). The hook pulls Tsuki to Anchor, then she pulls herself
 * up to Landing. Can be switched on and off (the timed summit targets in Area 3).
 */
UCLASS()
class TSUKI_AND_THE_MOON_API AHookTarget : public AActor
{
	GENERATED_BODY()

public:
	AHookTarget();

	/** The visible high-contrast marker; the hook attaches here. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hook")
	TObjectPtr<UStaticMeshComponent> Marker;

	/** Where Tsuki's feet end up after the pull. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hook")
	TObjectPtr<UArrowComponent> Landing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hook")
	bool bHookEnabled = true;

	UFUNCTION(BlueprintCallable, Category = "Hook")
	void SetHookEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Hook")
	FVector GetAnchorLocation() const;

	UFUNCTION(BlueprintPure, Category = "Hook")
	FVector GetLandingLocation() const;

	/** Called by the hook when this becomes or stops being the aimed target. */
	void SetAimed(bool bAimed);

	/** Hide/show the marker only (warning blink before a timed target switches off). */
	void SetMarkerFlash(bool bVisible);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hook")
	void OnHookEnabledChanged(bool bEnabled);

protected:
	virtual void BeginPlay() override;

private:
	void RefreshVisual();

	FVector MarkerBaseScale = FVector::OneVector;
	bool bIsAimed = false;
};
