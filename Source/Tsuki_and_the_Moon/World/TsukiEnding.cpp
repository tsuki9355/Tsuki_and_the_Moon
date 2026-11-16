#include "World/TsukiEnding.h"
#include "Player/TsukiCharacter.h"
#include "UI/TsukiHUD.h"
#include "Core/TsukiGameMode.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATsukiEnding::ATsukiEnding()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetBoxExtent(FVector(150.f, 150.f, 120.f));
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = Trigger;

	GlimpseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("GlimpseCamera"));
	GlimpseCamera->SetupAttachment(RootComponent);

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ATsukiEnding::HandleOverlap);
}

void ATsukiEnding::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	ATsukiCharacter* Tsuki = Cast<ATsukiCharacter>(OtherActor);
	if (bStarted || !Tsuki)
	{
		return;
	}
	bStarted = true;
	if (APlayerController* PC = Cast<APlayerController>(Tsuki->GetController()))
	{
		Tsuki->DisableInput(PC);
		PC->SetViewTargetWithBlend(this, 1.5f, VTBlend_EaseInOut, 2.f);
	}
	Tsuki->GetCharacterMovement()->StopMovementImmediately();
	Tsuki->GetCharacterMovement()->DisableMovement();
	if (ATsukiGameMode* GM = GetWorld()->GetAuthGameMode<ATsukiGameMode>())
	{
		GM->StopMusic(EndingMusic ? 1.f : 3.f);
	}
	if (EndingMusic)
	{
		UGameplayStatics::PlaySound2D(this, EndingMusic);
	}
	OnEndingStarted();
	LineIndex = 0;
	GetWorldTimerManager().SetTimer(LineTimer, this, &ATsukiEnding::NextLine, 1.5f, false);
}

void ATsukiEnding::NextLine()
{
	ATsukiHUD* HUD = ATsukiHUD::Get(this);
	if (Lines.IsValidIndex(LineIndex))
	{
		if (HUD)
		{
			HUD->ShowFairyLine(Lines[LineIndex], LineDuration);
		}
		++LineIndex;
		GetWorldTimerManager().SetTimer(LineTimer, this, &ATsukiEnding::NextLine, LineDuration, false);
	}
	else if (HUD)
	{
		HUD->ShowFairyLine(EndCard, 100000.f);
	}
}
