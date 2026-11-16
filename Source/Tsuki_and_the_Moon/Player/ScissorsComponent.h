#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Properties/MoonPropertyTypes.h"
#include "ScissorsComponent.generated.h"

class UPropertyHolderComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCarriedPropertyChanged, EMoonProperty, Property);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScissorsTargetChanged, UPropertyHolderComponent*, Target, EScissorsAction, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScissorsUsed, EScissorsAction, Action, EMoonProperty, Property);

/**
 * The Moon Scissors (GDD §3). Aims from the camera, picks a valid target, and runs the
 * cut / attach / return loop with a single carried property. Disabled until Tsuki picks them up.
 */
UCLASS(ClassGroup = (Tsuki), meta = (BlueprintSpawnableComponent))
class TSUKI_AND_THE_MOON_API UScissorsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScissorsComponent();

	/** Max distance from Tsuki to a target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scissors", meta = (ClampMin = "100.0"))
	float TraceRange = 1500.f;

	/** Radius of the aim sweep; larger is more forgiving. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scissors", meta = (ClampMin = "0.0"))
	float TraceRadius = 35.f;

	/** Snip played on every cut and attach, unless the property has its own sound in Project Settings. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scissors|Audio")
	TObjectPtr<USoundBase> SnipSound;

	UFUNCTION(BlueprintCallable, Category = "Scissors")
	bool UseScissors();

	/** Returns the carried property to the object it came from (or drops it if that is gone). */
	UFUNCTION(BlueprintCallable, Category = "Scissors")
	bool ReturnCarried();

	UFUNCTION(BlueprintCallable, Category = "Scissors")
	void SetScissorsEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Scissors")
	bool IsScissorsEnabled() const { return bScissorsEnabled; }

	UFUNCTION(BlueprintPure, Category = "Scissors")
	EMoonProperty GetCarried() const { return Carried; }

	UFUNCTION(BlueprintPure, Category = "Scissors")
	EScissorsAction GetCurrentAction() const { return CurrentAction; }

	UFUNCTION(BlueprintPure, Category = "Scissors")
	UPropertyHolderComponent* GetTarget() const { return Target.Get(); }

	/** The property the current action would cut or attach. */
	UFUNCTION(BlueprintPure, Category = "Scissors")
	EMoonProperty GetActionProperty() const;

	UPROPERTY(BlueprintAssignable, Category = "Scissors")
	FOnCarriedPropertyChanged OnCarriedChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scissors")
	FOnScissorsTargetChanged OnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scissors")
	FOnScissorsUsed OnScissorsUsed;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateTarget();
	UPropertyHolderComponent* FindAimedHolder() const;
	EScissorsAction EvaluateAction(const UPropertyHolderComponent* Holder) const;
	void SetCarried(EMoonProperty NewCarried, UPropertyHolderComponent* Source);
	void PlayPropertySound(EMoonProperty Property, bool bCut) const;

	mutable TArray<FHitResult> AimHits;
	TWeakObjectPtr<UPropertyHolderComponent> Target;
	TWeakObjectPtr<UPropertyHolderComponent> CarriedSource;
	EScissorsAction CurrentAction = EScissorsAction::None;
	EMoonProperty Carried = EMoonProperty::None;
	bool bScissorsEnabled = false;
};
