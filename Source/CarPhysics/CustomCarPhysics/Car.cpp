// Fill out your copyright notice in the Description page of Project Settings.


#include "Car.h"

#include "CarController.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ACar::ACar()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	Box->SetupAttachment(RootComponent);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(Box);
	
	FL_Wheel = CreateDefaultSubobject<USceneComponent>("FL_Wheel");
	FL_Wheel->SetupAttachment(Box);
	FR_Wheel = CreateDefaultSubobject<USceneComponent>("FR_Wheel");
	FR_Wheel->SetupAttachment(Box);
	BL_Wheel = CreateDefaultSubobject<USceneComponent>("BL_Wheel");
	BL_Wheel->SetupAttachment(Box);
	BR_Wheel = CreateDefaultSubobject<USceneComponent>("BR_Wheel");
	BR_Wheel->SetupAttachment(Box);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm Component");
	SpringArm->SetupAttachment(Box);
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);

	SpringArm->TargetArmLength = 600.f;
	SpringArm->bUsePawnControlRotation = false;

	Camera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ACar::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACar::Suspension(USceneComponent* Wheel)
{
	FVector const StartLocation = Wheel->GetComponentLocation();
	FVector const EndLocation = Wheel->GetComponentLocation() + SpringHeight;
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartLocation,EndLocation,static_cast<ETraceTypeQuery>(ECollisionChannel::ECC_Pawn),false,ActorsToIgnore,EDrawDebugTrace::ForOneFrame,HitResult,true,FLinearColor::Red, FLinearColor::Green);

	//Suspension spring force
	if(bHit)
	{
		//tính độ co và giãn của lò xo bánh xe (Bao gồm cả hướng)
		FVector Offset = (1 - UKismetMathLibrary::NormalizeToRange(HitResult.Distance, 0.0f, -SpringHeight.Z)) *
			UKismetMathLibrary::GetDirectionUnitVector(HitResult.TraceEnd, HitResult.TraceStart);
		
		//Tính vận tốc theo phương của lò xo
		FVector WheelVelocity = Box->GetPhysicsLinearVelocityAtPoint(Wheel->GetComponentLocation()); // tính vận tốc tại vị trí bánh xe
		FVector SpringDirection = UKismetMathLibrary::GetDirectionUnitVector(HitResult.TraceEnd, HitResult.TraceStart);
		float VelocityAlongSpring = FVector::DotProduct(SpringDirection, WheelVelocity); //vận tốc theo hướng lò xo

		//Tính lực giảm chấn
		FVector DampingForce = VelocityAlongSpring * DampingCoefficient * SpringDirection;

		//Tổng lực tác dụng lên bánh xe
		FVector TotalForce = (Offset * Strength) - DampingForce;
		SuspensionForce = TotalForce;
		Box->AddForceAtLocation(TotalForce, Wheel->GetComponentLocation());
	}
	else
	{
		SuspensionForce = FVector::ZeroVector;
		Box->AddForceAtLocation(FVector::ZeroVector, Wheel->GetComponentLocation());
	}
}

void ACar::SteeringForce(USceneComponent* Wheel, float TiresGrip)
{
	FVector const StartLocation = Wheel->GetComponentLocation();
	FVector const EndLocation = Wheel->GetComponentLocation() + SpringHeight;
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartLocation,EndLocation,static_cast<ETraceTypeQuery>(ECollisionChannel::ECC_Pawn),false,ActorsToIgnore,EDrawDebugTrace::ForOneFrame,HitResult,true,FLinearColor::Red, FLinearColor::Green);

	//Steering force
	if(bHit)
	{
		FVector SteeringDir = Wheel->GetRightVector(); //hướng lực lái
		FVector TireWorldVel = Box->GetPhysicsLinearVelocityAtPoint(Wheel->GetComponentLocation()); //tính vận tốc bánh xe
		float SteeringVel = FVector::DotProduct(TireWorldVel, SteeringDir); //vận tốc hướng lái

		//tính giá trị bám đường của lốp xe (0 = không bám, 1 = bám hoàn toàn)
		float TireGripFactor = TiresGrip; //điều chỉnh số này để chỉnh lốp xe trượt nhiều hay ít
		float DeriredVelChange = -SteeringVel * TireGripFactor;

		//chuyển đổi vận tốc thành gia tốc
		float DesiredAccel = DeriredVelChange / GetWorld()->GetDeltaSeconds();

		FVector SteeringForce = SteeringDir * TireMass * DesiredAccel;
		Box->AddForceAtLocation(SteeringForce, Wheel->GetComponentLocation());
	}
}

void ACar::SteeringWheel1(USceneComponent* Wheel, float ActionValue)
{
	ACarController* CarController = Cast<ACarController>(GetController());
	if (CarController)
	{
		WheelSteer = ActionValue * WheelSteerSpeed;
		float NewWheelRotation = UKismetMathLibrary::FInterpTo(FR_Wheel->GetRelativeRotation().Yaw, WheelSteer, UGameplayStatics::GetWorldDeltaSeconds(this), 3.0f);
		Wheel->SetRelativeRotation({0,NewWheelRotation,0}, false,nullptr, ETeleportType::None);
	}
}

void ACar::SteeringWheel2(USceneComponent* Wheel)
{
	//float NewWheelRotation = UKismetMathLibrary::FInterpTo(FR_Wheel->GetRelativeRotation().Yaw,  Box->GetForwardVector().Z, UGameplayStatics::GetWorldDeltaSeconds(this), 3.0f);
	float NewWheelRotation = Box->GetForwardVector().Z;
	Wheel->SetRelativeRotation({0,NewWheelRotation,0}, false,nullptr, ETeleportType::None);
}

