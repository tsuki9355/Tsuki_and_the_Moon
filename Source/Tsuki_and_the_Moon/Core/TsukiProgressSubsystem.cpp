#include "Core/TsukiProgressSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UTsukiProgressSubsystem* UTsukiProgressSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UTsukiProgressSubsystem>() : nullptr;
}

void UTsukiProgressSubsystem::GrantTool(ETsukiTool Tool)
{
	if (!Tools.Contains(Tool))
	{
		Tools.Add(Tool);
		OnToolGranted.Broadcast(Tool);
	}
}

void UTsukiProgressSubsystem::AddMoonFragment()
{
	++MoonFragments;
	OnFragmentsChanged.Broadcast(MoonFragments);
}

void UTsukiProgressSubsystem::MarkSolved(FName PuzzleId)
{
	if (!PuzzleId.IsNone() && !SolvedPuzzles.Contains(PuzzleId))
	{
		SolvedPuzzles.Add(PuzzleId);
		OnPuzzleSolved.Broadcast(PuzzleId);
	}
}

void UTsukiProgressSubsystem::SetRespawnTransform(const FTransform& Transform)
{
	RespawnTransform = Transform;
	bHasRespawn = true;
}

bool UTsukiProgressSubsystem::GetRespawnTransform(FTransform& OutTransform) const
{
	OutTransform = RespawnTransform;
	return bHasRespawn;
}
