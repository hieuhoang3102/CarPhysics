// Fill out your copyright notice in the Description page of Project Settings.


#include "Car.h"

#include "CarController.h"
#include "Camera/CameraComponent.h"
#include "CarPhysics/TraceUtils.h"
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

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
	
	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	SetRootComponent(Box);
	//Box->SetupAttachment(Root);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);

	FL_Staff_Wheel = CreateDefaultSubobject<USceneComponent>("FL_Staff_Wheel");
	FL_Staff_Wheel->SetupAttachment(RootComponent);
	FR_Staff_Wheel = CreateDefaultSubobject<USceneComponent>("FR_Staff_Wheel");
	FR_Staff_Wheel->SetupAttachment(RootComponent);
	BL_Staff_Wheel = CreateDefaultSubobject<USceneComponent>("BL_Staff_Wheel");
	BL_Staff_Wheel->SetupAttachment(RootComponent);
	BR_Staff_Wheel = CreateDefaultSubobject<USceneComponent>("BR_Staff_Wheel");
	BR_Staff_Wheel->SetupAttachment(RootComponent);
	
	FL_Wheel = CreateDefaultSubobject<USceneComponent>("FL_Wheel");
	FL_Wheel->SetupAttachment(FL_Staff_Wheel);
	FR_Wheel = CreateDefaultSubobject<USceneComponent>("FR_Wheel");
	FR_Wheel->SetupAttachment(FR_Staff_Wheel);
	BL_Wheel = CreateDefaultSubobject<USceneComponent>("BL_Wheel");
	BL_Wheel->SetupAttachment(BL_Staff_Wheel);
	BR_Wheel = CreateDefaultSubobject<USceneComponent>("BR_Wheel");
	BR_Wheel->SetupAttachment(BR_Staff_Wheel);

	SM_FL_Wheel = CreateDefaultSubobject<UStaticMeshComponent>("SM_FL_Wheel");
	SM_FL_Wheel->SetupAttachment(FL_Wheel);
	SM_FL_Wheel->SetRelativeRotation(FRotator(0, 0, 0));
	SM_FR_Wheel = CreateDefaultSubobject<UStaticMeshComponent>("SM_FR_Wheel");
	SM_FR_Wheel->SetupAttachment(FR_Wheel);
	SM_BL_Wheel = CreateDefaultSubobject<UStaticMeshComponent>("SM_BL_Wheel");
	SM_BL_Wheel->SetupAttachment(BL_Wheel);
	SM_BR_Wheel = CreateDefaultSubobject<UStaticMeshComponent>("SM_BR_Wheel");
	SM_BR_Wheel->SetupAttachment(BR_Wheel);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm Component");
	SpringArm->SetupAttachment(RootComponent);
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
	ActorsToIgnore.Add(this);
}

void ACar::Suspension(USceneComponent* StaffWheel, USceneComponent* Wheel)
{
	FVector const StartLocation = Wheel->GetComponentLocation();
	FVector const EndLocation = Wheel->GetComponentLocation() + FVector{0, 0, -90};
	FHitResult HitResult;
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartLocation,EndLocation,static_cast<ETraceTypeQuery>(ECollisionChannel::ECC_Pawn),false,ActorsToIgnore,EDrawDebugTrace::ForOneFrame,HitResult,true,FLinearColor::Red, FLinearColor::Green);

	//Suspension spring force
	if(bHit)
	{
		//tính độ co và giãn của lò xo bánh xe (Bao gồm cả hướng)
		FVector Offset = (1 - UKismetMathLibrary::NormalizeToRange(HitResult.Distance, 0.0f, 90.0f)) *
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

void ACar::SteeringForce(USceneComponent* Staff, USceneComponent* Wheel, float TiresGrip)
{
	FVector const StartLocation = Wheel->GetComponentLocation();
	FVector const EndLocation = Wheel->GetComponentLocation() + FVector{0, 0, -90};
	FHitResult HitResult;
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartLocation,EndLocation,static_cast<ETraceTypeQuery>(ECollisionChannel::ECC_Pawn),false,ActorsToIgnore,EDrawDebugTrace::ForOneFrame,HitResult,true,FLinearColor::Red, FLinearColor::Green);

	//Steering force
	if(bHit)
	{
		FVector SteeringDir = Staff->GetRightVector(); //hướng lực lái
		FVector TireWorldVel = Box->GetPhysicsLinearVelocityAtPoint(Wheel->GetComponentLocation()); //tính vận tốc bánh xe
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("TireWorldVel %s"), *TireWorldVel.ToString()));
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
	FVector const EndLocation = Wheel->GetComponentLocation() + FVector{0, 0, -90};
	FHitResult HitResult;
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartLocation,EndLocation,static_cast<ETraceTypeQuery>(ECollisionChannel::ECC_Pawn),false,ActorsToIgnore,EDrawDebugTrace::ForOneFrame,HitResult,true,FLinearColor::Red, FLinearColor::Green);

	//Acceleration force
	if(bHit)
	{
		FVector AccelDir = Wheel->GetForwardVector(); //hướng gia tốc theo Forward bánh xe
		float CarSpeed = FVector::DotProduct(Box->GetForwardVector(), Box->GetComponentVelocity()); //tính tốc độ hiện tại của xe theo hướng chuyển động
		float NormalizedSpeed = FMath::Clamp(FMath::Abs(CarSpeed)/ CarMaxSpeedCurve, 0.1f, 1.0f);
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("NormalizedSpeed: %.2f"), NormalizedSpeed))
		if (PowerCurve)
		{
			ACarController* CarController = Cast<ACarController>(GetController());
			if (CarController)
			{
				float AvailableTorque = PowerCurve->GetFloatValue(NormalizedSpeed) * CarController->AccelInput;
				FVector ForceMoveCar = AccelDir * AvailableTorque * CarSpeedChange; //Thay đổi giá trị CarSpeedChange để xe đi nhanh hoặc chậm 
				Box->AddForceAtLocation(ForceMoveCar, Wheel->GetComponentLocation());
				// UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("TotalForce %s"), *ForceMoveCar.ToString()));
			}
		}
	}
}

