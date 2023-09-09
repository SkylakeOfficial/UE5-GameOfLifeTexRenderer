

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ArknightsFP/GameOfLife/GameOfLifePattern.h"
#include "AFPEditorFuncionLib.generated.h"

UCLASS()
class ARKNIGHTSFPEDITOR_API UAFPEditorFuncionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//重新导入生命游戏Pattern到数据资产中，若未传入数据资产，则新建一个该数据资产
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "读取GOL到生命游戏资产"))
	static void ReadGameOfLifePattern(TArray<UObject*> PatternAssets);
	
};
