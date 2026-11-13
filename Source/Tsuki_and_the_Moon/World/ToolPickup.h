#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Properties/MoonPropertyTypes.h"
#include "ToolPickup.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class USoundBase;
class ATsukiCharacter;

/**
 * A tool resting in the world (the scissors' resting place; later the Hook and Bomb chests).
 * Interact in reach to collect: grants the tool, plays the item-get moment and one fairy line (GDD §4).
 */
UCLASS()
class TSUKI_AND_THE_MOON_API AToolPickup : public AActor
{
	GENERATED_BODY()

public:
	AToolPickup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USphereComponent> Reach;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (EditCondition = "!bIsMoonFragment"))
	ETsukiTool Tool = ETsukiTool::Scissors;

	/** Gives a Moon Fragment instead of a tool (optional chests, GDD §7). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bIsMoonFragment = false;

	/** Unique id remembered in progress so a collected fragment stays collected. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (EditCondition = "bIsMoonFragment"))
	FName CollectId;

	/** Bob and spin while waiting (off for chests). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bAnimateIdle = true;

	/** Prompt verb shown in the HUD. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FText PromptText = NSLOCTEXT("Tsuki", "PickUp", "Pick up");

	/** The fairy's single explanation line after pickup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (MultiLine = true))
	FText FairyLine;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0.5"))
	float FairyLineDuration = 5.f;

	/** Item-get jingle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USoundBase> PickupSound;

	/** Gentle idle bob so it reads as special. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	float BobHeight = 8.f;

	UFUNCTION(BlueprintPure, Category = "Pickup")
	bool CanBeCollected() const { return !bCollected; }

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void Collect(ATsukiCharacter* Collector);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnCollected(ATsukiCharacter* Collector);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	/** Visual change once collected. Pickups vanish; chests open their lid. */
	virtual void ShowCollected(bool bInstant);

	bool IsAlreadyCollected() const;

	FVector MeshRest = FVector::ZeroVector;
	bool bCollected = false;
};
