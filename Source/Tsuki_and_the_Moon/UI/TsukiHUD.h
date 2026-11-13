#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TsukiHUD.generated.h"

/**
 * Minimal HUD (GDD §2): aim reticle, carried property, contextual prompt, Moon Fragment count,
 * and the fairy's line. Canvas-drawn placeholder; a UMG skin can replace it in M4.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API ATsukiHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** Show a fairy line at the bottom of the screen. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowFairyLine(const FText& Line, float Duration);

	static ATsukiHUD* Get(const UObject* WorldContext);

	virtual void DrawHUD() override;

private:
	void DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale);
	void DrawPanel(float X, float Y, float W, float H, const FLinearColor& Color);

	FText FairyLine;
	float FairyLineEndTime = 0.f;
};
