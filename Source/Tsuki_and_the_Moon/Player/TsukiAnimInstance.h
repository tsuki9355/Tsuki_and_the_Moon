#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "TsukiAnimInstance.generated.h"

class UAnimSequenceBase;
class ATsukiCharacter;

/** What Tsuki's body is doing; picked every frame from gameplay state. */
UENUM(BlueprintType)
enum class ETsukiAnimState : uint8
{
	Idle, Walk, Run,
	Jump, Fall,
	Push, Pull, GrabIdle,
	CarryIdle, CarryWalk, CarryRun,
	SwimIdle, Swim,
	Aim, AimWalk,
	HookPull
};

/**
 * Evaluates the graph, then pins the root bone's scale to the reference pose. Tsuki's Mixamo/Blender
 * skeleton keeps a x100 scale on Hips; clips exported by the IK Retargeter write scale 1 there,
 * which would shrink her. Pinning makes old and retargeted clips agree, including mid-blend.
 */
USTRUCT()
struct FTsukiAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FTsukiAnimInstanceProxy() = default;
	explicit FTsukiAnimInstanceProxy(UAnimInstance* InAnimInstance) : FAnimInstanceProxy(InAnimInstance) {}

	virtual bool Evaluate_WithRoot(FPoseContext& Output, FAnimNode_Base* InRootNode) override;
};

/**
 * Drives Tsuki's animation from C++. The animation blueprint only needs one node:
 * Slot 'DefaultSlot' -> Output Pose. Looping states and one-shot actions are played as
 * dynamic montages in that slot, so every change cross-fades.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API UTsukiAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// --- Locomotion ---
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Locomotion") TObjectPtr<UAnimSequenceBase> Idle;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Locomotion") TObjectPtr<UAnimSequenceBase> Walk;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Locomotion") TObjectPtr<UAnimSequenceBase> Run;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Locomotion") TObjectPtr<UAnimSequenceBase> Jump;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Locomotion") TObjectPtr<UAnimSequenceBase> Fall;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Locomotion") TObjectPtr<UAnimSequenceBase> Land;

	// --- Objects ---
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Objects") TObjectPtr<UAnimSequenceBase> Push;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Objects") TObjectPtr<UAnimSequenceBase> Pull;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Objects") TObjectPtr<UAnimSequenceBase> CarryWalk;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Objects") TObjectPtr<UAnimSequenceBase> CarryRun;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Objects") TObjectPtr<UAnimSequenceBase> PickUp;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Objects") TObjectPtr<UAnimSequenceBase> Throw;

	// --- Water / tools ---
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Water") TObjectPtr<UAnimSequenceBase> SwimIdle;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Water") TObjectPtr<UAnimSequenceBase> Swim;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tools") TObjectPtr<UAnimSequenceBase> Aim;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tools") TObjectPtr<UAnimSequenceBase> AimWalk;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tools") TObjectPtr<UAnimSequenceBase> HookPull;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tools") TObjectPtr<UAnimSequenceBase> ItemGet;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tools") TObjectPtr<UAnimSequenceBase> ClimbUp;

	/** Ground speed above which Run replaces Walk (cm/s). */
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float RunThreshold = 330.f;

	/** Ground speeds the Walk/Run clips were authored at, used to match play rate to speed. */
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float WalkClipSpeed = 160.f;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float RunClipSpeed = 480.f;

	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float BlendTime = 0.18f;

	UPROPERTY(BlueprintReadOnly, Category = "Anims") ETsukiAnimState State = ETsukiAnimState::Idle;

	/** Play a one-shot action (throw, pick up, item-get, climb); looping states resume after it. */
	UFUNCTION(BlueprintCallable, Category = "Anims")
	void PlayAction(UAnimSequenceBase* Action, float PlayRate = 1.f, float MaxDuration = 0.f);

	/** Play the Land clip on touchdown. Off by default: a straight blend from fall to idle/walk reads cleaner. */
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") bool bPlayLandClip = true;

	/** Landing after falling at least this long plays the Land clip (briefly). */
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float LandAfterFallTime = 0.35f;
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float LandDuration = 0.35f;

	/** On the ground, the furthest (cm) a clip may lower her hips below rest before the mesh is lifted. */
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float MaxHipDip = 25.f;

	/** On the ground, the furthest (cm) a clip may raise her hips above rest before the mesh is lowered. */
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float MaxHipRise = 10.f;

	/** Seconds to ease the in-air vertical hip lock on/off. */
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") float AirZBlendTime = 0.15f;

	/**
	 * Keep clips "in place": cancel the hips' horizontal travel by offsetting the mesh each frame.
	 * Tsuki's skeleton has no separate root bone, so clips that walk forward would otherwise drift.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Anims|Tuning") bool bKeepInPlace = true;

	UTsukiAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativePostEvaluateAnimation() override;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return new FTsukiAnimInstanceProxy(this); }

private:
	ETsukiAnimState PickState() const;
	UAnimSequenceBase* SequenceFor(ETsukiAnimState InState) const;
	float PlayRateFor(ETsukiAnimState InState) const;
	void StartLoop(ETsukiAnimState InState);

	TWeakObjectPtr<ATsukiCharacter> Tsuki;
	float FallTime = 0.f;
	float AirZAlpha = 0.f;
	bool ActionIsClimb = false;
	bool bActionInterruptible = false;
	bool bAirborneNow = false;
	FVector MeshBaseLocation = FVector::ZeroVector;
	bool bHaveMeshBase = false;
	TWeakObjectPtr<UAnimSequenceBase> CurrentLoop;
	float ActionTimeLeft = 0.f;
	bool bWasFalling = false;
	bool bLoopStarted = false;
};
