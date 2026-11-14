#include "Player/TsukiCharacter.h"
#include "Player/ScissorsComponent.h"
#include "Player/GrabComponent.h"
#include "Player/HookComponent.h"
#include "Player/BombComponent.h"
#include "Properties/PropertyHolderComponent.h"
#include "Core/TsukiProgressSubsystem.h"
#include "World/FairyHintVolume.h"
#include "World/ToolPickup.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATsukiCharacter::ATsukiCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(38.f, 92.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Quick, grounded, forgiving movement.
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 720.f, 0.f);
	Move->MaxWalkSpeed = 520.f;
	Move->MinAnalogWalkSpeed = 40.f;
	Move->MaxAcceleration = 2600.f;
	Move->BrakingDecelerationWalking = 2600.f;
	Move->GroundFriction = 10.f;
	Move->JumpZVelocity = 560.f;
	Move->AirControl = 0.45f;
	Move->GravityScale = 1.6f;
	Move->BrakingDecelerationFalling = 1000.f;
	Move->MaxStepHeight = 40.f;
	Move->PerchRadiusThreshold = 12.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.f;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Scissors = CreateDefaultSubobject<UScissorsComponent>(TEXT("Scissors"));
	Grab = CreateDefaultSubobject<UGrabComponent>(TEXT("Grab"));
	Hook = CreateDefaultSubobject<UHookComponent>(TEXT("Hook"));
	Bombs = CreateDefaultSubobject<UBombComponent>(TEXT("Bombs"));

	Weight = CreateDefaultSubobject<UPropertyHolderComponent>(TEXT("Weight"));
	Weight->BaseWeight = 60.f;
	Weight->bTargetable = false;
	Weight->bShowGlow = false;
}

void ATsukiCharacter::BeginPlay()
{
	Super::BeginPlay();
	StartTransform = GetActorTransform();
	DefaultArmLength = CameraBoom->TargetArmLength;
	DefaultSocketOffset = CameraBoom->SocketOffset;
	DefaultFOV = FollowCamera->FieldOfView;

	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		Scissors->SetScissorsEnabled(Progress->HasTool(ETsukiTool::Scissors));
		Hook->SetHookEnabled(Progress->HasTool(ETsukiTool::MoonthreadHook));
		Bombs->SetBombsEnabled(Progress->HasTool(ETsukiTool::MoonlitBombs));
		Progress->OnToolGranted.AddDynamic(this, &ATsukiCharacter::HandleToolGranted);
	}
}

void ATsukiCharacter::HandleToolGranted(ETsukiTool Tool)
{
	if (Tool == ETsukiTool::Scissors)
	{
		Scissors->SetScissorsEnabled(true);
	}
	else if (Tool == ETsukiTool::MoonthreadHook)
	{
		Hook->SetHookEnabled(true);
	}
	else if (Tool == ETsukiTool::MoonlitBombs)
	{
		Bombs->SetBombsEnabled(true);
	}
}

void ATsukiCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		LastGroundedTime = GetWorld()->GetTimeSeconds();
		bCoyoteAvailable = true;
	}

	if (bSwimming)
	{
		TickSwimming(DeltaSeconds);
	}
	UpdateAimCamera(DeltaSeconds);

	if (bRecentering && Controller)
	{
		FRotator Control = Controller->GetControlRotation();
		const float TargetYaw = GetActorRotation().Yaw;
		const float Speed = 180.f / FMath::Max(RecenterTime, 0.05f);
		Control.Yaw = FMath::FixedTurn(Control.Yaw, TargetYaw, Speed * DeltaSeconds);
		Control.Pitch = FMath::FInterpTo(FRotator::NormalizeAxis(Control.Pitch), -15.f, DeltaSeconds, 8.f);
		Controller->SetControlRotation(Control);
		if (FMath::IsNearlyEqual(FRotator::NormalizeAxis(Control.Yaw - TargetYaw), 0.f, 1.f))
		{
			bRecentering = false;
		}
	}
}

