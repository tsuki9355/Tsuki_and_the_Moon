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

AFairyGuide::AFairyGuide()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	LeadPath = CreateDefaultSubobject<USplineComponent>(TEXT("LeadPath"));
	LeadPath->SetupAttachment(RootComponent);

	Body = CreateDefaultSubobject<USceneComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Body->SetUsingAbsoluteLocation(true);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Body);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.15f));

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
	const FVector Next = FMath::VInterpTo(Body->GetComponentLocation(), Desired, DeltaSeconds, 3.f);
	Body->SetWorldLocation(FMath::VInterpConstantTo(Body->GetComponentLocation(), Next, DeltaSeconds, FlySpeed));
	BodyMesh->SetRelativeLocation(Bob);
}
