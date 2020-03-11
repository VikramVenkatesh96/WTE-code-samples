// Fill out your copyright notice in the Description page of Project Settings.
#pragma region Header Files
#include "AssetFabricator.h"
#include "AssetRegistryModule.h"
#include "Components/StaticMeshComponent.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "IMeshMergeUtilities.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/MeshMerging.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Modules/ModuleManager.h"
#include "MeshUtilities.h"
#include "MeshMergeModule.h"
#pragma endregion

// Sets default values
AAssetFabricator::AAssetFabricator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Settings.bMergePhysicsData = true;
	Settings.LODSelectionType = EMeshLODSelectionType::AllLODs;
}

void AAssetFabricator::RunTool(FString fileName)
{
	//Make sure tool is run from FabricationLevel
	if (GetWorld()->GetName().Compare("FabricationLevel") != 0)
	{
#if defined(UE_BUILD_DEVELOPMENT) && defined(UE_BUILD_DEBUG)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Incorrect Level: Use the FabricationLevel for asset merging"));
#endif
		return;
	}

	//Load and Get the main MeshUtils module
	const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();

	TArray<AActor*> meshActors;
	//Aquire all static mesh actors in the level
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), meshActors);

	TArray<UPrimitiveComponent*> staticMeshComponents;

	//Slow-step merging
	for (AActor* actor : meshActors)
	{
		//Collect all static mesh components from each actor
		TArray<UStaticMeshComponent*> actorMeshComponents;
		actor->GetComponents<UStaticMeshComponent>(actorMeshComponents);

		//Move these components to the 'to-merge' staticMeshComponents array
		for (UStaticMeshComponent* component : actorMeshComponents)
		{
			if (component->GetStaticMesh() != nullptr)
			{
				staticMeshComponents.Add(component);
			}
		}
	}

	FVector MergedActorLocation;
	TArray<UObject*> NewAssetsToSync;

	if (staticMeshComponents.Num())
	{
		UWorld* World = staticMeshComponents[0]->GetWorld();
		checkf(World != nullptr, TEXT("Invalid World retrieved from Mesh components"));
		const float ScreenAreaSize = TNumericLimits<float>::Max();
		FString PackageName = FPackageName::FilenameToLongPackageName(FPaths::ProjectContentDir() + TEXT("Geometry/Meshes/Merged/"));

		if (fileName.IsEmpty())
		{
			PackageName += TEXT("SM_MergedMesh");
			PackageName = MakeUniqueObjectName(NULL, UPackage::StaticClass(), *PackageName).ToString();
		}
		else
		{
			PackageName += fileName;
		}

		MeshUtilities.MergeComponentsToStaticMesh(staticMeshComponents, World, Settings, nullptr, nullptr, PackageName, NewAssetsToSync, MergedActorLocation, ScreenAreaSize, true);
	}
	else
	{
#if defined(UE_BUILD_DEVELOPMENT) && defined(UE_BUILD_DEBUG)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Merge Failed : No mesh components found!"));
#endif
		return;
	}

	//Sync the new asset with the content browser
	if (NewAssetsToSync.Num())
	{
		FAssetRegistryModule& AssetRegistry = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		int32 AssetCount = NewAssetsToSync.Num();
		for (int32 AssetIndex = 0; AssetIndex < AssetCount; AssetIndex++)
		{
			AssetRegistry.AssetCreated(NewAssetsToSync[AssetIndex]);
			GEditor->BroadcastObjectReimported(NewAssetsToSync[AssetIndex]);
		}

		//Notify the content browser that the new assets exists
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(NewAssetsToSync, true);
#if defined(UE_BUILD_DEVELOPMENT) && defined(UE_BUILD_DEBUG)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Merge Successful!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString(TEXT("Number of Components Merged: ") + FString::FromInt(staticMeshComponents.Num())));
#endif
	}
}

