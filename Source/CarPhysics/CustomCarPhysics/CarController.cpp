// Fill out your copyright notice in the Description page of Project Settings.


#include "CarController.h"

#include "Car.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ACarController::ACarController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


void ACarController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Throught, ETriggerEvent::Triggered, this, &ACarController::ThroughtInputAction);
		EnhancedInputComponent->BindAction(IA_Throught, ETriggerEvent::Completed, this, &ACarController::ThroughtInputAction);
		EnhancedInputComponent->BindAction(IA_Steer, ETriggerEvent::Triggered, this, &ACarController::SteerInputAction);
		EnhancedInputComponent->BindAction(IA_Steer, ETriggerEvent::Started, this, &ACarController::SteerInputAction);
		EnhancedInputComponent->BindAction(IA_Steer, ETriggerEvent::Completed, this, &ACarController::SteerCompletedAction);
		
	}
}

void ACarController::ThroughtInputAction(const FInputActionValue& Value)
{
	const float ActionValue = Value.Get<float>();
	AccelInput = ActionValue;
}

void ACarController::SteerInputAction(const FInputActionValue& Value)
{
	const float ActionValue = Value.Get<float>();
	SteerInput = ActionValue;
	ACar* CarHiu = Cast<ACar>(GetPawn());
	CarHiu->SteeringWheel1(CarHiu->FL_Wheel, ActionValue);
	CarHiu->SteeringWheel1(CarHiu->FR_Wheel, ActionValue);
}

void ACarController::SteerCompletedAction(const FInputActionValue& Value)
{
	const float ActionValue = Value.Get<float>();
	SteerCompleted = ActionValue;
	ACar* CarHiu = Cast<ACar>(GetPawn());
	CarHiu->SteeringWheel2(CarHiu->FL_Wheel);
	CarHiu->SteeringWheel2(CarHiu->FR_Wheel);
}

// Called when the game starts or when spawned
void ACarController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACarController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