bool ATsukiCharacter::CanJumpInternal_Implementation() const
{
	if ((Grab && Grab->IsGrabbing()) || bSwimming)
	{
		return false;
	}
	if (Super::CanJumpInternal_Implementation())
	{
		return true;
	}
	// Coyote time: a jump just after stepping off a ledge still counts.
	const UCharacterMovementComponent* Move = GetCharacterMovement();
	return bCoyoteAvailable && Move->IsFalling() && GetVelocity().Z <= 0.f
		&& (GetWorld()->GetTimeSeconds() - LastGroundedTime) <= CoyoteTime;
}

void ATsukiCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	bCoyoteAvailable = false;
}

void ATsukiCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	Respawn(); // Never destroy Tsuki; return her nearby instead (GDD section 2).
}

// --- Swimming ---

void ATsukiCharacter::UpdateWater(float SurfaceZ)
{
	WaterSurfaceZ = SurfaceZ;
	LastWaterTime = GetWorld()->GetTimeSeconds();
	if (bSwimming || Hook->IsBusy())
	{
		return;
	}
	const float FeetZ = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	if (SurfaceZ - FeetZ > SwimStartDepth)
	{
		StartSwimming();
	}
}

void ATsukiCharacter::StartSwimming()
{
	bSwimming = true;
	Grab->Release();
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->SetMovementMode(MOVE_Flying);
	Move->MaxFlySpeed = SwimSpeed;
	Move->BrakingDecelerationFlying = 900.f;
	Move->Velocity.Z = 0.f;
}

void ATsukiCharacter::StopSwimming()
{
	if (!bSwimming)
	{
		return;
	}
	bSwimming = false;
	if (!Hook->IsBusy())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
}

void ATsukiCharacter::TickSwimming(float DeltaSeconds)
{
	if (Hook->IsBusy())
	{
		bSwimming = false; // The hook takes over movement; swimming resumes if she lands in water.
		return;
	}
	UCharacterMovementComponent* Move = GetCharacterMovement();
	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Loc = GetActorLocation();

	// Left the pond, or the water drained/shallowed under her: back to walking.
	FHitResult Floor;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SwimFloor), false, this);
	const bool bShallow = GetWorld()->LineTraceSingleByChannel(Floor, Loc, Loc - FVector(0.f, 0.f, HalfHeight + 60.f), ECC_Visibility, Params)
		&& (WaterSurfaceZ - Floor.ImpactPoint.Z) < SwimStartDepth - 20.f; // Hysteresis vs. SwimStartDepth.
	if (GetWorld()->GetTimeSeconds() - LastWaterTime > 0.15f || bShallow)
	{
		StopSwimming();
		return;
	}

	// Bob at the surface with the head above water.
	// Positional, not velocity-driven: flying braking would eat a small vertical velocity at low frame rates.
	const float TargetZ = WaterSurfaceZ - SwimFloatDepth;
	Move->Velocity.Z = 0.f;
	const float NewZ = FMath::FInterpTo(Loc.Z, TargetZ, DeltaSeconds, 6.f);
	AddActorWorldOffset(FVector(0.f, 0.f, NewZ - Loc.Z), true);

	if (!Move->GetCurrentAcceleration().IsNearlyZero())
	{
		TryClimbOut();
	}
}

bool ATsukiCharacter::TryClimbOut()
{
	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector Loc = GetActorLocation();
	const FVector Fwd = GetActorForwardVector();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ClimbOut), false, this);

	// Something in front at chest height, or right at the waterline (thin rims and overhanging edges)?
	FHitResult Wall;
	const FVector AtWaterline(Loc.X, Loc.Y, WaterSurfaceZ + 5.f);
	if (!GetWorld()->LineTraceSingleByChannel(Wall, Loc, Loc + Fwd * (Radius + 40.f), ECC_Visibility, Params)
		&& !GetWorld()->LineTraceSingleByChannel(Wall, AtWaterline, AtWaterline + Fwd * (Radius + 40.f), ECC_Visibility, Params))
	{
		return false;
	}
	// Find its top, no higher than ClimbOutHeight above the water.
	const FVector Over = Wall.ImpactPoint + Fwd * (Radius + 10.f);
	FHitResult Top;
	if (!GetWorld()->LineTraceSingleByChannel(Top, FVector(Over.X, Over.Y, WaterSurfaceZ + ClimbOutHeight + 5.f), FVector(Over.X, Over.Y, WaterSurfaceZ - 60.f), ECC_Visibility, Params))
	{
		return false;
	}
	if (Top.bStartPenetrating || Top.ImpactNormal.Z < 0.7f || Top.ImpactPoint.Z > WaterSurfaceZ + ClimbOutHeight)
	{
		return false;
	}
	const FVector Dest(Over.X, Over.Y, Top.ImpactPoint.Z + HalfHeight + 2.f);
	if (!SetActorLocation(Dest, false, nullptr, ETeleportType::TeleportPhysics))
	{
		return false;
	}
	bSwimming = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	return true;
}

