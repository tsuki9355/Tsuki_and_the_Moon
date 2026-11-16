#include "Core/TsukiTitleGameMode.h"
#include "Core/TsukiProgressSubsystem.h"
#include "UI/TsukiTitleWidget.h"
#include "World/TsukiTitleCamera.h"
#include "Components/AudioComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "TsukiTitle"

ATsukiTitleGameMode::ATsukiTitleGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = AHUD::StaticClass();
	TitleWidgetClass = UTsukiTitleWidget::StaticClass();
	GameMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Tsuki/Maps/L_World.L_World")));
	CreditsText = LOCTEXT("Credits",
		"Created by Angus\n\n"
		"MUSIC\n\"Path of Adventure\" by Kounine\nLicensed under CC BY 4.0  (creativecommons.org/licenses/by/4.0)\n\n"
		"SOUND EFFECTS\nPixabay and Freesound contributors\n\n"
		"ENVIRONMENT ART\nLow Poly Nature Asset Pack by Naranjo Studios\n\n"
		"Made with Unreal Engine");
}

void ATsukiTitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		for (TActorIterator<ATsukiTitleCamera> It(GetWorld()); It; ++It)
		{
			PC->SetViewTarget(*It);
			break;
		}
		if (PC->PlayerCameraManager && FadeInTime > 0.f)
		{
			PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeInTime, FLinearColor::Black, true, false);
		}
	}

	if (TitleMusic)
	{
		MusicComponent = UGameplayStatics::CreateSound2D(this, TitleMusic, MusicVolume, 1.f, 0.f, nullptr, false, false);
		if (MusicComponent)
		{
			MusicComponent->FadeIn(MusicFadeInTime, MusicVolume);
		}
	}

	if (PC && TitleWidgetClass)
	{
		TitleWidget = CreateWidget<UTsukiTitleWidget>(PC, TitleWidgetClass);
		if (TitleWidget)
		{
			TitleWidget->AddToViewport();
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(TitleWidget->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(Mode);
			PC->SetShowMouseCursor(true);
		}
	}
}

bool ATsukiTitleGameMode::CanContinue() const
{
	return UTsukiProgressSubsystem::DoesSaveExist();
}

void ATsukiTitleGameMode::StartNewGame()
{
	if (bLeaving)
	{
		return;
	}
	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		Progress->ResetProgress();
	}
	LeaveToGame();
}

void ATsukiTitleGameMode::ContinueGame()
{
	if (bLeaving)
	{
		return;
	}
	if (UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this))
	{
		Progress->LoadProgress();
	}
	LeaveToGame();
}

void ATsukiTitleGameMode::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
}

void ATsukiTitleGameMode::PlayUISound(bool bConfirm) const
{
	if (USoundBase* Sound = bConfirm ? ConfirmSound.Get() : SelectSound.Get())
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

void ATsukiTitleGameMode::LeaveToGame()
{
	bLeaving = true;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetShowMouseCursor(false);
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeOutTime, FLinearColor::Black, true, true);
		}
	}
	if (MusicComponent)
	{
		MusicComponent->FadeOut(FadeOutTime, 0.f);
	}
	GetWorldTimerManager().SetTimer(LeaveTimer, this, &ATsukiTitleGameMode::OpenGameMap, FadeOutTime + 0.1f, false);
}

void ATsukiTitleGameMode::OpenGameMap()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameMap);
}

#undef LOCTEXT_NAMESPACE
