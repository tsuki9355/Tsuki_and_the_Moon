#include "Properties/PropertyActor.h"
#include "Properties/PropertyHolderComponent.h"
#include "Components/StaticMeshComponent.h"

APropertyActor::APropertyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Mesh->SetGenerateOverlapEvents(true);
	RootComponent = Mesh;

	Holder = CreateDefaultSubobject<UPropertyHolderComponent>(TEXT("Holder"));
}
