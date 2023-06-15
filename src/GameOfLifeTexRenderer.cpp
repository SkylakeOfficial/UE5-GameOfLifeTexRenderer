// Skylake Game Studio ArknightsFP

#include "GameOfLifeTexRenderer.h"
#include "Kismet/KismetMathLibrary.h"
#include "RenderingThread.h"
#include "RHICommandList.h"


// Sets default values for this component's properties
UGameOfLifeTexRenderer::UGameOfLifeTexRenderer()
{
	PrimaryComponentTick.bCanEverTick = false;
	// ...
}

// Called when the game starts
void UGameOfLifeTexRenderer::BeginPlay()
{
	Super::BeginPlay();

	//历史步数别太多了
	HistorySteps = UKismetMathLibrary::Clamp(HistorySteps, 0, 7);

	GameTiles.Init(false, GameWidth*GameHeight);
	HistoryTiles.Init(GameTiles, HistorySteps);

	if (GameInitData) {
		static const FString ContextString(TEXT("GridData"));
		TArray<FName> RowNames = GameInitData->GetRowNames();
		for (int32 Index = 0; Index != GameTiles.Num(); ++Index)
		{
			if (RowNames.IsValidIndex(Index)) {
				const FStructOnlyBools* TmpStruct = GameInitData->FindRow<FStructOnlyBools>(RowNames[Index], ContextString);
				GameTiles[Index] = TmpStruct->IsTrue;
			}
			else break;
		}

	}

	if (!PatternsData.IsEmpty())
	{
		for (int32 i = 0; i != PatternsData.Num(); ++i)
		{
			TArray<bool> Pattern;
			GetPatternsFromTable(Pattern, PatternsData[i].PatternData);
			Patterns.Emplace(Pattern);
		}
	}

	GameTexture = UTexture2D::CreateTransient(GameWidth, GameHeight, PF_R8);
	GameTexture->NeverStream = true;
	GameTexture->Filter = TF_Nearest;
	//GameTexture->MipGenSettings = TMGS_NoMipmaps;
	GameTexture->UpdateResource();
	ReDrawCanvas();
}

// Called every frame
void UGameOfLifeTexRenderer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UGameOfLifeTexRenderer::UpdateGameOfLife()
{
	//先将GameTiles写入历史记录
	if (HistoryTiles.Num()>0)
	{
		for (int32 i = HistoryTiles.Num()-1; i>=0;--i)
		{
			if (i>0)
			{
				HistoryTiles[i] = HistoryTiles[i - 1];
			}
			else
			{
				HistoryTiles[0] = GameTiles;
			}
		}
		
	}
	//再迭代GameTiles
	TArray<bool> TmpTiles = GameTiles;
	for (int32 Index = 0; Index != GameTiles.Num(); ++Index)
	{
		int32 LivingCellsCount = 0;

		int32 IndexToFind = Index - 1 - GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind]) { LivingCellsCount++; } }
		IndexToFind = Index - 1;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind]) { LivingCellsCount++; } }
		IndexToFind = Index - 1 + GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind]) { LivingCellsCount++; } }
		IndexToFind = Index - GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind]) { LivingCellsCount++; } }
		IndexToFind = Index + GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind]) { LivingCellsCount++; } }
		IndexToFind = Index + 1 - GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind]) { LivingCellsCount++; } }
		IndexToFind = Index + 1;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind]) { LivingCellsCount++; } }
		IndexToFind = Index + 1 + GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind]) { LivingCellsCount++; } }
		
		//规则S23
		if (GameTiles[Index]) {
			if (LivingCellsCount != 2 && LivingCellsCount != 3) {
				TmpTiles[Index] = false;
			}
		}
		//规则B3
		else if (LivingCellsCount == 3) {
			TmpTiles[Index] = true;
		}
	}
	GameTiles = TmpTiles;
	ReDrawCanvas();


}

bool UGameOfLifeTexRenderer::DrawDotOnCanvas(FVector2D Coord)//�ڻ����ϻ���
{
	//处理越界UV
	UKismetMathLibrary::FMod(Coord.X, 1, Coord.X);
	UKismetMathLibrary::FMod(Coord.Y, 1, Coord.Y);
	Coord = UKismetMathLibrary::ClampAxes2D(Coord, 0, 1);
	int32 TargetIndex = GameWidth * (UKismetMathLibrary::Round(Coord.Y * GameHeight) - 1) + UKismetMathLibrary::Round(Coord.X * GameWidth);
	TargetIndex = UKismetMathLibrary::Clamp(TargetIndex, 0, GameWidth * GameHeight - 1);
	//计时器时间内如果击中同一个点，则忽略
	if (TargetIndex == LastDrawIndex)
	{
		if (GetWorld()->GetTimerManager().TimerExists(DuplicateAvoidTimer))
		{
			return false;
		}
	}

	LastDrawIndex = TargetIndex;
	GetWorld()->GetTimerManager().SetTimer(DuplicateAvoidTimer, 0.1f, false);

	if (GameTiles[TargetIndex]) {
		GameTiles[TargetIndex] = false;
	}
	else GameTiles[TargetIndex] = true;
	ReDrawCanvas();
	return true;
}

