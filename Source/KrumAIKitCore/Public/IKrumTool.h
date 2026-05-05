#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Dom/JsonObject.h"

class IKrumTool
{
public:
    virtual ~IKrumTool() = default;

    /** Returns the unique name of the tool */
    virtual FString GetName() const = 0;

    /** Returns a brief description of what the tool does */
    virtual FString GetDescription() const = 0;

    /** Returns the JSON schema describing the inputs for this tool */
    virtual TSharedPtr<FJsonObject> GetSchema() const = 0;

    /** Executes the tool with the given parameters and returns a string result */
    virtual FString Execute(const TSharedPtr<FJsonObject>& Params) = 0;
};