void ACar::Friction(USceneComponent* Wheel)
{
	FVector const StartLocation = Wheel->GetComponentLocation();
	FVector const EndLocation = Wheel->GetComponentLocation() + FVector{0, 0, -90};
	FHitResult HitResult;
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartLocation,EndLocation,static_cast<ETraceTypeQuery>(ECollisionChannel::ECC_Pawn),false,ActorsToIgnore,EDrawDebugTrace::ForOneFrame,HitResult,true,FLinearColor::Red, FLinearColor::Green);

	if(bHit)
	{
		//Box->GetForwardVector() FVector::ForwardVector
		//tính lực pháp tuyến (N=mg)
		float NormalForce = CarMass * Gravity;

		//Lấy vận tốc hiện tại của xe
		FVector CurrentSpeedCar = Box->GetComponentVelocity();

		//nếu xe đứng yên, áp dụng ma sát tĩnh
		if (CurrentSpeedCar.Size() < 2.0f)
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

	// Suspension(FL_Staff_Wheel, FL_Wheel);
	// Suspension(FR_Staff_Wheel, FR_Wheel);
	// Suspension(BL_Staff_Wheel, BL_Wheel);
	// Suspension(BR_Staff_Wheel, BR_Wheel);

	SteeringForce(FL_Staff_Wheel, FL_Wheel, FrontTiresGrip);
	SteeringForce(FR_Staff_Wheel, FR_Wheel, FrontTiresGrip);
	SteeringForce(BL_Staff_Wheel, BL_Wheel, RearTiresGrip);
	SteeringForce(BR_Staff_Wheel, BR_Wheel, RearTiresGrip);

	Acceleration(FL_Wheel);
	Acceleration(FR_Wheel);
	Acceleration(BL_Wheel);
	Acceleration(BR_Wheel);

	Friction(FL_Wheel);
	Friction(FR_Wheel);
	Friction(BL_Wheel);
	Friction(BR_Wheel);

	// DrawDebugLine(GetWorld(), FL_Wheel->GetComponentLocation(),
	// 	FL_Wheel->GetComponentLocation() + FL_Wheel->GetForwardVector() *100.f, FColor::Blue, false, 0.03f,100.f, 5.f);
	// DrawDebugLine(GetWorld(), FR_Wheel->GetComponentLocation(),
	// 	FR_Wheel->GetComponentLocation() + FR_Wheel->GetForwardVector() *100.f, FColor::Blue, false, 0.03f, 100.f, 5.f);
	// DrawDebugLine(GetWorld(), FL_Wheel->GetComponentLocation(),
	// 	FL_Wheel->GetComponentLocation() + FVector::UpVector * SuspensionForce.Length(), FColor::Yellow, false, 0.02f, 100.f, 5.f);
	// DrawDebugLine(GetWorld(), FR_Wheel->GetComponentLocation(),
	// 	FR_Wheel->GetComponentLocation() + FVector::UpVector * SuspensionForce.Length(), FColor::Yellow, false, 0.02f, 100.f, 5.f);

	TraceCapsule(FL_Staff_Wheel,FL_Wheel);
	TraceCapsule(FR_Staff_Wheel,FR_Wheel);
	TraceCapsule(BL_Staff_Wheel,BL_Wheel);
	TraceCapsule(BR_Staff_Wheel,BR_Wheel);
}
void ACar::TraceCapsule(USceneComponent* StaffWheel, USceneComponent* Wheel)
{
	
	bool bHit = false;
	FHitResult HitResult;
	FHitResult MinHitResult;
	
	float MinDistance = 99999999;
	float Radius = 70.f;
	constexpr int InitRepeat = 9;
	float Difference = 140.0f;
	while (Difference > CastLimit)
	{
		FVector MaxCapsule, MinCapsule;
		float cosA = (Difference*Difference - 2.f * Radius * Radius) / (- 2.f * Radius * Radius);
		float AngleCenter = FMath::Acos(cosA);
		float AngleRight = (PI - AngleCenter) / 2.f;
		
		int iMin = -1;
		for (int i = 0; i < InitRepeat; i++)
		{
			float Radians = (AngleCenter/InitRepeat * i);
			float z = FMath::Sin(Radians + AngleRight) * 70.f;
			float x = FMath::Cos(Radians + AngleRight) * 70.f;
			FVector VectorTransform = Wheel->GetComponentTransform().TransformPosition({x,z,0.f});
			bool thisHit = UTraceUtils::CapsuleTraceSingle(GetWorld(), Wheel->GetComponentLocation(),
			VectorTransform, 10.f, 30.f,
			Wheel->GetComponentRotation(),
			TraceTypeQuery1, false,
			ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true,
			FLinearColor::Green, FLinearColor::Green, GetWorld()->GetDeltaSeconds()*1.2f);
			if (thisHit && HitResult.Distance < MinDistance)
			{
				MinHitResult = HitResult;
				MinDistance = HitResult.Distance;
				iMin = i;
			}
			bHit = bHit || thisHit;
		}
		

		//check the MinHitResult whether

		//binary search
		while ()
		{
			
		}
		
		Difference = (MaxCapsule - MinCapsule).Size();
	}
	/*	
	// for (int i = 0; i < InitRepeat; i++)
	// {
	// 	float Radians = (360.0f/InitRepeat * i) * PI / 180.f;
	// 	//float Radians = PI / 4; 
	// 	float z = sin(Radians) * 70.f;
	// 	float x = cos(Radians) * 70.f;
	// 	FVector VectorTransform = Wheel->GetComponentTransform().TransformPosition({x,z,0.f});
	// 	bool thisHit = UTraceUtils::CapsuleTraceSingle(GetWorld(), Wheel->GetComponentLocation(),
	// 	VectorTransform, 10.f, 30.f,
	// 	Wheel->GetComponentRotation(),
	// 	TraceTypeQuery1, false,
	// 	ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true,
	// 	FLinearColor::Green, FLinearColor::Green, GetWorld()->GetDeltaSeconds()*1.2f);
	// 	if (thisHit && HitResult.Distance < MinDistance)
	// 	{
	// 		MinHitResult = HitResult;
	// 		MinDistance = HitResult.Distance;
	// 	}
	// 	bHit = bHit || thisHit;
	// }
	*/
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit %d, Min %f"), bHit, MinDistance));
	if(bHit)
	{
		//tính độ co và giãn của lò xo bánh xe (Bao gồm cả hướng)
		FVector SpringDirection = StaffWheel->GetUpVector();
		
		FVector Offset = (1 - UKismetMathLibrary::NormalizeToRange(MinHitResult.Distance, 0.0f, 90.0f)) * SpringDirection;
		
		//Tính vận tốc theo phương của lò xo
		FVector WheelVelocity = Box->GetPhysicsLinearVelocityAtPoint(Wheel->GetComponentLocation()); // tính vận tốc tại vị trí bánh xe
		float VelocityAlongSpring = FVector::DotProduct(SpringDirection, WheelVelocity); //vận tốc theo hướng lò xo

		//Tính lực giảm chấn
		FVector DampingForce = VelocityAlongSpring * DampingCoefficient * SpringDirection;

		//Tổng lực tác dụng lên bánh xe
		FVector TotalForce = (Offset * Strength) - DampingForce;
		SuspensionForce = TotalForce;
		Box->AddForceAtLocation(TotalForce, StaffWheel->GetComponentLocation());

		float DebugTime = GetWorld()->GetDeltaSeconds()*2;
		DrawDebugDirectionalArrow(GetWorld(),StaffWheel->GetComponentLocation(), StaffWheel->GetComponentLocation() + SpringDirection * 100.f, 10.f, FColor::Magenta, false, DebugTime, 0.f, 5.f);
		DrawDebugDirectionalArrow(GetWorld(),StaffWheel->GetComponentLocation(), StaffWheel->GetComponentLocation() + Offset.GetSafeNormal() * 100.f, 10.f, FColor::Blue, false, DebugTime, 0.f, 5.f);
		DrawDebugDirectionalArrow(GetWorld(),StaffWheel->GetComponentLocation(), StaffWheel->GetComponentLocation() + DampingForce.GetSafeNormal() * 100.f, 10.f, FColor::Red, false, DebugTime, 0.f, 5.f);
		
		
	}
	else
	{
		SuspensionForce = FVector::ZeroVector;
		Box->AddForceAtLocation(FVector::ZeroVector, StaffWheel->GetComponentLocation());
	}
}
// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
