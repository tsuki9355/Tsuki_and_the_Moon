#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrackedSurface.generated.h"

class UStaticMeshComponent;
class USoundBase;

/**
 * A clearly marked cracked surface that a Moonlit Bomb breaks open (GDD §7).
 * Remembered in progress by BreakId so it stays open.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ACrackedSurface : public AActor
{
	GENERATED_BODY()

public:
	ACrackedSurface();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cracked")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cracked")
	FName BreakId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cracked")
	TObjectPtr<USoundBase> BreakSound;

	/**
	 * Optional art mesh. It is stretched to fill the collision box exactly (after ArtRotation, which
	 * should be a multiple of 90 degrees), so the art always matches what blocks the player.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cracked|Art")
	TObjectPtr<UStaticMesh> Art;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cracked|Art")
	FRotator ArtRotation = FRotator::ZeroRotator;

	/** Hide the graybox box when art is set (it still blocks). Off = art sits on a visible slab. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cracked|Art")
	bool bHideBoxWithArt = true;

	/** Grow the art past the box on each side (cm), e.g. Z = 1 so art on a visible slab doesn't flicker. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cracked|Art")
	FVector ArtPadding = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cracked|Art")
	TObjectPtr<UStaticMeshComponent> ArtVisual;

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Cracked")
	void Break();

	UFUNCTION(BlueprintPure, Category = "Cracked")
	bool IsBroken() const { return bBroken; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Cracked")
	void OnBroken();

protected:
	virtual void BeginPlay() override;

private:
	void ApplyBroken();
	bool bBroken = false;
};
