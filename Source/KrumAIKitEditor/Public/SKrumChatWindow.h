#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class SMultiLineEditableTextBox;

class SKrumChatWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SKrumChatWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<ITableRow> OnGenerateMessageRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	FReply OnSendClicked();

private:
	TArray<TSharedPtr<FString>> ChatHistory;
	TSharedPtr<SListView<TSharedPtr<FString>>> ChatListView;
	TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
};
