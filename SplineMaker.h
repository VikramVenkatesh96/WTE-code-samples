#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplineMaker.generated.h"

class USplineComponent;

UCLASS()
class WELCOMETOEARTH_API ASplineMaker : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASplineMaker();

	virtual void OnConstruction(const FTransform &) override;

private:
	void RemoveAllMeshComponents();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* splineMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollisionEnabled;

private:
	UPROPERTY(VisibleAnywhere)
	USplineComponent* splineComponent;

	float segmentLength;
};
