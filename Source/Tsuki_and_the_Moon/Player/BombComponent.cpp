#include "Player/BombComponent.h"
#include "World/TsukiBomb.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

UBombComponent::UBombComponent()
{
	BombClass = ATsukiBomb::StaticClass();
}

bool UBombComponent::IsHoldingBomb() const
{
	return Held.IsValid() && !Held->HasExploded();
}

bool UBombComponent::PlaceBomb()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	ATsukiBomb* Bomb = Held.Get();
	if (!Character || !Bomb || Bomb->HasExploded())
	{
		return false;
	}
	const FVector Ahead = Character->GetActorLocation() + Character->GetActorForwardVector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 30.f);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BombPlace), false, Character);
	Params.AddIgnoredActor(Bomb);
	FHitResult Ground;
	if (!GetWorld()->LineTraceSingleByChannel(Ground, Ahead + FVector(0.f, 0.f, 50.f), Ahead - FVector(0.f, 0.f, 400.f), ECC_Visibility, Params))
	{
		return false; // Nothing to set it on.
	}
	const float Radius = Bomb->Mesh->Bounds.SphereRadius;
	Bomb->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Bomb->SetActorLocation(Ground.ImpactPoint + FVector(0.f, 0.f, Radius + 1.f));
	Bomb->Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Held = nullptr;
	return true;
}

void UBombComponent::UseBomb()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!bBombsEnabled || !Character)
	{
		return;
	}

	if (ATsukiBomb* Bomb = Held.Get())
	{
		// Release partway into the throw animation so the bomb leaves her hands on the swing.
		Held = nullptr;
		TWeakObjectPtr<ATsukiBomb> WeakBomb = Bomb;
		TWeakObjectPtr<ACharacter> WeakChar = Character;
		const float Forward = TossForward, Up = TossUp;
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [WeakBomb, WeakChar, Forward, Up]()
		{
			ATsukiBomb* B = WeakBomb.Get();
			ACharacter* C = WeakChar.Get();
			if (B && C && !B->HasExploded())
			{
				B->Throw(C->GetActorForwardVector() * Forward + FVector(0.f, 0.f, Up) + C->GetVelocity() * 0.5f);
			}
		}, FMath::Max(ThrowReleaseDelay, 0.01f), false);
		return;
	}

	// One bomb in the world at a time.
	if (Active.IsValid() && !Active->HasExploded())
	{
		return;
	}
	const float Top = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATsukiBomb* Bomb = GetWorld()->SpawnActor<ATsukiBomb>(BombClass, Character->GetActorLocation() + FVector(0.f, 0.f, Top + 25.f), FRotator::ZeroRotator, Params);
	if (!Bomb)
	{
		return;
	}
	Bomb->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Held overhead; collides once tossed.
	Bomb->AttachToComponent(Character->GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
	Held = Bomb;
	Active = Bomb;
}
