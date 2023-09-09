

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameOfLifePattern.generated.h"

/**
 * 
 */
UCLASS()
class ARKNIGHTSFP_API UGameOfLifePattern : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameOfLife")
	TArray<uint8> Pattern;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameOfLife")
	FIntPoint PatternSize = FIntPoint(1,1);
};
