#include "World/CrackedSurface.h"
#include "Core/TsukiProgressSubsystem.h"
#include "Properties/TsukiPropertySettings.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

ACrackedSurface::ACrackedSurface()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	RootComponent = Mesh;

	ArtVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArtVisual"));
	ArtVisual->SetupAttachment(Mesh);
	ArtVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArtVisual->SetUsingAbsoluteLocation(true);
	ArtVisual->SetUsingAbsoluteRotation(true);
	ArtVisual->SetUsingAbsoluteScale(true);
}

void ACrackedSurface::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ArtVisual->SetStaticMesh(Art);
	ArtVisual->SetVisibility(Art != nullptr);
	Mesh->SetVisibility(!(Art && bHideBoxWithArt), false);
	if (!Art)
	{
		return;
	}

	// Target: the collision box in world space (assumes the actor is only yawed, as the level is built).
	const FBox Target = Mesh->CalcBounds(Mesh->GetComponentTransform()).GetBox().ExpandBy(ArtPadding);
	const FBox ArtBox = Art->GetBoundingBox();
	const FVector ArtSize = ArtBox.GetSize().ComponentMax(FVector(0.01f));
	const FQuat Rot = FQuat(GetActorRotation()) * FQuat(ArtRotation);

	// Which world axis does each of the art's axes end up on? Scale that art axis to fill it.
	FVector Scale(1.f);
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		FVector Dir(0.f);
		Dir[Axis] = 1.f;
		const FVector W = Rot.RotateVector(Dir).GetAbs();
		const int32 WorldAxis = (W.X >= W.Y && W.X >= W.Z) ? 0 : (W.Y >= W.Z ? 1 : 2);
		Scale[Axis] = Target.GetSize()[WorldAxis] / ArtSize[Axis];
	}
	ArtVisual->SetWorldRotation(Rot);
	ArtVisual->SetWorldScale3D(Scale);
	// Put the art's bounds centre on the target's centre.
	ArtVisual->SetWorldLocation(Target.GetCenter() - Rot.RotateVector(ArtBox.GetCenter() * Scale));
}

void ACrackedSurface::BeginPlay()
{
	Super::BeginPlay();
	// Cracked marking: warm amber glow on the overlay so it reads as "bomb me" from a distance.
	if (UMaterialInterface* Overlay = UTsukiPropertySettings::Get()->HighlightOverlayMaterial.LoadSynchronous())
	{
		Mesh->SetOverlayMaterial(Overlay);
		ArtVisual->SetOverlayMaterial(Overlay);
	}
	Mesh->SetCustomPrimitiveDataVector4(0, FVector4(1.f, 0.55f, 0.15f, 0.8f));
	ArtVisual->SetCustomPrimitiveDataVector4(0, FVector4(1.f, 0.55f, 0.15f, 0.8f));
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
	ArtVisual->SetVisibility(false);
}
