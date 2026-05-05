#pragma once

#include "CoreMinimal.h"
#include "IKrumAgent.h"
#include "Interfaces/IHttpRequest.h"

class FKrumOpenRouterAgent : public IKrumAgent
{
public:
	FKrumOpenRouterAgent();
	virtual ~FKrumOpenRouterAgent() override;

	//~ Begin IKrumAgent Interface
	virtual void Connect() override;
	virtual void Disconnect() override;
	virtual bool IsConnected() const override;
	virtual FString GetName() const override;
	virtual void SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError) override;
	virtual void StopCurrent() override;
	//~ End IKrumAgent Interface

	void SetApiKey(const FString& InApiKey);
	void SetModel(const FString& Model);

private:
	void OnStreamProgress(FHttpRequestPtr Req, uint64 BytesSent, uint64 BytesReceived, FOnMessageReceived OnResponse, FOnMessageReceived OnError);
	void OnStreamComplete(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess, FOnMessageReceived OnError);

private:
	FString ApiKey;
	FString StreamBuffer;
	FString CurrentModel = TEXT("anthropic/claude-3.5-sonnet");
	bool bIsConnected;
	TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
};
