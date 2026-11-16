#include "UI/TsukiTitleWidget.h"
#include "Core/TsukiTitleGameMode.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "TsukiTitle"

UTsukiTitleWidget::UTsukiTitleWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleText = LOCTEXT("Title", "Tsuki and the Moon");
	PromptText = LOCTEXT("Prompt", "Press any button");
	SetIsFocusable(true);
}

void UTsukiTitleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildTree();
	}
}

TSharedRef<SWidget> UTsukiTitleWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildTree();
	}
	return Super::RebuildWidget();
}

ATsukiTitleGameMode* UTsukiTitleWidget::GetTitleMode() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetAuthGameMode<ATsukiTitleGameMode>() : nullptr;
}

UTextBlock* UTsukiTitleWidget::MakeText(const FText& Text, int32 Size, const FName Typeface)
{
	UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Block->SetText(Text);
	Block->SetFont(FCoreStyle::GetDefaultFontStyle(Typeface, Size));
	Block->SetColorAndOpacity(FSlateColor(TextColor));
	Block->SetShadowOffset(FVector2D(2.f, 2.f));
	Block->SetShadowColorAndOpacity(FLinearColor(0.02f, 0.03f, 0.08f, 0.75f));
	Block->SetJustification(ETextJustify::Center);
	return Block;
}

UButton* UTsukiTitleWidget::MakeButton(const FText& Label, int32 Size)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	FSlateBrush None;
	None.DrawAs = ESlateBrushDrawType::NoDrawType;
	FButtonStyle Style = Button->GetStyle();
	Style.SetNormal(None).SetHovered(None).SetPressed(None).SetDisabled(None);
	Style.SetNormalPadding(FMargin(24.f, 4.f)).SetPressedPadding(FMargin(24.f, 4.f));
	Button->SetStyle(Style);

	UTextBlock* Text = MakeText(Label, Size, TEXT("Bold"));
	Button->SetContent(Text);
	Button->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	AllButtons.Add(Button);
	AllLabels.Add(Text);
	return Button;
}

UBorder* UTsukiTitleWidget::MakePanel(UWidget* Content, float Opacity)
{
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	FSlateBrush Brush;
	Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
	Brush.TintColor = FSlateColor(FLinearColor(0.02f, 0.03f, 0.07f, Opacity));
	Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(18.f, 18.f, 18.f, 18.f),
		FSlateColor(FLinearColor(0.95f, 0.88f, 0.6f, 0.35f)), 1.5f);
	Panel->SetBrush(Brush);
	Panel->SetPadding(FMargin(56.f, 40.f));
	Panel->SetContent(Content);
	Panel->SetVisibility(ESlateVisibility::Collapsed);
	return Panel;
}

