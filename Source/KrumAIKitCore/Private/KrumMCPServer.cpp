#include "KrumMCPServer.h"
#include "KrumToolRegistry.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformMisc.h"
#include <iostream>
#include <string>

FKrumMCPServer::FKrumMCPServer()
{
	bShouldRun = false;
}

FKrumMCPServer::~FKrumMCPServer()
{
	Stop();
}

bool FKrumMCPServer::Start()
{
	if (Thread) return false;
	
	bShouldRun = true;
	Thread = FRunnableThread::Create(this, TEXT("FKrumMCPServer"), 0, TPri_BelowNormal);
	return Thread != nullptr;
}

void FKrumMCPServer::Stop()
{
	if (!Thread) return;
	
	bShouldRun = false;
	Thread->WaitForCompletion();
	delete Thread;
	Thread = nullptr;
}

void FKrumMCPServer::Exit()
{
	bShouldRun = false;
}

uint32 FKrumMCPServer::Run()
{
	while (bShouldRun)
	{
		if (FPlatformMisc::GetEnvironmentVariable(TEXT("KRUMAIKIT_MCP")) != TEXT("1"))
		{
			FPlatformProcess::Sleep(0.1f);
			continue;
		}

		std::string StdLine;
		if (!std::getline(std::cin, StdLine)) 
		{
			// if stream is closed, exit or sleep
			FPlatformProcess::Sleep(0.1f);
			continue; 
		}

		FString JsonLine = FString(UTF8_TO_TCHAR(StdLine.c_str()));
		if (JsonLine.IsEmpty()) continue;
		
		FString Response = HandleRequest(JsonLine);
		
		if (!Response.IsEmpty())
		{
			FTCHARToUTF8 Converted(*Response);
			std::cout << Converted.Get() << "\n";
			std::cout.flush();
		}
	}
	return 0;
}

FString FKrumMCPServer::HandleRequest(const FString& RawJson)
{
	TSharedPtr<FJsonObject> Req;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
	if (!FJsonSerializer::Deserialize(Reader, Req) || !Req.IsValid())
	{
		return MakeErrorResponse(TEXT("null"), -32700, TEXT("Parse error"));
	}

	FString Method;
	if (!Req->TryGetStringField(TEXT("method"), Method))
	{
		return MakeErrorResponse(TEXT("null"), -32600, TEXT("Invalid Request"));
	}

	FString Id = TEXT("null");
	Req->TryGetStringField(TEXT("id"), Id);

	if (Method == TEXT("tools/list")) return HandleToolsList(Id);
	if (Method == TEXT("tools/call")) 
	{
		const TSharedPtr<FJsonObject>* ParamsObj;
		if (Req->TryGetObjectField(TEXT("params"), ParamsObj))
		{
			return HandleToolsCall(*ParamsObj, Id);
		}
		return MakeErrorResponse(Id, -32602, TEXT("Invalid params"));
	}
	if (Method == TEXT("initialize")) return MakeSuccessResponse(Id, InitializeResultObject());

	return MakeErrorResponse(Id, -32601, TEXT("Method not found"));
}

FString FKrumMCPServer::HandleToolsList(const FString& RequestId)
{
	TArray<TSharedPtr<FJsonValue>> ToolsArray;

	for (const TSharedPtr<IKrumTool>& Tool : FKrumToolRegistry::Get().GetTools())
	{
		if (!Tool.IsValid()) continue;

		TSharedPtr<FJsonObject> ToolObj = MakeShareable(new FJsonObject());
		ToolObj->SetStringField(TEXT("name"), Tool->GetName());
		ToolObj->SetStringField(TEXT("description"), Tool->GetDescription());
		
		TSharedPtr<FJsonObject> Schema = Tool->GetSchema();
		if (Schema.IsValid())
		{
			ToolObj->SetObjectField(TEXT("inputSchema"), Schema);
		}

		ToolsArray.Add(MakeShareable(new FJsonValueObject(ToolObj)));
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetArrayField(TEXT("tools"), ToolsArray);

	return MakeSuccessResponse(RequestId, ResultObj);
}

FString FKrumMCPServer::HandleToolsCall(const TSharedPtr<FJsonObject>& Params, const FString& RequestId)
{
	FString ToolName;
	if (!Params->TryGetStringField(TEXT("name"), ToolName))
	{
		return MakeErrorResponse(RequestId, -32602, TEXT("Missing tool name"));
	}

	const TSharedPtr<FJsonObject>* ArgsObj;
	TSharedPtr<FJsonObject> Args;
	if (Params->TryGetObjectField(TEXT("arguments"), ArgsObj))
	{
		Args = *ArgsObj;
	}
	else
	{
		Args = MakeShareable(new FJsonObject()); // empty args
	}

	TSharedPtr<IKrumTool> Tool = FKrumToolRegistry::Get().FindTool(ToolName);
	if (!Tool.IsValid())
	{
		return MakeErrorResponse(RequestId, -32602, FString::Printf(TEXT("Tool not found: %s"), *ToolName));
	}

	FString Result = Tool->Execute(Args);

	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("content"), Result);

	return MakeSuccessResponse(RequestId, ResultObj);
}

TSharedPtr<FJsonObject> FKrumMCPServer::InitializeResultObject()
{
	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("protocolVersion"), TEXT("2025-03-26"));
	
	TSharedPtr<FJsonObject> ServerInfo = MakeShareable(new FJsonObject());
	ServerInfo->SetStringField(TEXT("name"), TEXT("KrumAIKit"));
	ServerInfo->SetStringField(TEXT("version"), TEXT("0.1"));
	ResultObj->SetObjectField(TEXT("serverInfo"), ServerInfo);

	TSharedPtr<FJsonObject> Capabilities = MakeShareable(new FJsonObject());
	Capabilities->SetObjectField(TEXT("tools"), MakeShareable(new FJsonObject()));
	ResultObj->SetObjectField(TEXT("capabilities"), Capabilities);

	return ResultObj;
}

FString FKrumMCPServer::MakeErrorResponse(const FString& Id, int32 Code, const FString& Message)
{
	TSharedPtr<FJsonObject> ResponseObj = MakeShareable(new FJsonObject());
	ResponseObj->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	if (Id != TEXT("null")) ResponseObj->SetStringField(TEXT("id"), Id);

	TSharedPtr<FJsonObject> ErrorObj = MakeShareable(new FJsonObject());
	ErrorObj->SetNumberField(TEXT("code"), Code);
	ErrorObj->SetStringField(TEXT("message"), Message);
	ResponseObj->SetObjectField(TEXT("error"), ErrorObj);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(ResponseObj.ToSharedRef(), Writer);
	return OutputString;
}

FString FKrumMCPServer::MakeSuccessResponse(const FString& Id, TSharedPtr<FJsonObject> Result)
{
	TSharedPtr<FJsonObject> ResponseObj = MakeShareable(new FJsonObject());
	ResponseObj->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	if (Id != TEXT("null")) ResponseObj->SetStringField(TEXT("id"), Id);
	ResponseObj->SetObjectField(TEXT("result"), Result);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(ResponseObj.ToSharedRef(), Writer);
	return OutputString;
}
