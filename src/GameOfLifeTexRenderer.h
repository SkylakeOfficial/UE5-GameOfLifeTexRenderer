// Skylake Game Studio ArknightsFP

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "GameOfLifeTexRenderer.generated.h"


USTRUCT(BlueprintType)
struct FStructOnlyBools : public FTableRowBase
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsTrue;
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARKNIGHTSFP_API UGameOfLifeTexRenderer : public UActorComponent
{
	GENERATED_BODY()

public:	

	// Sets default values for this component's properties
	UGameOfLifeTexRenderer();
	//记录的历史步数
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

	//模拟更新1 tick。
	UFUNCTION(BlueprintCallable, Category = "GameOfLife")
		void UpdateGameOfLife();
	//在画布上打点（取反）。
	UFUNCTION(BlueprintCallable, Category = "GameOfLife")
		bool DrawDotOnCanvas(FVector2D Coord);
	//在画布上绘制pattern（取或）
	UFUNCTION(BlueprintCallable, Category = "GameOfLife")
		bool DrawPatternOnCanvas(FVector2D Coord, const TArray<bool> Pattern, int32 sizeX, int32 sizeY);

	

private:

	void ReDrawCanvas();
	TArray<bool> GameTiles;
	FTimerHandle DuplicateAvoidTimer;
	int32 LastDrawIndex = 0;
	TArray<TArray<bool>> HistoryTiles;
	static int32 IntPow(int32 x, uint8 p);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
