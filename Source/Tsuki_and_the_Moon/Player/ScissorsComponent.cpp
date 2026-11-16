#include "Player/ScissorsComponent.h"
#include "Properties/PropertyHolderComponent.h"
#include "Properties/TsukiPropertySettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UScissorsComponent::UScissorsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UScissorsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateTarget();
}

void UScissorsComponent::SetScissorsEnabled(bool bEnabled)
{
	bScissorsEnabled = bEnabled;
	if (!bEnabled)
	{
		UpdateTarget();
	}
}

UPropertyHolderComponent* UScissorsComponent::FindAimedHolder() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	if (!Controller)
	{
		return nullptr;
	}

	FVector ViewLoc;
	FRotator ViewRot;
	Controller->GetPlayerViewPoint(ViewLoc, ViewRot);
	const FVector Dir = ViewRot.Vector();
	const FVector PawnLoc = Pawn->GetActorLocation();

	// Start level with Tsuki along the view ray so objects behind her are never picked.
	const float StartDist = FMath::Max(0.f, FVector::DotProduct(PawnLoc - ViewLoc, Dir));
	const FVector Start = ViewLoc + Dir * StartDist;
	const FVector End = Start + Dir * TraceRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ScissorsAim), false, Pawn);
	TArray<FHitResult>& Hits = AimHits; // Reused each tick to avoid per-frame allocation.
	Hits.Reset();
	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TraceRadius), Params);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}
		UPropertyHolderComponent* Holder = HitActor->FindComponentByClass<UPropertyHolderComponent>();
		if (Holder && Holder->bTargetable && (Holder->IsSource() || Holder->IsReceiver()))
		{
			if (FVector::Dist(PawnLoc, Hit.ImpactPoint) <= TraceRange)
			{
				return Holder;
			}
		}
		if (Hit.bBlockingHit)
		{
			break; // A wall in front of the target hides it.
		}
	}
	return nullptr;
}

EScissorsAction UScissorsComponent::EvaluateAction(const UPropertyHolderComponent* Holder) const
{
	if (!Holder)
	{
		return EScissorsAction::None;
	}
	if (Carried != EMoonProperty::None)
	{
		return Holder->CanAttach(Carried) ? EScissorsAction::Attach : EScissorsAction::None;
	}
	return Holder->IsSource() ? EScissorsAction::Cut : EScissorsAction::None;
}

void UScissorsComponent::UpdateTarget()
{
	UPropertyHolderComponent* NewTarget = bScissorsEnabled ? FindAimedHolder() : nullptr;
	const EScissorsAction NewAction = EvaluateAction(NewTarget);

	if (NewTarget == Target.Get() && NewAction == CurrentAction)
	{
		return;
	}
	if (UPropertyHolderComponent* Old = Target.Get())
	{
		if (Old != NewTarget)
		{
			Old->SetTargeted(false, EScissorsAction::None);
		}
	}
	Target = NewTarget;
	CurrentAction = NewAction;
	if (NewTarget)
	{
		NewTarget->SetTargeted(true, NewAction);
	}
	OnTargetChanged.Broadcast(NewTarget, NewAction);
}

EMoonProperty UScissorsComponent::GetActionProperty() const
{
	switch (CurrentAction)
	{
	case EScissorsAction::Cut:
		return Target.IsValid() ? Target->GetFirstCuttable() : EMoonProperty::None;
	case EScissorsAction::Attach:
		return Carried;
	default:
		return EMoonProperty::None;
	}
}

bool UScissorsComponent::UseScissors()
{
	UpdateTarget();
	UPropertyHolderComponent* Holder = Target.Get();
	if (!bScissorsEnabled || !Holder)
	{
		return false;
	}

	if (CurrentAction == EScissorsAction::Cut && Carried == EMoonProperty::None)
	{
		const EMoonProperty Prop = Holder->GetFirstCuttable();
		UPropertyHolderComponent* Origin = Holder->GetOriginalSource(Prop);
		if (Holder->CutProperty(Prop))
		{
			SetCarried(Prop, Origin);
			PlayPropertySound(Prop, true);
			OnScissorsUsed.Broadcast(EScissorsAction::Cut, Prop);
			UpdateTarget();
			return true;
		}
	}
	else if (CurrentAction == EScissorsAction::Attach && Carried != EMoonProperty::None)
	{
		const EMoonProperty Prop = Carried;
		if (Holder->AttachProperty(Prop, CarriedSource.Get()))
		{
			SetCarried(EMoonProperty::None, nullptr);
			PlayPropertySound(Prop, false);
			OnScissorsUsed.Broadcast(EScissorsAction::Attach, Prop);
			UpdateTarget();
			return true;
		}
	}
	return false;
}

bool UScissorsComponent::ReturnCarried()
{
	if (Carried == EMoonProperty::None)
	{
		return false;
	}
	const EMoonProperty Prop = Carried;
	if (UPropertyHolderComponent* Source = CarriedSource.Get())
	{
		Source->RestoreProperty(Prop);
	}
	SetCarried(EMoonProperty::None, nullptr);
	PlayPropertySound(Prop, false);
	UpdateTarget();
	return true;
}

void UScissorsComponent::SetCarried(EMoonProperty NewCarried, UPropertyHolderComponent* Source)
{
	Carried = NewCarried;
	CarriedSource = Source;
	OnCarriedChanged.Broadcast(Carried);
}

void UScissorsComponent::PlayPropertySound(EMoonProperty Property, bool bCut) const
{
	const FMoonPropertyInfo& Info = UTsukiPropertySettings::Get()->GetInfo(Property);
	USoundBase* Sound = (bCut ? Info.CutSound : Info.AttachSound).LoadSynchronous();
	if (!Sound)
	{
		Sound = SnipSound;
	}
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}
