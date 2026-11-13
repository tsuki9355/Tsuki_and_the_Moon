#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundBase.h"
#include "MoonPropertyTypes.generated.h"

/** An abstract property the Moon Scissors can cut from one object and attach to another (GDD §3). */
UENUM(BlueprintType)
enum class EMoonProperty : uint8
{
	None,
	Lightness,
	Heaviness,
	Buoyancy,
	Slowness
};

/** What the scissors would do if used on the current target. */
UENUM(BlueprintType)
enum class EScissorsAction : uint8
{
	None,
	Cut,
	Attach
};

/** Tools Tsuki obtains during the slice (GDD §7). */
UENUM(BlueprintType)
enum class ETsukiTool : uint8
{
	Scissors,
	MoonthreadHook,
	MoonlitBombs
};

/** Presentation and tuning for one property. Edited in Project Settings > Game > Tsuki Properties. */
USTRUCT(BlueprintType)
struct FMoonPropertyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	FText DisplayName;

	/** Stable identifier color shared by source, carried and attached states. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	FLinearColor Color = FLinearColor::White;

	/** Multiplier on an object's base weight while it holds this property (1 = no effect). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property", meta = (ClampMin = "0.0"))
	float WeightMultiplier = 1.f;

	/** Time scale applied to an object holding this property (1 = no effect). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float TimeScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TSoftObjectPtr<USoundBase> CutSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TSoftObjectPtr<USoundBase> AttachSound;
};
