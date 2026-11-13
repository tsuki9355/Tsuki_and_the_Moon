#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Properties/MoonPropertyTypes.h"
#include "TsukiCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UScissorsComponent;
class UGrabComponent;
class UPropertyHolderComponent;
class AFairyHintVolume;
class AToolPickup;
struct FInputActionValue;

/**
 * Tsuki: third-person movement with responsive, grounded feel (GDD Â§2), the Moon Scissors,
 * push/pull, interact, fairy hint requests and checkpoint respawn.
 * Input assets can be assigned in BP_Tsuki; if left empty, default keyboard/gamepad bindings are built in code.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATsukiCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tsuki")
	TObjectPtr<UScissorsComponent> Scissors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tsuki")
	TObjectPtr<UGrabComponent> Grab;

	/** Tsuki's own weight on pressure plates. Not a scissors target. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tsuki")
	TObjectPtr<UPropertyHolderComponent> Weight;

	// --- Feel tuning ---

	/** Seconds after leaving a ledge during which a jump still works. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Feel", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float CoyoteTime = 0.15f;

	/** Seconds to swing the camera behind Tsuki when recentering. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Feel", meta = (ClampMin = "0.05"))
	float RecenterTime = 0.25f;

	/** Seconds input is locked during the tool pickup (item-get) moment. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tsuki|Feel", meta = (ClampMin = "0.0"))
	float ItemGetDuration = 2.f;

	// --- Input (optional asset overrides) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ScissorsAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ReturnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> HintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RecenterAction;

	/** Return to the last checkpoint (or the start) after a fall. */
	UFUNCTION(BlueprintCallable, Category = "Tsuki")
	void Respawn();

	/** Tool pickup moment: brief input lock and a Blueprint hook for pose, jingle and camera. */
	UFUNCTION(BlueprintCallable, Category = "Tsuki")
	void PlayItemGet(ETsukiTool Tool);

	/** Nearest tool pickup in reach, for the prompt. */
	UFUNCTION(BlueprintPure, Category = "Tsuki")
	AToolPickup* FindPickupInReach() const;

	void SetActiveHintVolume(AFairyHintVolume* Volume, bool bEntered);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tsuki")
	void OnItemGet(ETsukiTool Tool);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tsuki")
	void OnRespawned();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;

private:
	void BuildDefaultInput();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void UseScissors();
	void InteractStarted();
	void InteractCompleted();
	void ReturnProperty();
	void RequestHint();
	void Recenter();
	void EndItemGet();

	UFUNCTION()
	void HandleToolGranted(ETsukiTool Tool);

	mutable TArray<AActor*> PickupOverlapCache;
	TWeakObjectPtr<AFairyHintVolume> ActiveHintVolume;
	FTransform StartTransform;
	float LastGroundedTime = -100.f;
	bool bCoyoteAvailable = false;
	bool bRecentering = false;
	bool bInputLocked = false;
	FTimerHandle ItemGetTimer;
};
