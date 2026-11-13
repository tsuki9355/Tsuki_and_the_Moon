#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PropertyActor.generated.h"

class UStaticMeshComponent;
class UPropertyHolderComponent;

/**
 * A movable mesh with a property holder: feather plant, blocking stone, boulder, pebble, crate.
 * Configure the mesh and the holder per Blueprint child or per placed instance.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API APropertyActor : public AActor
{
	GENERATED_BODY()

public:
	APropertyActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Property")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Property")
	TObjectPtr<UPropertyHolderComponent> Holder;
};
