#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrabComponent.generated.h"

class UPropertyHolderComponent;
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabEvent, UPropertyHolderComponent*, Object);

/** What Interact would do with the object in front of Tsuki. */
UENUM(BlueprintType)
enum class EGrabPrompt : uint8
{
	None,
	Grab,      // push / pull a block
	PickUp,    // lift a small object
	TooHeavy,
	Pushing,
	PutDown
};

/**
 * Push/pull and carry (GDD §2, §6 Area 1).
 * - Pushable blocks: hold Interact facing one; Tsuki snaps to it and moves it slowly along that axis.
 * - Carryable small objects: press Interact to lift it overhead and walk freely; press again to set it down.
 * Objects heavier than the limits refuse; this is how Lightness "enables push/pull".
 */
UCLASS(ClassGroup = (Tsuki), meta = (BlueprintSpawnableComponent))
class TSUKI_AND_THE_MOON_API UGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGrabComponent();

	/** Heaviest effective weight Tsuki can push or pull. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = "0.0"))
	float MaxGrabWeight = 150.f;

	/** Heaviest effective weight Tsuki can lift. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = "0.0"))
	float MaxCarryWeight = 150.f;

	/** How far in front of Tsuki's capsule to look for a grabbable object. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = "10.0"))
	float GrabReach = 60.f;

	/** Steady push/pull speed (cm/s). Slow and weighty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = "10.0"))
	float PushSpeed = 140.f;

	/** Walk speed multiplier while carrying overhead. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float CarrySpeedScale = 0.8f;

	/** Gap between the top of Tsuki's capsule and the bottom of a carried object. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = "0.0"))
	float CarryHeightOffset = 15.f;

	/** Grabbable or carryable object in front of Tsuki, or null. */
	UFUNCTION(BlueprintPure, Category = "Grab")
	UPropertyHolderComponent* FindGrabbable(FVector& OutNormal) const;

	/** What pressing Interact would do right now (for the HUD). */
	UFUNCTION(BlueprintPure, Category = "Grab")
	EGrabPrompt GetPrompt() const;

	/** Interact pressed: put down if carrying, else pick up or start pushing. */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	bool TryGrab();

	/** Interact released: ends push/pull (carrying continues until the next press). */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	void Release();

	/** Set the carried object down in front of Tsuki. Returns false if there is no room. */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	bool PutDown();

	/** Toss the carried object forward; it flies with physics, then settles where it lands. */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	bool Throw();

	/** Throw speed forward / up (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	float ThrowForward = 550.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	float ThrowUp = 350.f;

	/** Drop whatever is held without checks (respawn, item-get). */
	void ForceReleaseAll();

	UFUNCTION(BlueprintPure, Category = "Grab")
	bool IsGrabbing() const { return Grabbed.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Grab")
	bool IsCarrying() const { return Carried.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Grab")
	FVector GetGrabAxis() const { return GrabAxis; }

	UPROPERTY(BlueprintAssignable, Category = "Grab")
	FOnGrabEvent OnGrabStarted;

	UPROPERTY(BlueprintAssignable, Category = "Grab")
	FOnGrabEvent OnGrabReleased;

	UPROPERTY(BlueprintAssignable, Category = "Grab")
	FOnGrabEvent OnPickedUp;

	UPROPERTY(BlueprintAssignable, Category = "Grab")
	FOnGrabEvent OnPutDown;

	/** Object was too heavy: play a shudder / strain reaction. */
	UPROPERTY(BlueprintAssignable, Category = "Grab")
	FOnGrabEvent OnGrabRefused;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	ACharacter* GetCharacter() const;
	bool StartPush(UPropertyHolderComponent* Holder, const FVector& Normal);
	bool StartCarry(UPropertyHolderComponent* Holder);
	bool FindPutDownLocation(FVector& OutLocation) const;

	TWeakObjectPtr<UPropertyHolderComponent> Grabbed;
	TWeakObjectPtr<UPropertyHolderComponent> Carried;
	TWeakObjectPtr<UPrimitiveComponent> Thrown;
	float ThrownTime = 0.f;
	TEnumAsByte<ECollisionEnabled::Type> CarriedSavedCollision = ECollisionEnabled::QueryAndPhysics;
	FVector GrabAxis = FVector::ForwardVector;
	FVector LastOwnerLocation = FVector::ZeroVector;
	float SavedMaxWalkSpeed = 0.f;
	bool bSavedOrientToMovement = true;
};
