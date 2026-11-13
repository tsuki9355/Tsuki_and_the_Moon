#include "UI/TsukiHUD.h"
#include "Player/TsukiCharacter.h"
#include "Player/ScissorsComponent.h"
#include "Player/GrabComponent.h"
#include "Properties/PropertyHolderComponent.h"
#include "Properties/TsukiPropertySettings.h"
#include "Core/TsukiProgressSubsystem.h"
#include "World/ToolPickup.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"

ATsukiHUD* ATsukiHUD::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	return PC ? Cast<ATsukiHUD>(PC->GetHUD()) : nullptr;
}

void ATsukiHUD::ShowFairyLine(const FText& Line, float Duration)
{
	FairyLine = Line;
	FairyLineEndTime = GetWorld()->GetTimeSeconds() + Duration;
}

void ATsukiHUD::DrawPanel(float X, float Y, float W, float H, const FLinearColor& Color)
{
	FCanvasTileItem Tile(FVector2D(X, Y), FVector2D(W, H), Color);
	Tile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tile);
}

void ATsukiHUD::DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale)
{
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	DrawText(Text, Color, (Canvas->ClipX - W) * 0.5f, Y, Font, Scale);
}

void ATsukiHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}
	const ATsukiCharacter* Tsuki = Cast<ATsukiCharacter>(GetOwningPawn());
	UFont* Font = GEngine->GetMediumFont();
	const float S = FMath::Max(Canvas->ClipY / 1080.f, 0.6f);
	const UTsukiPropertySettings* Settings = UTsukiPropertySettings::Get();
	const UTsukiProgressSubsystem* Progress = UTsukiProgressSubsystem::Get(this);

	// Moon Fragments (top right).
	if (Progress)
	{
		const FString Frag = FText::Format(NSLOCTEXT("TsukiHUD", "Fragments", "Moon Fragments  {0}"), Progress->GetMoonFragments()).ToString();
		float W = 0.f, H = 0.f;
		GetTextSize(Frag, W, H, Font, 1.4f * S);
		DrawText(Frag, FLinearColor(0.95f, 0.92f, 0.7f), Canvas->ClipX - W - 40.f * S, 30.f * S, Font, 1.4f * S);
	}

	if (!Tsuki)
	{
		return;
	}

	const UScissorsComponent* Scissors = Tsuki->Scissors;
	const bool bHasScissors = Scissors && Scissors->IsScissorsEnabled();

	if (bHasScissors)
	{
		// Carried property (top left): swatch + name, so color is never the only cue.
		const EMoonProperty Carried = Scissors->GetCarried();
		const FMoonPropertyInfo& Info = Settings->GetInfo(Carried);
		const float X = 40.f * S, Y = 30.f * S;
		DrawPanel(X - 10.f * S, Y - 8.f * S, 330.f * S, 52.f * S, FLinearColor(0.f, 0.f, 0.f, 0.35f));
		DrawPanel(X, Y + 4.f * S, 28.f * S, 28.f * S, Carried == EMoonProperty::None ? FLinearColor(0.3f, 0.3f, 0.3f, 0.6f) : Info.Color);
		const FString Label = (Carried == EMoonProperty::None ? NSLOCTEXT("TsukiHUD", "Empty", "Scissors: empty") : FText::Format(NSLOCTEXT("TsukiHUD", "Carrying", "Carrying: {0}"), Info.DisplayName)).ToString();
		DrawText(Label, FLinearColor::White, X + 42.f * S, Y + 4.f * S, Font, 1.3f * S);

		// Reticle: dim when nothing valid, colored by the action when aiming at a valid target.
		const EScissorsAction Action = Scissors->GetCurrentAction();
		FLinearColor ReticleColor(1.f, 1.f, 1.f, 0.35f);
		if (Action == EScissorsAction::Cut)
		{
			ReticleColor = Settings->GetInfo(Scissors->GetActionProperty()).Color;
		}
		else if (Action == EScissorsAction::Attach)
		{
			ReticleColor = Settings->ReceiverColor;
		}
		const float R = (Action == EScissorsAction::None ? 4.f : 7.f) * S;
		DrawPanel(Canvas->ClipX * 0.5f - R, Canvas->ClipY * 0.5f - R, R * 2.f, R * 2.f, ReticleColor);
	}

	// Interact prompt (E): tool pickup, grab, lift, put down. Its own line so it never hides behind scissors prompts.
	FText InteractPrompt;
	const bool bBusyHands = Tsuki->Grab && (Tsuki->Grab->IsGrabbing() || Tsuki->Grab->IsCarrying());
	if (const AToolPickup* Pickup = Tsuki->FindPickupInReach())
	{
		InteractPrompt = FText::Format(NSLOCTEXT("TsukiHUD", "PickupPrompt", "[E] {0}"), Pickup->PromptText);
	}
	else if (Tsuki->Grab)
	{
		switch (Tsuki->Grab->GetPrompt())
		{
		case EGrabPrompt::Grab:     InteractPrompt = NSLOCTEXT("TsukiHUD", "Grab", "[E] Hold to Grab"); break;
		case EGrabPrompt::PickUp:   InteractPrompt = NSLOCTEXT("TsukiHUD", "Lift", "[E] Pick Up"); break;
		case EGrabPrompt::TooHeavy: InteractPrompt = NSLOCTEXT("TsukiHUD", "TooHeavy", "Too heavy to move"); break;
		case EGrabPrompt::Pushing:  InteractPrompt = NSLOCTEXT("TsukiHUD", "Pushing", "Push / Pull   (release [E] to let go)"); break;
		case EGrabPrompt::PutDown:  InteractPrompt = NSLOCTEXT("TsukiHUD", "PutDown", "[E] Put Down"); break;
		default: break;
		}
	}

	// Scissors prompt (LMB / R). Hidden while hands are busy, since the scissors can't be used then.
	FString ScissorsPrompt;
	if (bHasScissors && !bBusyHands && Scissors->GetCurrentAction() != EScissorsAction::None)
	{
		const FText Prop = Settings->GetInfo(Scissors->GetActionProperty()).DisplayName;
		ScissorsPrompt = (Scissors->GetCurrentAction() == EScissorsAction::Cut
			? FText::Format(NSLOCTEXT("TsukiHUD", "Cut", "[LMB] Cut {0}"), Prop)
			: FText::Format(NSLOCTEXT("TsukiHUD", "Attach", "[LMB] Attach {0}"), Prop)).ToString();
	}
	if (bHasScissors && Scissors->GetCarried() != EMoonProperty::None)
	{
		ScissorsPrompt += (ScissorsPrompt.IsEmpty() ? TEXT("") : TEXT("     ")) + NSLOCTEXT("TsukiHUD", "Return", "[R] Return property").ToString();
	}

	const float LineH = 42.f * S;
	float PromptY = Canvas->ClipY * 0.70f;
	if (!InteractPrompt.IsEmpty())
	{
		DrawCenteredText(InteractPrompt.ToString(), PromptY, FLinearColor(1.f, 0.95f, 0.75f), Font, 1.4f * S);
		PromptY += LineH;
	}
	if (!ScissorsPrompt.IsEmpty())
	{
		DrawCenteredText(ScissorsPrompt, PromptY, FLinearColor::White, Font, 1.4f * S);
	}

	// Fairy line (bottom).
	if (!FairyLine.IsEmpty() && GetWorld()->GetTimeSeconds() < FairyLineEndTime)
	{
		const FString Line = FairyLine.ToString();
		float W = 0.f, H = 0.f;
		GetTextSize(Line, W, H, Font, 1.5f * S);
		const float PanelW = W + 60.f * S, PanelH = H + 30.f * S;
		const float PX = (Canvas->ClipX - PanelW) * 0.5f, PY = Canvas->ClipY * 0.82f;
		DrawPanel(PX, PY, PanelW, PanelH, FLinearColor(0.08f, 0.05f, 0.18f, 0.7f));
		DrawText(Line, FLinearColor(0.92f, 0.85f, 1.f), PX + 30.f * S, PY + 15.f * S, Font, 1.5f * S);
	}
}
