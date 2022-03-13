// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBaseClass.generated.h"

UENUM(BlueprintType, Category = "PlayerCombatModes")
enum EPlayerCombatModes
{
	EBT_Relaxed  UMETA(DisplayName = "Relaxed"),
	EBT_Steady UMETA(DisplayName = "Steady"),
	EBT_Aiming UMETA(DisplayName = "Aiming")

};

class USpringArmComponent;
class UCameraComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementEvent, float, LandingDownVelocity);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAnimationEvents);

UCLASS()
class BIGHEADGUYGAME_API ACharacterBaseClass : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBaseClass();
	UPROPERTY(BlueprintReadWrite)
		TEnumAsByte<EPlayerCombatModes> PlayerCombatMode = EBT_Relaxed;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere)
		USpringArmComponent* CharacterSpringArm;
	UPROPERTY(EditAnywhere)
  	    UCameraComponent* CharacterCamera;
	

    FVector2D CameraInput;
	
	FVector DefaultSocketOffset;
    FVector DirectionalAcceleration;
	

	bool bIsSprinting=false;
    bool bHasJumped = false;
    bool bBodyFollowView = false;
	bool bReceivesMovementInput=false;
	bool bAimDownSightIsPressed = false;
	//bool bFightModeActive = false;
	
	//floats
       UPROPERTY(EditAnywhere)
	   float SteadyModeOffDelay=4;

       float DefaultTargetArmLength;
       float Health;
	   float DistanceToGround;

    //Setup Input
	    void MoveForward(float AxisValue);
	    void MoveRight(float AxisValue);


	//Action functions
   
	virtual void CharacterCrouchAction();
	virtual void CharacterUncrouchAction();

	void SetSteadyModeOn();
	void SetRelaxedModeOn();
	

	//others
	FTimerHandle SteadyModeOffTimerHandle;
	

public:	

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)override;

	//Get Functions
	UFUNCTION(BlueprintPure)
	float GetHealthPrecent()const;
	UFUNCTION(BlueprintPure)
	bool GetbHasJumped()const;
	UFUNCTION(BlueprintPure)
	float GetDistanceToGround()const;
	UFUNCTION(BlueprintPure)
	FRotator GetUpperBodyRotationRelToBody();
	UFUNCTION(BlueprintPure)
	FVector GetDirectionalAcceleration();
	UFUNCTION(BlueprintPure)
	bool GetbReceivesMovementInput();

	UFUNCTION(BlueprintPure)
	EPlayerCombatModes GetCombatMode();


	UFUNCTION(BlueprintPure)
	float GetSprintSpeed();
	UFUNCTION(BlueprintPure)
	float GetDefaultMaxWalkSpeed();
	UFUNCTION(BlueprintPure)
	bool GetbIsSprinting();

	UFUNCTION(BlueprintCallable)
	void IKFootTrace(FName SocketName, float Distance, FVector& OutHitLocation,FRotator& OutSurfaceRotation,float& OutFootTraceOffset);

	UPROPERTY(BlueprintAssignable, Category = "MovementEvents")
		FMovementEvent LandingEvent ;
	UPROPERTY(BlueprintAssignable, Category = "AnimationEvents")
	FAnimationEvents ChangeWeapon;

private:
	//Setup Input
	
    void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);
	
    void CharacterJump();
	void CharacterJumpReleased();
    void CharacterCrouch();
	void CharacterSprint();
	void CharacterSprintReleased();

	//Camera 
	
	////////////////Properties




	///bools
	
	bool InterpolateCrouchCameraMovement=false;
	bool PreviousbIsFallingState = false;
	
	//bool bReceivesForwardMovementInput = false;
	//bool bReceivesRightMovementInput = false;
	///floats
	UPROPERTY(EditAnywhere)
	  float DefaultHealth=100.f;
    UPROPERTY(EditAnywhere)
	  float DistanceToGroundDetectMaxRange=2000;
	UPROPERTY(EditAnywhere)
		float CameraDescendWhileCrouch = 30;
    UPROPERTY(EditAnywhere)
	    float ViewRightLimit=90;
	UPROPERTY(EditAnywhere)
		float ViewLeftLimit = 90;
	UPROPERTY(EditAnywhere)
		float DefaultMaxWalkSpeed ;
	UPROPERTY(EditAnywhere)
		float SprintSpeed = 1000;

	FVector DirectionalVelocity;
	FVector PreviousDirectionalVelocity=FVector(0,0,0);
	  
	   
	


	 ////others
	  FRotator UpperBodyRotation;
	  FRotator UpperBodyRotationRelToBody;
	  FRotator LowerBodyRotation;
	  APlayerController* PlayerController;

	  FTimerHandle MovementInputRemoveTimeHandle;


    /////////Functions
	  ///Actions
	  bool DistanceToTrace(FHitResult& Hit, FVector& ShotDirection,FVector Start,FVector End);

	  void CalculateDirectionalAcceleration();
	  void CalculateDistanceToGround();
	  void CrouchCameraLocationInterpolate(float DeltaTime);
	  void CalculateLowerBodyRotation(float DeltaTime);

	  void SetbReceivesMovementInputFALSE();
	  


	//Verifing Functions
	bool IsDead()const;

};
