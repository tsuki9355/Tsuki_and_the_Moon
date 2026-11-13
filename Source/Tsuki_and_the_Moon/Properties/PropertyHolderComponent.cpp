#include "Properties/PropertyHolderComponent.h"
#include "Properties/TsukiPropertySettings.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"

UPropertyHolderComponent::UPropertyHolderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPropertyHolderComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeHolder();
}

void UPropertyHolderComponent::InitializeHolder()
{
	Properties.Reset();
	CurrentCuttable.Reset();
	OriginalSources.Reset();
	for (EMoonProperty P : InitialProperties)
	{
		if (P != EMoonProperty::None)
		{
			Properties.AddUnique(P);
		}
	}
	for (EMoonProperty P : CuttableProperties)
	{
		if (Properties.Contains(P))
		{
			CurrentCuttable.AddUnique(P);
		}
	}
	ApplyEffects();
	RefreshGlow();
}

bool UPropertyHolderComponent::CanCut(EMoonProperty Property) const
{
	return Property != EMoonProperty::None && Properties.Contains(Property) && CurrentCuttable.Contains(Property);
}

EMoonProperty UPropertyHolderComponent::GetFirstCuttable() const
{
	for (EMoonProperty P : CurrentCuttable)
	{
		if (Properties.Contains(P))
		{
			return P;
		}
	}
	return EMoonProperty::None;
}

bool UPropertyHolderComponent::CanAttach(EMoonProperty Property) const
{
	return Property != EMoonProperty::None && AcceptedProperties.Contains(Property) && !Properties.Contains(Property);
}

bool UPropertyHolderComponent::CutProperty(EMoonProperty Property)
{
	if (!CanCut(Property))
	{
		return false;
	}
	Properties.Remove(Property);
	CurrentCuttable.Remove(Property);
	ApplyEffects();
	RefreshGlow();
	OnPropertyChanged.Broadcast(Property, false);
	return true;
}

bool UPropertyHolderComponent::AttachProperty(EMoonProperty Property, UPropertyHolderComponent* Source)
{
	if (!CanAttach(Property))
	{
		return false;
	}
	Properties.Add(Property);
	CurrentCuttable.AddUnique(Property);
	OriginalSources.Add(Property, Source ? Source : this);
	ApplyEffects();
	RefreshGlow();
	OnPropertyChanged.Broadcast(Property, true);
	return true;
}

bool UPropertyHolderComponent::RestoreProperty(EMoonProperty Property)
{
	if (Property == EMoonProperty::None || Properties.Contains(Property))
	{
		return false;
	}
	Properties.Add(Property);
	CurrentCuttable.AddUnique(Property);
	OriginalSources.Remove(Property);
	ApplyEffects();
	RefreshGlow();
	OnPropertyChanged.Broadcast(Property, true);
	return true;
}

UPropertyHolderComponent* UPropertyHolderComponent::GetOriginalSource(EMoonProperty Property)
{
	if (const TWeakObjectPtr<UPropertyHolderComponent>* Found = OriginalSources.Find(Property))
	{
		if (Found->IsValid())
		{
			return Found->Get();
		}
	}
	return this;
}

float UPropertyHolderComponent::GetEffectiveWeight() const
{
	return ComputeEffectiveWeight(BaseWeight, Properties, *UTsukiPropertySettings::Get());
}

float UPropertyHolderComponent::GetTimeScale() const
{
	return ComputeTimeScale(Properties, *UTsukiPropertySettings::Get());
}

float UPropertyHolderComponent::ComputeEffectiveWeight(float Base, const TArray<EMoonProperty>& Props, const UTsukiPropertySettings& Settings)
{
	const bool bLight = Props.Contains(EMoonProperty::Lightness);
	const bool bHeavy = Props.Contains(EMoonProperty::Heaviness);
	float Multiplier = 1.f;
	if (bLight && !bHeavy)
	{
		Multiplier = Settings.GetInfo(EMoonProperty::Lightness).WeightMultiplier;
	}
	else if (bHeavy && !bLight)
	{
		Multiplier = Settings.GetInfo(EMoonProperty::Heaviness).WeightMultiplier;
	}
	return Base * Multiplier;
}

float UPropertyHolderComponent::ComputeTimeScale(const TArray<EMoonProperty>& Props, const UTsukiPropertySettings& Settings)
{
	float Scale = 1.f;
	for (EMoonProperty P : Props)
	{
		Scale = FMath::Min(Scale, Settings.GetInfo(P).TimeScale);
	}
	return Scale;
}

void UPropertyHolderComponent::SetTargeted(bool bInTargeted, EScissorsAction Action)
{
	if (bTargeted == bInTargeted && TargetAction == Action)
	{
		return;
	}
	bTargeted = bInTargeted;
	TargetAction = bInTargeted ? Action : EScissorsAction::None;
	RefreshGlow();
	OnTargetChanged.Broadcast(bTargeted, TargetAction);
}

void UPropertyHolderComponent::ApplyEffects()
{
	if (AActor* Owner = GetOwner())
	{
		Owner->CustomTimeDilation = GetTimeScale();
	}
}

void UPropertyHolderComponent::RefreshGlow()
{
	AActor* Owner = GetOwner();
	if (!Owner || !bShowGlow)
	{
		return;
	}
	const UTsukiPropertySettings* Settings = UTsukiPropertySettings::Get();
	const EMoonProperty SourceProp = GetFirstCuttable();
	const bool bHasRole = SourceProp != EMoonProperty::None || IsReceiver();

	// Sources show their property color; receivers show neutral white. When targeted, the
	// color matches the action the scissors would take, so the valid interaction is clear.
	FLinearColor Color = Settings->ReceiverColor;
	EMoonProperty ShownProp = EMoonProperty::None;
	if (SourceProp != EMoonProperty::None && !(bTargeted && TargetAction == EScissorsAction::Attach))
	{
		Color = Settings->GetInfo(SourceProp).Color;
		ShownProp = SourceProp;
	}
	const float Intensity = !bHasRole ? 0.f : (bTargeted ? Settings->TargetedGlowIntensity : Settings->IdleGlowIntensity);

	UMaterialInterface* Overlay = Settings->HighlightOverlayMaterial.LoadSynchronous();
	TArray<UMeshComponent*> Meshes;
	Owner->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		if (Overlay && Mesh->GetOverlayMaterial() != Overlay)
		{
			Mesh->SetOverlayMaterial(Overlay);
		}
		Mesh->SetCustomPrimitiveDataVector4(0, FVector4(Color.R, Color.G, Color.B, Intensity));
		Mesh->SetCustomPrimitiveDataFloat(4, static_cast<float>(ShownProp));
	}
}
