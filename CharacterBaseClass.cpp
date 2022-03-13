// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBaseClass.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"



// Sets default values
ACharacterBaseClass::ACharacterBaseClass()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CharacterSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CharacterSpringArm"));
	CharacterCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CharacterCamera"));

	CharacterSpringArm->SetupAttachment(GetMesh());

	//CharacterSpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, 
	//	                                                                       EAttachmentRule::KeepWorld, 
	//	                                                                       EAttachmentRule::SnapToTarget,true));
	CharacterCamera->SetupAttachment(CharacterSpringArm, USpringArmComponent::SocketName);
	
	Health = 70;
	
}

// Called when the game starts or when spawned
void ACharacterBaseClass::BeginPlay()
{
	Super::BeginPlay();
	DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	PlayerController = Cast<APlayerController>(GetController());
	DefaultSocketOffset = CharacterSpringArm->GetComponentLocation();
	UpperBodyRotation = CharacterSpringArm->GetComponentRotation();
    //bUseControllerRotationYaw = false;
}




void ACharacterBaseClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateDistanceToGround();
	if(InterpolateCrouchCameraMovement)
	CrouchCameraLocationInterpolate( DeltaTime);

	

	if (GetCharacterMovement()->IsFalling() == false && PreviousbIsFallingState == true)
	{
		//bIsLanding = true;

	
		LandingEvent.Broadcast(PreviousDirectionalVelocity.Z);

		//UE_LOG(LogTemp, Error, TEXT("%f"), PreviousDirectionalVelocity.Z);
		
	}

  /*
	if (bIsSprinting)
	{

		FRotator CapsuleRotation= GetCapsuleComponent()->GetComponentRotation();
		
		GetCapsuleComponent()->SetWorldRotation( FRotator(CapsuleRotation.Pitch, LowerBodyRotation.Yaw,CapsuleRotation.Pitch) );
		
		
		

	 

	}
	
*/

    CalculateLowerBodyRotation(DeltaTime);
	CalculateDirectionalAcceleration();

	PreviousbIsFallingState = GetCharacterMovement()->IsFalling();

}

float ACharacterBaseClass::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApply = FMath::Min(Health, DamageToApply);
	Health = Health - DamageToApply;
	
	if (IsDead())
	{
		DetachFromControllerPendingDestroy();
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		

	}
	
	


	return DamageToApply;
}


//Input Functions

void ACharacterBaseClass::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Movement-Forward_Axis"), this, &ACharacterBaseClass::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Movement-Right_Axis"), this, &ACharacterBaseClass::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACharacterBaseClass::PitchCamera);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &ACharacterBaseClass::YawCamera);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacterBaseClass::CharacterJump);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Released, this, &ACharacterBaseClass::CharacterJumpReleased);
	PlayerInputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Pressed, this, &ACharacterBaseClass::CharacterCrouch);
	PlayerInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Pressed, this, &ACharacterBaseClass::CharacterSprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Released, this, &ACharacterBaseClass::CharacterSprintReleased);
}


void ACharacterBaseClass::MoveForward(float AxisValue)
{
	AddMovementInput(GetActorForwardVector() * AxisValue);
	if (AxisValue != 0)bReceivesMovementInput = true;
	else if (AxisValue==0 && !GetWorldTimerManager().IsTimerActive(MovementInputRemoveTimeHandle))
	GetWorldTimerManager().SetTimer(MovementInputRemoveTimeHandle, this,&ACharacterBaseClass::SetbReceivesMovementInputFALSE, 0.1f);
	/*
	if (bIsSprinting)
	{

		FRotator CapsuleRotation = GetCapsuleComponent()->GetComponentRotation();

		float Angle=0;
		if (AxisValue >= 0)
		{
			Angle = 0;
		}
		else
		{
			Angle = 180;
			//AxisValue = 1;
		}
		GetCapsuleComponent()->SetWorldRotation(FRotator(CapsuleRotation.Pitch, LowerBodyRotation.Yaw+Angle, CapsuleRotation.Pitch));


	}
*/
	


}

