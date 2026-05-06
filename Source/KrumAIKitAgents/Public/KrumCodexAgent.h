#pragma once

#include "CoreMinimal.h"
#include "IKrumAgent.h"

class KRUMAIKITAGENTS_API FKrumCodexAgent : public IKrumAgent
{
public:
	FKrumCodexAgent();
	virtual ~FKrumCodexAgent() override;

	//~ Begin IKrumAgent Interface
	virtual void Connect() override;
	virtual void Disconnect() override;
	virtual bool IsConnected() const override;
	virtual FString GetName() const override;
	virtual void SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError) override;
	virtual void StopCurrent() override;
	//~ End IKrumAgent Interface

private:
	bool CheckBinaryExists() const;

private:
	bool bIsConnected;
	TSharedPtr<class FInteractiveProcess> CurrentProcess;
	
	FString CodexLineBuffer;
	FString CodexAccumulatedResponse;
};
