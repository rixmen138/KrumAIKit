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

void FKrumOpenRouterAgent::SetModel(const FString& Model)
{
	CurrentModel = Model;
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
	RequestJson->SetStringField(TEXT("model"), CurrentModel);

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
	RequestJson->SetBoolField(TEXT("stream"), true); 

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestJson.ToSharedRef(), Writer);

	CurrentRequest->SetContentAsString(RequestBody);

	StreamBuffer.Empty();

	CurrentRequest->OnRequestProgress64().BindRaw(this, &FKrumOpenRouterAgent::OnStreamProgress, OnResponse, OnError);
	CurrentRequest->OnProcessRequestComplete().BindRaw(this, &FKrumOpenRouterAgent::OnStreamComplete, OnError);

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

void FKrumOpenRouterAgent::OnStreamProgress(FHttpRequestPtr Req, uint64 BytesSent, uint64 BytesReceived, FOnMessageReceived OnResponse, FOnMessageReceived OnError)
{
	if (!Req.IsValid() || !Req->GetResponse().IsValid()) return;

	FString RawContent = Req->GetResponse()->GetContentAsString();
	FString NewChunk = RawContent.Mid(StreamBuffer.Len());
	StreamBuffer = RawContent;
	
	TArray<FString> Lines;
	NewChunk.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		if (!Line.StartsWith(TEXT("data: "))) continue;
		FString JsonPart = Line.Mid(6); // skip "data: "
		if (JsonPart == TEXT("[DONE]")) continue;
		
		TSharedPtr<FJsonObject> Chunk;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonPart);
		if (!FJsonSerializer::Deserialize(Reader, Chunk) || !Chunk.IsValid()) continue;
		
		const TArray<TSharedPtr<FJsonValue>>* Choices;
		if (!Chunk->TryGetArrayField(TEXT("choices"), Choices) || Choices->IsEmpty()) continue;
		
		const TSharedPtr<FJsonObject>* DeltaObj;
		if (!(*Choices)[0]->AsObject()->TryGetObjectField(TEXT("delta"), DeltaObj)) continue;
		
		FString Content;
		if (!(*DeltaObj)->TryGetStringField(TEXT("content"), Content)) continue;
		if (Content.IsEmpty()) continue;
		
		OnResponse.ExecuteIfBound(Content);
	}
}

void FKrumOpenRouterAgent::OnStreamComplete(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess, FOnMessageReceived OnError)
{
	StreamBuffer.Empty();
	CurrentRequest.Reset();

	if (!bSuccess || !Resp.IsValid())
	{
		OnError.ExecuteIfBound(TEXT("Stream connection failed or invalid response from OpenRouter."));
		return;
	}

	if (Resp->GetResponseCode() < 200 || Resp->GetResponseCode() >= 300)
	{
		FString ErrorMsg = FString::Printf(TEXT("HTTP Error %d: %s"), Resp->GetResponseCode(), *Resp->GetContentAsString());
		OnError.ExecuteIfBound(ErrorMsg);
	}
}
