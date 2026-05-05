#include "SetMaterialVectorParamTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Materials/MaterialInstanceConstant.h"

FString FSetMaterialVectorParamTool::GetName() const
{
	return TEXT("SetMaterialVectorParam");
}

FString FSetMaterialVectorParamTool::GetDescription() const
{
	return TEXT("Sets a vector parameter on a material instance.");
}

TSharedPtr<FJsonObject> FSetMaterialVectorParamTool::GetSchema() const
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

	for (const FString& Comp : {TEXT("r"), TEXT("g"), TEXT("b"), TEXT("a")})
	{
		TSharedPtr<FJsonObject> CompProp = MakeShareable(new FJsonObject());
		CompProp->SetStringField(TEXT("type"), TEXT("number"));
		Properties->SetObjectField(Comp, CompProp);
	}

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("instance_path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("param_name"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("r"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("g"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("b"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("a"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FSetMaterialVectorParamTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString InstancePath, ParamName;
	double R, G, B, A;

	if (!Params->TryGetStringField(TEXT("instance_path"), InstancePath) ||
		!Params->TryGetStringField(TEXT("param_name"), ParamName) ||
		!Params->TryGetNumberField(TEXT("r"), R) ||
		!Params->TryGetNumberField(TEXT("g"), G) ||
		!Params->TryGetNumberField(TEXT("b"), B) ||
		!Params->TryGetNumberField(TEXT("a"), A))
	{
		return TEXT("{\"error\": \"Missing required parameters\"}");
	}

	UMaterialInstanceConstant* MIC = LoadObject<UMaterialInstanceConstant>(nullptr, *InstancePath);
	if (!MIC)
	{
		return TEXT("{\"error\": \"Failed to load Material Instance.\"}");
	}

	FMaterialParameterInfo ParamInfo(*ParamName, EMaterialParameterAssociation::GlobalParameter, INDEX_NONE);
	MIC->SetVectorParameterValueEditorOnly(ParamInfo, FLinearColor(R, G, B, A));
	MIC->MarkPackageDirty();

	return TEXT("{\"status\": \"success\"}");
}
