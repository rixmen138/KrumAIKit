#include "KrumToolRegistry.h"
#include "Serialization/JsonSerializer.h"

FKrumToolRegistry& FKrumToolRegistry::Get()
{
    static FKrumToolRegistry Instance;
    return Instance;
}

void FKrumToolRegistry::RegisterTool(TSharedPtr<IKrumTool> Tool)
{
    if (Tool.IsValid())
    {
        Tools.Add(Tool);
    }
}

void FKrumToolRegistry::UnregisterAll()
{
    Tools.Empty();
}

TSharedPtr<IKrumTool> FKrumToolRegistry::FindTool(const FString& Name) const
{
    for (const TSharedPtr<IKrumTool>& Tool : Tools)
    {
        if (Tool.IsValid() && Tool->GetName() == Name)
        {
            return Tool;
        }
    }
    return nullptr;
}

FString FKrumToolRegistry::GetToolsSchemaJson() const
{
    TArray<TSharedPtr<FJsonValue>> ToolsArray;

    for (const TSharedPtr<IKrumTool>& Tool : Tools)
    {
        if (!Tool.IsValid()) continue;

        TSharedPtr<FJsonObject> ToolObj = MakeShareable(new FJsonObject());
        ToolObj->SetStringField(TEXT("name"), Tool->GetName());
        ToolObj->SetStringField(TEXT("description"), Tool->GetDescription());
        
        TSharedPtr<FJsonObject> Schema = Tool->GetSchema();
        if (Schema.IsValid())
        {
            ToolObj->SetObjectField(TEXT("parameters"), Schema);
        }

        ToolsArray.Add(MakeShareable(new FJsonValueObject(ToolObj)));
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ToolsArray, Writer);
    
    return OutputString;
}
