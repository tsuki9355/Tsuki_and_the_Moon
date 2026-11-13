#include "World/ToolPickup.h"
#include "Player/TsukiCharacter.h"
#include "Core/TsukiProgressSubsystem.h"
#include "UI/TsukiHUD.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AToolPickup::AToolPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	Reach = CreateDefaultSubobject<USphereComponent>(TEXT("Reach"));
	Reach->InitSphereRadius(160.f);
	Reach->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = Reach;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetMobility(EComponentMobility::Movable);
}

bool AToolPickup::IsAlreadyCollected() const
{
	const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this);
	if (!Progress)
	{
		return false;
	}
	return bIsMoonFragment ? Progress->IsSolved(CollectId) : Progress->HasTool(Tool);
}

void AToolPickup::BeginPlay()
{
	Super::BeginPlay();
	MeshRest = Mesh->GetRelativeLocation();
	if (IsAlreadyCollected())
	{
		bCollected = true;
		ShowCollected(true);
	}
	SetActorTickEnabled(bAnimateIdle && !bCollected);
}

void AToolPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bCollected && bAnimateIdle)
	{
		const float T = GetWorld()->GetTimeSeconds();
		Mesh->SetRelativeLocation(MeshRest + FVector(0.f, 0.f, FMath::Sin(T * 2.f) * BobHeight));
		Mesh->AddLocalRotation(FRotator(0.f, 45.f * DeltaSeconds, 0.f));
	}
}

void AToolPickup::ShowCollected(bool bInstant)
{
	Mesh->SetVisibility(false, true);
}

void AToolPickup::Collect(ATsukiCharacter* Collector)
{
	if (bCollected || !Collector)
	{
		return;
	}
	bCollected = true;
	ShowCollected(false);

	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		if (bIsMoonFragment)
		{
			Progress->AddMoonFragment();
			Progress->MarkSolved(CollectId);
		}
		else
		{
			Progress->GrantTool(Tool);
		}
	}
	if (PickupSound)
	{
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}
	Collector->PlayItemGet(Tool);
	if (ATsukiHUD* HUD = ATsukiHUD::Get(this))
	{
		if (!FairyLine.IsEmpty())
		{
			HUD->ShowFairyLine(FairyLine, FairyLineDuration);
		}
	}
	OnCollected(Collector);
}
