#include "World/FairyGuide.h"
#include "Core/TsukiProgressSubsystem.h"
#include "UI/TsukiHUD.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFairyGuide::AFairyGuide()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	LeadPath = CreateDefaultSubobject<USplineComponent>(TEXT("LeadPath"));
	LeadPath->SetupAttachment(RootComponent);

	Body = CreateDefaultSubobject<USceneComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Body->SetUsingAbsoluteLocation(true);

	Sprite = CreateDefaultSubobject<USceneComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(Body);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Sprite);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.15f));
	BodyMesh->SetCastShadow(false);
	if (SphereMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(SphereMesh.Object);
	}

	WingUpperL = MakeWing(TEXT("WingUpperL"));
	WingUpperR = MakeWing(TEXT("WingUpperR"));
	WingLowerL = MakeWing(TEXT("WingLowerL"));
	WingLowerR = MakeWing(TEXT("WingLowerR"));
	for (UStaticMeshComponent* Wing : { WingUpperL.Get(), WingUpperR.Get(), WingLowerL.Get(), WingLowerR.Get() })
	{
		if (SphereMesh.Succeeded())
		{
			Wing->SetStaticMesh(SphereMesh.Object);
		}
	}
	PoseWings(0.f);

	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(Body);
	Glow->SetIntensity(800.f);
	Glow->SetAttenuationRadius(400.f);
	Glow->SetLightColor(FLinearColor(0.75f, 0.6f, 1.f));
	Glow->SetCastShadows(false);

	OpeningCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OpeningCamera"));
	OpeningCamera->SetupAttachment(RootComponent);
	OpeningCamera->SetRelativeLocation(FVector(0.f, 0.f, 600.f));
	OpeningCamera->SetRelativeRotation(FRotator(-70.f, 0.f, 0.f));
}

UStaticMeshComponent* AFairyGuide::MakeWing(const TCHAR* Name)
{
	UStaticMeshComponent* Wing = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Wing->SetupAttachment(Sprite);
	Wing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Wing->SetCastShadow(false);
	return Wing;
}

void AFairyGuide::PoseWings(float Flap)
{
	// Each wing is a slender flattened sphere; pose it as if hinged at its root on the core's back.
	struct FWingPose { UStaticMeshComponent* Wing; float Side; float RootZ; float Length; float Height; float Lift; float Sweep; };
	const FWingPose Poses[] = {
		{ WingUpperL, -1.f,  3.f, 16.f, 6.f,  28.f, 25.f },
		{ WingUpperR,  1.f,  3.f, 16.f, 6.f,  28.f, 25.f },
		{ WingLowerL, -1.f, -2.f, 11.f, 4.5f, -18.f, 40.f },
		{ WingLowerR,  1.f, -2.f, 11.f, 4.5f, -18.f, 40.f },
	};
	for (const FWingPose& P : Poses)
	{
		if (!P.Wing)
		{
			continue;
		}
		// Lift raises the wing tip, sweep swings it backwards; the flap beats around the root.
		const float Lift = P.Lift + Flap;
		const FRotator Rot(0.f, -P.Side * P.Sweep, P.Side * Lift);
		const FVector Root(-3.f, P.Side * 4.f, P.RootZ);
		const FVector Out = Rot.RotateVector(FVector(0.f, P.Side * P.Length * 0.5f, 0.f));
		P.Wing->SetRelativeLocation(Root + Out);
		P.Wing->SetRelativeRotation(Rot);
		P.Wing->SetRelativeScale3D(FVector(0.012f, P.Length / 100.f, P.Height / 100.f));
	}
}

void AFairyGuide::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (CoreMaterial)
	{
		BodyMesh->SetMaterial(0, CoreMaterial);
	}
	for (UStaticMeshComponent* Wing : { WingUpperL.Get(), WingUpperR.Get(), WingLowerL.Get(), WingLowerR.Get() })
	{
		if (Wing && WingMaterial)
		{
			Wing->SetMaterial(0, WingMaterial);
		}
	}
	PoseWings(0.f);
}

