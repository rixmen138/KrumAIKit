#include "KrumOpenRouterAgent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "KrumAIKitCore.h"

FKrumOpenRouterAgent::FKrumOpenRouterAgent()
	: bIsConnected(false)
{
}

FKrumOpenRouterAgent::~FKrumOpenRouterAgent()
{
	Disconnect();
}

void FKrumOpenRouterAgent::Connect()
{
	// For HTTP agents, connection is just ensuring we have the HTTP module and an API key
	bIsConnected = true;
}

void FKrumOpenRouterAgent::Disconnect()
{
	StopCurrent();
	bIsConnected = false;
}

bool FKrumOpenRouterAgent::IsConnected() const
{
	return bIsConnected;
}

FString FKrumOpenRouterAgent::GetName() const
{
	return TEXT("OpenRouter HTTP");
}

void FKrumOpenRouterAgent::SetApiKey(const FString& InApiKey)
{
	ApiKey = InApiKey;
}

void FKrumOpenRouterAgent::SendMessage(const FString& Prompt, const FString& Context, FOnMessageReceived OnResponse, FOnMessageReceived OnError)
{
	if (!bIsConnected)
	{
		OnError.ExecuteIfBound(TEXT("Agent is not connected."));
		return;
	}

	if (ApiKey.IsEmpty())
	{
		OnError.ExecuteIfBound(TEXT("OpenRouter API key is missing. Please configure it in settings."));
		return;
	}

	FHttpModule* HttpModule = &FHttpModule::Get();
	CurrentRequest = HttpModule->CreateRequest();

	CurrentRequest->SetURL(TEXT("https://openrouter.ai/api/v1/chat/completions"));
	CurrentRequest->SetVerb(TEXT("POST"));
	CurrentRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	CurrentRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	CurrentRequest->SetHeader(TEXT("HTTP-Referer"), TEXT("https://github.com/KrumAI/KrumAIKit")); // Required by OpenRouter
	CurrentRequest->SetHeader(TEXT("X-Title"), TEXT("KrumAIKit for Unreal Engine"));

	// Construct JSON payload
	TSharedPtr<FJsonObject> RequestJson = MakeShareable(new FJsonObject());
	RequestJson->SetStringField(TEXT("model"), TEXT("anthropic/claude-3.5-sonnet")); // Default model

	TArray<TSharedPtr<FJsonValue>> MessagesArray;

	// System / Context message
	if (!Context.IsEmpty())
	{
		TSharedPtr<FJsonObject> SystemMessage = MakeShareable(new FJsonObject());
		SystemMessage->SetStringField(TEXT("role"), TEXT("system"));
		SystemMessage->SetStringField(TEXT("content"), Context);
		MessagesArray.Add(MakeShareable(new FJsonValueObject(SystemMessage)));
	}

	// User prompt
	TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject());
	UserMessage->SetStringField(TEXT("role"), TEXT("user"));
	UserMessage->SetStringField(TEXT("content"), Prompt);
	MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMessage)));

	RequestJson->SetArrayField(TEXT("messages"), MessagesArray);
	// We disable streaming for the basic implementation, can upgrade to SSE later
	RequestJson->SetBoolField(TEXT("stream"), false); 

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestJson.ToSharedRef(), Writer);

	CurrentRequest->SetContentAsString(RequestBody);

	CurrentRequest->OnProcessRequestComplete().BindRaw(this, &FKrumOpenRouterAgent::OnResponseReceived, OnResponse, OnError);

	CurrentRequest->ProcessRequest();
}

void FKrumOpenRouterAgent::StopCurrent()
{
	if (CurrentRequest.IsValid() && CurrentRequest->GetStatus() == EHttpRequestStatus::Processing)
	{
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
	}
}

void FKrumOpenRouterAgent::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnMessageReceived OnResponseCallback, FOnMessageReceived OnErrorCallback)
{
	CurrentRequest.Reset();

	if (!bWasSuccessful || !Response.IsValid())
	{
		OnErrorCallback.ExecuteIfBound(TEXT("Network error or invalid response from OpenRouter."));
		return;
	}

	if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300)
	{
		FString ErrorMsg = FString::Printf(TEXT("HTTP Error %d: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
		OnErrorCallback.ExecuteIfBound(ErrorMsg);
		return;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> ResponseJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	if (FJsonSerializer::Deserialize(Reader, ResponseJson) && ResponseJson.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* Choices;
		if (ResponseJson->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
		{
			TSharedPtr<FJsonObject> MessageObj = (*Choices)[0]->AsObject()->GetObjectField(TEXT("message"));
			FString Content = MessageObj->GetStringField(TEXT("content"));
			
			OnResponseCallback.ExecuteIfBound(Content);
			return;
		}
	}

	OnErrorCallback.ExecuteIfBound(TEXT("Failed to parse response JSON from OpenRouter."));
}
