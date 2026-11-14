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
