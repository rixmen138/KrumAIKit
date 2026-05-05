#include "SKrumChatWindow.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "KrumOpenRouterAgent.h"

SKrumChatWindow::~SKrumChatWindow()
{
	if (OpenRouterAgent.IsValid())
	{
		OpenRouterAgent->Disconnect();
	}
}

void SKrumChatWindow::Construct(const FArguments& InArgs)
{
	ChatHistory.Add(MakeShared<FString>(TEXT("<System>Welcome to KrumAIKit!</>")));

	OpenRouterAgent = MakeShared<FKrumOpenRouterAgent>();
	OpenRouterAgent->Connect();

	ChildSlot
	[
		SNew(SVerticalBox)

		// API Key / Settings Area (Temporary for testing)
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
				SNew(STextBlock)
				.Text(FText::FromString("OpenRouter API Key:"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(ApiKeyTextBox, SEditableTextBox)
				.HintText(FText::FromString("sk-or-v1-..."))
				.IsPassword(true)
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
			if (OpenRouterAgent.IsValid())
			{
				if (ApiKeyTextBox.IsValid())
				{
					OpenRouterAgent->SetApiKey(ApiKeyTextBox->GetText().ToString());
				}

				ChatHistory.Add(MakeShared<FString>(TEXT("<System>Waiting for agent response...</>")));
				ChatListView->RequestListRefresh();
				ChatListView->ScrollToBottom();

				FOnMessageReceived OnResponse = FOnMessageReceived::CreateSP(this, &SKrumChatWindow::OnAgentResponse);
				FOnMessageReceived OnError = FOnMessageReceived::CreateSP(this, &SKrumChatWindow::OnAgentError);
				
				OpenRouterAgent->SendMessage(InputText, TEXT("You are KrumAI, a helpful Unreal Engine 5 assistant."), OnResponse, OnError);
			}
		}
	}
	return FReply::Handled();
}

void SKrumChatWindow::OnAgentResponse(const FString& ResponseText)
{
	// Remove the "Waiting..." message if it's the last one
	if (ChatHistory.Num() > 0 && ChatHistory.Last()->Contains(TEXT("Waiting for agent response")))
	{
		ChatHistory.Pop();
	}

	ChatHistory.Add(MakeShared<FString>(FString::Printf(TEXT("<Agent>Agent: %s</>"), *ResponseText)));
	ChatListView->RequestListRefresh();
	ChatListView->ScrollToBottom();
}

void SKrumChatWindow::OnAgentError(const FString& ErrorText)
{
	// Remove the "Waiting..." message if it's the last one
	if (ChatHistory.Num() > 0 && ChatHistory.Last()->Contains(TEXT("Waiting for agent response")))
	{
		ChatHistory.Pop();
	}

	ChatHistory.Add(MakeShared<FString>(FString::Printf(TEXT("<System>Error: %s</>"), *ErrorText)));
	ChatListView->RequestListRefresh();
	ChatListView->ScrollToBottom();
}
