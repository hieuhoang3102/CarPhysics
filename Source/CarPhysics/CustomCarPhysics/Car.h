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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Strength = 1000000.0f; //độ cứng của lò xo nâng lốp xe

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DampingCoefficient = 1000.0f; //lực giảm chấn của lốp xe
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CarMaxSpeedCurve = 2000.0f; //tốc độ lớn nhất của xe	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CarSpeedChange = 30;	//tốc độ gia tăng của xe

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float TireMass = 50.0f;	//khối lượng lốp xe
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float WheelSteer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float WheelSteerSpeed = 15.0f; //tăng giảm tốc độ cua của bánh xe

	UPROPERTY(VIsibleAnywhere, BlueprintReadWrite)
	float FrontTiresGrip = 0.8f; //độ bám đường lốp trước

	UPROPERTY(VIsibleAnywhere, BlueprintReadWrite)
	float RearTiresGrip = 0.8f;	//độ bám đường lốp sau

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float FrictionStatic = 1.2f; //hệ số ma sát tĩnh

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float FrictionDynamic = 0.8f; //hệ số ma sát động

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Gravity = 980.0f; //Trọng lực

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CarMass = 30.0f; //Khối lượng xe
	
	void Suspension(USceneComponent* Wheel);

	void Acceleration(USceneComponent* Wheel);

	void Friction(USceneComponent* Wheel);

public:
	void SteeringForce(USceneComponent* Wheel, float TiresGrip);

	void SteeringWheel1(USceneComponent* Wheel, float ActionValue);

	void SteeringWheel2(USceneComponent* Wheel);

	void Break(USceneComponent* Wheel);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector SuspensionForce;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
