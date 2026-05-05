#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"

// Runs a background thread reading JSON-RPC 2.0 from stdin,
// dispatching to FKrumToolRegistry, writing responses to stdout.
class KRUMAIKITCORE_API FKrumMCPServer : public FRunnable
{
public:
    FKrumMCPServer();
    virtual ~FKrumMCPServer();

    bool Start();
    void Stop();

    // FRunnable interface
    virtual uint32 Run() override;
    virtual void Exit() override;

private:
    FString HandleRequest(const FString& RawJson);
    FString HandleToolsList(const FString& RequestId);
    FString HandleToolsCall(const TSharedPtr<FJsonObject>& Params, const FString& RequestId);
    FString MakeErrorResponse(const FString& Id, int32 Code, const FString& Message);
    FString MakeSuccessResponse(const FString& Id, TSharedPtr<FJsonObject> Result);
	TSharedPtr<FJsonObject> InitializeResultObject();

    TAtomic<bool> bShouldRun;
    FRunnableThread* Thread = nullptr;
};
