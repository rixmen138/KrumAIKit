#include "SKrumChatWindow.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Layout/SScrollBox.h"

void SKrumChatWindow::Construct(const FArguments& InArgs)
{
	ChatHistory.Add(MakeShared<FString>(TEXT("<System>Welcome to KrumAIKit!</>")));

	ChildSlot
	[
		SNew(SVerticalBox)

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

			// TODO: Send to Agent here
		}
	}
	return FReply::Handled();
}
