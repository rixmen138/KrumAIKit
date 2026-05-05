#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class SMultiLineEditableTextBox;
class SEditableTextBox;
class FKrumOpenRouterAgent;

class SKrumChatWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SKrumChatWindow) {}
	SLATE_END_ARGS()

	~SKrumChatWindow();

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<ITableRow> OnGenerateMessageRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	FReply OnSendClicked();

	void OnAgentResponse(const FString& ResponseText);
	void OnAgentError(const FString& ErrorText);

private:
	TArray<TSharedPtr<FString>> ChatHistory;
	TSharedPtr<SListView<TSharedPtr<FString>>> ChatListView;
	TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
	TSharedPtr<SEditableTextBox> ApiKeyTextBox;

	TSharedPtr<FKrumOpenRouterAgent> OpenRouterAgent;
};
