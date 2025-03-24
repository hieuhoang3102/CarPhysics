// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include "CarController.generated.h"

UCLASS()
class CARPHYSICS_API ACarController : public AController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACarController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
