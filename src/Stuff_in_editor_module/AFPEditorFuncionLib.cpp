
#include "AFPEditorFuncionLib.h"
#include "Windows/WindowsSystemIncludes.h"
#include <Windows.h>
#include "Misc/FileHelper.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DesktopPlatformModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/DataAssetFactory.h"
#include "UObject/SavePackage.h"

void UAFPEditorFuncionLib::ReadGameOfLifePattern(TArray<UObject*> PatternAssets)
{

	TArray<FString> files;
	FDesktopPlatformModule::Get()
		->OpenFileDialog(GetActiveWindow(),
			TEXT("选择*一个*生命游戏Pattern"),
			TEXT("E:/Arknights_FP/assets/GameOfLife"),
			TEXT(""),
			TEXT("(GOL文件)|*.gol;)"),
			(uint32_t)EFileDialogFlags::None,
			files
		);
	if(!files.IsValidIndex(0))
	{
		return;
	}
	FString PatternText;
	FFileHelper::LoadFileToString(PatternText , *files[0]);

	TArray<FString> PatternTexts;
	const FString Delimit = ",";
	PatternText.ParseIntoArray(PatternTexts ,*Delimit ,true);

	FIntPoint Size = FIntPoint(FCString::Atoi(*PatternTexts[0]), FCString::Atoi(*PatternTexts[1]));

	TArray<uint8> PatternsArray;
	if (PatternTexts.Num()<3)
	{
		return;
	}
	for (int i = 2; i< PatternTexts.Num(); i++)
	{
		PatternsArray.Add(FCString::Atoi(*PatternTexts[i]));
	}

	if (PatternAssets.IsEmpty())
	{
		TArray<FString> FileName;
		const FString FileNameDelimit = "/";
		files[0].ParseIntoArray(FileName, *FileNameDelimit, false);
		FString BasePackageName = FString("/Game/GameOfLife/").Append(FileName.Last().LeftChop(4));


		// 新建DataAsset
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		FString Name, PackageName;
		AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, TEXT(""), PackageName, Name);
		const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

		UPackage* package = CreatePackage(*PackageName);
		UDataAssetFactory* MyFactory = NewObject<UDataAssetFactory>(UDataAssetFactory::StaticClass());
		MyFactory->DataAssetClass = UGameOfLifePattern::StaticClass();
		UObject* NewObject = AssetToolsModule.Get().CreateAsset(Name, PackagePath, UDataAsset::StaticClass(), MyFactory);
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		UPackage::Save(package, NewObject, *FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension()), SaveArgs);

		AssetRegistry.AssetCreated(NewObject);

		TArray<UObject*> Objects;
		Objects.Add(NewObject);
		ContentBrowserModule.Get().SyncBrowserToAssets(Objects);

		PatternAssets.Add(NewObject);
	}
	for (UObject* Object : PatternAssets)
	{
		if(UGameOfLifePattern* Pattern= Cast<UGameOfLifePattern>(Object))
		{
			Pattern->Pattern = PatternsArray;
			Pattern->PatternSize = Size;
			
		}
	}
}
