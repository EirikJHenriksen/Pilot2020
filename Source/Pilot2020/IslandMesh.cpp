// Fill out your copyright notice in the Description page of Project Settings.

#include "IslandMesh.h"

#include "Engine/TextureDefines.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "DrawDebugHelpers.h"

// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("UV %f %f"), u, v));

// Sets default values
AIslandMesh::AIslandMesh()
{
	proceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = proceduralMeshComponent;
	// New in UE 4.17, multi-threaded PhysX cooking.
	proceduralMeshComponent->bUseAsyncCooking = true;
}

// This is called when actor is spawned (at runtime or when you drop it into the world in editor)
void AIslandMesh::PostActorCreated()
{
	Super::PostActorCreated();

	createIslandMesh();
}

// This is called when actor is already in level and map is opened
void AIslandMesh::PostLoad()
{
	Super::PostLoad();

	createIslandMesh();
}

void AIslandMesh::createIslandMesh()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("make an island~")));


	// make a mesh (subdivisions/resolution should go in here?)
	generateFlatMesh();

	readHeightmap();
	changeHeight();

	changeHeightDataToFitVertices();

	createNormals();

	// actually make the mesh - all calculations should be done
	proceduralMeshComponent->CreateMeshSection_LinearColor(0, vertices, indices, normals, UVs, vertexColors, tangents, true);

	//drawDebugLines();
}

void AIslandMesh::generateFlatMesh()//int32 subdivisions)
{
	int32 sqrs = subdivisions + 1; // squares going one direction

	vertices.Reset();
	indices.Reset();
	normals.Reset();
	UVs.Reset();
	tangents.Reset();
	vertexColors.Reset();

	// ========================== create vertices 

	float radius = size / 2;
	float step = size / FGenericPlatformMath::Max(1, subdivisions + 1);

	for (int i = 0; i <= subdivisions + 1; i++) // x
	{
		for (int j = 0; j <= subdivisions + 1; j++) // y
		{
			vertices.Add(FVector(-radius + (step*i) + GetActorLocation().X, -radius + (step*j) + GetActorLocation().Y, 0 + GetActorLocation().Z));


			// UV
			float u = (i * step / size);
			float v = (j * step / size);
			UVs.Add(FVector2D(u, v));
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("UV %f %f"), u, v));

		}
	}

	// ========================== create indices - (loops once per square)
	for (int i = 0; i < (sqrs * sqrs) + subdivisions; i++)
	{
		if ((i + 1) % (sqrs + 1) == 0)
			i++;

		indices.Add(i);
		indices.Add(i + 1);
		indices.Add(i + sqrs + 1);

		indices.Add(i + sqrs + 1);
		indices.Add(i + 1);
		indices.Add(i + sqrs + 2);
	}

	//TArray<FVector2D> UVs;
	UVs.Add(FVector2D(0, 0));
	UVs.Add(FVector2D(10, 0));
	UVs.Add(FVector2D(0, 10));
	UVs.Add(FVector2D(10, 10));

	//TArray<FProcMeshTangent> tangents;
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	//TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

}

TArray<float> AIslandMesh::readHeightmap()
{
	pixelValues.Empty();

	// ============ [https://isaratech.com/ue4-reading-the-pixels-from-a-utexture2d/]

	// save old settings
	TextureCompressionSettings OldCompressionSettings = myHeightmap->CompressionSettings;
	/*TextureMipGenSettings OldMipGenSettings = myHeightmap->MipGenSettings;*/
	bool OldSRGB = myHeightmap->SRGB;

	// give new settings
	myHeightmap->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	/*myHeightmap->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;*/
	myHeightmap->SRGB = false;
	myHeightmap->UpdateResource();

	// lock data - for reading
	const FColor* FormatedImageData = static_cast<const FColor*>(myHeightmap->PlatformData->Mips[0].BulkData.LockReadOnly());

	for (int32 X = 0; X < myHeightmap->GetSizeX(); X++)
	{
		for (int32 Y = 0; Y < myHeightmap->GetSizeY(); Y++)
		{
			FColor PixelColor = FormatedImageData[Y * myHeightmap->GetSizeX() + X];

			// add height value between 0 and 1
			pixelValues.Add(float(PixelColor.R + PixelColor.G + PixelColor.B) / 765.f);
		}
	}

	// unlock data again
	myHeightmap->PlatformData->Mips[0].BulkData.Unlock();

	// give old settings back
	myHeightmap->CompressionSettings = OldCompressionSettings;
	/*myHeightmap->MipGenSettings = OldMipGenSettings;*/
	myHeightmap->SRGB = OldSRGB;
	myHeightmap->UpdateResource();



	return pixelValues;
}

