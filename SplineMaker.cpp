#include "SplineMaker.h"
#include "Engine/StaticMesh.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

// Sets default values
ASplineMaker::ASplineMaker()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Create spline component and make it the root
	splineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = splineComponent;
}

void ASplineMaker::OnConstruction(const FTransform &)
{
	//If any mesh component exists, destroy them
	RemoveAllMeshComponents();

	if (splineMesh)
	{
		FBox BoundingBox = splineMesh->GetBoundingBox();
		//Set segment length to the bounding box length along X-Axis(model should be aligned along the X-Axis)
		segmentLength = (BoundingBox.Max - BoundingBox.Min).X;
		//Get number of spline meshes to spawn
		unsigned int NumberOfSplineMeshes = (unsigned int)splineComponent->GetSplineLength() / segmentLength;

		for (unsigned int i = 0; i <= NumberOfSplineMeshes; ++i)
		{
			//Setup the spline mesh component to spawn
			FString ComponentName = TEXT("SplineMeshComponent_") + FString::FromInt(i + 1);
			USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), FName(*ComponentName));

			//Register and attach component to root
			SplineMeshComponent->RegisterComponent();
			SplineMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

			//Setup spline mesh attributes
			SplineMeshComponent->SetStaticMesh(splineMesh);
			SplineMeshComponent->SetForwardAxis(ESplineMeshAxis::X);

			//Add collisions if required
			if (bIsCollisionEnabled)
			{
				SplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}

			FVector StartPos = splineComponent->GetLocationAtDistanceAlongSpline(i * segmentLength, ESplineCoordinateSpace::World);
			FVector StartTangent = splineComponent->GetTangentAtDistanceAlongSpline(i * segmentLength, ESplineCoordinateSpace::World).GetClampedToSize(0.0f, segmentLength);
			FVector EndPos = splineComponent->GetLocationAtDistanceAlongSpline((i + 1) * segmentLength, ESplineCoordinateSpace::World);
			FVector EndTangent = splineComponent->GetTangentAtDistanceAlongSpline((i + 1) * segmentLength, ESplineCoordinateSpace::World).GetClampedToSize(0.0f, segmentLength);
			SplineMeshComponent->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
		}
	}
}

void ASplineMaker::RemoveAllMeshComponents()
{
	TArray<UActorComponent*> MeshComponents = GetComponentsByClass(USplineMeshComponent::StaticClass());
	for (UActorComponent* meshComponent : MeshComponents)
	{
		meshComponent->SetActive(false);
		meshComponent->DestroyComponent();
	}
}
