#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HookComponent.generated.h"

class AHookTarget;
class UStaticMeshComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHookEvent, AHookTarget*, Target);

/**
 * The Moonthread Hook (GDD §7), with a hookshot feel: the reticle locks onto a marked target in a
 * forgiving cone, the thread fires out straight, then pulls Tsuki fast to the anchor and she
 * pulls herself up onto the landing.
 */
UCLASS(ClassGroup = (Tsuki), meta = (BlueprintSpawnableComponent))
class TSUKI_AND_THE_MOON_API UHookComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHookComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hook", meta = (ClampMin = "100.0"))
	float Range = 1800.f;

	/** Half-angle of the aim cone (degrees). Larger is more forgiving. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hook", meta = (ClampMin = "1.0", ClampMax = "45.0"))
	float AimConeDegrees = 12.f;

	/** How fast the thread flies out (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hook", meta = (ClampMin = "100.0"))
	float ShootSpeed = 5000.f;

	/** How fast Tsuki is pulled (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hook", meta = (ClampMin = "100.0"))
	float PullSpeed = 2200.f;

	/** Seconds for the final climb from the anchor onto the landing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hook", meta = (ClampMin = "0.05"))
	float MantleTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hook")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hook")
	TObjectPtr<USoundBase> LatchSound;

	UFUNCTION(BlueprintCallable, Category = "Hook")
	void SetHookEnabled(bool bEnabled) { bHookEnabled = bEnabled; }

	UFUNCTION(BlueprintPure, Category = "Hook")
	bool IsHookEnabled() const { return bHookEnabled; }

	/** True while firing, pulling or climbing up; movement input is ignored. */
	UFUNCTION(BlueprintPure, Category = "Hook")
	bool IsBusy() const { return State != EState::Idle; }

	/** Best target in the aim cone, or null. */
	UFUNCTION(BlueprintPure, Category = "Hook")
	AHookTarget* GetAimedTarget() const { return AimedTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Hook")
	bool Fire();

	/** Abort (respawn, item-get). */
	UFUNCTION(BlueprintCallable, Category = "Hook")
	void Cancel();

	UPROPERTY(BlueprintAssignable, Category = "Hook")
	FOnHookEvent OnLatched;

	UPROPERTY(BlueprintAssignable, Category = "Hook")
	FOnHookEvent OnArrived;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	enum class EState : uint8 { Idle, Firing, Pulling, Mantling };

	AHookTarget* FindTarget() const;
	void UpdateAim();
	void UpdateThread(const FVector& Tip);
	void Finish();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Thread;

	TWeakObjectPtr<AHookTarget> AimedTarget;
	TWeakObjectPtr<AHookTarget> ActiveTarget;
	EState State = EState::Idle;
	FVector TipLocation = FVector::ZeroVector;
	FVector MantleStart = FVector::ZeroVector;
	float StateTime = 0.f;
	float LastDistance = 0.f;
	float StuckTime = 0.f;
	bool bHookEnabled = false;
};
