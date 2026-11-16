#pragma once

#include "CoreMinimal.h"
#include "World/ToolPickup.h"
#include "TsukiChest.generated.h"

/**
 * A treasure chest: the Moonthread Hook chest, the Moonlit Bomb chest and the Moon Fragment chests.
 * Interact to open; the lid swings up and the contents are granted with the item-get moment.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiChest : public AToolPickup
{
	GENERATED_BODY()

public:
	ATsukiChest();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest")
	TObjectPtr<UStaticMeshComponent> Lid;

	/** Lid pitch when open (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
	float OpenAngle = -110.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest", meta = (ClampMin = "1.0"))
	float OpenSpeed = 220.f;

	/**
	 * Optional art: chest body and lid meshes from an asset pack. Their pivots can sit anywhere, so they
	 * are re-centred automatically: the body's bottom-centre lands on the (hidden) collision box's
	 * bottom-centre, and the lid swings about its back-bottom edge. When unset the graybox cubes show.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest|Art")
	TObjectPtr<UStaticMesh> BodyArt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest|Art")
	TObjectPtr<UStaticMesh> LidArt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest|Art", meta = (ClampMin = "0.01"))
	float ArtScale = 0.8f;

	/** Extra yaw for the art so its front faces the same way as the chest actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest|Art")
	float ArtYaw = 0.f;

	/** Hinge on the lid's +Y edge (true) or -Y edge (false), in the art's own space. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest|Art")
	bool bHingeOnPositiveY = true;

	/** Editor only: show the lid open, to check the hinge. Ignored in play. */
	UPROPERTY(EditAnywhere, Category = "Chest|Art")
	bool bPreviewOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest|Art")
	TObjectPtr<UStaticMeshComponent> BodyVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest|Art")
	TObjectPtr<USceneComponent> LidHinge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest|Art")
	TObjectPtr<UStaticMeshComponent> LidVisual;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void ShowCollected(bool bInstant) override;

private:
	bool UsesArt() const { return BodyArt && LidArt; }
	void ApplyLidAngle();

	float LidPitch = 0.f;
	bool bOpening = false;
};
