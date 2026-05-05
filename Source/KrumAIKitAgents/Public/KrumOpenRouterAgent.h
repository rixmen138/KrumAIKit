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

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnMessageReceived OnResponseCallback, FOnMessageReceived OnErrorCallback);

private:
	FString ApiKey;
	bool bIsConnected;
	TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
};
