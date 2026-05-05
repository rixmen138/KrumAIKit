#include "KrumOpenCodeAgent.h"
#include "Misc/InteractiveProcess.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "KrumAIKitCore.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

FKrumOpenCodeAgent::FKrumOpenCodeAgent()
	: bIsConnected(false)
{
}

FKrumOpenCodeAgent::~FKrumOpenCodeAgent()
{
	Disconnect();
}

void FKrumOpenCodeAgent::Connect()
{
	bIsConnected = CheckBinaryExists();
}

void FKrumOpenCodeAgent::Disconnect()
{
	StopCurrent();
	bIsConnected = false;
}

bool FKrumOpenCodeAgent::IsConnected() const
{
	return bIsConnected;
}

FString FKrumOpenCodeAgent::GetName() const
{
	return TEXT("OpenCode CLI");
}

bool FKrumOpenCodeAgent::CheckBinaryExists() const
{
#if PLATFORM_WINDOWS
	FString BinaryName = TEXT("opencode.cmd");
#else
	FString BinaryName = TEXT("opencode");
#endif

	FString OutPath;
	bool bFound = FPlatformProcess::FindProgramByName(*BinaryName, OutPath);
	if (!bFound)
	{
		UE_LOG(LogKrumAIKit, Warning, TEXT("OpenCode CLI not found in PATH."));
	}
	return bFound;
}

void FKrumOpenCodeAgent::SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError)
{
	if (!bIsConnected)
	{
		OnError.ExecuteIfBound(TEXT("OpenCode CLI is not available in PATH."));
		return;
	}

	StopCurrent();

	FString TempFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("KrumAIKit") / TEXT("opencode_prompt_temp.txt"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(TempFilePath));
	FFileHelper::SaveStringToFile(Prompt, *TempFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

#if PLATFORM_WINDOWS
	FString URL = TEXT("cmd.exe");
	FString Args = FString::Printf(TEXT("/c opencode run --output-format json < \"%s\""), *TempFilePath);
#else
	FString URL = TEXT("sh");
	FString Args = FString::Printf(TEXT("-c \"opencode run --output-format json < '%s'\""), *TempFilePath);
#endif

	OpenCodeAccumulatedOutput.Empty();

	CurrentProcess = MakeShareable(new FInteractiveProcess(URL, Args, true));

	CurrentProcess->OnOutput().BindLambda([this](const FString& Output)
	{
		OpenCodeAccumulatedOutput += Output;
	});

	CurrentProcess->OnCompleted().BindLambda([this, OnResponse, OnError](int32 ExitCode, bool bCanceled)
	{
		if (bCanceled)
		{
			OnError.ExecuteIfBound(TEXT("OpenCode process canceled."));
			return;
		}

		TSharedPtr<FJsonObject> JsonObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OpenCodeAccumulatedOutput);
		if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
		{
			FString TextResult;
			
			const TSharedPtr<FJsonObject>* ResultObj;
			if (JsonObj->TryGetObjectField(TEXT("result"), ResultObj))
			{
				if ((*ResultObj)->TryGetStringField(TEXT("content"), TextResult))
				{
					OnResponse.ExecuteIfBound(TextResult);
					return;
				}
			}
			
			if (JsonObj->TryGetStringField(TEXT("output"), TextResult) || 
				JsonObj->TryGetStringField(TEXT("message"), TextResult))
			{
				OnResponse.ExecuteIfBound(TextResult);
				return;
			}
		}

		// Fallback to raw output if JSON parsing fails or fields missing
		if (!OpenCodeAccumulatedOutput.IsEmpty())
		{
			OnResponse.ExecuteIfBound(OpenCodeAccumulatedOutput);
			return;
		}

		OnError.ExecuteIfBound(TEXT("OpenCode process completed but returned no valid output."));
	});

	CurrentProcess->Launch();
}

void FKrumOpenCodeAgent::StopCurrent()
{
	if (CurrentProcess.IsValid() && CurrentProcess->IsRunning())
	{
		CurrentProcess->Cancel(true);
	}
	CurrentProcess.Reset();
}