void ACharacterBaseClass::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector() * AxisValue);
	if (AxisValue != 0)bReceivesMovementInput = true;
	else if (AxisValue == 0 && !GetWorldTimerManager().IsTimerActive(MovementInputRemoveTimeHandle))
	GetWorldTimerManager().SetTimer(MovementInputRemoveTimeHandle, this, &ACharacterBaseClass::SetbReceivesMovementInputFALSE, 0.1f);
	/*
	if (bIsSprinting)
	{

		FRotator CapsuleRotation = GetCapsuleComponent()->GetComponentRotation();

		float Angle = 0;
		if (AxisValue >= 0)
		{
			Angle = -90;
		}
		else
		{
			Angle = 90;
			//AxisValue = 1;
		}
		GetCapsuleComponent()->SetWorldRotation(FRotator(CapsuleRotation.Pitch, LowerBodyRotation.Yaw + Angle, CapsuleRotation.Pitch));
       

	}
*/
     

}

void ACharacterBaseClass::PitchCamera(float AxisValue)
{
	CameraInput.Y = AxisValue;

	UpperBodyRotation.Pitch = FMath::Clamp(UpperBodyRotation.Pitch + CameraInput.Y, -80.0f, 80.0f);
	UpperBodyRotation.Pitch = UpperBodyRotation.Pitch + CameraInput.Y;
	CharacterSpringArm->SetWorldRotation(UpperBodyRotation);

	//AddControllerPitchInput(AxisValue);
}

void ACharacterBaseClass::YawCamera(float AxisValue)
{
	CameraInput.X = AxisValue;
	
	
	UpperBodyRotation = CharacterSpringArm->GetComponentRotation();
	
	UpperBodyRotation.Yaw = UpperBodyRotation.Yaw + CameraInput.X;
	
		if (bReceivesMovementInput && bAimDownSightIsPressed)
		{
		UpperBodyRotation.Yaw =FMath::ClampAngle
		(UpperBodyRotation.Yaw, LowerBodyRotation.Yaw - ViewLeftLimit, LowerBodyRotation.Yaw + ViewRightLimit);
		
		}

		CharacterSpringArm->SetWorldRotation(UpperBodyRotation);
        UpperBodyRotationRelToBody = UpperBodyRotation - GetActorRotation();
		if (UpperBodyRotationRelToBody.Yaw > 180 || UpperBodyRotationRelToBody.Yaw < -180)
		{
			if(UpperBodyRotation.Yaw<0)
			UpperBodyRotationRelToBody.Yaw = 360 + UpperBodyRotation.Yaw - GetActorRotation().Yaw;
			else if(GetActorRotation().Yaw<0)
			UpperBodyRotationRelToBody.Yaw = -360 + UpperBodyRotation.Yaw - GetActorRotation().Yaw;
		}
		if(!bBodyFollowView &&(UpperBodyRotationRelToBody.Yaw> ViewRightLimit || UpperBodyRotationRelToBody.Yaw<-ViewLeftLimit))bBodyFollowView = true;

		
	//	UE_LOG(LogTemp, Error, TEXT("%s"), *UpperBodyRotationRelToBody.ToString());
	
	
		
	//	AddControllerYawInput(AxisValue);
	

	//UE_LOG(LogTemp, Warning, TEXT("%s"), *UpperBodyRotation.ToString());
//	UE_LOG(LogTemp, Error, TEXT("%s"), *( GetActorRotation()).ToString());
	

}
void ACharacterBaseClass::CharacterJump()
{
    if (GetCharacterMovement()->IsCrouching())CharacterUncrouchAction();
	else 
	{
		bHasJumped = true;
		Jump();
	}
	
}
void ACharacterBaseClass::CharacterJumpReleased()
{
	bHasJumped = false;
	
}
void ACharacterBaseClass::CharacterCrouch()
{

	if (!GetCharacterMovement()->IsCrouching() && !GetCharacterMovement()->IsFalling()) 
	{
		CharacterCrouchAction();
		
	}
	else
	{
		CharacterUncrouchAction();
		
	}
	
}
void ACharacterBaseClass::CharacterSprint()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		bIsSprinting = true;
		SetRelaxedModeOn();
	}

}
void ACharacterBaseClass::CharacterSprintReleased()
{
	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
		bIsSprinting = false;
	}
}
void ACharacterBaseClass::CharacterCrouchAction()
{
  Crouch();
  InterpolateCrouchCameraMovement = true;
}
void ACharacterBaseClass::CharacterUncrouchAction()
{
	
  UnCrouch();
  InterpolateCrouchCameraMovement = true;
}

