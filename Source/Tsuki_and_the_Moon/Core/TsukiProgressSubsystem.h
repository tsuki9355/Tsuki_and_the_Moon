#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Properties/MoonPropertyTypes.h"
#include "TsukiProgressSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToolGranted, ETsukiTool, Tool);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFragmentsChanged, int32, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPuzzleSolved, FName, PuzzleId);

/**
 * Slice progress: obtained tools, Moon Fragments, solved puzzles and the active checkpoint.
 * Written to the "Tsuki" save slot whenever it changes (checkpoints, pickups, solved puzzles) and
 * loaded when the game starts. In the editor (PIE) loading is off unless bLoadSaveInEditor is set,
 * so test runs start fresh.
 */
UCLASS(Config = Game)
class TSUKI_AND_THE_MOON_API UTsukiProgressSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Load the save in Play-In-Editor too (off by default so PIE starts fresh). */
	UPROPERTY(Config)
	bool bLoadSaveInEditor = false;

	/** Erase the save and reset progress (used by "New Game"). */
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void ResetProgress();

	/** Load the save slot into progress (title screen Continue). Returns false if there is no save. */
	UFUNCTION(BlueprintCallable, Category = "Progress")
	bool LoadProgress();

	/** True if the save slot exists on disk. Usage: Continue is offered only when this is true. */
	UFUNCTION(BlueprintPure, Category = "Progress")
	static bool DoesSaveExist();

	UFUNCTION(BlueprintPure, Category = "Progress")
	bool HasLoadedSave() const { return bLoadedFromSave; }
	UFUNCTION(BlueprintPure, Category = "Progress")
	bool HasTool(ETsukiTool Tool) const { return Tools.Contains(Tool); }

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void GrantTool(ETsukiTool Tool);

	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetMoonFragments() const { return MoonFragments; }

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void AddMoonFragment();

	UFUNCTION(BlueprintPure, Category = "Progress")
	bool IsSolved(FName PuzzleId) const { return !PuzzleId.IsNone() && SolvedPuzzles.Contains(PuzzleId); }

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void MarkSolved(FName PuzzleId);

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void SetRespawnTransform(const FTransform& Transform);

	/** Returns false if no checkpoint has been reached yet. */
	UFUNCTION(BlueprintPure, Category = "Progress")
	bool GetRespawnTransform(FTransform& OutTransform) const;

	UPROPERTY(BlueprintAssignable, Category = "Progress")
	FOnToolGranted OnToolGranted;

	UPROPERTY(BlueprintAssignable, Category = "Progress")
	FOnFragmentsChanged OnFragmentsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progress")
	FOnPuzzleSolved OnPuzzleSolved;

	static UTsukiProgressSubsystem* Get(const UObject* WorldContext);

private:
	UPROPERTY()
	TSet<ETsukiTool> Tools;

	UPROPERTY()
	TSet<FName> SolvedPuzzles;

	void Save() const;
	void Load();

	int32 MoonFragments = 0;
	FTransform RespawnTransform;
	bool bHasRespawn = false;
	bool bLoadedFromSave = false;
};
