#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KrumSettings.generated.h"

UCLASS(config=EditorPerProjectUserSettings, defaultconfig)
class KRUMAIKITCORE_API UKrumSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    UKrumSettings();

    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
    virtual FName GetSectionName()  const override { return TEXT("KrumAIKit"); }

    UPROPERTY(config, EditAnywhere, Category="Agents", meta=(DisplayName="OpenRouter API Key"))
    FString OpenRouterApiKey;

    UPROPERTY(config, EditAnywhere, Category="Agents", meta=(DisplayName="OpenRouter Model"))
    FString OpenRouterModel = TEXT("anthropic/claude-3.5-sonnet");

    UPROPERTY(config, EditAnywhere, Category="Agents", meta=(DisplayName="Ollama Base URL"))
    FString OllamaBaseUrl = TEXT("http://localhost:11434");

    UPROPERTY(config, EditAnywhere, Category="Agents", meta=(DisplayName="Ollama Model"))
    FString OllamaModel = TEXT("llama3");

    UPROPERTY(config, EditAnywhere, Category="Agents", meta=(DisplayName="Claude CLI Binary Path Override"))
    FString ClaudeBinaryPath;

    UPROPERTY(config, EditAnywhere, Category="Agents", meta=(DisplayName="Gemini CLI Binary Path Override"))
    FString GeminiBinaryPath;

    UPROPERTY(config, EditAnywhere, Category="Features", meta=(DisplayName="Enable Project Indexer"))
    bool bEnableIndexer = true;

    UPROPERTY(config, EditAnywhere, Category="Features", meta=(DisplayName="Auto-inject asset context into prompts"))
    bool bAutoInjectAssetContext = true;

    UPROPERTY(config, EditAnywhere, Category="Features", meta=(DisplayName="Enable MCP Server"))
    bool bEnableMCPServer = false;
};