void ATsukiCharacter::Respawn()
{
	Grab->Release();
	Hook->Cancel();
	StopSwimming();
	FTransform Target = StartTransform;
	if (const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		Progress->GetRespawnTransform(Target);
	}
	GetCharacterMovement()->StopMovementImmediately();
	TeleportTo(Target.GetLocation(), Target.Rotator(), false, true);
	if (Controller)
	{
		Controller->SetControlRotation(FRotator(-15.f, Target.Rotator().Yaw, 0.f));
	}
	OnRespawned();
}

void ATsukiCharacter::PlayItemGet(ETsukiTool Tool)
{
	Grab->Release();
	GetCharacterMovement()->StopMovementImmediately();
	bInputLocked = true;
	OnItemGet(Tool);
	GetWorldTimerManager().SetTimer(ItemGetTimer, this, &ATsukiCharacter::EndItemGet, FMath::Max(ItemGetDuration, 0.01f), false);
}

void ATsukiCharacter::EndItemGet()
{
	bInputLocked = false;
}

AToolPickup* ATsukiCharacter::FindPickupInReach() const
{
	TArray<AActor*>& Overlaps = PickupOverlapCache; // Reused; the HUD calls this every frame.
	Overlaps.Reset();
	GetOverlappingActors(Overlaps, AToolPickup::StaticClass());
	for (AActor* Actor : Overlaps)
	{
		AToolPickup* Pickup = Cast<AToolPickup>(Actor);
		if (Pickup && Pickup->CanBeCollected())
		{
			return Pickup;
		}
	}
	return nullptr;
}

void ATsukiCharacter::SetActiveHintVolume(AFairyHintVolume* Volume, bool bEntered)
{
	if (bEntered)
	{
		ActiveHintVolume = Volume;
	}
	else if (ActiveHintVolume.Get() == Volume)
	{
		ActiveHintVolume = nullptr;
	}
}

// --- Input ---

