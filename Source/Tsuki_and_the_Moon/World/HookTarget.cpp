#include "World/HookTarget.h"
#include "Properties/TsukiPropertySettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"

AHookTarget::AHookTarget()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
	Marker->SetupAttachment(RootComponent);
	Marker->SetCollisionProfileName(TEXT("NoCollision"));

	Landing = CreateDefaultSubobject<UArrowComponent>(TEXT("Landing"));
	Landing->SetupAttachment(RootComponent);
	Landing->SetRelativeLocation(FVector(120.f, 0.f, 0.f));
}

void AHookTarget::BeginPlay()
{
	Super::BeginPlay();
	MarkerBaseScale = Marker->GetRelativeScale3D();
	RefreshVisual();
}

void AHookTarget::SetHookEnabled(bool bEnabled)
{
	if (bHookEnabled != bEnabled)
	{
		bHookEnabled = bEnabled;
		RefreshVisual();
		OnHookEnabledChanged(bEnabled);
	}
}

void AHookTarget::SetAimed(bool bAimed)
{
	if (bIsAimed != bAimed)
	{
		bIsAimed = bAimed;
		RefreshVisual();
	}
}

FVector AHookTarget::GetAnchorLocation() const
{
	return Marker->GetComponentLocation();
}

FVector AHookTarget::GetLandingLocation() const
{
	return Landing->GetComponentLocation();
}

void AHookTarget::RefreshVisual()
{
	// High-contrast moon-gold marker; brighter and larger when aimed, dim when switched off.
	const FLinearColor Gold(1.f, 0.85f, 0.35f);
	const float Intensity = !bHookEnabled ? 0.f : (bIsAimed ? 3.f : 1.f);
	if (UMaterialInterface* Overlay = UTsukiPropertySettings::Get()->HighlightOverlayMaterial.LoadSynchronous())
	{
		Marker->SetOverlayMaterial(Overlay);
	}
	Marker->SetCustomPrimitiveDataVector4(0, FVector4(Gold.R, Gold.G, Gold.B, Intensity));
	Marker->SetRelativeScale3D(MarkerBaseScale * (bIsAimed && bHookEnabled ? 1.25f : 1.f));
	Marker->SetVisibility(bHookEnabled);
}