void AIslandMesh::createNormals()
{
	// ========================== TArray<FVector> normals;

	// make temporary arrays
	TArray<FVector> currentNormals;
	currentNormals.Init(FVector(0, 0, 0), vertices.Num());
	TArray<int32> numbGivenNormals;
	numbGivenNormals.Init(0, vertices.Num());

	// for each triangle (3 indices)
	for (int i = 0; i < int(indices.Num()); i += 3)
	{
		// triangle abc
		FVector a = vertices[indices[i]];
		FVector b = vertices[indices[i + 1]];

		FVector c = vertices[indices[i + 2]];

		FVector ab = b - a;
		FVector ac = c - a;

		FVector crossProd = ac ^ ab;
		crossProd = crossProd.GetSafeNormal();

		// fill the temporary arrays
		numbGivenNormals[indices[i]] ++;
		numbGivenNormals[indices[i + 1]] ++;
		numbGivenNormals[indices[i + 2]] ++;

		currentNormals[indices[i]] += crossProd;
		currentNormals[indices[i + 1]] += crossProd;
		currentNormals[indices[i + 2]] += crossProd;
	}

	for (int i = 0; i < int(vertices.Num()); i++) // for each vertex
	{
		normals.Add(currentNormals[i].GetSafeNormal());
	}
}

// pixelValues -> heightData
void AIslandMesh::changeHeightDataToFitVertices()
{
	heightData.Empty();

	int pixelsWidth = myHeightmap->GetSizeX();
	int vertexWidth = 2 + subdivisions;

	// make heightData into vectors in same space as vertices
	for (int i = 0; i < pixelValues.Num(); i++)
	{
		int row = round(i / pixelsWidth);
		int column = FGenericPlatformMath::Fmod(i, pixelsWidth);

		float boardRadius = (size / 2);
		float x = (row * (size / pixelsWidth)) - boardRadius;
		float y = (column * (size / pixelsWidth)) - boardRadius;

		heightData.Add(FVector(x, y, pixelValues[i]));
	}

	// give vertices height based on surrounding heightData

	float radius = (size / (subdivisions + 1)) / 2;

	for (int i = 0; i < vertices.Num(); i++)
	{
		int tempValuesNumb = 0;
		float tempHeightCollected = 0;


		// find the heightData within radius?
		float minX = vertices[i].X - radius;
		float maxX = vertices[i].X + radius;
		float minY = vertices[i].Y - radius;
		float maxY = vertices[i].Y + radius;

		// for each pixel, see if its withing vertex area
		for (int j = 0; j < heightData.Num(); j++)
		{
			if ((heightData[j].X < maxX) && (heightData[j].X > minX) && (heightData[j].Y < maxY) && (heightData[j].Y > minY))
			{
				// crash
				tempValuesNumb++;
				tempHeightCollected += heightData[j].Z;
			}
		}

		// give vertex height based on collected info
		if (tempValuesNumb > 0)
			vertices[i].Z = (tempHeightCollected / tempValuesNumb) * maxHeight;
		else
		{
			vertices[i].Z = heightData[0].Z * maxHeight; // just in case, not a good fix though
			GEngine->AddOnScreenDebugMessage(-1, 40.f, FColor::Red, "vertex without height data :(");
		}
	}
}

// assumes there are as many heightdata as vert
void AIslandMesh::changeHeight()
{
	for (int i = 0; i < vertices.Num(); i++)
	{
		float tempHeight = pixelValues[i% pixelValues.Num()] * maxHeight;

		vertices[i].Z = tempHeight;
	}
}

void AIslandMesh::drawDebugLines()
{
	float lineLength = 500;

	FVector actorLocation = GetActorLocation();

	for (int i = 0; i < vertices.Num(); i++)
	{
		DrawDebugLine(GetWorld(), vertices[i] + actorLocation, (normals[i] * lineLength) + vertices[i] + actorLocation, FColor(255, 0, 0), false, 20.f, (uint8)'\000', 60.f);

		//GEngine->AddOnScreenDebugMessage(-1, 40.f, FColor::Red, normals[i].ToString());
	}
}