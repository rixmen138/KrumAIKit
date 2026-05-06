#include "SKrumChatWindow.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "KrumOpenRouterAgent.h"
#include "KrumClaudeAgent.h"
#include "KrumGeminiAgent.h"
#include "KrumOllamaAgent.h"
#include "KrumCodexAgent.h"
#include "KrumOpenCodeAgent.h"
#include "Widgets/Input/SComboBox.h"
#include "KrumSettings.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "KrumProjectIndexer.h"
#include "Async/Async.h"

SKrumChatWindow::~SKrumChatWindow()
{
	for (auto& Agent : AvailableAgents)
	{
		if (Agent.IsValid())
		{
			Agent->Disconnect();
		}
	}
}

void SKrumChatWindow::Construct(const FArguments& InArgs)
{
	TSharedPtr<FKrumOpenRouterAgent> OpenRouter = MakeShared<FKrumOpenRouterAgent>();
	TSharedPtr<FKrumClaudeAgent>     Claude     = MakeShared<FKrumClaudeAgent>();
	TSharedPtr<FKrumGeminiAgent>     Gemini     = MakeShared<FKrumGeminiAgent>();
	TSharedPtr<FKrumOllamaAgent>     Ollama     = MakeShared<FKrumOllamaAgent>();
	TSharedPtr<FKrumCodexAgent>      Codex      = MakeShared<FKrumCodexAgent>();
	TSharedPtr<FKrumOpenCodeAgent>   OpenCode   = MakeShared<FKrumOpenCodeAgent>();

	AvailableAgents = { OpenRouter, Claude, Gemini, Ollama, Codex, OpenCode };

	for (auto& Agent : AvailableAgents)
	{
		Agent->Connect();
		AgentNames.Add(MakeShared<FString>(Agent->GetName()));
	}
	ActiveAgent = AvailableAgents[0]; // default: OpenRouter

	const UKrumSettings* S = GetDefault<UKrumSettings>();
	if (S)
	{
		OpenRouter->SetApiKey(S->OpenRouterApiKey);
		OpenRouter->SetModel(S->OpenRouterModel);
		Ollama->SetBaseUrl(S->OllamaBaseUrl);
		Ollama->SetModel(S->OllamaModel);
	}

	LoadChatHistory();
	if (ChatHistory.Num() == 0)
	{
		ChatHistory.Add(MakeShared<FString>(TEXT("<System>Welcome to KrumAIKit!</>")));
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		// Agent Selector & API Key Area
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Agent:"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
			[
				SAssignNew(AgentSelectorCombo, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&AgentNames)
				.OnSelectionChanged(this, &SKrumChatWindow::OnAgentSelected)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) {
					return SNew(STextBlock).Text(FText::FromString(*Item));
				})
				[
					SNew(STextBlock).Text_Lambda([this]{
						return ActiveAgent.IsValid()
							? FText::FromString(ActiveAgent->GetName())
							: FText::FromString("None");
					})
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SAssignNew(AgentStatusDot, STextBlock)
				.Text_Lambda([this]{
					bool bOk = ActiveAgent.IsValid() && ActiveAgent->IsConnected();
					return FText::FromString(bOk ? TEXT("● ") : TEXT("● "));
				})
				.ColorAndOpacity_Lambda([this]{
					bool bOk = ActiveAgent.IsValid() && ActiveAgent->IsConnected();
					return FSlateColor(bOk ? FLinearColor::Green : FLinearColor::Red);
				})
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.7f)
			[
				SAssignNew(ApiKeyTextBox, SEditableTextBox)
				.HintText(FText::FromString("API Key (OpenRouter only)"))
				.IsPassword(true)
				.Visibility_Lambda([this]{
					if (!ActiveAgent.IsValid()) return EVisibility::Collapsed;
					return ActiveAgent->GetName().Contains(TEXT("OpenRouter"))
						? EVisibility::Visible : EVisibility::Collapsed;
				})
			]
		]

		// Chat History Area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(5.0f)
		[
			SAssignNew(ChatListView, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&ChatHistory)
			.OnGenerateRow(this, &SKrumChatWindow::OnGenerateMessageRow)
			.SelectionMode(ESelectionMode::None)
		]

		// Input Area
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(InputTextBox, SMultiLineEditableTextBox)
				.HintText(FText::FromString("Type a message or press Ctrl+Enter to send..."))
				.AutoWrapText(true)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("Send"))
				.OnClicked(this, &SKrumChatWindow::OnSendClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("New Chat"))
				.OnClicked_Lambda([this]() -> FReply {
					ChatHistory.Empty();
					ChatHistory.Add(MakeShared<FString>(TEXT("<System>New conversation started.</>")));
					ChatListView->RequestListRefresh();
					SaveChatHistory();
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("Stop"))
				.IsEnabled_Lambda([this]{ return bIsWaitingForAgent; })
				.OnClicked_Lambda([this]() -> FReply {
					if (ActiveAgent.IsValid()) ActiveAgent->StopCurrent();
					
					AsyncTask(ENamedThreads::GameThread, [this]()
					{
						bIsWaitingForAgent = false;
						if (ChatHistory.Num() > 0 && ChatHistory.Last()->Contains(TEXT("Waiting for agent response")))
						{
							ChatHistory.Pop();
						}
						bIsStreamingResponse = false;
						ChatHistory.Add(MakeShared<FString>(TEXT("<System>Stopped.</>")));
						ChatListView->RequestListRefresh();
						SaveChatHistory();
					});
					
					return FReply::Handled();
				})
			]
		]
	];
}

