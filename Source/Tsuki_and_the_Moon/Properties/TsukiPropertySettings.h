#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Materials/MaterialInterface.h"
#include "Properties/MoonPropertyTypes.h"
#include "TsukiPropertySettings.generated.h"

/**
 * Project-wide property tuning and identifiers (Project Settings > Game > Tsuki Properties).
 * Stored in Config/DefaultGame.ini so numbers stay out of code.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Tsuki Properties"))
class TSUKI_AND_THE_MOON_API UTsukiPropertySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UTsukiPropertySettings();

	/** Per-property color, weight multiplier, time scale, icon and sounds. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Properties")
	TMap<EMoonProperty, FMoonPropertyInfo> Properties;

	/** Neutral receiver glow (GDD §3: initially white). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Highlight")
	FLinearColor ReceiverColor = FLinearColor::White;

	/** Overlay material applied to sources and receivers. Reads custom primitive data 0-3 (RGB, intensity) and 4 (property index). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Highlight")
	TSoftObjectPtr<UMaterialInterface> HighlightOverlayMaterial;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Highlight", meta = (ClampMin = "0.0"))
	float IdleGlowIntensity = 0.5f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Highlight", meta = (ClampMin = "0.0"))
	float TargetedGlowIntensity = 2.f;

	/** Info for a property, or a neutral default if it is not configured. */
	const FMoonPropertyInfo& GetInfo(EMoonProperty Property) const;

	static const UTsukiPropertySettings* Get() { return GetDefault<UTsukiPropertySettings>(); }

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
