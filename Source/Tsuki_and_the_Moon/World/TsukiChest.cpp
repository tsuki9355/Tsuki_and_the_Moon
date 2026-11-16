#include "World/TsukiChest.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ATsukiChest::ATsukiChest()
{
	bAnimateIdle = false;
	PromptText = NSLOCTEXT("Tsuki", "OpenChest", "Open");

	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	Lid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lid"));
	Lid->SetupAttachment(Mesh);
	Lid->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Lid->SetMobility(EComponentMobility::Movable);

	BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyVisual"));
	BodyVisual->SetupAttachment(RootComponent);
	BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyVisual->SetMobility(EComponentMobility::Movable);

	LidHinge = CreateDefaultSubobject<USceneComponent>(TEXT("LidHinge"));
	LidHinge->SetupAttachment(RootComponent);
	LidHinge->SetMobility(EComponentMobility::Movable);

	LidVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidVisual"));
	LidVisual->SetupAttachment(LidHinge);
	LidVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LidVisual->SetMobility(EComponentMobility::Movable);
}

void ATsukiChest::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const bool bArt = UsesArt();
	Mesh->SetVisibility(!bArt, false); // The cube keeps blocking the player either way.
	Lid->SetVisibility(!bArt, false);
	BodyVisual->SetVisibility(bArt);
	LidVisual->SetVisibility(bArt);
	if (!bArt)
	{
		BodyVisual->SetStaticMesh(nullptr);
		LidVisual->SetStaticMesh(nullptr);
		return;
	}

	// Bottom-centre of the collision cube, in root (actor) space.
	const FBox Cube = Mesh->CalcBounds(Mesh->GetComponentTransform()).GetBox();
	const FVector BottomWorld(Cube.GetCenter().X, Cube.GetCenter().Y, Cube.Min.Z);
	const FVector Bottom = RootComponent->GetComponentTransform().InverseTransformPosition(BottomWorld);

	const FRotator ArtRot(0.f, ArtYaw, 0.f);
	const float S = ArtScale;

	const FBox BodyBox = BodyArt->GetBoundingBox();
	const FVector BodyAnchor(BodyBox.GetCenter().X, BodyBox.GetCenter().Y, BodyBox.Min.Z);
	const FVector BodyLoc = Bottom - ArtRot.RotateVector(BodyAnchor * S);
	BodyVisual->SetStaticMesh(BodyArt);
	BodyVisual->SetRelativeScale3D(FVector(S));
	BodyVisual->SetRelativeRotation(ArtRot);
	BodyVisual->SetRelativeLocation(BodyLoc);

	const FBox LidBox = LidArt->GetBoundingBox();
	const FVector Hinge(LidBox.GetCenter().X, bHingeOnPositiveY ? LidBox.Max.Y : LidBox.Min.Y, LidBox.Min.Z);
	LidHinge->SetRelativeLocation(BodyLoc + ArtRot.RotateVector(Hinge * S));
	LidVisual->SetStaticMesh(LidArt);
	LidVisual->SetRelativeScale3D(FVector(S));
	LidVisual->SetRelativeLocation(-Hinge * S);
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		LidPitch = bPreviewOpen ? OpenAngle : 0.f;
	}
	ApplyLidAngle();
}

void ATsukiChest::ApplyLidAngle()
{
	if (UsesArt())
	{
		// The art's hinge runs along its X axis, so the lid opens with roll about the hinge.
		LidHinge->SetRelativeRotation(FRotator(0.f, ArtYaw, 0.f) + FRotator(0.f, 0.f, bHingeOnPositiveY ? -LidPitch : LidPitch));
	}
	else
	{
		FRotator Rot = Lid->GetRelativeRotation();
		Rot.Pitch = LidPitch;
		Lid->SetRelativeRotation(Rot);
	}
}

void ATsukiChest::ShowCollected(bool bInstant)
{
	bOpening = true;
	if (bInstant)
	{
		LidPitch = OpenAngle;
		ApplyLidAngle();
		return;
	}
	SetActorTickEnabled(true);
}

void ATsukiChest::Tick(float DeltaSeconds)
{
	AActor::Tick(DeltaSeconds); // Skip the pickup bob.
	if (!bOpening)
	{
		return;
	}
	LidPitch = FMath::FInterpConstantTo(LidPitch, OpenAngle, DeltaSeconds, OpenSpeed);
	ApplyLidAngle();
	if (FMath::IsNearlyEqual(LidPitch, OpenAngle, 0.1f))
	{
		SetActorTickEnabled(false);
	}
}
