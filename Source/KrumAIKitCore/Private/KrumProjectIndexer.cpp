#include "KrumProjectIndexer.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/PlatformProcess.h"

FKrumProjectIndexer& FKrumProjectIndexer::Get()
{
	static FKrumProjectIndexer Instance;
	return Instance;
}

void FKrumProjectIndexer::Start()
{
	if (Thread) return;
	bShouldRun = true;
	Thread = FRunnableThread::Create(this, TEXT("FKrumProjectIndexer"), 0, TPri_Lowest);
}

void FKrumProjectIndexer::Stop()
{
	if (!Thread) return;
	bShouldRun = false;
	Thread->WaitForCompletion();
	delete Thread;
	Thread = nullptr;
}

uint32 FKrumProjectIndexer::Run()
{
	RebuildIndex();
	
	float ElapsedSec = 0.f;
	while (bShouldRun)
	{
		FPlatformProcess::Sleep(1.0f);
		ElapsedSec += 1.0f;
		if (ElapsedSec >= 30.0f)
		{
			RebuildIndex();
			ElapsedSec = 0.f;
		}
	}
	return 0;
}

void FKrumProjectIndexer::Exit()
{
	bShouldRun = false;
}

void FKrumProjectIndexer::RebuildIndex()
{
	IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	TArray<FAssetData> AllAssets;
	AR.GetAllAssets(AllAssets, true);

	TArray<FKrumAssetEntry> NewIndex;
	for (const FAssetData& AD : AllAssets)
	{
		FKrumAssetEntry Entry;
		Entry.AssetPath  = AD.GetSoftObjectPath().ToString();
		Entry.AssetName  = AD.AssetName.ToString();
		Entry.AssetClass = AD.AssetClassPath.GetAssetName().ToString();
		
		int64 SizeBytes = 0;
		if (AD.GetTagValue(TEXT("ApproxSizeBytes"), SizeBytes))
		{
			Entry.SizeBytes = SizeBytes;
		}

		NewIndex.Add(Entry);
	}

	FScopeLock Lock(&IndexLock);
	Index = MoveTemp(NewIndex);
}

TArray<FKrumAssetEntry> FKrumProjectIndexer::SearchIndex(const FString& Query, int32 MaxResults) const
{
	FScopeLock Lock(&IndexLock);
	TArray<FKrumAssetEntry> Results;
	if (Query.IsEmpty()) return Results;

	TArray<FString> Words;
	Query.ParseIntoArray(Words, TEXT(" "), true);
	
	if (Words.IsEmpty()) return Results;

	for (const FKrumAssetEntry& Entry : Index)
	{
		bool bMatch = false;
		for (const FString& Word : Words)
		{
			if (Entry.AssetName.Contains(Word, ESearchCase::IgnoreCase))
			{
				bMatch = true;
				break;
			}
		}

		if (bMatch)
		{
			Results.Add(Entry);
			if (Results.Num() >= MaxResults) break;
		}
	}

	return Results;
}

FString FKrumProjectIndexer::GetContextSnippet(const FString& Query) const
{
	TArray<FKrumAssetEntry> Results = SearchIndex(Query, 3);
	if (Results.IsEmpty()) return TEXT("");

	FString Snippet = TEXT("Relevant assets found in project:\n");
	for (const FKrumAssetEntry& Entry : Results)
	{
		Snippet += FString::Printf(TEXT("- %s: %s\n"), *Entry.AssetClass, *Entry.AssetPath);
	}
	return Snippet;
}
