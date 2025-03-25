// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Car.generated.h"

UCLASS()
class CARPHYSICS_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* FL_Wheel;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* FR_Wheel;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* BL_Wheel;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* BR_Wheel;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* Box;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Stats")
	UCurveFloat* PowerCurve;
	
	void Suspension(USceneComponent* Wheel);

	void Acceleration(USceneComponent* Wheel);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CarMaxSpeed = 2000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float WheelSteer;
public:
	void SteeringForce(USceneComponent* Wheel);

	void SteeringWheel1(USceneComponent* Wheel, float ActionValue);

	void SteeringWheel2(USceneComponent* Wheel, float ActionValue);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector SuspensionForce;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
