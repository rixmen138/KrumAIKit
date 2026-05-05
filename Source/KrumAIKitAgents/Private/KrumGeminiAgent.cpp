#include "KrumGeminiAgent.h"
#include "Misc/InteractiveProcess.h"
#include "HAL/PlatformProcess.h"

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
	return true; // Simplified check
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

	CurrentProcess = MakeShareable(new FInteractiveProcess(URL, Args, true));

	CurrentProcess->OnOutput().BindLambda([OnResponse](const FString& Output)
	{
		OnResponse.ExecuteIfBound(Output);
	});

	CurrentProcess->OnCanceled().BindLambda([OnError]()
	{
		OnError.ExecuteIfBound(TEXT("Gemini process was canceled."));
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
