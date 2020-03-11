// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AssetFabricator.generated.h"

struct FMeshMergingSettings;

UCLASS()
class WELCOMETOEARTH_API AAssetFabricator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAssetFabricator();

	//Main Mesh Merge functionality
	UFUNCTION(BlueprintCallable)
	void RunTool(FString fileName);

public:	
	UPROPERTY(editanywhere, meta = (ShowOnlyInnerProperties), Category = MergeSettings)
	FMeshMergingSettings Settings;
};
