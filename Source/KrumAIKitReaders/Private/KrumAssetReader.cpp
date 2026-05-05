#include "KrumAssetReader.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/DataTable.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundCue.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"

FString FKrumAssetReader::ReadAsset(const FString& AssetPath)
{
	IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	FSoftObjectPath ObjPath(AssetPath);
	TArray<FAssetData> Results;
	
	AR.GetAssetsByPackageName(FName(*ObjPath.GetLongPackageName()), Results);
	
	if (Results.IsEmpty())
	{
		return FString::Printf(TEXT("{\"error\": \"Could not find asset at path: %s\"}"), *AssetPath);
	}
	
	FAssetData& AD = Results[0];
	FString ClassName = AD.AssetClassPath.GetAssetName().ToString();

	if (ClassName == TEXT("Blueprint")) return ReadBlueprint(AD);
	if (ClassName == TEXT("Material") || ClassName == TEXT("MaterialInstanceConstant")) return ReadMaterial(AD);
	if (ClassName == TEXT("StaticMesh")) return ReadStaticMesh(AD);
	if (ClassName == TEXT("DataTable")) return ReadDataTable(AD);
	
	// Default
	return ReadGeneric(AD);
}

FString FKrumAssetReader::SearchAssets(const FString& Query, const FString& ClassFilter)
{
	IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	TArray<FAssetData> AllAssets;
	AR.GetAllAssets(AllAssets, true);

	TArray<TSharedPtr<FJsonValue>> ResultsArray;
	FString QueryLower = Query.ToLower();
	FString ClassLower = ClassFilter.ToLower();

	for (const FAssetData& AD : AllAssets)
	{
		FString AssetName = AD.AssetName.ToString();
		if (QueryLower.IsEmpty() || AssetName.ToLower().Contains(QueryLower))
		{
			FString ClassName = AD.AssetClassPath.GetAssetName().ToString();
			if (ClassLower.IsEmpty() || ClassName.ToLower() == ClassLower)
			{
				TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject());
				Obj->SetStringField(TEXT("path"), AD.GetSoftObjectPath().ToString());
				Obj->SetStringField(TEXT("name"), AssetName);
				Obj->SetStringField(TEXT("class"), ClassName);
				
				int64 SizeBytes = 0;
				if (AD.GetTagValue(TEXT("ApproxSizeBytes"), SizeBytes))
				{
					Obj->SetNumberField(TEXT("size_kb"), SizeBytes / 1024);
				}

				ResultsArray.Add(MakeShareable(new FJsonValueObject(Obj)));
				
				if (ResultsArray.Num() >= 20) break; // limit to 20
			}
		}
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(ResultsArray, Writer);
	return OutputString;
}

FString FKrumAssetReader::ReadBlueprint(const FAssetData& AD)
{
	UBlueprint* BP = Cast<UBlueprint>(AD.GetAsset());
	if (!BP) return ReadGeneric(AD);

	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("type"), TEXT("Blueprint"));
	ResultObj->SetStringField(TEXT("path"), AD.GetSoftObjectPath().ToString());
	
	if (BP->ParentClass)
	{
		ResultObj->SetStringField(TEXT("parent_class"), BP->ParentClass->GetName());
	}

	// Variables
	TArray<TSharedPtr<FJsonValue>> VarsArray;
	for (const FBPVariableDescription& VarDesc : BP->NewVariables)
	{
		TSharedPtr<FJsonObject> VarObj = MakeShareable(new FJsonObject());
		VarObj->SetStringField(TEXT("name"), VarDesc.VarName.ToString());
		VarObj->SetStringField(TEXT("type"), VarDesc.VarType.PinCategory.ToString());
		VarsArray.Add(MakeShareable(new FJsonValueObject(VarObj)));
	}
	ResultObj->SetArrayField(TEXT("variables"), VarsArray);

	// Components
	TArray<TSharedPtr<FJsonValue>> CompsArray;
	USimpleConstructionScript* SCS = BP->SimpleConstructionScript.Get();
	if (SCS)
	{
		for (USCS_Node* Node : SCS->GetAllNodes())
		{
			if (Node && Node->ComponentClass)
			{
				TSharedPtr<FJsonObject> CompObj = MakeShareable(new FJsonObject());
				CompObj->SetStringField(TEXT("name"), Node->GetVariableName().ToString());
				CompObj->SetStringField(TEXT("class"), Node->ComponentClass->GetName());
				CompsArray.Add(MakeShareable(new FJsonValueObject(CompObj)));
			}
		}
	}
	ResultObj->SetArrayField(TEXT("components"), CompsArray);

	// Functions
	TArray<TSharedPtr<FJsonValue>> FuncsArray;
	for (UEdGraph* Graph : BP->FunctionGraphs)
	{
		if (Graph)
		{
			TSharedPtr<FJsonObject> FuncObj = MakeShareable(new FJsonObject());
			FuncObj->SetStringField(TEXT("name"), Graph->GetName());
			FuncsArray.Add(MakeShareable(new FJsonValueObject(FuncObj)));
		}
	}
	ResultObj->SetArrayField(TEXT("functions"), FuncsArray);

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
	return ResultString;
}

