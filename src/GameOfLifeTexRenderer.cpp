// Skylake Game Studio ArknightsFP

#include "ArknightsFP/GameOfLife/GameOfLifeTexRenderer.h"
#include "Kismet/KismetMathLibrary.h"
#include "RenderingThread.h"
#include "RHICommandList.h"

#define INDEX_FROM_X_Y(X,Y) (GameWidth*Y+X)

UGameOfLifeTexRenderer::UGameOfLifeTexRenderer()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGameOfLifeTexRenderer::BeginPlay()
{
	Super::BeginPlay();

	HistorySteps = UKismetMathLibrary::Clamp(HistorySteps, 0, 63);
	if (HistorySteps != 0)
	{
		HistoryFalloff = 256 / (1 + HistorySteps) - ((256 % (HistorySteps+1) == 0) ? 1 : 0);
		HistoryFalloff2x = HistoryFalloff * 2;
	}

	// Init GOL Canvas
	if (GameInitCanvas) {
		GameWidth = GameInitCanvas->PatternSize.X;
		GameHeight = GameInitCanvas->PatternSize.Y;
		for (uint8 Num : GameInitCanvas->Pattern)
		{
			TArray<uint8> NumArray;
			if (Num > 240)
			{
				NumArray.Init(255, Num - 240);
				GameTiles.Append(NumArray);
			}
			else
			{
				NumArray.Init(0, Num);
				GameTiles.Append(NumArray);
			}
		}
		GameTiles.SetNum(GameWidth * GameHeight);
	}
	else
	{
		GameTiles.Init(0, GameWidth * GameHeight);
	}

	// Init GOL brush patterns
	if (!BrushPatterns.IsEmpty())
	{
		for (int32 i = 0; i != BrushPatterns.Num(); ++i)
		{
			FPatternWithSize Pattern;
			GetPatternsFromAsset(Pattern, BrushPatterns[i]);
			BrushPatternsData.Emplace(Pattern);
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
}


void UGameOfLifeTexRenderer::UpdateGameOfLife()
{
	TBitArray<> TmpTiles;

	TmpTiles.Init(false, GameTiles.Num());
	int32 X = -1;
	int32 Y = -1;
	for (int32 Index = 0; Index != GameTiles.Num(); ++Index)
	{
		X = (X + 1) % GameWidth;
		if (X == 0) Y++;

		int32 LivingCellsCount = 0;

		int32 tY = Y;
		int32 tX = X;


		int32 IndexToFind = Index - GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind] == 255) { LivingCellsCount++; } }//↑
		else if (tY = ((Y + GameHeight - 1) % GameHeight); GameTiles[INDEX_FROM_X_Y(X, tY)] == 255) { LivingCellsCount++; }

		IndexToFind = Index + GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind] == 255) { LivingCellsCount++; } }//↓
		else if (tY = ((Y + GameHeight + 1) % GameHeight); GameTiles[INDEX_FROM_X_Y(X, tY)] == 255) { LivingCellsCount++; }

		IndexToFind = Index - 1;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind] == 255) { LivingCellsCount++; } }//←
		else if (tX = (X + GameWidth - 1) % GameWidth; GameTiles[INDEX_FROM_X_Y(tX, Y)] == 255) { LivingCellsCount++; }

		IndexToFind = Index + 1;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind] == 255) { LivingCellsCount++; } }//→
		else if (tX = (X + GameWidth + 1) % GameWidth; GameTiles[INDEX_FROM_X_Y(tX, Y)] == 255) { LivingCellsCount++; }
		if (LivingCellsCount > 3) { continue; }//S23

		IndexToFind = Index - 1 - GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind] == 255) { LivingCellsCount++; } }//↑←
		else {
			tX = (X + GameWidth - 1) % GameWidth;
			tY = ((Y + GameHeight - 1) % GameHeight);
			if (GameTiles[INDEX_FROM_X_Y(tX, tY)] == 255) { LivingCellsCount++; }
		}
		if (LivingCellsCount > 3) { continue; }//S23

		IndexToFind = Index + 1 - GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind] == 255) { LivingCellsCount++; } } //↑→
		else {
			tX = (X + GameWidth + 1) % GameWidth;
			tY = ((Y + GameHeight - 1) % GameHeight);
			if (GameTiles[INDEX_FROM_X_Y(tX, tY)] == 255) { LivingCellsCount++; }
		}
		if ((GameTiles[Index] != 255)&&LivingCellsCount < 1) { continue; }//B3
		if (LivingCellsCount > 3) { continue; }//S23

		IndexToFind = Index - 1 + GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind] == 255) { LivingCellsCount++; } }//↓←
		else {
			tX = (X + GameWidth - 1) % GameWidth;
			tY = ((Y + GameHeight + 1) % GameHeight);
			if (GameTiles[INDEX_FROM_X_Y(tX, tY)] == 255) { LivingCellsCount++; }
		}
		if (LivingCellsCount < (GameTiles[Index] == 255 ? 1 : 2)) { continue; }//B3S2
		if (LivingCellsCount > 3) { continue; }//S23

		IndexToFind = Index + 1 + GameWidth;
		if (GameTiles.IsValidIndex(IndexToFind)) { if (GameTiles[IndexToFind] == 255) { LivingCellsCount++; } } //↓→
		else {
			tX = (X + GameWidth + 1) % GameWidth;
			tY = ((Y + GameHeight + 1) % GameHeight);
			if (GameTiles[INDEX_FROM_X_Y(tX, tY)] == 255) { LivingCellsCount++; }
		}

		//B3S23
		if (LivingCellsCount == 3) {
			TmpTiles[Index] = true;
		}
		else if (GameTiles[Index]== 255 && LivingCellsCount == 2) {
			TmpTiles[Index] = true;
		}
	}
	
	for (int32 i = 0; i != TmpTiles.Num(); ++i)
	{
		if (TmpTiles[i])
		{
			GameTiles[i] = 255;
		}
		else
		{
			if (const uint8 Current = GameTiles[i]; Current != 0)
			{
				if (Current < HistoryFalloff2x)
				{
					GameTiles[i] = 0;
				}
				else
				{
					GameTiles[i] = Current - HistoryFalloff;
				}
			}
		}
	}
	ReDrawCanvas();


}

