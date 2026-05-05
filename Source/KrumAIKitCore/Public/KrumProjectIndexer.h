#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"

struct FKrumAssetEntry
{
    FString AssetPath;   // e.g. /Game/Blueprints/BP_Guard
    FString AssetName;   // e.g. BP_Guard
    FString AssetClass;  // e.g. Blueprint
    int64   SizeBytes = 0;
};

class KRUMAIKITCORE_API FKrumProjectIndexer : public FRunnable
{
public:
    static FKrumProjectIndexer& Get();

    void Start();
    void Stop();

    // Returns up to MaxResults entries whose name contains any word from Query
    TArray<FKrumAssetEntry> SearchIndex(const FString& Query, int32 MaxResults = 5) const;

    // Returns top 3 matching entries as formatted string for injecting into agent context
    FString GetContextSnippet(const FString& Query) const;

    // FRunnable
    virtual uint32 Run() override;
    virtual void Exit() override;

private:
    void RebuildIndex();

    TArray<FKrumAssetEntry> Index;
    mutable FCriticalSection IndexLock;
    TAtomic<bool> bShouldRun;
    FRunnableThread* Thread = nullptr;
};