FString FKrumAssetReader::ReadMaterial(const FAssetData& AD)
{
	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("type"), AD.AssetClassPath.GetAssetName().ToString());
	ResultObj->SetStringField(TEXT("path"), AD.GetSoftObjectPath().ToString());

	if (UMaterial* Mat = Cast<UMaterial>(AD.GetAsset()))
	{
		ResultObj->SetNumberField(TEXT("shading_model"), (int)Mat->GetShadingModels().GetFirstShadingModel());
		ResultObj->SetNumberField(TEXT("blend_mode"), (int)Mat->BlendMode);
		ResultObj->SetBoolField(TEXT("two_sided"), Mat->TwoSided);

		TArray<FMaterialParameterInfo> Params; 
		TArray<FGuid> Guids;
		
		Mat->GetAllScalarParameterInfo(Params, Guids);
		ResultObj->SetNumberField(TEXT("scalar_param_count"), Params.Num());
		
		Params.Empty(); Guids.Empty();
		Mat->GetAllVectorParameterInfo(Params, Guids);
		ResultObj->SetNumberField(TEXT("vector_param_count"), Params.Num());
	}
	else if (UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(AD.GetAsset()))
	{
		if (MIC->Parent)
		{
			ResultObj->SetStringField(TEXT("parent"), MIC->Parent->GetName());
		}
	}

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
	return ResultString;
}

FString FKrumAssetReader::ReadStaticMesh(const FAssetData& AD)
{
	UStaticMesh* SM = Cast<UStaticMesh>(AD.GetAsset());
	if (!SM) return ReadGeneric(AD);

	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("type"), TEXT("StaticMesh"));
	ResultObj->SetStringField(TEXT("path"), AD.GetSoftObjectPath().ToString());
	ResultObj->SetNumberField(TEXT("lod_count"), SM->GetNumLODs());
	ResultObj->SetNumberField(TEXT("material_slots"), SM->GetStaticMaterials().Num());
	ResultObj->SetNumberField(TEXT("triangle_count"), SM->GetNumTriangles(0));

	FBoxSphereBounds Bounds = SM->GetBounds();
	TSharedPtr<FJsonObject> BoundsObj = MakeShareable(new FJsonObject());
	BoundsObj->SetNumberField(TEXT("x"), Bounds.BoxExtent.X);
	BoundsObj->SetNumberField(TEXT("y"), Bounds.BoxExtent.Y);
	BoundsObj->SetNumberField(TEXT("z"), Bounds.BoxExtent.Z);
	ResultObj->SetObjectField(TEXT("bounds"), BoundsObj);

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
	return ResultString;
}

FString FKrumAssetReader::ReadDataTable(const FAssetData& AD)
{
	UDataTable* DT = Cast<UDataTable>(AD.GetAsset());
	if (!DT) return ReadGeneric(AD);

	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("type"), TEXT("DataTable"));
	ResultObj->SetStringField(TEXT("path"), AD.GetSoftObjectPath().ToString());
	ResultObj->SetStringField(TEXT("row_struct"), DT->RowStruct ? DT->RowStruct->GetName() : TEXT(""));
	ResultObj->SetNumberField(TEXT("row_count"), DT->GetRowMap().Num());

	TArray<TSharedPtr<FJsonValue>> RowNamesArray;
	int32 Count = 0;
	for (auto& Pair : DT->GetRowMap())
	{
		RowNamesArray.Add(MakeShareable(new FJsonValueString(Pair.Key.ToString())));
		if (++Count >= 10) break;
	}
	ResultObj->SetArrayField(TEXT("row_names"), RowNamesArray);

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
	return ResultString;
}

FString FKrumAssetReader::ReadGeneric(const FAssetData& AD)
{
	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("type"), AD.AssetClassPath.GetAssetName().ToString());
	ResultObj->SetStringField(TEXT("path"), AD.GetSoftObjectPath().ToString());
	ResultObj->SetStringField(TEXT("name"), AD.AssetName.ToString());
	
	int64 SizeBytes = 0;
	if (AD.GetTagValue(TEXT("ApproxSizeBytes"), SizeBytes))
	{
		ResultObj->SetNumberField(TEXT("size_kb"), SizeBytes / 1024);
	}

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
	return ResultString;
}