bool UGameOfLifeTexRenderer::DrawDotOnCanvas(FVector2D Coord)
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

	if (GameTiles[TargetIndex] == 255) {
		GameTiles[TargetIndex] = 255 - HistoryFalloff;
	}
	else GameTiles[TargetIndex] = 255;
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

	TArray<bool> Pattern = { false,true,false,false,true,true,true,false,true,true };//Default Glider
	FIntPoint Size = FIntPoint(3, 3);//Default Glider

	if(!BrushPatternsData.IsEmpty())
	{
		const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, BrushPatternsData.Num() - 1);
		Pattern = BrushPatternsData[RandIndex].Pattern;
		Size = BrushPatternsData[RandIndex].Size;
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
						if(GameTiles[FinalIndex] == 255 || Pattern[PatternIndex]) GameTiles[FinalIndex] = 255;
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
	UTexture2D* Texture = GameTexture;
	if (GameTexture->GetResource()) {
		bool bFreeData = false;
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
		RegionData->SrcData = GameTiles.GetData();

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

void UGameOfLifeTexRenderer::GetPatternsFromAsset(FPatternWithSize& PatternToWrite, const UGameOfLifePattern* PatternToRead)
{

	if (PatternToRead) {
		for (uint8 Num : PatternToRead->Pattern)
		{
			TArray<bool> NumArray;
			if (Num > 240)
			{
				NumArray.Init(true, Num - 240);
				PatternToWrite.Pattern.Append(NumArray);
			}
			else
			{
				NumArray.Init(false, Num);
				PatternToWrite.Pattern.Append(NumArray);
			}
		}
		PatternToWrite.Size = PatternToRead->PatternSize;
		PatternToWrite.Pattern.SetNum(PatternToRead->PatternSize.X* PatternToRead->PatternSize.Y);

	}
}