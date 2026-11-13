#include "World/TsukiMechanism.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATsukiMechanism::ATsukiMechanism()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RevealCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("RevealCamera"));
	RevealCamera->SetupAttachment(RootComponent);
	RevealCamera->SetRelativeLocation(FVector(-600.f, 0.f, 250.f));
	RevealCamera->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	RevealCamera->bAutoActivate = true;
}

void ATsukiMechanism::SetMechanismActive(bool bActive)
{
	if (bActive == bIsActive)
	{
		return;
	}
	bIsActive = bActive;
	if (bActive)
	{
		if (bPlayRevealOnFirstActivate && !bRevealPlayed)
		{
			bRevealPlayed = true;
			BeginReveal();
		}
		else
		{
			ApplyActivated();
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(ActivateTimer);
		HandleDeactivated();
		OnMechanismDeactivated();
	}
}

void ATsukiMechanism::ApplyActivated()
{
	if (bIsActive)
	{
		HandleActivated();
		OnMechanismActivated();
	}
}

void ATsukiMechanism::BeginReveal()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		ApplyActivated();
		return;
	}
	PC->GetPawn()->DisableInput(PC);
	PC->SetViewTargetWithBlend(this, RevealBlendTime, VTBlend_Cubic);
	if (RevealSound)
	{
		UGameplayStatics::PlaySound2D(this, RevealSound);
	}
	// Start moving once the camera has arrived so the player sees the whole motion.
	GetWorldTimerManager().SetTimer(ActivateTimer, this, &ATsukiMechanism::ApplyActivated, FMath::Max(RevealBlendTime, 0.01f), false);
	GetWorldTimerManager().SetTimer(RevealTimer, this, &ATsukiMechanism::FinishReveal, RevealDuration, false);
}

void ATsukiMechanism::FinishReveal()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		PC->SetViewTargetWithBlend(PC->GetPawn(), RevealBlendTime, VTBlend_Cubic);
		PC->GetPawn()->EnableInput(PC);
	}
}
