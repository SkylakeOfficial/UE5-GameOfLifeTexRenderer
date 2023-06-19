// Skylake Game Studio ArknightsFP

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Containers/BitArray.h"
#include "GameOfLifeTexRenderer.generated.h"


USTRUCT(BlueprintType)
struct FStructOnlyBools : public FTableRowBase
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsTrue;
};

USTRUCT(BlueprintType, Category = "GameOfLife")
struct FGameOfLifePatterns : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (RequiredAssetDataTags = "RowStructure=StructOnlyBools"))
	UDataTable* PatternData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint PatternSize = FIntPoint(3,3);
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARKNIGHTSFP_API UGameOfLifeTexRenderer : public UActorComponent
{
	GENERATED_BODY()

public:	

	// Sets default values for this component's properties
	UGameOfLifeTexRenderer();
	//记录的历史步数(0~63)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameOfLife")
		int32 HistorySteps = 0;
	//模拟宽度
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "GameOfLife")
		int32 GameWidth = 128;
	//模拟高度
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "GameOfLife")
		int32 GameHeight = 128;
	//模拟产生的纹理，用蓝图Get到，赋给材质。
	UPROPERTY(BlueprintReadOnly, Category = "GameOfLife")
		UTexture2D* GameTexture;
	//游戏图表。
	UPROPERTY(EditAnywhere, Category = "GameOfLife", meta = (RequiredAssetDataTags = "RowStructure=StructOnlyBools"))
		UDataTable* GameInitData;
	//用于绘制的pattern。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameOfLife")
		TArray<FGameOfLifePatterns> PatternsData;

	//模拟更新1 tick。
	UFUNCTION(BlueprintCallable, Category = "GameOfLife")
		void UpdateGameOfLife();
	//在画布上打点（取反）。
	UFUNCTION(BlueprintCallable, Category = "GameOfLife")
		bool DrawDotOnCanvas(FVector2D Coord);
	//在画布上绘制pattern（取或）
	UFUNCTION(BlueprintCallable, Category = "GameOfLife")
		bool DrawPatternOnCanvas(FVector2D Coord);

	

private:

	void ReDrawCanvas();
	TArray<uint8> GameTiles;
	FTimerHandle DuplicateAvoidTimer;
	uint8 HistoryFalloff = 255;
	uint8 HistoryFalloff2x = 255;
	int32 LastDrawIndex = 0;
	TArray<TBitArray<>> Patterns;
	void GetPatternsFromTable(TBitArray<>& PatternToWrite, const UDataTable* TableToRead) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
