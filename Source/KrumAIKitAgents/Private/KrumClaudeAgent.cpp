#include "KrumClaudeAgent.h"
#include "Misc/InteractiveProcess.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "KrumAIKitCore.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

FKrumClaudeAgent::FKrumClaudeAgent()
	: bIsConnected(false)
{
}

FKrumClaudeAgent::~FKrumClaudeAgent()
{
	Disconnect();
}

void FKrumClaudeAgent::Connect()
{
	bIsConnected = CheckBinaryExists();
}

void FKrumClaudeAgent::Disconnect()
{
	StopCurrent();
	bIsConnected = false;
}

bool FKrumClaudeAgent::IsConnected() const
{
	return bIsConnected;
}

FString FKrumClaudeAgent::GetName() const
{
	return TEXT("Claude Code CLI");
}

bool FKrumClaudeAgent::CheckBinaryExists() const
{
	// Assuming it's in PATH or let the OS resolve it when executing via cmd/sh.
	// FPlatformProcess::FindProgramByName is not available in UE5.7.
	return true;
}

void FKrumClaudeAgent::SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError)
{
	if (!bIsConnected)
	{
		OnError.ExecuteIfBound(TEXT("Claude CLI is not available in PATH. Please install it via npm."));
		return;
	}

	StopCurrent();

	FString TempFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("KrumAIKit") / TEXT("prompt_temp.txt"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(TempFilePath));
	FFileHelper::SaveStringToFile(Prompt, *TempFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

#if PLATFORM_WINDOWS
	FString URL = TEXT("cmd.exe");
	FString Args = FString::Printf(TEXT("/c claude --print --output-format stream-json --verbose < \"%s\""), *TempFilePath);
#else
	FString URL = TEXT("sh");
	FString Args = FString::Printf(TEXT("-c \"claude --print --output-format stream-json --verbose < '%s'\""), *TempFilePath);
#endif

	ClaudeLineBuffer.Empty();
	ClaudeAccumulatedResponse.Empty();

	CurrentProcess = MakeShareable(new FInteractiveProcess(URL, Args, true));

	CurrentProcess->OnOutput().BindLambda([this, OnResponse](const FString& Output)
	{
		ClaudeLineBuffer += Output;

		// Process lines
		int32 NewlineIndex;
		while (ClaudeLineBuffer.FindChar('\n', NewlineIndex))
		{
			FString Line = ClaudeLineBuffer.Left(NewlineIndex).TrimStartAndEnd();
			ClaudeLineBuffer = ClaudeLineBuffer.Mid(NewlineIndex + 1);

			if (Line.IsEmpty())
			{
				continue;
			}

			TSharedPtr<FJsonObject> JsonObj;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Line);
			if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
			{
				FString Type;
				if (JsonObj->TryGetStringField(TEXT("type"), Type))
				{
					if (Type == TEXT("content_block_delta"))
					{
						const TSharedPtr<FJsonObject>* DeltaObj;
						if (JsonObj->TryGetObjectField(TEXT("delta"), DeltaObj))
						{
							FString TextDelta;
							if ((*DeltaObj)->TryGetStringField(TEXT("text"), TextDelta))
							{
								ClaudeAccumulatedResponse += TextDelta;
							}
						}
					}
					else if (Type == TEXT("message_stop"))
					{
						OnResponse.ExecuteIfBound(ClaudeAccumulatedResponse);
					}
				}
			}
		}
	});

	CurrentProcess->OnCanceled().BindLambda([OnError]()
	{
		OnError.ExecuteIfBound(TEXT("Claude process was canceled."));
	});

	CurrentProcess->Launch();
}

void FKrumClaudeAgent::StopCurrent()
{
	if (CurrentProcess.IsValid() && CurrentProcess->IsRunning())
	{
		CurrentProcess->Cancel(true);
	}
	CurrentProcess.Reset();
}
