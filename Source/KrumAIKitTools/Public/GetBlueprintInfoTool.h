#pragma once
#include "CoreMinimal.h"
#include "IKrumTool.h"

class FGetBlueprintInfoTool : public IKrumTool
{
public:
	virtual FString GetName() const override;
	virtual FString GetDescription() const override;
	virtual TSharedPtr<FJsonObject> GetSchema() const override;
	virtual FString Execute(const TSharedPtr<FJsonObject>& Params) override;
};