void UTsukiTitleWidget::BuildTree()
{
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	// A light dim so the text reads over bright foliage.
	UImage* Shade = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Shade->SetColorAndOpacity(FLinearColor(0.f, 0.01f, 0.04f, 0.18f));
	UOverlaySlot* ShadeSlot = Root->AddChildToOverlay(Shade);
	ShadeSlot->SetHorizontalAlignment(HAlign_Fill);
	ShadeSlot->SetVerticalAlignment(VAlign_Fill);

	// Title, prompt and menu, centred.
	UVerticalBox* Main = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UOverlaySlot* MainSlot = Root->AddChildToOverlay(Main);
	MainSlot->SetHorizontalAlignment(HAlign_Center);
	MainSlot->SetVerticalAlignment(VAlign_Top);
	MainSlot->SetPadding(FMargin(0.f, 170.f, 0.f, 0.f));

	TitleBlock = MakeText(TitleText, 84, TEXT("Bold"));
	TitleBlock->SetShadowOffset(FVector2D(3.f, 3.f));
	TitleBlock->SetRenderOpacity(0.f);
	Main->AddChildToVerticalBox(TitleBlock)->SetHorizontalAlignment(HAlign_Center);

	USpacer* Gap = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
	Gap->SetSize(FVector2D(1.f, 250.f));
	Main->AddChildToVerticalBox(Gap);

	PromptBlock = MakeText(PromptText, 28);
	PromptBlock->SetRenderOpacity(0.f);
	Main->AddChildToVerticalBox(PromptBlock)->SetHorizontalAlignment(HAlign_Center);

	// The menu sits on a soft dark backing so it reads over bright foliage.
	MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UBorder* MenuBacking = MakePanel(MenuBox, 0.4f);
	MenuBacking->SetPadding(FMargin(48.f, 18.f));
	MenuBacking->SetVisibility(ESlateVisibility::Collapsed); // Shown with the menu; see SetStage.
	UVerticalBoxSlot* MenuSlot = Main->AddChildToVerticalBox(MenuBacking);
	MenuSlot->SetHorizontalAlignment(HAlign_Center);

	StartButton = MakeButton(LOCTEXT("Start", "Start"), 34);
	ContinueButton = MakeButton(LOCTEXT("Continue", "Continue"), 34);
	CreditsButton = MakeButton(LOCTEXT("Credits", "Credits"), 34);
	QuitButton = MakeButton(LOCTEXT("Quit", "Quit"), 34);
	for (UButton* Button : { StartButton.Get(), ContinueButton.Get(), CreditsButton.Get(), QuitButton.Get() })
	{
		UVerticalBoxSlot* ButtonSlot = MenuBox->AddChildToVerticalBox(Button);
		ButtonSlot->SetHorizontalAlignment(HAlign_Center);
		ButtonSlot->SetPadding(FMargin(0.f, 6.f));
	}
	StartButton->OnClicked.AddDynamic(this, &UTsukiTitleWidget::HandleStart);
	ContinueButton->OnClicked.AddDynamic(this, &UTsukiTitleWidget::HandleContinue);
	CreditsButton->OnClicked.AddDynamic(this, &UTsukiTitleWidget::HandleCredits);
	QuitButton->OnClicked.AddDynamic(this, &UTsukiTitleWidget::HandleQuit);

	// Credits panel.
	UVerticalBox* CreditsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	CreditsBox->AddChildToVerticalBox(MakeText(LOCTEXT("CreditsHeader", "Credits"), 44, TEXT("Bold")))->SetHorizontalAlignment(HAlign_Center);
	const ATsukiTitleGameMode* Mode = GetTitleMode();
	UTextBlock* CreditsBody = MakeText(Mode ? Mode->CreditsText : FText::GetEmpty(), 22);
	CreditsBox->AddChildToVerticalBox(CreditsBody)->SetPadding(FMargin(0.f, 24.f));
	CreditsBackButton = MakeButton(LOCTEXT("Back", "Back"), 30);
	CreditsBackButton->OnClicked.AddDynamic(this, &UTsukiTitleWidget::HandleBack);
	CreditsBox->AddChildToVerticalBox(CreditsBackButton)->SetHorizontalAlignment(HAlign_Center);
	CreditsPanel = MakePanel(CreditsBox);
	UOverlaySlot* CreditsSlot = Root->AddChildToOverlay(CreditsPanel);
	CreditsSlot->SetHorizontalAlignment(HAlign_Center);
	CreditsSlot->SetVerticalAlignment(VAlign_Center);

	// New-game confirmation (only when a save exists).
	UVerticalBox* ConfirmBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	ConfirmBox->AddChildToVerticalBox(MakeText(LOCTEXT("ConfirmTitle", "Start a new journey?"), 36, TEXT("Bold")))->SetHorizontalAlignment(HAlign_Center);
	ConfirmBox->AddChildToVerticalBox(MakeText(LOCTEXT("ConfirmBody", "Your saved progress will be erased."), 24))->SetPadding(FMargin(0.f, 16.f, 0.f, 24.f));
	UHorizontalBox* ConfirmRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	ConfirmYesButton = MakeButton(LOCTEXT("Yes", "Yes"), 30);
	ConfirmNoButton = MakeButton(LOCTEXT("No", "No"), 30);
	ConfirmRow->AddChildToHorizontalBox(ConfirmYesButton)->SetPadding(FMargin(20.f, 0.f));
	ConfirmRow->AddChildToHorizontalBox(ConfirmNoButton)->SetPadding(FMargin(20.f, 0.f));
	ConfirmYesButton->OnClicked.AddDynamic(this, &UTsukiTitleWidget::HandleConfirmYes);
	ConfirmNoButton->OnClicked.AddDynamic(this, &UTsukiTitleWidget::HandleBack);
	ConfirmBox->AddChildToVerticalBox(ConfirmRow)->SetHorizontalAlignment(HAlign_Center);
	ConfirmPanel = MakePanel(ConfirmBox);
	UOverlaySlot* ConfirmSlot = Root->AddChildToOverlay(ConfirmPanel);
	ConfirmSlot->SetHorizontalAlignment(HAlign_Center);
	ConfirmSlot->SetVerticalAlignment(VAlign_Center);
}

void UTsukiTitleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Elapsed += InDeltaTime;
	StageTime += InDeltaTime;

	// Title fades in once, then stays.
	const float TitleAlpha = FMath::SmoothStep(0.f, 1.f, (Elapsed - TitleDelay) / TitleFadeTime);
	TitleBlock->SetRenderOpacity(FMath::Max(TitleBlock->GetRenderOpacity(), TitleAlpha));

	if (Stage == EStage::Intro && Elapsed >= TitleDelay + TitleFadeTime)
	{
		SetStage(EStage::Prompt);
	}
	if (Stage == EStage::Prompt)
	{
		// Slow breathing pulse.
		const float In = FMath::Clamp(StageTime / 0.8f, 0.f, 1.f);
		PromptBlock->SetRenderOpacity(In * (0.6f + 0.4f * FMath::Cos(StageTime * 2.2f)));
	}
	if (Stage == EStage::Menu)
	{
		MenuBox->GetParent()->SetRenderOpacity(FMath::Clamp(StageTime / 0.35f, 0.f, 1.f));
	}
	if (Stage == EStage::Leaving)
	{
		SetRenderOpacity(FMath::Clamp(1.f - StageTime / 0.6f, 0.f, 1.f));
		return;
	}

	// Hovering selects only when the mouse actually moves, so a resting cursor never steals the
	// keyboard / gamepad selection. Mouse, keys and gamepad share one highlight.
	const FVector2D CursorNow = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetCursorPos() : FVector2D::ZeroVector;
	const bool bMouseMoved = !CursorNow.Equals(LastCursorPos, 1.f);
	LastCursorPos = CursorNow;
	UButton* Selected = nullptr;
	for (int32 i = 0; i < AllButtons.Num(); ++i)
	{
		UButton* Button = AllButtons[i];
		if (!Button->IsVisible() || !Button->GetIsEnabled())
		{
			AllLabels[i]->SetColorAndOpacity(FSlateColor(TextColor * FLinearColor(1.f, 1.f, 1.f, 0.35f)));
			continue;
		}
		if (bMouseMoved && Button->IsHovered() && !Button->HasKeyboardFocus())
		{
			FocusButton(Button);
		}
		const bool bSelected = Button->HasKeyboardFocus();
		if (bSelected)
		{
			Selected = Button;
		}
		AllLabels[i]->SetColorAndOpacity(FSlateColor(bSelected ? SelectedColor : TextColor));
		const float Scale = FMath::FInterpTo(Button->GetRenderTransform().Scale.X, bSelected ? 1.12f : 1.f, InDeltaTime, 14.f);
		Button->SetRenderScale(FVector2D(Scale, Scale));
	}
	if (Selected && Selected != LastSelected)
	{
		if (LastSelected)
		{
			if (const ATsukiTitleGameMode* Mode = GetTitleMode())
			{
				Mode->PlayUISound(false);
			}
		}
		LastSelected = Selected;
	}
}

FReply UTsukiTitleWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	const bool bBack = Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::BackSpace;
	switch (Stage)
	{
	case EStage::Intro:
	case EStage::Prompt:
		ShowMenu();
		return FReply::Handled();
	case EStage::Credits:
	case EStage::Confirm:
		if (bBack)
		{
			HandleBack();
			return FReply::Handled();
		}
		break;
	case EStage::Menu:
		// Focus was lost (e.g. a click on empty space): any key brings the selection back.
		if (!LastSelected || !LastSelected->HasKeyboardFocus())
		{
			FocusButton(LastSelected && LastSelected->GetIsEnabled() ? LastSelected.Get() : FirstEnabledMenuButton());
			return FReply::Handled();
		}
		break;
	default:
		break;
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UTsukiTitleWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (Stage == EStage::Intro || Stage == EStage::Prompt)
	{
		ShowMenu();
	}
	return FReply::Handled();
}