bool UGameOfLifeTexRenderer::DrawPatternOnCanvas(FVector2D Coord)
{
	//处理越界UV
	UKismetMathLibrary::FMod(Coord.X, 1, Coord.X);
	UKismetMathLibrary::FMod(Coord.Y, 1, Coord.Y);
	Coord = UKismetMathLibrary::ClampAxes2D(Coord, 0, 1);
	int32 TargetIndex = GameWidth * (UKismetMathLibrary::Round(Coord.Y * GameHeight) - 1) + UKismetMathLibrary::Round(Coord.X * GameWidth);
	TargetIndex = UKismetMathLibrary::Clamp(TargetIndex, 0, GameWidth * GameHeight - 1);
	//计时器时间内如果击中同一个点，则忽略
	if (GetWorld()->GetTimerManager().TimerExists(DuplicateAvoidTimer))
	{
		return false;
	}

	GetWorld()->GetTimerManager().SetTimer(DuplicateAvoidTimer, 0.1f, false);

	TArray<bool> Pattern = { false, true, false, false, true, true, true, false, true};//Default Glider
	FIntPoint Size = FIntPoint(3, 3);//Default Glider

	if(!PatternsData.IsEmpty())
	{
		const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, PatternsData.Num() - 1);
		Pattern = Patterns[RandIndex];
		Size = PatternsData[RandIndex].PatternSize;

	}
	

	if (Pattern.Num()<Size.X*Size.Y)
	{
		return false;
	}

	for (int32 i = 0; i<Size.Y; ++i)
	{
		if(const int32 CoordY =i - Size.Y / 2 + TargetIndex / GameWidth; CoordY + 1 >= 0 && CoordY <= GameHeight)
		{
			for (int32 j = 0; j<Size.X; ++j)
			{
				if (const int32 CoordX =j - Size.X / 2 + TargetIndex % GameWidth; CoordX >= 0 && CoordX <= GameWidth)
				{
					int32 FinalIndex = CoordY * GameWidth + CoordX;
					if(GameTiles.IsValidIndex(FinalIndex))
					{
						//int32 PatternIndex = UKismetMathLibrary::Clamp(i * Size.X + j,0,Size.X*Size.Y-1);
						int32 PatternIndex = i * Size.X + j;
						GameTiles[FinalIndex] = GameTiles[FinalIndex] || Pattern[PatternIndex];
					}
				}
			}
		}
	}
	ReDrawCanvas();
	return true;
}

void UGameOfLifeTexRenderer::ReDrawCanvas()
{
	//ColorArray Gen
	TArray<uint8> ColorArray;

	for (int32 Index = 0; Index != GameTiles.Num(); ++Index)
	{
		uint8 tmpColor = 0;

		if (GameTiles[Index])
		{
			tmpColor = 255;
		}
		else if (HistoryTiles.Num()>0)
		{
			for (int32 i = 0; i!= HistoryTiles.Num();++i)
			{
				if (HistoryTiles[i][Index])
				{
					tmpColor = 255 / (IntPow(2, i+1));
					break;
				}
			}
		}
		ColorArray.Emplace(tmpColor);
	}

	bool bFreeData = false;
	UTexture2D* Texture = GameTexture;
	if (GameTexture->GetResource()) {
		struct FUpdateTextureRegionsData {
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};
		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;
		RegionData->Texture2DResource = (FTexture2DResource*)GameTexture->GetResource();
		RegionData->MipIndex = 0;
		RegionData->NumRegions = 1;
		RegionData->Regions = new FUpdateTextureRegion2D(0, 0, 0, 0, GameWidth, GameHeight);
		RegionData->SrcPitch = GameWidth;
		RegionData->SrcBpp = 1;
		RegionData->SrcData = ColorArray.GetData();

		ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
			[RegionData, bFreeData, Texture](FRHICommandListImmediate& RHICmdList)
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					const int32 CurrentFirstMip = Texture->FirstResourceMemMip;
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							Texture->GetResource()->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if (bFreeData) {
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
	}
}

int32 UGameOfLifeTexRenderer::IntPow(int32 x, uint8 p)
{
	if (p == 0) return 1;
	if (p == 1) return x;

	int32 tmp = IntPow(x, p / 2);
	if (p % 2 == 0) return tmp * tmp;
	else return x * tmp * tmp;
}

void UGameOfLifeTexRenderer::GetPatternsFromTable(TArray<bool> & PatternToWrite, const UDataTable* TableToRead) const
{
	static const FString ContextString(TEXT("GridData"));
	TArray<FName> RowNames = TableToRead->GetRowNames();
	for (int32 Index = 0; Index != GameTiles.Num(); ++Index)
	{
		if (RowNames.IsValidIndex(Index)) {
			const FStructOnlyBools* TmpStruct = TableToRead->FindRow<FStructOnlyBools>(RowNames[Index], ContextString);
			PatternToWrite.Emplace(TmpStruct->IsTrue);
		}
		else break;
	}


}

