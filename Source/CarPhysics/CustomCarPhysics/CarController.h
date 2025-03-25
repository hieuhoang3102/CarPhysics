// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "GameFramework/Controller.h"
#include "CarController.generated.h"

UCLASS()
class CARPHYSICS_API ACarController : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACarController();

	//MappingContext 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Throught;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Steer;
	
	virtual void SetupInputComponent() override;

	void ThroughtInputAction(const FInputActionValue& Value);

	void SteerInputAction(const FInputActionValue& Value);

	void SteerCompletedAction(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteerInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteerCompleted;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
