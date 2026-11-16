#include "Core/TsukiProgressSubsystem.h"
#include "Core/TsukiSaveGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const TCHAR* SlotName = TEXT("Tsuki");
	constexpr int32 UserIndex = 0;
}

UTsukiProgressSubsystem* UTsukiProgressSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UTsukiProgressSubsystem>() : nullptr;
}

void UTsukiProgressSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if WITH_EDITOR
	const bool bInEditor = GIsEditor;
#else
	const bool bInEditor = false;
#endif
	if (!bInEditor || bLoadSaveInEditor)
	{
		Load();
	}
}

void UTsukiProgressSubsystem::Load()
{
	const UTsukiSaveGame* Data = Cast<UTsukiSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if (!Data)
	{
		return;
	}
	Tools = TSet<ETsukiTool>(Data->Tools);
	SolvedPuzzles = TSet<FName>(Data->SolvedPuzzles);
	MoonFragments = Data->MoonFragments;
	RespawnTransform = Data->RespawnTransform;
	bHasRespawn = Data->bHasRespawn;
	bLoadedFromSave = true;
}

bool UTsukiProgressSubsystem::LoadProgress()
{
	Load();
	return bLoadedFromSave;
}

bool UTsukiProgressSubsystem::DoesSaveExist()
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

void UTsukiProgressSubsystem::Save() const
{
	UTsukiSaveGame* Data = Cast<UTsukiSaveGame>(UGameplayStatics::CreateSaveGameObject(UTsukiSaveGame::StaticClass()));
	if (!Data)
	{
		return;
	}
	Data->Tools = Tools.Array();
	Data->SolvedPuzzles = SolvedPuzzles.Array();
	Data->MoonFragments = MoonFragments;
	Data->RespawnTransform = RespawnTransform;
	Data->bHasRespawn = bHasRespawn;
	UGameplayStatics::AsyncSaveGameToSlot(Data, SlotName, UserIndex);
}

void UTsukiProgressSubsystem::ResetProgress()
{
	Tools.Reset();
	SolvedPuzzles.Reset();
	MoonFragments = 0;
	bHasRespawn = false;
	bLoadedFromSave = false;
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
	OnFragmentsChanged.Broadcast(MoonFragments);
}

void UTsukiProgressSubsystem::GrantTool(ETsukiTool Tool)
{
	if (!Tools.Contains(Tool))
	{
		Tools.Add(Tool);
		OnToolGranted.Broadcast(Tool);
		Save();
	}
}

void UTsukiProgressSubsystem::AddMoonFragment()
{
	++MoonFragments;
	OnFragmentsChanged.Broadcast(MoonFragments);
	Save();
}

void UTsukiProgressSubsystem::MarkSolved(FName PuzzleId)
{
	if (!PuzzleId.IsNone() && !SolvedPuzzles.Contains(PuzzleId))
	{
		SolvedPuzzles.Add(PuzzleId);
		OnPuzzleSolved.Broadcast(PuzzleId);
		Save();
	}
}

void UTsukiProgressSubsystem::SetRespawnTransform(const FTransform& Transform)
{
	RespawnTransform = Transform;
	bHasRespawn = true;
	Save();
}

bool UTsukiProgressSubsystem::GetRespawnTransform(FTransform& OutTransform) const
{
	OutTransform = RespawnTransform;
	return bHasRespawn;
}
