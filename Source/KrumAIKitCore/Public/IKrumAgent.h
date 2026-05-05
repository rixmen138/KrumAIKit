#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

DECLARE_DELEGATE_OneParam(FOnMessageReceived, const FString& /* Message */);
DECLARE_DELEGATE(FOnAgentConnected);
DECLARE_DELEGATE_OneParam(FOnAgentDisconnected, const FString& /* Reason */);

class IKrumAgent
{
public:
    virtual ~IKrumAgent() = default;

    /** Connects or initializes the agent */
    virtual void Connect() = 0;

    /** Disconnects the agent */
    virtual void Disconnect() = 0;

    /** Checks if the agent is currently connected/ready */
    virtual bool IsConnected() const = 0;

    /** Returns the name of the agent (e.g. "Claude Code", "OpenRouter") */
    virtual FString GetName() const = 0;

    /** Sends a message to the agent */
    virtual void SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError) = 0;

    /** Stops the currently running task/message generation */
    virtual void StopCurrent() = 0;
};
