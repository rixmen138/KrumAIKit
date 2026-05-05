#include "KrumOllamaAgent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

FKrumOllamaAgent::FKrumOllamaAgent()
{
}

FKrumOllamaAgent::~FKrumOllamaAgent()
{
	Disconnect();
}

void FKrumOllamaAgent::SetBaseUrl(const FString& Url)
{
	BaseUrl = Url;
}

void FKrumOllamaAgent::SetModel(const FString& Model)
{
	CurrentModel = Model;
}

void FKrumOllamaAgent::Connect()
{
	bIsConnected = false;

	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
	Request->SetURL(BaseUrl + TEXT("/api/tags"));
	Request->SetVerb(TEXT("GET"));

	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
	{
		if (bSuccess && Resp.IsValid() && Resp->GetResponseCode() == 200)
		{
			bIsConnected = true;
		}
		else
		{
			// Try to log it safely. Since UE_LOG requires log category from core which we might not have exposed cleanly here,
			// just failing silently for now, status dot will be red.
			bIsConnected = false;
		}
	});

	Request->ProcessRequest();
}

void FKrumOllamaAgent::Disconnect()
{
	StopCurrent();
	bIsConnected = false;
}

bool FKrumOllamaAgent::IsConnected() const
{
	return bIsConnected;
}

FString FKrumOllamaAgent::GetName() const
{
	return TEXT("Ollama (local)");
}

void FKrumOllamaAgent::RefreshModelList(TFunction<void(TArray<FString>)> OnDone)
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
	Request->SetURL(BaseUrl + TEXT("/api/tags"));
	Request->SetVerb(TEXT("GET"));

	Request->OnProcessRequestComplete().BindLambda([OnDone](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
	{
		TArray<FString> Models;
		if (bSuccess && Resp.IsValid() && Resp->GetResponseCode() == 200)
		{
			TSharedPtr<FJsonObject> JsonObj;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* ModelsArray;
				if (JsonObj->TryGetArrayField(TEXT("models"), ModelsArray))
				{
					for (const TSharedPtr<FJsonValue>& Val : *ModelsArray)
					{
						FString ModelName;
						if (Val->AsObject()->TryGetStringField(TEXT("name"), ModelName))
						{
							Models.Add(ModelName);
						}
					}
				}
			}
		}
		OnDone(Models);
	});

	Request->ProcessRequest();
}

void FKrumOllamaAgent::SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError)
{
	StopCurrent();

	FHttpModule* HttpModule = &FHttpModule::Get();
	CurrentRequest = HttpModule->CreateRequest();
	CurrentRequest->SetURL(BaseUrl + TEXT("/api/chat"));
	CurrentRequest->SetVerb(TEXT("POST"));
	CurrentRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedPtr<FJsonObject> RequestJson = MakeShareable(new FJsonObject());
	RequestJson->SetStringField(TEXT("model"), CurrentModel);
	RequestJson->SetBoolField(TEXT("stream"), false);

	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	
	if (!Context.IsEmpty())
	{
		TSharedPtr<FJsonObject> SysMsg = MakeShareable(new FJsonObject());
		SysMsg->SetStringField(TEXT("role"), TEXT("system"));
		SysMsg->SetStringField(TEXT("content"), Context);
		MessagesArray.Add(MakeShareable(new FJsonValueObject(SysMsg)));
	}

	TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject());
	UserMsg->SetStringField(TEXT("role"), TEXT("user"));
	UserMsg->SetStringField(TEXT("content"), Prompt);
	MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMsg)));

	RequestJson->SetArrayField(TEXT("messages"), MessagesArray);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestJson.ToSharedRef(), Writer);

	CurrentRequest->SetContentAsString(RequestBody);
	CurrentRequest->OnProcessRequestComplete().BindRaw(this, &FKrumOllamaAgent::OnChatResponseReceived, OnResponse, OnError);
	CurrentRequest->ProcessRequest();
}

void FKrumOllamaAgent::StopCurrent()
{
	if (CurrentRequest.IsValid() && CurrentRequest->GetStatus() == EHttpRequestStatus::Processing)
	{
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
	}
}

void FKrumOllamaAgent::OnChatResponseReceived(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess, FOnMessageReceived OnResponse, FOnMessageReceived OnError)
{
	CurrentRequest.Reset();

	if (!bSuccess || !Resp.IsValid())
	{
		OnError.ExecuteIfBound(TEXT("Network error connecting to Ollama."));
		return;
	}

	if (Resp->GetResponseCode() != 200)
	{
		OnError.ExecuteIfBound(FString::Printf(TEXT("Ollama HTTP Error %d"), Resp->GetResponseCode()));
		return;
	}

	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
	{
		const TSharedPtr<FJsonObject>* MessageObj;
		if (JsonObj->TryGetObjectField(TEXT("message"), MessageObj))
		{
			FString Content;
			if ((*MessageObj)->TryGetStringField(TEXT("content"), Content))
			{
				OnResponse.ExecuteIfBound(Content);
				return;
			}
		}
	}

	OnError.ExecuteIfBound(TEXT("Failed to parse Ollama response."));
}
