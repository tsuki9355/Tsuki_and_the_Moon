#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TsukiTitleWidget.generated.h"

class UBorder;
class UButton;
class UTextBlock;
class UVerticalBox;
class UWidget;
class ATsukiTitleGameMode;

/**
 * Title menu, built in C++ (no widget asset needed). Stages: the title fades in, "Press any button",
 * then Start / Continue / Credits / Quit. Mouse, keyboard and gamepad all work; hovering a button selects it.
 * Start over an existing save asks first. Escape or gamepad B closes Credits and the confirmation.
 * Added to the viewport by ATsukiTitleGameMode, which it calls for the actual actions.
 */
UCLASS()
class TSUKI_AND_THE_MOON_API UTsukiTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	FText TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	FText PromptText;

	/** Seconds before the title starts fading in, and how long the fade takes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title", meta = (ClampMin = "0.0"))
	float TitleDelay = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title", meta = (ClampMin = "0.01"))
	float TitleFadeTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	FLinearColor TextColor = FLinearColor(0.95f, 0.92f, 0.84f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	FLinearColor SelectedColor = FLinearColor(1.f, 0.8f, 0.38f);

	UTsukiTitleWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	enum class EStage : uint8 { Intro, Prompt, Menu, Credits, Confirm, Leaving };

	void BuildTree();
	UTextBlock* MakeText(const FText& Text, int32 Size, const FName Typeface = TEXT("Regular"));
	UButton* MakeButton(const FText& Label, int32 Size);
	UBorder* MakePanel(UWidget* Content, float Opacity = 0.86f);

	void ShowMenu();
	void SetStage(EStage NewStage);
	void FocusButton(UButton* Button);
	UButton* FirstEnabledMenuButton() const;
	ATsukiTitleGameMode* GetTitleMode() const;

	/** Buttons ignore presses for a moment after each screen change, so one press can't act twice. */
	bool CanActivate() const { return StageTime >= ActivationDelay; }
	static constexpr float ActivationDelay = 0.3f;

	UFUNCTION() void HandleStart();
	UFUNCTION() void HandleContinue();
	UFUNCTION() void HandleCredits();
	UFUNCTION() void HandleQuit();
	UFUNCTION() void HandleConfirmYes();
	UFUNCTION() void HandleBack();

	UPROPERTY(Transient) TObjectPtr<UTextBlock> TitleBlock;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PromptBlock;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> MenuBox;
	UPROPERTY(Transient) TObjectPtr<UBorder> CreditsPanel;
	UPROPERTY(Transient) TObjectPtr<UBorder> ConfirmPanel;
	UPROPERTY(Transient) TObjectPtr<UButton> StartButton;
	UPROPERTY(Transient) TObjectPtr<UButton> ContinueButton;
	UPROPERTY(Transient) TObjectPtr<UButton> CreditsButton;
	UPROPERTY(Transient) TObjectPtr<UButton> QuitButton;
	UPROPERTY(Transient) TObjectPtr<UButton> CreditsBackButton;
	UPROPERTY(Transient) TObjectPtr<UButton> ConfirmYesButton;
	UPROPERTY(Transient) TObjectPtr<UButton> ConfirmNoButton;

	/** Every button and its label, for the selection highlight. */
	UPROPERTY(Transient) TArray<TObjectPtr<UButton>> AllButtons;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> AllLabels;

	/** Button to reselect when a panel closes. */
	UPROPERTY(Transient) TObjectPtr<UButton> ReturnButton;
	UPROPERTY(Transient) TObjectPtr<UButton> LastSelected;

	EStage Stage = EStage::Intro;
	float Elapsed = 0.f;
	float StageTime = 0.f;
	FVector2D LastCursorPos = FVector2D::ZeroVector;
};
