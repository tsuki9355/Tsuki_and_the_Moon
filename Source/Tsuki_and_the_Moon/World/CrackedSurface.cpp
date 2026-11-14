#include "World/CrackedSurface.h"
#include "Core/TsukiProgressSubsystem.h"
#include "Properties/TsukiPropertySettings.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ACrackedSurface::ACrackedSurface()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	RootComponent = Mesh;
}

void ACrackedSurface::BeginPlay()
{
	Super::BeginPlay();
	// Cracked marking: warm amber glow on the overlay so it reads as "bomb me" from a distance.
	if (UMaterialInterface* Overlay = UTsukiPropertySettings::Get()->HighlightOverlayMaterial.LoadSynchronous())
	{
		Mesh->SetOverlayMaterial(Overlay);
	}
	Mesh->SetCustomPrimitiveDataVector4(0, FVector4(1.f, 0.55f, 0.15f, 0.8f));
	if (const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		if (Progress->IsSolved(BreakId))
		{
			bBroken = true;
			ApplyBroken();
		}
	}
}

void ACrackedSurface::Break()
{
	if (bBroken)
	{
		return;
	}
	bBroken = true;
	ApplyBroken();
	if (BreakSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BreakSound, GetActorLocation());
	}
	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		Progress->MarkSolved(BreakId);
	}
	OnBroken();
}

void ACrackedSurface::ApplyBroken()
{
	Mesh->SetVisibility(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
