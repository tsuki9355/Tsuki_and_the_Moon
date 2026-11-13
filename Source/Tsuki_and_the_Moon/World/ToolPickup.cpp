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

void AToolPickup::BeginPlay()
{
	Super::BeginPlay();
	MeshRest = Mesh->GetRelativeLocation();
	if (const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		if (Progress->HasTool(Tool))
		{
			bCollected = true;
			Mesh->SetVisibility(false, true);
		}
	}
}

void AToolPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bCollected)
	{
		const float T = GetWorld()->GetTimeSeconds();
		Mesh->SetRelativeLocation(MeshRest + FVector(0.f, 0.f, FMath::Sin(T * 2.f) * BobHeight));
		Mesh->AddLocalRotation(FRotator(0.f, 45.f * DeltaSeconds, 0.f));
	}
}

void AToolPickup::Collect(ATsukiCharacter* Collector)
{
	if (bCollected || !Collector)
	{
		return;
	}
	bCollected = true;
	Mesh->SetVisibility(false, true);
	SetActorTickEnabled(false);

	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		Progress->GrantTool(Tool);
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
