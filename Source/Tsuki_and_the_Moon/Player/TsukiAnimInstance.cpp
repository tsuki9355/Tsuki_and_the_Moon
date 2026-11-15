#include "Player/TsukiAnimInstance.h"
#include "Player/TsukiCharacter.h"
#include "Player/GrabComponent.h"
#include "Player/HookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimMontage.h"

namespace
{
	const FName TsukiAnimSlot(TEXT("DefaultSlot"));
}

bool FTsukiAnimInstanceProxy::Evaluate_WithRoot(FPoseContext& Output, FAnimNode_Base* InRootNode)
{
	EvaluateAnimationNode_WithRoot(Output, InRootNode);
	if (Output.Pose.GetNumBones() > 0)
	{
		const FCompactPoseBoneIndex Root(0);
		Output.Pose[Root].SetScale3D(Output.Pose.GetRefPose(Root).GetScale3D());
	}
	return true;
}

UTsukiAnimInstance::UTsukiAnimInstance()
{
	RootMotionMode = ERootMotionMode::IgnoreRootMotion; // The capsule drives movement, never the clips.
}

void UTsukiAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Tsuki = Cast<ATsukiCharacter>(TryGetPawnOwner());
}

void UTsukiAnimInstance::NativePostEvaluateAnimation()
{
	Super::NativePostEvaluateAnimation();
	USkeletalMeshComponent* SkMesh = GetSkelMeshComponent();
	if (!bKeepInPlace || !SkMesh || !SkMesh->GetSkinnedAsset() || !Tsuki.IsValid())
	{
		return;
	}
	if (!bHaveMeshBase)
	{
		MeshBaseLocation = SkMesh->GetRelativeLocation();
		bHaveMeshBase = true;
	}
	// Root bone (the hips) horizontal offset from its reference pose, in component space.
	const FReferenceSkeleton& RefSkel = SkMesh->GetSkinnedAsset()->GetRefSkeleton();
	if (RefSkel.GetNum() == 0 || SkMesh->GetComponentSpaceTransforms().Num() == 0)
	{
		return;
	}
	const FVector Current = SkMesh->GetComponentSpaceTransforms()[0].GetLocation();
	const FVector Ref = RefSkel.GetRefBonePose()[0].GetLocation();
	// In the air the capsule owns height: cancel the clip's vertical hip travel too (jump/fall clips
	// carry baked root motion), easing in and out so landing doesn't pop.
	const bool bAirborne = bAirborneNow || ActionIsClimb;
	const float Dt = GetDeltaSeconds();
	AirZAlpha = FMath::FInterpConstantTo(AirZAlpha, bAirborne ? 1.f : 0.f, Dt, 1.f / FMath::Max(AirZBlendTime, 0.01f));
	// On the ground, keep the clip's natural hip bob/crouch but never let the hips sink more than
	// MaxHipDip below rest (that is what pushes her feet through the floor on landings).
	const float DZ = Current.Z - Ref.Z;
	// Same for rising: the landing clip starts with her hips high in the air (baked fall), which
	// would lift her off the ground. Only the part of the hip travel outside the band is cancelled.
	const float GroundZ = DZ - FMath::Clamp(DZ, -MaxHipDip, MaxHipRise);
	const FVector Drift(Current.X - Ref.X, Current.Y - Ref.Y, FMath::Lerp(GroundZ, DZ, AirZAlpha));
	SkMesh->SetRelativeLocation(MeshBaseLocation - SkMesh->GetRelativeRotation().RotateVector(Drift));
}

ETsukiAnimState UTsukiAnimInstance::PickState() const
{
	const ATsukiCharacter* T = Tsuki.Get();
	if (!T)
	{
		return ETsukiAnimState::Idle;
	}
	const UCharacterMovementComponent* Move = T->GetCharacterMovement();
	const FVector Vel = T->GetVelocity();
	const float Speed = Vel.Size2D();

	if (T->Hook && T->Hook->IsLatched())
	{
		return ETsukiAnimState::HookPull;
	}
	if (T->IsSwimming())
	{
		return Speed > 40.f ? ETsukiAnimState::Swim : ETsukiAnimState::SwimIdle;
	}
	if (T->Grab && T->Grab->IsGrabbing())
	{
		const float Along = FVector::DotProduct(Vel, T->Grab->GetGrabAxis());
		return Along > 10.f ? ETsukiAnimState::Push : (Along < -10.f ? ETsukiAnimState::Pull : ETsukiAnimState::GrabIdle);
	}
	if (Move && Move->IsFalling())
	{
		return Vel.Z > 50.f ? ETsukiAnimState::Jump : ETsukiAnimState::Fall;
	}
	if (T->Grab && T->Grab->IsCarrying())
	{
		return Speed < 20.f ? ETsukiAnimState::CarryIdle : (Speed > RunThreshold * 0.8f ? ETsukiAnimState::CarryRun : ETsukiAnimState::CarryWalk);
	}
	if (T->Hook && T->Hook->IsAiming())
	{
		return Speed > 30.f ? ETsukiAnimState::AimWalk : ETsukiAnimState::Aim;
	}
	if (Speed < 20.f)
	{
		return ETsukiAnimState::Idle;
	}
	return Speed < RunThreshold ? ETsukiAnimState::Walk : ETsukiAnimState::Run;
}