TSharedRef<ITableRow> SKrumChatWindow::OnGenerateMessageRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Padding(FMargin(0.0f, 5.0f))
		[
			SNew(SRichTextBlock)
			.Text(FText::FromString(*InItem))
			.AutoWrapText(true)
		];
}

FReply SKrumChatWindow::OnSendClicked()
{
	if (InputTextBox.IsValid())
	{
		FString InputText = InputTextBox->GetText().ToString();
		if (!InputText.IsEmpty())
		{
			// Add User message
			ChatHistory.Add(MakeShared<FString>(FString::Printf(TEXT("<User>%s</>"), *InputText)));
			
			// Clear input
			InputTextBox->SetText(FText::GetEmpty());

			// Scroll to bottom
			ChatListView->RequestListRefresh();
			ChatListView->ScrollToBottom();

			// Send to Agent
			if (ActiveAgent.IsValid())
			{
				// Inject API key if it's OpenRouter
				if (ActiveAgent->GetName().Contains(TEXT("OpenRouter")) && ApiKeyTextBox.IsValid())
				{
					TSharedPtr<FKrumOpenRouterAgent> ORAgent = StaticCastSharedPtr<FKrumOpenRouterAgent>(ActiveAgent);
					if (ORAgent.IsValid())
					{
						ORAgent->SetApiKey(ApiKeyTextBox->GetText().ToString());
					}
				}

				bIsWaitingForAgent = true;

				ChatHistory.Add(MakeShared<FString>(TEXT("<System>Waiting for agent response...</>")));
				ChatListView->RequestListRefresh();
				ChatListView->ScrollToBottom();

				FOnMessageReceived OnResponse = FOnMessageReceived::CreateSP(this, &SKrumChatWindow::OnAgentResponse);
				FOnMessageReceived OnError = FOnMessageReceived::CreateSP(this, &SKrumChatWindow::OnAgentError);
				
				FString FullContext = TEXT("You are KrumAI, a helpful Unreal Engine 5 assistant.");
				const UKrumSettings* S = GetDefault<UKrumSettings>();
				if (S && S->bAutoInjectAssetContext)
				{
					FString ContextSnippet = FKrumProjectIndexer::Get().GetContextSnippet(InputText);
					if (!ContextSnippet.IsEmpty())
					{
						FullContext += TEXT("\n\n") + ContextSnippet;
					}
				}

				SaveChatHistory();
				ActiveAgent->SendMessage(InputText, FullContext, OnResponse, OnError);
			}
		}
	}
	return FReply::Handled();
}

void SKrumChatWindow::OnAgentSelected(TSharedPtr<FString> Item, ESelectInfo::Type SelectType)
{
	for (auto& Agent : AvailableAgents)
	{
		if (Agent->GetName() == *Item)
		{
			ActiveAgent = Agent;
			break;
		}
	}
}

void SKrumChatWindow::OnAgentResponse(const FString& ResponseText)
{
	AsyncTask(ENamedThreads::GameThread, [this, ResponseText]()
	{
		// Remove the "Waiting..." message if it's the last one
		if (bIsWaitingForAgent && ChatHistory.Num() > 0 && ChatHistory.Last()->Contains(TEXT("Waiting for agent response")))
		{
			ChatHistory.Pop();
			ChatHistory.Add(MakeShared<FString>(TEXT("<Agent>")));
			bIsStreamingResponse = true;
			bIsWaitingForAgent = false;
		}

		if (bIsStreamingResponse)
		{
			*ChatHistory.Last() += ResponseText;
		}
		else
		{
			ChatHistory.Add(MakeShared<FString>(FString::Printf(TEXT("<Agent>Agent: %s</>"), *ResponseText)));
		}

		ChatListView->RequestListRefresh();
		ChatListView->ScrollToBottom();
		SaveChatHistory();
	});
}

void SKrumChatWindow::OnAgentError(const FString& ErrorText)
{
	AsyncTask(ENamedThreads::GameThread, [this, ErrorText]()
	{
		bIsWaitingForAgent = false;
		bIsStreamingResponse = false;

		// Remove the "Waiting..." message if it's the last one
		if (ChatHistory.Num() > 0 && ChatHistory.Last()->Contains(TEXT("Waiting for agent response")))
		{
			ChatHistory.Pop();
		}

		ChatHistory.Add(MakeShared<FString>(FString::Printf(TEXT("<System>Error: %s</>"), *ErrorText)));
		ChatListView->RequestListRefresh();
		ChatListView->ScrollToBottom();
		SaveChatHistory();
	});
}

void SKrumChatWindow::SaveChatHistory()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("KrumAIKit") / TEXT("chat_history.json");
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const auto& Msg : ChatHistory)
	{
		JsonArray.Add(MakeShareable(new FJsonValueString(*Msg)));
	}
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonArray, Writer);
	FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

void SKrumChatWindow::LoadChatHistory()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("KrumAIKit") / TEXT("chat_history.json");
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		TArray<TSharedPtr<FJsonValue>> JsonArray;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
		if (FJsonSerializer::Deserialize(Reader, JsonArray))
		{
			for (const auto& Val : JsonArray)
			{
				ChatHistory.Add(MakeShared<FString>(Val->AsString()));
			}
		}
	}
}
