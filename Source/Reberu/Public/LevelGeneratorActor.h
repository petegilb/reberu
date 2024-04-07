// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGeneratorActor.generated.h"

class UReberuData;

UCLASS()
class REBERU_API ALevelGeneratorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelGeneratorActor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBillboardComponent* SpriteComponent;
	
	// Icon sprite
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UTexture2D* SpriteTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UReberuData* ReberuData = nullptr;
};
