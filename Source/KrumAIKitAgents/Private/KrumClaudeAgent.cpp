#include "KrumClaudeAgent.h"
#include "Misc/InteractiveProcess.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

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
	// Check if 'claude' exists in PATH or common locations
	// For simplicity in this implementation, we assume it's in PATH if we can get a valid binary path.
	// In a real scenario, you'd execute `claude --version` or similar, or find it directly.
#if PLATFORM_WINDOWS
	FString BinaryName = TEXT("claude.cmd");
#else
	FString BinaryName = TEXT("claude");
#endif
	
	// Just return true for now assuming it's available in PATH
	// (Actual check could block or requires launching a dummy process)
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

#if PLATFORM_WINDOWS
	FString URL = TEXT("cmd.exe");
	// Note: We're passing the prompt in arguments, escaping quotes might be needed
	FString EscapedPrompt = Prompt.Replace(TEXT("\""), TEXT("\\\""));
	FString Args = FString::Printf(TEXT("/c claude \"%s\" --print --output-format stream-json"), *EscapedPrompt);
#else
	FString URL = TEXT("claude");
	FString Args = FString::Printf(TEXT("\"%s\" --print --output-format stream-json"), *Prompt.Replace(TEXT("\""), TEXT("\\\"")));
#endif

	CurrentProcess = MakeShareable(new FInteractiveProcess(URL, Args, true));

	CurrentProcess->OnOutput().BindLambda([OnResponse](const FString& Output)
	{
		// Claude CLI with stream-json outputs JSON lines.
		// For basic implementation, we just pass the raw output (which UI could parse or just display).
		// A full implementation would buffer and parse JSON here.
		OnResponse.ExecuteIfBound(Output);
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