void ACar::Acceleration(USceneComponent* Wheel)
{
	FVector const StartLocation = Wheel->GetComponentLocation();
	FVector const EndLocation = Wheel->GetComponentLocation() + SpringHeight;
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartLocation,EndLocation,static_cast<ETraceTypeQuery>(ECollisionChannel::ECC_Pawn),false,ActorsToIgnore,EDrawDebugTrace::ForOneFrame,HitResult,true,FLinearColor::Red, FLinearColor::Green);

	//Acceleration force
	if(bHit)
	{
		FVector AccelDir = Wheel->GetForwardVector(); //hướng gia tốc theo Forward bánh xe
		float CarSpeed = FVector::DotProduct(Box->GetForwardVector(), Box->GetComponentVelocity()); //tính tốc độ hiện tại của xe theo hướng chuyển động
		float NormalizedSpeed = FMath::Clamp(FMath::Abs(CarSpeed)/ CarMaxSpeedCurve, 0.1f, 1.0f);
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("NormalizedSpeed: %.2f"), NormalizedSpeed));

		float AvailableTorque = 0.0f;
		if (PowerCurve)
		{
			ACarController* CarController = Cast<ACarController>(GetController());
			if (CarController)
			{
				float AccelInput = CarController->AccelInput;
				AvailableTorque = PowerCurve->GetFloatValue(NormalizedSpeed) * CarController->AccelInput;
				FVector ForceMoveCar = AccelDir* AvailableTorque * CarSpeedChange; //Thay đổi giá trị CarSpeedChange để xe đi nhanh hoặc chậm 
				Box->AddForceAtLocation(ForceMoveCar, Wheel->GetComponentLocation());
			}
		}
	}
}

void ACar::Friction(USceneComponent* Wheel)
{
	FVector const StartLocation = Wheel->GetComponentLocation();
	FVector const EndLocation = Wheel->GetComponentLocation() + SpringHeight;
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartLocation,EndLocation,static_cast<ETraceTypeQuery>(ECollisionChannel::ECC_Pawn),false,ActorsToIgnore,EDrawDebugTrace::ForOneFrame,HitResult,true,FLinearColor::Red, FLinearColor::Green);

	if(bHit)
	{
		
		//tính lực pháp tuyến (N=mg)
		float NormalForce = CarMass * Gravity;
		
		//Lấy vận tốc hiện tại của xe
		FVector CurrentSpeedCar = Box->GetComponentVelocity();
		
		//nếu xe đứng yên, áp dụng ma sát tĩnh
		if (CurrentSpeedCar.Size() < 5.0f)
		{
			FVector FrictionForce = -CurrentSpeedCar.GetSafeNormal() * (FrictionStatic * NormalForce);
			Box->AddForce(FrictionForce);
		}
		//nếu xe di chuyển, áp dụng ma sát động
		else
		{
			FVector FrictionForce = -CurrentSpeedCar.GetSafeNormal() * (FrictionDynamic * NormalForce);
			Box->AddForce(FrictionForce);
		}
	}
}

void ACar::Break(USceneComponent* Wheel)
{
	
}

// Called every frame
void ACar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Suspension(FL_Wheel);
	Suspension(FR_Wheel);
	Suspension(BL_Wheel);
	Suspension(BR_Wheel);

	SteeringForce(FL_Wheel, FrontTiresGrip);
	SteeringForce(FR_Wheel, FrontTiresGrip);
	SteeringForce(BL_Wheel, RearTiresGrip);
	SteeringForce(BR_Wheel, RearTiresGrip);

	Acceleration(FL_Wheel);
	Acceleration(FR_Wheel);
	Acceleration(BL_Wheel);
	Acceleration(BR_Wheel);

	Friction(FL_Wheel);
	Friction(FR_Wheel);
	Friction(BL_Wheel);
	Friction(BR_Wheel);

	DrawDebugLine(GetWorld(), FL_Wheel->GetComponentLocation(),
		FL_Wheel->GetComponentLocation() + FL_Wheel->GetForwardVector() *100.f, FColor::Blue, false, 0.05f,100.f, 5.f);
	DrawDebugLine(GetWorld(), FR_Wheel->GetComponentLocation(),
		FR_Wheel->GetComponentLocation() + FR_Wheel->GetForwardVector() *100.f, FColor::Blue, false, 0.05f, 100.f, 5.f);
	DrawDebugLine(GetWorld(), FL_Wheel->GetComponentLocation(),
		FL_Wheel->GetComponentLocation() + FVector::UpVector * SuspensionForce.Length(), FColor::Yellow, false, 0.05f, 100.f, 5.f);
	DrawDebugLine(GetWorld(), FR_Wheel->GetComponentLocation(),
		FR_Wheel->GetComponentLocation() + FVector::UpVector * SuspensionForce.Length(), FColor::Yellow, false, 0.05f, 100.f, 5.f);
}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("TotalForce %s"), *TotalForce.ToString()));
//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Distance: %.f"), WheelComponent->GetComponentLocation().Z));
//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Distance: %.2f"), HitResult.Distance));
//DrawDebugLine(GetWorld(), Wheel->GetComponentLocation(), TotalForce, FColor::Green, false, 1, 0, 1);