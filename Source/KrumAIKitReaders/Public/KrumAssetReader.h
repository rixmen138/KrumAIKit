#pragma once
#include "CoreMinimal.h"

class KRUMAIKITREADERS_API FKrumAssetReader
{
public:
    // Returns rich JSON description of any asset at the given path.
    // Dispatches to the appropriate typed reader based on asset class.
    static FString ReadAsset(const FString& AssetPath);

    // Searches Content Browser by query string and optional class filter.
    // Returns JSON array of { "path", "name", "class", "size_kb" }
    static FString SearchAssets(const FString& Query, const FString& ClassFilter = TEXT(""));

private:
	static FString ReadBlueprint(const struct FAssetData& AD);
	static FString ReadMaterial(const struct FAssetData& AD);
	static FString ReadStaticMesh(const struct FAssetData& AD);
	static FString ReadDataTable(const struct FAssetData& AD);
	static FString ReadGeneric(const struct FAssetData& AD);
};
