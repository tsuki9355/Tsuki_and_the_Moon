#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

class UBoxComponent;
class UArrowComponent;

/**
 * Walking through sets Tsuki's respawn point (GDD §2). Optionally marks a puzzle solved,
 * e.g. a checkpoint just past the blocking stone marks the stone puzzle done for hints.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ACheckpoint : public AActor
{
	GENERATED_BODY()

public:
	ACheckpoint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	TObjectPtr<UBoxComponent> Trigger;

	/** Respawn location and facing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	TObjectPtr<UArrowComponent> SpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	FName MarkSolvedOnEnter;

protected:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

/** Falling into this returns Tsuki to her last checkpoint. Use under gaps and water. */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiFallVolume : public AActor
{
	GENERATED_BODY()

public:
	ATsukiFallVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fall")
	TObjectPtr<UBoxComponent> Trigger;

protected:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
