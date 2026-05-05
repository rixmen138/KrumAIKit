#include "AddBlackboardKeyTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_String.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Class.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"

FString FAddBlackboardKeyTool::GetName() const
{
	return TEXT("AddBlackboardKey");
}

FString FAddBlackboardKeyTool::GetDescription() const
{
	return TEXT("Adds a new key to a Blackboard asset.");
}

TSharedPtr<FJsonObject> FAddBlackboardKeyTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("bb_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShareable(new FJsonObject());
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("key_name"), NameProp);

	TSharedPtr<FJsonObject> TypeProp = MakeShareable(new FJsonObject());
	TypeProp->SetStringField(TEXT("type"), TEXT("string"));
	TypeProp->SetStringField(TEXT("description"), TEXT("Bool, Int, Float, String, Vector, Rotator, Object, Class, Enum"));
	Properties->SetObjectField(TEXT("key_type"), TypeProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("bb_path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("key_name"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("key_type"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FAddBlackboardKeyTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString BBPath, KeyName, KeyType;
	if (!Params->TryGetStringField(TEXT("bb_path"), BBPath) ||
		!Params->TryGetStringField(TEXT("key_name"), KeyName) ||
		!Params->TryGetStringField(TEXT("key_type"), KeyType))
	{
		return TEXT("{\"error\": \"Missing required parameters\"}");
	}

	UBlackboardData* BB = LoadObject<UBlackboardData>(nullptr, *BBPath);
	if (!BB)
	{
		return TEXT("{\"error\": \"Failed to load BlackboardData asset.\"}");
	}

	FBlackboardEntry NewKey;
	NewKey.EntryName = FName(*KeyName);

	if (KeyType == TEXT("Bool")) NewKey.KeyType = NewObject<UBlackboardKeyType_Bool>(BB);
	else if (KeyType == TEXT("Int")) NewKey.KeyType = NewObject<UBlackboardKeyType_Int>(BB);
	else if (KeyType == TEXT("Float")) NewKey.KeyType = NewObject<UBlackboardKeyType_Float>(BB);
	else if (KeyType == TEXT("String")) NewKey.KeyType = NewObject<UBlackboardKeyType_String>(BB);
	else if (KeyType == TEXT("Vector")) NewKey.KeyType = NewObject<UBlackboardKeyType_Vector>(BB);
	else if (KeyType == TEXT("Rotator")) NewKey.KeyType = NewObject<UBlackboardKeyType_Rotator>(BB);
	else if (KeyType == TEXT("Object")) NewKey.KeyType = NewObject<UBlackboardKeyType_Object>(BB);
	else if (KeyType == TEXT("Class")) NewKey.KeyType = NewObject<UBlackboardKeyType_Class>(BB);
	else if (KeyType == TEXT("Enum")) NewKey.KeyType = NewObject<UBlackboardKeyType_Enum>(BB);
	else
	{
		return FString::Printf(TEXT("{\"error\": \"Unsupported key_type: %s\"}"), *KeyType);
	}

	BB->Keys.Add(NewKey);
	BB->MarkPackageDirty();

	return TEXT("{\"status\": \"success\"}");
}
