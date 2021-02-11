// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"
#include "ProceduralMeshComponent.h"
#include "IslandMesh.generated.h"

UCLASS()
class PILOT2020_API AIslandMesh : public AActor
{
	GENERATED_BODY()

		//		UPROPERTY(VisibleAnywhere)
		//		UProceduralMeshComponent * proceduralMeshComponent;

public:
	// Sets default values for this actor's properties
	AIslandMesh();

	//AIslandMesh();

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float maxHeight = 2000;

	void changeHeightDataToFitVertices();

	void createIslandMesh();

	void createNormals();

	UFUNCTION(BlueprintCallable)
		void drawDebugLines();

	UFUNCTION(BlueprintCallable)
		void changeHeight();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* myHeightmap;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> pixelValues;
	TArray<FVector> heightData;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProceduralMeshComponent* proceduralMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 subdivisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float size = 1000.f;

	void generateFlatMesh();

	UFUNCTION(BlueprintCallable)
		TArray<float> readHeightmap();

	void PostActorCreated();
	void PostLoad();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> vertices;
	TArray<int32> indices;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> tangents; // 90 deg from normal
	TArray<FLinearColor> vertexColors; // useless?

	// '''Don't use this function'''. It is deprecated. Use LinearColor version.
	void CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, const TArray<FColor>& VertexColors, const TArray<FProcMeshTangent>& Tangents, bool bCreateCollision);

	// In this one you can send FLinearColor instead of FColor for the Vertex Colors.
	void CreateMeshSection_LinearColor(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, const TArray<FLinearColor>& VertexColors, const TArray<FProcMeshTangent>& Tangents, bool bCreateCollision);

	// Updates a section of this procedural mesh component. This is faster than CreateMeshSection, but does not let you change topology. Collision info is also updated.
	void UpdateMeshSection_LinearColor(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, const TArray<FLinearColor>& VertexColors, const TArray<FProcMeshTangent>& Tangents);
};