UAnimSequenceBase* UTsukiAnimInstance::SequenceFor(ETsukiAnimState InState) const
{
	switch (InState)
	{
	case ETsukiAnimState::Walk: return Walk;
	case ETsukiAnimState::Run: return Run;
	// Jump and fall share the jump clip, held on its last frame, so there is no separate fall loop.
	case ETsukiAnimState::Jump:
	case ETsukiAnimState::Fall: return Jump ? Jump.Get() : Fall.Get();
	case ETsukiAnimState::Push: return Push;
	case ETsukiAnimState::Pull: return Pull ? Pull.Get() : Push.Get();
	case ETsukiAnimState::GrabIdle: return Push;
	case ETsukiAnimState::CarryIdle: return CarryWalk;
	case ETsukiAnimState::CarryWalk: return CarryWalk;
	case ETsukiAnimState::CarryRun: return CarryRun ? CarryRun.Get() : CarryWalk.Get();
	case ETsukiAnimState::SwimIdle: return SwimIdle;
	case ETsukiAnimState::Swim: return Swim ? Swim.Get() : SwimIdle.Get();
	case ETsukiAnimState::Aim: return Aim;
	case ETsukiAnimState::AimWalk: return AimWalk ? AimWalk.Get() : Aim.Get();
	case ETsukiAnimState::HookPull: return HookPull ? HookPull.Get() : Fall.Get();
	default: return Idle;
	}
}

float UTsukiAnimInstance::PlayRateFor(ETsukiAnimState InState) const
{
	const ATsukiCharacter* T = Tsuki.Get();
	const float Speed = T ? T->GetVelocity().Size2D() : 0.f;
	switch (InState)
	{
	case ETsukiAnimState::Walk: return FMath::Clamp(Speed / FMath::Max(WalkClipSpeed, 1.f), 0.6f, 1.6f);
	case ETsukiAnimState::Run: return FMath::Clamp(Speed / FMath::Max(RunClipSpeed, 1.f), 0.7f, 1.5f);
	case ETsukiAnimState::GrabIdle:
	case ETsukiAnimState::CarryIdle: return 0.f; // Hold the pose.
	default: return 1.f;
	}
}

void UTsukiAnimInstance::StartLoop(ETsukiAnimState InState)
{
	UAnimSequenceBase* Seq = SequenceFor(InState);
	if (!Seq)
	{
		return;
	}
	PlaySlotAnimationAsDynamicMontage(Seq, TsukiAnimSlot, BlendTime, BlendTime, FMath::Max(PlayRateFor(InState), 0.001f), 100000);
	CurrentLoop = Seq;
	bLoopStarted = true;
}

void UTsukiAnimInstance::PlayAction(UAnimSequenceBase* Action, float PlayRate, float MaxDuration)
{
	if (!Action)
	{
		return;
	}
	PlaySlotAnimationAsDynamicMontage(Action, TsukiAnimSlot, 0.12f, 0.2f, PlayRate, 1);
	ActionTimeLeft = FMath::Max(Action->GetPlayLength() / FMath::Max(PlayRate, 0.01f) - 0.2f, 0.05f);
	if (MaxDuration > 0.f)
	{
		ActionTimeLeft = FMath::Min(ActionTimeLeft, MaxDuration);
	}
	CurrentLoop = nullptr; // Resume the right loop when the action finishes.
	ActionIsClimb = (Action == ClimbUp);
	bActionInterruptible = (Action == PickUp || Action == Land);
}

void UTsukiAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Tsuki.IsValid())
	{
		Tsuki = Cast<ATsukiCharacter>(TryGetPawnOwner());
		if (!Tsuki.IsValid())
		{
			return;
		}
	}
	// Landing: a short settle when coming down from a real fall.
	const UCharacterMovementComponent* Move = Tsuki->GetCharacterMovement();
	const bool bFalling = Move && Move->IsFalling();
	if (bFalling)
	{
		FallTime += DeltaSeconds;
	}
	else
	{
		// Standing landings go straight back to idle; moving landings keep the land-into-walk blend.
		if (bPlayLandClip && Tsuki->GetVelocity().Size2D() > 60.f && FallTime > LandAfterFallTime && Land && Move && Move->IsMovingOnGround() && ActionTimeLeft <= 0.f)
		{
			PlayAction(Land, 1.5f, LandDuration);
			bActionInterruptible = true;
		}
		FallTime = 0.f;
	}
	bAirborneNow = bFalling;

	// Short one-shots (pick up, land) give way as soon as she starts moving, so she never glides.
	if (ActionTimeLeft > 0.f && bActionInterruptible && Tsuki->GetVelocity().Size2D() > 60.f)
	{
		ActionTimeLeft = 0.f;
	}
	if (ActionTimeLeft > 0.f)
	{
		ActionTimeLeft -= DeltaSeconds;
		return;
	}
	ActionIsClimb = false;
	bActionInterruptible = false;

	State = PickState();
	UAnimSequenceBase* Seq = SequenceFor(State);
	if (!bLoopStarted || Seq != CurrentLoop.Get())
	{
		StartLoop(State);
	}
	else if (UAnimMontage* Active = GetCurrentActiveMontage())
	{
		float Rate = PlayRateFor(State);
		// Hold the jump pose on its last frame instead of looping it.
		if ((State == ETsukiAnimState::Jump || State == ETsukiAnimState::Fall) && Seq
			&& Montage_GetPosition(Active) >= Seq->GetPlayLength() - 0.06f)
		{
			Rate = 0.f;
		}
		Montage_SetPlayRate(Active, FMath::Max(Rate, 0.001f));
	}
}
