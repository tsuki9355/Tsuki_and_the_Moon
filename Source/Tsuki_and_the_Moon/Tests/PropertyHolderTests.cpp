#include "Misc/AutomationTest.h"
#include "Properties/PropertyHolderComponent.h"
#include "Properties/TsukiPropertySettings.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace TsukiTestFixtures
{
	constexpr float StoneBase = 400.f; // Area 1 blocking stone
	constexpr float PebbleBase = 5.f;  // Area 1 pebble

	UPropertyHolderComponent* MakeHolder(float Base, TArray<EMoonProperty> Initial, TArray<EMoonProperty> Cuttable, TArray<EMoonProperty> Accepted)
	{
		UPropertyHolderComponent* H = NewObject<UPropertyHolderComponent>();
		H->BaseWeight = Base;
		H->InitialProperties = MoveTemp(Initial);
		H->CuttableProperties = MoveTemp(Cuttable);
		H->AcceptedProperties = MoveTemp(Accepted);
		H->bShowGlow = false;
		H->InitializeHolder();
		return H;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTsukiHolderWeightTest, "Tsuki.Properties.HolderWeight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTsukiHolderWeightTest::RunTest(const FString& Parameters)
{
	using namespace TsukiTestFixtures;
	const UTsukiPropertySettings& Settings = *UTsukiPropertySettings::Get();
	const float LightMul = Settings.GetInfo(EMoonProperty::Lightness).WeightMultiplier;
	const float HeavyMul = Settings.GetInfo(EMoonProperty::Heaviness).WeightMultiplier;

	TestEqual(TEXT("No properties: base weight"), UPropertyHolderComponent::ComputeEffectiveWeight(StoneBase, {}, Settings), StoneBase);
	TestEqual(TEXT("Lightness scales"), UPropertyHolderComponent::ComputeEffectiveWeight(StoneBase, { EMoonProperty::Lightness }, Settings), StoneBase * LightMul);
	TestEqual(TEXT("Heaviness scales"), UPropertyHolderComponent::ComputeEffectiveWeight(PebbleBase, { EMoonProperty::Heaviness }, Settings), PebbleBase * HeavyMul);
	TestEqual(TEXT("Lightness + Heaviness cancel"), UPropertyHolderComponent::ComputeEffectiveWeight(PebbleBase, { EMoonProperty::Lightness, EMoonProperty::Heaviness }, Settings), PebbleBase);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTsukiHolderTransferTest, "Tsuki.Properties.TransferLoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTsukiHolderTransferTest::RunTest(const FString& Parameters)
{
	using namespace TsukiTestFixtures;
	UPropertyHolderComponent* Plant = MakeHolder(1.f, { EMoonProperty::Lightness }, { EMoonProperty::Lightness }, {});
	UPropertyHolderComponent* Stone = MakeHolder(StoneBase, {}, {}, { EMoonProperty::Lightness });

	TestTrue(TEXT("Plant is a source"), Plant->IsSource());
	TestTrue(TEXT("Stone is a receiver"), Stone->IsReceiver());
	TestFalse(TEXT("Stone cannot take Heaviness"), Stone->CanAttach(EMoonProperty::Heaviness));

	// 1-2: cut from source.
	TestTrue(TEXT("Cut Lightness from plant"), Plant->CutProperty(EMoonProperty::Lightness));
	TestFalse(TEXT("Plant lost Lightness"), Plant->HasProperty(EMoonProperty::Lightness));
	TestFalse(TEXT("Cannot cut twice"), Plant->CutProperty(EMoonProperty::Lightness));

	// 3-4: attach to receiver; weight changes.
	TestTrue(TEXT("Attach to stone"), Stone->AttachProperty(EMoonProperty::Lightness, Plant));
	TestTrue(TEXT("Stone lighter"), Stone->GetEffectiveWeight() < StoneBase);
	TestTrue(TEXT("Original source remembered"), Stone->GetOriginalSource(EMoonProperty::Lightness) == Plant);

	// 5: reversible - cut off target and return to source.
	TestTrue(TEXT("Attached property can be cut back"), Stone->CanCut(EMoonProperty::Lightness));
	TestTrue(TEXT("Cut from stone"), Stone->CutProperty(EMoonProperty::Lightness));
	TestEqual(TEXT("Stone back to base"), Stone->GetEffectiveWeight(), StoneBase);
	TestTrue(TEXT("Restore to plant"), Plant->RestoreProperty(EMoonProperty::Lightness));
	TestTrue(TEXT("Plant is a source again"), Plant->CanCut(EMoonProperty::Lightness));
	return true;
}

#endif
