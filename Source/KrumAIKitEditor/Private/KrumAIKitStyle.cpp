#include "KrumAIKitStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FKrumAIKitStyle::StyleInstance = nullptr;

void FKrumAIKitStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FKrumAIKitStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		EnsureOrVcheck(StyleInstance.IsUnique(), TEXT("FKrumAIKitStyle has more than one reference."));
		StyleInstance.Reset();
	}
}

FName FKrumAIKitStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("KrumAIKitStyle"));
	return StyleSetName;
}

const ISlateStyle& FKrumAIKitStyle::Get()
{
	return *StyleInstance;
}

TSharedRef< FSlateStyleSet > FKrumAIKitStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("KrumAIKit")->GetBaseDir() / TEXT("Resources"));

	return Style;
}

void FKrumAIKitStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}