void ACharacterBaseClass::CalculateDirectionalAcceleration()
{
	DirectionalVelocity = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), GetVelocity());

	DirectionalAcceleration = ((DirectionalVelocity - PreviousDirectionalVelocity) / 100) / GetWorld()->GetDeltaSeconds();

	// = (DirectionalAcceleration / ( (GetCharacterMovement()->MaxAcceleration + 200) / 100)  );
	if (DirectionalVelocity.X > 0)
	{
		if (DirectionalAcceleration.X > 0)
			DirectionalAcceleration.X = (DirectionalAcceleration.X / ((GetCharacterMovement()->MaxAcceleration + 200) / 100)) / 2;
		else if (DirectionalAcceleration.X < 0)
			DirectionalAcceleration.X = (DirectionalAcceleration.X / ((GetCharacterMovement()->BrakingDecelerationWalking + 400) / 100)) / 5;
	}
	else
	{
		if (DirectionalAcceleration.X > 0)
			DirectionalAcceleration.X = (DirectionalAcceleration.X / ((GetCharacterMovement()->MaxAcceleration + 200) / 100)) / 5;
		else if (DirectionalAcceleration.X < 0)
			DirectionalAcceleration.X = (DirectionalAcceleration.X / ((GetCharacterMovement()->BrakingDecelerationWalking + 400) / 100)) / 2;
	}

	if (DirectionalVelocity.Y > 0)
	{
		if (DirectionalAcceleration.Y > 0)
			DirectionalAcceleration.Y = (DirectionalAcceleration.Y / ((GetCharacterMovement()->MaxAcceleration + 200) / 100)) / 3;
		else if (DirectionalAcceleration.Y < 0)
			DirectionalAcceleration.Y = (DirectionalAcceleration.Y / ((GetCharacterMovement()->BrakingDecelerationWalking + 400) / 100)) / 6;
	}
	else
	{
		if (DirectionalAcceleration.Y > 0)
			DirectionalAcceleration.Y = (DirectionalAcceleration.Y / ((GetCharacterMovement()->MaxAcceleration + 200) / 100)) / 5;
		else if (DirectionalAcceleration.Y < 0)
			DirectionalAcceleration.Y = (DirectionalAcceleration.Y / ((GetCharacterMovement()->BrakingDecelerationWalking + 400) / 100)) / 2;
	}


	//	UE_LOG(LogTemp, Warning, TEXT("%s"), *DirectionalVelocity.ToString());



	//if (DirectionalAcceleration.Y != 0 && DirectionalAcceleration.X != 0)
	//	UE_LOG(LogTemp, Warning, TEXT("%s"), *DirectionalAcceleration.ToString());

	PreviousDirectionalVelocity = DirectionalVelocity;

}
void ACharacterBaseClass::CalculateDistanceToGround()
{
	FHitResult Ground_Distance_Hit;
	FVector Start =GetCapsuleComponent()->GetComponentLocation()  - FVector(0, 0, 1) * GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-2.149994;
	FVector Ground_Distance_ShotDirection;
	
	//FVector Start = this->GetActorLocation() - FVector(0, 0, 1) * GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector End = Start - FVector(0, 0, 1) * DistanceToGroundDetectMaxRange;

	DistanceToTrace(Ground_Distance_Hit, Ground_Distance_ShotDirection,Start,End);

	
	DistanceToGround = sqrt( (pow(Ground_Distance_Hit.Location.X - Start.X, 2),
	                    	  pow(Ground_Distance_Hit.Location.Y - Start.Y, 2),
		                      pow(Ground_Distance_Hit.Location.Z - Start.Z, 2) ) )*(-1);
	//	UE_LOG(LogTemp, Error, TEXT("%f"), DistanceToGround);

}
void ACharacterBaseClass::CrouchCameraLocationInterpolate(float DeltaTime)
{
	if (GetCharacterMovement()->IsCrouching())
	{
		CharacterSpringArm->SocketOffset.Z =
			FMath::FInterpConstantTo(CharacterSpringArm->SocketOffset.Z, DefaultSocketOffset.Z - CameraDescendWhileCrouch, DeltaTime, 500.f);

		if (CharacterSpringArm->SocketOffset.Z ==( DefaultSocketOffset.Z - CameraDescendWhileCrouch))
			InterpolateCrouchCameraMovement = false;

	}
	else if (!GetCharacterMovement()->IsCrouching())
	{
		CharacterSpringArm->SocketOffset.Z =
			FMath::FInterpConstantTo(CharacterSpringArm->SocketOffset.Z, DefaultSocketOffset.Z, DeltaTime, 500.f);

		if (CharacterSpringArm->SocketOffset.Z == DefaultSocketOffset.Z )
			InterpolateCrouchCameraMovement = false;

	}
	

}
void ACharacterBaseClass::CalculateLowerBodyRotation(float DeltaTime)
{
	if ((bBodyFollowView && !(bReceivesMovementInput && bAimDownSightIsPressed)) ||
		(bReceivesMovementInput && !bAimDownSightIsPressed) ||
		(!bReceivesMovementInput && bAimDownSightIsPressed))

	{
		float LowerBodyYaw;


		LowerBodyYaw = FMath::FInterpTo(GetActorRotation().Yaw, GetActorRotation().Yaw + UpperBodyRotationRelToBody.Yaw, DeltaTime, 4.f);
		LowerBodyRotation = FRotator(GetActorRotation().Pitch, LowerBodyYaw, GetActorRotation().Roll);
		PlayerController->SetControlRotation(LowerBodyRotation);


		if (int(UpperBodyRotationRelToBody.Yaw) == 0)
		{
			bBodyFollowView = false;

		}

	}
}

