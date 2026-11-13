#include "Properties/TsukiPropertySettings.h"

namespace
{
	FMoonPropertyInfo MakeInfo(const TCHAR* Name, const FLinearColor& Color, float WeightMultiplier, float TimeScale)
	{
		FMoonPropertyInfo Info;
		Info.DisplayName = FText::FromString(Name);
		Info.Color = Color;
		Info.WeightMultiplier = WeightMultiplier;
		Info.TimeScale = TimeScale;
		return Info;
	}
}

UTsukiPropertySettings::UTsukiPropertySettings()
{
	// Defaults; DefaultGame.ini overrides them. Colors are kept away from the forest greens and browns.
	// Lightness x0.05 lets a 400 stone (=20) fall under the 150 grab limit; Heaviness x20 lets a 5 pebble (=100) pass an 80 plate.
	Properties.Add(EMoonProperty::Lightness, MakeInfo(TEXT("Lightness"), FLinearColor(0.30f, 0.90f, 1.00f), 0.05f, 1.f));
	Properties.Add(EMoonProperty::Heaviness, MakeInfo(TEXT("Heaviness"), FLinearColor(0.95f, 0.20f, 0.55f), 20.f, 1.f));
	Properties.Add(EMoonProperty::Buoyancy, MakeInfo(TEXT("Buoyancy"), FLinearColor(0.20f, 0.40f, 1.00f), 1.f, 1.f));
	Properties.Add(EMoonProperty::Slowness, MakeInfo(TEXT("Slowness"), FLinearColor(1.00f, 0.60f, 0.10f), 1.f, 0.25f));
}

const FMoonPropertyInfo& UTsukiPropertySettings::GetInfo(EMoonProperty Property) const
{
	static const FMoonPropertyInfo Neutral;
	const FMoonPropertyInfo* Found = Properties.Find(Property);
	return Found ? *Found : Neutral;
}