void UTsukiTitleWidget::ShowMenu()
{
	TitleBlock->SetRenderOpacity(1.f);
	PromptBlock->SetVisibility(ESlateVisibility::Collapsed);
	const ATsukiTitleGameMode* Mode = GetTitleMode();
	ContinueButton->SetIsEnabled(Mode && Mode->CanContinue());
	if (Mode)
	{
		Mode->PlayUISound(true);
	}
	SetStage(EStage::Menu);
}

void UTsukiTitleWidget::SetStage(EStage NewStage)
{
	Stage = NewStage;
	StageTime = 0.f;
	const bool bPanel = NewStage == EStage::Credits || NewStage == EStage::Confirm;
	// The menu hides while a panel is open, so navigation can't wander back into it.
	MenuBox->GetParent()->SetVisibility(NewStage == EStage::Menu ? ESlateVisibility::SelfHitTestInvisible : (bPanel ? ESlateVisibility::Hidden : ESlateVisibility::Collapsed));
	CreditsPanel->SetVisibility(NewStage == EStage::Credits ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ConfirmPanel->SetVisibility(NewStage == EStage::Confirm ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	TitleBlock->SetVisibility(bPanel ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);

	switch (NewStage)
	{
	case EStage::Menu:
		FocusButton(ReturnButton ? ReturnButton.Get() : (ContinueButton->GetIsEnabled() ? ContinueButton.Get() : StartButton.Get()));
		ReturnButton = nullptr;
		break;
	case EStage::Credits:
		FocusButton(CreditsBackButton);
		break;
	case EStage::Confirm:
		FocusButton(ConfirmNoButton); // Erasing a save should never be one accidental press away.
		break;
	case EStage::Leaving:
		SetIsEnabled(false);
		break;
	default:
		break;
	}
}

void UTsukiTitleWidget::FocusButton(UButton* Button)
{
	if (Button)
	{
		Button->SetKeyboardFocus();
	}
}

UButton* UTsukiTitleWidget::FirstEnabledMenuButton() const
{
	return ContinueButton->GetIsEnabled() ? ContinueButton.Get() : StartButton.Get();
}

void UTsukiTitleWidget::HandleStart()
{
	ATsukiTitleGameMode* Mode = GetTitleMode();
	if (!Mode || Stage != EStage::Menu || !CanActivate())
	{
		return;
	}
	Mode->PlayUISound(true);
	if (Mode->CanContinue())
	{
		ReturnButton = StartButton;
		SetStage(EStage::Confirm);
		return;
	}
	SetStage(EStage::Leaving);
	Mode->StartNewGame();
}

void UTsukiTitleWidget::HandleConfirmYes()
{
	// Erases the save: only from the open confirmation, never from a press carried over from Start.
	if (Stage != EStage::Confirm || !CanActivate())
	{
		return;
	}
	if (ATsukiTitleGameMode* Mode = GetTitleMode())
	{
		Mode->PlayUISound(true);
		SetStage(EStage::Leaving);
		Mode->StartNewGame();
	}
}

void UTsukiTitleWidget::HandleContinue()
{
	ATsukiTitleGameMode* Mode = GetTitleMode();
	if (Mode && Stage == EStage::Menu && CanActivate())
	{
		Mode->PlayUISound(true);
		SetStage(EStage::Leaving);
		Mode->ContinueGame();
	}
}

void UTsukiTitleWidget::HandleCredits()
{
	if (Stage == EStage::Menu && CanActivate())
	{
		if (const ATsukiTitleGameMode* Mode = GetTitleMode())
		{
			Mode->PlayUISound(true);
		}
		ReturnButton = CreditsButton;
		SetStage(EStage::Credits);
	}
}

void UTsukiTitleWidget::HandleQuit()
{
	if (ATsukiTitleGameMode* Mode = GetTitleMode())
	{
		if (Stage == EStage::Menu && CanActivate())
		{
			Mode->QuitGame();
		}
	}
}

void UTsukiTitleWidget::HandleBack()
{
	if ((Stage == EStage::Credits || Stage == EStage::Confirm) && CanActivate())
	{
		SetStage(EStage::Menu);
	}
}

#undef LOCTEXT_NAMESPACE
