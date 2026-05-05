#include "SetMaterialScalarParamTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Materials/MaterialInstanceConstant.h"

FString FSetMaterialScalarParamTool::GetName() const
{
	return TEXT("SetMaterialScalarParam");
}

FString FSetMaterialScalarParamTool::GetDescription() const
{
	return TEXT("Sets a scalar parameter on a material instance.");
}

TSharedPtr<FJsonObject> FSetMaterialScalarParamTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> InstProp = MakeShareable(new FJsonObject());
	InstProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("instance_path"), InstProp);

	TSharedPtr<FJsonObject> ParamProp = MakeShareable(new FJsonObject());
	ParamProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("param_name"), ParamProp);

	TSharedPtr<FJsonObject> ValProp = MakeShareable(new FJsonObject());
	ValProp->SetStringField(TEXT("type"), TEXT("number"));
	Properties->SetObjectField(TEXT("value"), ValProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("instance_path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("param_name"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("value"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FSetMaterialScalarParamTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString InstancePath, ParamName;
	double Value = 0.0;

	if (!Params->TryGetStringField(TEXT("instance_path"), InstancePath) ||
		!Params->TryGetStringField(TEXT("param_name"), ParamName) ||
		!Params->TryGetNumberField(TEXT("value"), Value))
	{
		return TEXT("{\"error\": \"Missing required parameters\"}");
	}

	UMaterialInstanceConstant* MIC = LoadObject<UMaterialInstanceConstant>(nullptr, *InstancePath);
	if (!MIC)
	{
		return TEXT("{\"error\": \"Failed to load Material Instance.\"}");
	}

	FMaterialParameterInfo ParamInfo(*ParamName, EMaterialParameterAssociation::GlobalParameter, INDEX_NONE);
	MIC->SetScalarParameterValueEditorOnly(ParamInfo, (float)Value);
	MIC->MarkPackageDirty();

	return TEXT("{\"status\": \"success\"}");
}
