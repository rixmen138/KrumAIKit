#include "KrumCodexAgent.h"
#include "Misc/InteractiveProcess.h"
#include "HAL/PlatformProcess.h"
#include "KrumAIKitCore.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

FKrumCodexAgent::FKrumCodexAgent()
	: bIsConnected(false)
{
}

FKrumCodexAgent::~FKrumCodexAgent()
{
	Disconnect();
}

void FKrumCodexAgent::Connect()
{
	bIsConnected = CheckBinaryExists();
}

void FKrumCodexAgent::Disconnect()
{
	StopCurrent();
	bIsConnected = false;
}

bool FKrumCodexAgent::IsConnected() const
{
	return bIsConnected;
}

FString FKrumCodexAgent::GetName() const
{
	return TEXT("OpenAI Codex CLI");
}

bool FKrumCodexAgent::CheckBinaryExists() const
{
#if PLATFORM_WINDOWS
	FString BinaryName = TEXT("codex.cmd");
#else
	FString BinaryName = TEXT("codex");
#endif

	FString OutPath;
	bool bFound = FPlatformProcess::FindProgramByName(*BinaryName, OutPath);
	if (!bFound)
	{
		UE_LOG(LogKrumAIKit, Warning, TEXT("Codex CLI not found in PATH."));
	}
	return bFound;
}

void FKrumCodexAgent::SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError)
{
	if (!bIsConnected)
	{
		OnError.ExecuteIfBound(TEXT("Codex CLI is not available in PATH."));
		return;
	}

	StopCurrent();

#if PLATFORM_WINDOWS
	FString URL = TEXT("cmd.exe");
	FString EscapedPrompt = Prompt.Replace(TEXT("\""), TEXT("\\\""));
	FString Args = FString::Printf(TEXT("/c codex \"%s\" --json"), *EscapedPrompt);
#else
	FString URL = TEXT("codex");
	FString Args = FString::Printf(TEXT("\"%s\" --json"), *Prompt.Replace(TEXT("\""), TEXT("\\\"")));
#endif

	CodexLineBuffer.Empty();
	CodexAccumulatedResponse.Empty();

	CurrentProcess = MakeShareable(new FInteractiveProcess(URL, Args, true));

	CurrentProcess->OnOutput().BindLambda([this, OnError](const FString& Output)
	{
		CodexLineBuffer += Output;

		int32 NewlineIndex;
		while (CodexLineBuffer.FindChar('\n', NewlineIndex))
		{
			FString Line = CodexLineBuffer.Left(NewlineIndex).TrimStartAndEnd();
			CodexLineBuffer = CodexLineBuffer.Mid(NewlineIndex + 1);

			if (Line.IsEmpty()) continue;

			TSharedPtr<FJsonObject> JsonObj;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Line);
			if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
			{
				FString Type;
				if (JsonObj->TryGetStringField(TEXT("type"), Type))
				{
					if (Type == TEXT("message"))
					{
						FString Content;
						if (JsonObj->TryGetStringField(TEXT("content"), Content))
						{
							CodexAccumulatedResponse += Content;
						}
					}
					else if (Type == TEXT("error"))
					{
						FString ErrorMsg;
						if (JsonObj->TryGetStringField(TEXT("message"), ErrorMsg))
						{
							OnError.ExecuteIfBound(ErrorMsg);
						}
					}
				}
			}
		}
	});

	CurrentProcess->OnCompleted().BindLambda([this, OnResponse, OnError](int32 ExitCode, bool bCanceled)
	{
		if (bCanceled)
		{
			OnError.ExecuteIfBound(TEXT("Codex process canceled."));
			return;
		}

		if (ExitCode != 0 && CodexAccumulatedResponse.IsEmpty())
		{
			OnError.ExecuteIfBound(FString::Printf(TEXT("Codex process exited with code %d"), ExitCode));
			return;
		}

		OnResponse.ExecuteIfBound(CodexAccumulatedResponse);
	});

	CurrentProcess->Launch();
}

void FKrumCodexAgent::StopCurrent()
{
	if (CurrentProcess.IsValid() && CurrentProcess->IsRunning())
	{
		CurrentProcess->Cancel(true);
	}
	CurrentProcess.Reset();
}