void AFairyGuide::BeginPlay()
{
	Super::BeginPlay();
	LeadPath->UpdateSpline(); // Points may be authored by tools that skip the editor's spline refresh.
	Body->SetWorldLocation(LeadPath->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World) + FVector(0.f, 0.f, 120.f));

	const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this);
	const bool bAlreadyStarted = Progress && Progress->HasTool(ETsukiTool::Scissors);
	if (bPlayOpening && !bAlreadyStarted)
	{
		// Wait one frame so the player pawn exists.
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (!PC || !PC->GetPawn())
			{
				GiveControl();
				return;
			}
			PC->GetPawn()->DisableInput(PC);
			PC->SetViewTarget(this);
			GetWorldTimerManager().SetTimer(OpeningTimer, this, &AFairyGuide::EndOpeningShot, FMath::Max(OpeningHold, 0.01f), false);
		});
	}
}

void AFairyGuide::EndOpeningShot()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		PC->SetViewTargetWithBlend(PC->GetPawn(), OpeningBlend, VTBlend_EaseInOut, 2.f);
	}
	GetWorldTimerManager().SetTimer(OpeningTimer, this, &AFairyGuide::GiveControl, FMath::Max(OpeningBlend, 0.01f), false);
}

void AFairyGuide::GiveControl()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		PC->GetPawn()->EnableInput(PC);
	}
	NextGreeting = 0;
	ShowNextGreeting();
}

void AFairyGuide::ShowNextGreeting()
{
	if (!GreetingLines.IsValidIndex(NextGreeting))
	{
		return;
	}
	if (ATsukiHUD* HUD = ATsukiHUD::Get(this))
	{
		HUD->ShowFairyLine(GreetingLines[NextGreeting], GreetingLineDuration);
	}
	++NextGreeting;
	GetWorldTimerManager().SetTimer(GreetingTimer, this, &AFairyGuide::ShowNextGreeting, GreetingLineDuration, false);
}

FVector AFairyGuide::ComputeDesiredLocation(const APawn* Tsuki) const
{
	const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this);
	if (Progress && Progress->HasTool(ETsukiTool::Scissors))
	{
		// Companion mode: hover at Tsuki's shoulder.
		return Tsuki->GetActorLocation() + Tsuki->GetActorRotation().RotateVector(FollowOffset);
	}

	// Lead mode: stay LeadDistance ahead of Tsuki along the path, never moving backwards.
	const float Length = LeadPath->GetSplineLength();
	const float TsukiKey = LeadPath->FindInputKeyClosestToWorldLocation(Tsuki->GetActorLocation());
	const float TsukiDist = LeadPath->GetDistanceAlongSplineAtSplineInputKey(TsukiKey);
	const float Dist = FMath::Clamp(FMath::Max(PathProgress, TsukiDist + LeadDistance), 0.f, Length);
	return LeadPath->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World) + FVector(0.f, 0.f, 120.f);
}

void AFairyGuide::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	const APawn* Tsuki = PC ? PC->GetPawn() : nullptr;
	if (!Tsuki)
	{
		return;
	}

	const FVector Desired = ComputeDesiredLocation(Tsuki);
	if (const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		if (!Progress->HasTool(ETsukiTool::Scissors))
		{
			const float Key = LeadPath->FindInputKeyClosestToWorldLocation(Desired);
			PathProgress = FMath::Max(PathProgress, LeadPath->GetDistanceAlongSplineAtSplineInputKey(Key));
		}
	}

	const float T = GetWorld()->GetTimeSeconds();
	const FVector Bob(0.f, 0.f, FMath::Sin(T * 3.f) * 10.f);
	const FVector Prev = Body->GetComponentLocation();
	const FVector Next = FMath::VInterpTo(Prev, Desired, DeltaSeconds, 3.f);
	Body->SetWorldLocation(FMath::VInterpConstantTo(Prev, Next, DeltaSeconds, FlySpeed));

	// Face the way it flies; when hovering, turn to look at Tsuki.
	const FVector Move = Body->GetComponentLocation() - Prev;
	const FVector Look = Move.SizeSquared2D() > FMath::Square(20.f * DeltaSeconds) ? Move : (Tsuki->GetActorLocation() - Body->GetComponentLocation());
	if (!Look.IsNearlyZero())
	{
		FacingYaw = FMath::FixedTurn(FacingYaw, Look.Rotation().Yaw, 360.f * DeltaSeconds);
	}
	Sprite->SetWorldRotation(FRotator(0.f, FacingYaw, 0.f));
	Sprite->SetRelativeLocation(Bob);
	PoseWings(FMath::Sin(T * FlapRate * UE_TWO_PI) * FlapAngle);
}
