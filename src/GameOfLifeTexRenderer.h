// Skylake Game Studio ArknightsFP

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameOfLifePattern.h"
#include "GameOfLifeTexRenderer.generated.h"

USTRUCT()
struct FPatternWithSize
{
	GENERATED_BODY()
	FIntPoint Size;
	TArray<bool> Pattern;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARKNIGHTSFP_API UGameOfLifeTexRenderer : public UActorComponent
{
	GENERATED_BODY()

public:	

	// Sets default values for this component's properties
	UGameOfLifeTexRenderer();
	//记录的历史步数(0~63)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameOfLife", meta = (ClampMin = "0", ClampMax = "63", UIMin = "0", UIMax = "63"))
		int32 HistorySteps = 0;
	//默认画布为空时的回退模拟宽度
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "GameOfLife", meta = (DisplayName = "FallBackGameWidth",ClampMin = "0", UIMin = "0", UIMax = "1024"))
		int32 GameWidth = 128;
	//默认画布为空时的回退模拟高度
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "GameOfLife",meta =(DisplayName = "FallBackGameHeight", ClampMin = "0", UIMin = "0", UIMax = "1024"))
		int32 GameHeight = 128;
	//模拟产生的纹理，用蓝图Get到，赋给材质。
	UPROPERTY(BlueprintReadOnly, Category = "GameOfLife")
		UTexture2D* GameTexture = nullptr;
	//游戏初始画布Pattern。
	UPROPERTY(EditAnywhere, Category = "GameOfLife")
		UGameOfLifePattern* GameInitCanvas = nullptr;
	//用于绘制的笔刷pattern。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameOfLife")
		TArray<UGameOfLifePattern*> BrushPatterns;

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
	TArray<FPatternWithSize> BrushPatternsData;
	static void GetPatternsFromAsset(FPatternWithSize& PatternToWrite, const UGameOfLifePattern* TableToRead);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
