#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Properties/MoonPropertyTypes.h"
#include "PropertyHolderComponent.generated.h"

class UTsukiPropertySettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoonPropertyChanged, EMoonProperty, Property, bool, bAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHolderTargetChanged, bool, bTargeted, EScissorsAction, Action);

/**
 * Lives on every scissors source and receiver. Holds the properties an object currently has,
 * which can be cut, which can be attached, and derives the object's effective weight and time scale.
 * Visual reactions (fronds drooping, glow) hook into OnPropertyChanged in Blueprint.
 */
UCLASS(ClassGroup = (Tsuki), meta = (BlueprintSpawnableComponent))
class TSUKI_AND_THE_MOON_API UPropertyHolderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPropertyHolderComponent();

	/** Weight before properties apply. Compared against grab limits and pressure plates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property", meta = (ClampMin = "0.0"))
	float BaseWeight = 100.f;

	/** Properties the object starts with. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TArray<EMoonProperty> InitialProperties;

	/** Which of the initial properties the scissors may cut (makes this object a source). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TArray<EMoonProperty> CuttableProperties;

	/** Properties that may be attached here (makes this object a receiver). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TArray<EMoonProperty> AcceptedProperties;

	/** Whether the scissors can aim at this object at all. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	bool bTargetable = true;

	/** Whether Tsuki can grab this object to push and pull it (still subject to the grab weight limit). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	bool bPushable = false;

	/** Small objects (the pebble): Tsuki lifts them overhead and walks freely, then puts them down. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	bool bCarryable = false;

	/** Floats in water without Buoyancy (the lily pad). Heaviness still sinks it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	bool bFloatsNaturally = false;

	/** Whether this object rides the water surface right now (Buoyancy or natural float, and not Heaviness). */
	UFUNCTION(BlueprintPure, Category = "Property")
	bool FloatsInWater() const
	{
		return (bFloatsNaturally || HasProperty(EMoonProperty::Buoyancy)) && !HasProperty(EMoonProperty::Heaviness);
	}

	/** Whether to apply the source/receiver glow overlay to the owner's meshes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	bool bShowGlow = true;

	/** Set while Tsuki is holding this object, so ponds and other systems leave it alone. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Property")
	bool bHeldByPlayer = false;

	UPROPERTY(BlueprintAssignable, Category = "Property")
	FOnMoonPropertyChanged OnPropertyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Property")
	FOnHolderTargetChanged OnTargetChanged;

	/** Copies the initial setup into runtime state. Called from BeginPlay; callable directly in tests. */
	void InitializeHolder();

	UFUNCTION(BlueprintPure, Category = "Property")
	bool HasProperty(EMoonProperty Property) const { return Properties.Contains(Property); }

	UFUNCTION(BlueprintPure, Category = "Property")
	bool CanCut(EMoonProperty Property) const;

	/** First property the scissors could cut here, or None. */
	UFUNCTION(BlueprintPure, Category = "Property")
	EMoonProperty GetFirstCuttable() const;

	UFUNCTION(BlueprintPure, Category = "Property")
	bool CanAttach(EMoonProperty Property) const;

	/** Removes a cuttable property. Returns false if it cannot be cut. */
	UFUNCTION(BlueprintCallable, Category = "Property")
	bool CutProperty(EMoonProperty Property);

	/** Attaches a property brought from Source. It becomes cuttable here so the transfer can be reversed. */
	UFUNCTION(BlueprintCallable, Category = "Property")
	bool AttachProperty(EMoonProperty Property, UPropertyHolderComponent* Source);

	/** Puts a property back on its original source, ignoring the accepted list. */
	UFUNCTION(BlueprintCallable, Category = "Property")
	bool RestoreProperty(EMoonProperty Property);

	/** Where a property attached here originally came from (itself if it started here). */
	UFUNCTION(BlueprintPure, Category = "Property")
	UPropertyHolderComponent* GetOriginalSource(EMoonProperty Property);

	UFUNCTION(BlueprintPure, Category = "Property")
	float GetEffectiveWeight() const;

	UFUNCTION(BlueprintPure, Category = "Property")
	float GetTimeScale() const;

	UFUNCTION(BlueprintPure, Category = "Property")
	bool IsSource() const { return GetFirstCuttable() != EMoonProperty::None; }

	UFUNCTION(BlueprintPure, Category = "Property")
	bool IsReceiver() const { return AcceptedProperties.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Property")
	const TArray<EMoonProperty>& GetProperties() const { return Properties; }

	/** Called by the scissors when this object becomes or stops being the aimed target. */
	void SetTargeted(bool bInTargeted, EScissorsAction Action);

	/** Base weight with Lightness/Heaviness applied. Both together cancel out. */
	static float ComputeEffectiveWeight(float Base, const TArray<EMoonProperty>& Props, const UTsukiPropertySettings& Settings);

	/** Slowest time scale among held properties (1 when none slow it). */
	static float ComputeTimeScale(const TArray<EMoonProperty>& Props, const UTsukiPropertySettings& Settings);

protected:
	virtual void BeginPlay() override;

private:
	void ApplyEffects();
	void RefreshGlow();

	UPROPERTY(VisibleInstanceOnly, Category = "Property")
	TArray<EMoonProperty> Properties;

	UPROPERTY(VisibleInstanceOnly, Category = "Property")
	TArray<EMoonProperty> CurrentCuttable;

	TMap<EMoonProperty, TWeakObjectPtr<UPropertyHolderComponent>> OriginalSources;

	bool bTargeted = false;
	EScissorsAction TargetAction = EScissorsAction::None;
};