void ATsukiCharacter::BuildDefaultInput()
{
	auto MakeAction = [this](const TCHAR* Name, EInputActionValueType Type)
	{
		UInputAction* Action = NewObject<UInputAction>(this, Name);
		Action->ValueType = Type;
		return Action;
	};
	auto Swizzle = [](UObject* Outer) { return NewObject<UInputModifierSwizzleAxis>(Outer); };
	auto Negate = [](UObject* Outer, bool bX, bool bY)
	{
		UInputModifierNegate* N = NewObject<UInputModifierNegate>(Outer);
		N->bX = bX;
		N->bY = bY;
		N->bZ = false;
		return N;
	};

	if (!MoveAction) { MoveAction = MakeAction(TEXT("IA_Tsuki_Move"), EInputActionValueType::Axis2D); }
	if (!LookAction) { LookAction = MakeAction(TEXT("IA_Tsuki_Look"), EInputActionValueType::Axis2D); }
	if (!JumpAction) { JumpAction = MakeAction(TEXT("IA_Tsuki_Jump"), EInputActionValueType::Boolean); }
	if (!ScissorsAction) { ScissorsAction = MakeAction(TEXT("IA_Tsuki_Scissors"), EInputActionValueType::Boolean); }
	if (!InteractAction) { InteractAction = MakeAction(TEXT("IA_Tsuki_Interact"), EInputActionValueType::Boolean); }
	if (!ReturnAction) { ReturnAction = MakeAction(TEXT("IA_Tsuki_Return"), EInputActionValueType::Boolean); }
	if (!HintAction) { HintAction = MakeAction(TEXT("IA_Tsuki_Hint"), EInputActionValueType::Boolean); }
	if (!RecenterAction) { RecenterAction = MakeAction(TEXT("IA_Tsuki_Recenter"), EInputActionValueType::Boolean); }
	if (!HookAction) { HookAction = MakeAction(TEXT("IA_Tsuki_Hook"), EInputActionValueType::Boolean); }
	if (!BombAction) { BombAction = MakeAction(TEXT("IA_Tsuki_Bomb"), EInputActionValueType::Boolean); }

	UInputMappingContext* Ctx = NewObject<UInputMappingContext>(this, TEXT("IMC_Tsuki_Default"));

	// Move: WASD + left stick.
	Ctx->MapKey(MoveAction, EKeys::W).Modifiers.Add(Swizzle(Ctx));
	{
		FEnhancedActionKeyMapping& S = Ctx->MapKey(MoveAction, EKeys::S);
		S.Modifiers.Add(Swizzle(Ctx));
		S.Modifiers.Add(Negate(Ctx, true, true));
	}
	Ctx->MapKey(MoveAction, EKeys::D);
	Ctx->MapKey(MoveAction, EKeys::A).Modifiers.Add(Negate(Ctx, true, true));
	Ctx->MapKey(MoveAction, EKeys::Gamepad_Left2D).Modifiers.Add(NewObject<UInputModifierDeadZone>(Ctx));

	// Look: mouse + right stick (mouse Y inverted so pushing up looks up).
	Ctx->MapKey(LookAction, EKeys::Mouse2D).Modifiers.Add(Negate(Ctx, false, true));
	Ctx->MapKey(LookAction, EKeys::Gamepad_Right2D).Modifiers.Add(NewObject<UInputModifierDeadZone>(Ctx));

	Ctx->MapKey(JumpAction, EKeys::SpaceBar);
	Ctx->MapKey(JumpAction, EKeys::Gamepad_FaceButton_Bottom);
	Ctx->MapKey(ScissorsAction, EKeys::LeftMouseButton);
	Ctx->MapKey(ScissorsAction, EKeys::Gamepad_RightTrigger);
	Ctx->MapKey(InteractAction, EKeys::E);
	Ctx->MapKey(InteractAction, EKeys::Gamepad_FaceButton_Left);
	Ctx->MapKey(ReturnAction, EKeys::R);
	Ctx->MapKey(ReturnAction, EKeys::Gamepad_FaceButton_Top);
	Ctx->MapKey(HintAction, EKeys::Q);
	Ctx->MapKey(HintAction, EKeys::Gamepad_DPad_Up);
	Ctx->MapKey(RecenterAction, EKeys::MiddleMouseButton);
	Ctx->MapKey(RecenterAction, EKeys::Gamepad_LeftTrigger);
	Ctx->MapKey(HookAction, EKeys::RightMouseButton);
	Ctx->MapKey(HookAction, EKeys::Gamepad_RightShoulder);
	Ctx->MapKey(BombAction, EKeys::F);
	Ctx->MapKey(BombAction, EKeys::Gamepad_FaceButton_Right);

	MappingContext = Ctx;
}

void ATsukiCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	if (!MappingContext)
	{
		BuildDefaultInput();
	}
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

void ATsukiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!MappingContext)
	{
		BuildDefaultInput();
	}
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		return;
	}
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATsukiCharacter::Move);
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATsukiCharacter::Look);
	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ATsukiCharacter::StartJump);
	Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATsukiCharacter::StopJump);
	Input->BindAction(ScissorsAction, ETriggerEvent::Started, this, &ATsukiCharacter::UseScissors);
	Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ATsukiCharacter::InteractStarted);
	Input->BindAction(InteractAction, ETriggerEvent::Completed, this, &ATsukiCharacter::InteractCompleted);
	Input->BindAction(ReturnAction, ETriggerEvent::Started, this, &ATsukiCharacter::ReturnProperty);
	Input->BindAction(HintAction, ETriggerEvent::Started, this, &ATsukiCharacter::RequestHint);
	Input->BindAction(RecenterAction, ETriggerEvent::Started, this, &ATsukiCharacter::Recenter);
	Input->BindAction(HookAction, ETriggerEvent::Started, this, &ATsukiCharacter::FireHook);
	Input->BindAction(HookAction, ETriggerEvent::Completed, this, &ATsukiCharacter::ReleaseHook);
	Input->BindAction(BombAction, ETriggerEvent::Started, this, &ATsukiCharacter::UseBomb);
}

