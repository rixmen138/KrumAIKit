#pragma once

#include "CoreMinimal.h"
#include "IKrumAgent.h"
#include "Interfaces/IHttpRequest.h"

class KRUMAIKITAGENTS_API FKrumOllamaAgent : public IKrumAgent
{
public:
	FKrumOllamaAgent();
	virtual ~FKrumOllamaAgent() override;

	//~ Begin IKrumAgent Interface
	virtual void Connect() override;
	virtual void Disconnect() override;
	virtual bool IsConnected() const override;
	virtual FString GetName() const override;
	virtual void SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError) override;
	virtual void StopCurrent() override;
	//~ End IKrumAgent Interface

	void SetBaseUrl(const FString& Url);
	void SetModel(const FString& Model);
	void RefreshModelList(TFunction<void(TArray<FString>)> OnDone);

private:
	void OnChatResponseReceived(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess, FOnMessageReceived OnResponse, FOnMessageReceived OnError);

private:
	FString BaseUrl = TEXT("http://localhost:11434");
	FString CurrentModel = TEXT("llama3");
	TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
	bool bIsConnected = false;
};
