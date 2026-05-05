#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

#include "IKrumAgent.h"

class SMultiLineEditableTextBox;
class SEditableTextBox;
class STextBlock;
template <typename OptionType> class SComboBox;

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
	void OnAgentSelected(TSharedPtr<FString> Item, ESelectInfo::Type SelectType);

private:
	TArray<TSharedPtr<FString>> ChatHistory;
	TSharedPtr<SListView<TSharedPtr<FString>>> ChatListView;
	TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
	TSharedPtr<SEditableTextBox> ApiKeyTextBox;

	TArray<TSharedPtr<IKrumAgent>> AvailableAgents;
	TSharedPtr<IKrumAgent> ActiveAgent;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> AgentSelectorCombo;
	TArray<TSharedPtr<FString>> AgentNames;
	TSharedPtr<STextBlock> AgentStatusDot;
};