void ATsukiCharacter::Move(const FInputActionValue& Value)
{
	if (IsActionLocked() || !Controller)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();

	if (Grab->IsGrabbing())
	{
		// While grabbing, only movement along the grab axis counts (push / pull).
		const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector World = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X) * Axis.Y + FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y) * Axis.X;
		const float Along = FVector::DotProduct(World, Grab->GetGrabAxis());
		AddMovementInput(Grab->GetGrabAxis(), FMath::Abs(Along) > 0.3f ? FMath::Sign(Along) : 0.f);
		return;
	}

	const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

void ATsukiCharacter::Look(const FInputActionValue& Value)
{
	if (bInputLocked)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Axis.IsNearlyZero())
	{
		bRecentering = false;
	}
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ATsukiCharacter::StartJump()
{
	if (!IsActionLocked())
	{
		Jump();
	}
}

void ATsukiCharacter::StopJump()
{
	StopJumping();
}

void ATsukiCharacter::UseScissors()
{
	if (IsActionLocked() || Grab->IsGrabbing() || Hook->IsAiming())
	{
		return;
	}
	// Hands full: the same button throws what Tsuki is holding.
	if (Grab->IsCarrying())
	{
		Grab->Throw();
		return;
	}
	if (Bombs->IsHoldingBomb())
	{
		Bombs->UseBomb();
		return;
	}
	{
		Scissors->UseScissors();
	}
}

void ATsukiCharacter::InteractStarted()
{
	if (bInputLocked || Hook->IsAiming())
	{
		return;
	}
	if (Bombs->IsHoldingBomb())
	{
		Bombs->PlaceBomb();
		return;
	}
	if (!Grab->IsCarrying())
	{
		if (AToolPickup* Pickup = FindPickupInReach())
		{
			Pickup->Collect(this);
			return;
		}
	}
	Grab->TryGrab(); // Also puts down a carried object.
}

void ATsukiCharacter::InteractCompleted()
{
	Grab->Release();
}

void ATsukiCharacter::ReturnProperty()
{
	if (!bInputLocked)
	{
		Scissors->ReturnCarried();
	}
}

void ATsukiCharacter::RequestHint()
{
	if (AFairyHintVolume* Volume = ActiveHintVolume.Get())
	{
		Volume->RequestHint();
	}
}

void ATsukiCharacter::Recenter()
{
	bRecentering = true;
}

bool ATsukiCharacter::IsActionLocked() const
{
	return bInputLocked || (Hook && Hook->IsBusy()); // Aiming still allows walking/strafing.
}

void ATsukiCharacter::FireHook()
{
	// Hold to aim (camera zooms in), release to fire.
	if (!IsActionLocked() && !Grab->IsGrabbing() && !Grab->IsCarrying() && !Bombs->IsHoldingBomb() && !bSwimming)
	{
		Hook->StartAiming();
	}
}

void ATsukiCharacter::ReleaseHook()
{
	Hook->ReleaseAim();
}

void ATsukiCharacter::UpdateAimCamera(float DeltaSeconds)
{
	const bool bAim = Hook && Hook->IsAiming();
	// Ocarina-style hookshot aim: pull in over the shoulder, narrow the view, face the crosshair.
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, bAim ? AimArmLength : DefaultArmLength, DeltaSeconds, 10.f);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, bAim ? AimSocketOffset : DefaultSocketOffset, DeltaSeconds, 10.f);
	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, bAim ? AimFOV : DefaultFOV, DeltaSeconds, 10.f));
	bUseControllerRotationYaw = bAim;
	if (!Grab->IsGrabbing())
	{
		GetCharacterMovement()->bOrientRotationToMovement = !bAim;
	}
}

void ATsukiCharacter::UseBomb()
{
	if (!IsActionLocked() && !Grab->IsGrabbing() && !Grab->IsCarrying() && !bSwimming && !Hook->IsAiming())
	{
		Bombs->UseBomb();
	}
}