void ACharacterBaseClass::IKFootTrace(FName SocketName, float Distance, FVector& OutHitLocation, FRotator& OutSurfaceRotation, float& OutFootTraceOffset )
{
	FVector SocketLocation= GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart = FVector(SocketLocation.X, SocketLocation.Y,GetActorLocation().Z );
	FVector TraceEnd = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z -Distance);
	FHitResult TraceHitData;
	FVector TraceDirection;

	if (DistanceToTrace(TraceHitData, TraceDirection, TraceStart, TraceEnd))
	{
		
		OutHitLocation= TraceHitData.Location;
		OutSurfaceRotation = FRotator(FMath::RadiansToDegrees(atan2(TraceHitData.ImpactNormal.Y, TraceHitData.ImpactNormal.Z))   ,
			                          FMath::RadiansToDegrees(atan2(TraceHitData.ImpactNormal.X, TraceHitData.ImpactNormal.Z))*-1,  0 );
		


		OutFootTraceOffset = (TraceHitData.Location -GetMesh()->GetComponentLocation()  ).Z;     ///Distance ;
	

		UE_LOG(LogTemp, Warning, TEXT("%s %f"), *SocketName.ToString(), OutFootTraceOffset);

		DrawDebugPoint(GetWorld(), OutHitLocation, 10, FColor::Green, true, 1, 1);
	}
	else
	{
		OutHitLocation = FVector(0,0,0);
		OutSurfaceRotation = FRotator(0, 0, 0);
		OutFootTraceOffset = 0;

	}
	
}
bool ACharacterBaseClass::DistanceToTrace(FHitResult& Hit, FVector& ShotDirection, FVector Start, FVector End)
{
	FHitResult HitOut;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_GameTraceChannel2, CollisionParams);

}

void ACharacterBaseClass::SetbReceivesMovementInputFALSE()
{
	bReceivesMovementInput = false;
}
void ACharacterBaseClass::SetSteadyModeOn()
{
	PlayerCombatMode = EBT_Steady;
}
void ACharacterBaseClass::SetRelaxedModeOn()
{
	PlayerCombatMode = EBT_Relaxed;
}

//Verifing Functions
bool ACharacterBaseClass::IsDead() const
{
	if (Health <= 0)
	{
	//	if (Gun)
	//		Gun->ReleaseTrigger();
		return true;
	}
	else return false;
}

//Get Functions
float ACharacterBaseClass::GetHealthPrecent()const
{
	return Health/DefaultHealth;
}

bool ACharacterBaseClass::GetbHasJumped() const
{
	return bHasJumped;
}

float ACharacterBaseClass::GetDistanceToGround() const
{
	return DistanceToGround;
}

FRotator ACharacterBaseClass::GetUpperBodyRotationRelToBody()
{
	return UpperBodyRotationRelToBody;
}

FVector ACharacterBaseClass::GetDirectionalAcceleration()
{
	return DirectionalAcceleration;
}

bool ACharacterBaseClass::GetbReceivesMovementInput()
{
	return bReceivesMovementInput;
}

EPlayerCombatModes ACharacterBaseClass::GetCombatMode()
{
	return  PlayerCombatMode;
}

float ACharacterBaseClass::GetSprintSpeed()
{
	return SprintSpeed;
}

float ACharacterBaseClass::GetDefaultMaxWalkSpeed()
{
	return DefaultMaxWalkSpeed;
}

bool ACharacterBaseClass::GetbIsSprinting()
{
	return bIsSprinting;
}




