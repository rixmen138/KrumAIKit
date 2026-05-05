#pragma once
#include "CoreMinimal.h"
#include "IKrumTool.h"

class KRUMAIKITCORE_API FKrumToolRegistry
{
public:
    static FKrumToolRegistry& Get();

    void RegisterTool(TSharedPtr<IKrumTool> Tool);
    void UnregisterAll();

    const TArray<TSharedPtr<IKrumTool>>& GetTools() const { return Tools; }
    TSharedPtr<IKrumTool> FindTool(const FString& Name) const;

    // Returns JSON schema array of all tools (for injecting into agent system prompt)
    FString GetToolsSchemaJson() const;

private:
    TArray<TSharedPtr<IKrumTool>> Tools;
};
