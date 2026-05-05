#include "KrumGeminiAgent.h"
#include "Misc/InteractiveProcess.h"
#include "HAL/PlatformProcess.h"
#include "KrumAIKitCore.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

FKrumGeminiAgent::FKrumGeminiAgent()
	: bIsConnected(false)
{
}

FKrumGeminiAgent::~FKrumGeminiAgent()
{
	Disconnect();
}

void FKrumGeminiAgent::Connect()
{
	bIsConnected = CheckBinaryExists();
}

void FKrumGeminiAgent::Disconnect()
{
	StopCurrent();
	bIsConnected = false;
}

bool FKrumGeminiAgent::IsConnected() const
{
	return bIsConnected;
}

FString FKrumGeminiAgent::GetName() const
{
	return TEXT("Gemini CLI");
}

bool FKrumGeminiAgent::CheckBinaryExists() const
{
#if PLATFORM_WINDOWS
	FString BinaryName = TEXT("gemini.cmd");
#else
	FString BinaryName = TEXT("gemini");
#endif

	FString OutPath;
	bool bFound = FPlatformProcess::FindProgramByName(*BinaryName, OutPath);
	if (!bFound)
	{
		UE_LOG(LogKrumAIKit, Warning, TEXT("Gemini CLI not found in PATH. Please install it (e.g. npm install -g @google/gemini-cli)."));
	}
	return bFound;
}

void FKrumGeminiAgent::SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError)
{
	if (!bIsConnected)
	{
		OnError.ExecuteIfBound(TEXT("Gemini CLI is not available in PATH."));
		return;
	}

	StopCurrent();

#if PLATFORM_WINDOWS
	FString URL = TEXT("cmd.exe");
	FString EscapedPrompt = Prompt.Replace(TEXT("\""), TEXT("\\\""));
	FString Args = FString::Printf(TEXT("/c gemini -p \"%s\" --format json"), *EscapedPrompt);
#else
	FString URL = TEXT("gemini");
	FString Args = FString::Printf(TEXT("-p \"%s\" --format json"), *Prompt.Replace(TEXT("\""), TEXT("\\\"")));
#endif

	GeminiAccumulatedOutput.Empty();

	CurrentProcess = MakeShareable(new FInteractiveProcess(URL, Args, true));

	CurrentProcess->OnOutput().BindLambda([this](const FString& Output)
	{
		GeminiAccumulatedOutput += Output;
	});

	CurrentProcess->OnCompleted().BindLambda([this, OnResponse, OnError](int32 ExitCode, bool bCanceled)
	{
		if (bCanceled)
		{
			OnError.ExecuteIfBound(TEXT("Gemini process was canceled."));
			return;
		}

		if (ExitCode != 0)
		{
			OnError.ExecuteIfBound(FString::Printf(TEXT("Gemini process exited with code %d. Output: %s"), ExitCode, *GeminiAccumulatedOutput));
			return;
		}

		TSharedPtr<FJsonObject> JsonObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(GeminiAccumulatedOutput);
		if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* Candidates;
			if (JsonObj->TryGetArrayField(TEXT("candidates"), Candidates) && Candidates->Num() > 0)
			{
				const TSharedPtr<FJsonObject>* ContentObj;
				if ((*Candidates)[0]->AsObject()->TryGetObjectField(TEXT("content"), ContentObj))
				{
					const TArray<TSharedPtr<FJsonValue>>* Parts;
					if ((*ContentObj)->TryGetArrayField(TEXT("parts"), Parts) && Parts->Num() > 0)
					{
						FString TextResult;
						if ((*Parts)[0]->AsObject()->TryGetStringField(TEXT("text"), TextResult))
						{
							OnResponse.ExecuteIfBound(TextResult);
							return;
						}
					}
				}
			}
		}

		OnError.ExecuteIfBound(TEXT("Failed to parse Gemini JSON output."));
	});

	CurrentProcess->Launch();
}

void FKrumGeminiAgent::StopCurrent()
{
	if (CurrentProcess.IsValid() && CurrentProcess->IsRunning())
	{
		CurrentProcess->Cancel(true);
	}
	CurrentProcess.Reset();
}
