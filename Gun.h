// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

UENUM(BlueprintType, Category = "WeaponType")
enum EWeaponType
{
	EBT_Shotgun UMETA(DisplayName = "Shotgun"),
	EBT_AsaultRifle  UMETA(DisplayName = "Asault Rifle"),
	EBT_Complete UMETA(DisplayName = "Complete")
};

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDamageIndicator, float, Damage, FVector, Location);


class ACharacterBaseClass;
class AController;
class APlayerCharacter;
UCLASS()
class BIGHEADGUYGAME_API AGun : public AActor
{
	GENERATED_BODY()
	
public:	
	AGun();
	//Functions
	void RequestShooting();
	void RequestReloading();
	void ReleaseTrigger();
	void AimDownSight();
	void ReleaseAim();
	void OwnerIsCrouching();
	void OwnerIsUNCrouching();
	void ChangeOwner();
	void ActivebCanUseWeaponTimer();
	void SetTRUEbCanUseWeapon();
	void SetFALSEbCanUseWeapon();


	//Get functions
	UFUNCTION(BlueprintPure)
	int32 GetGunAccuracyValue()const;
	UFUNCTION(BlueprintPure)
	int32 GetCurrentAmmo()const;
	UFUNCTION(BlueprintPure)
	float GetReloadingTimer();
	UFUNCTION(BlueprintPure)
	EWeaponType GetWeaponType();
	UFUNCTION(BlueprintPure)
	FVector GetWeaponGripLocation();
	UFUNCTION(BlueprintPure)
	FRotator GetWeaponGripRotation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EWeaponType> WeaponType;//= EBT_AsaultRifle;
     
	//UPROPERTY(BlueprintAssignable, Category = "DamageIndicator")
	//	FDamageIndicator IndicateDamageDelegate;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

//	Visuals
	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent* GunMesh;
	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere)
		USoundBase* MuzzleSound;
	UPROPERTY(EditAnywhere)
		USoundBase* ImpactSound;
	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactEffect;
//Gun_Properties

	  //floats
	UPROPERTY(EditAnywhere)
		float MaxRange = 1000;
	UPROPERTY(EditAnywhere)
		float NoDamageFalloffRange = 70;
	UPROPERTY(EditAnywhere)
		float DamageFalloffRate=1;


	UPROPERTY(EditAnywhere)
		float DefaultDamage = 20;
	
	UPROPERTY(EditAnywhere)
		float DefaultAmmo = 30;
    UPROPERTY(EditAnywhere)
		float MovementAccuracyValueMultiplier=10;
	UPROPERTY(EditAnywhere)
		float CrosshairshrinkDelayAfterShooting=0.1f;
	UPROPERTY(EditAnywhere)
		float ShotingCrosshairGrowingSpeed=300;
	UPROPERTY(EditAnywhere)
	    float ShotingCrosshairShrinkingSpeed=1; 
	UPROPERTY(EditAnywhere)
		float AimingCrosshairGrowingSpeed = 300;
    UPROPERTY(EditAnywhere)
		float AimingCrosshairShrinkingSpeed = 1;
    UPROPERTY(EditAnywhere)
	   float ShootingDelay = 1;
    UPROPERTY(EditAnywhere)
	   float ReloadTime = 3;
	UPROPERTY(EditAnywhere)
		float PullOutTime = 0.3f;
	UPROPERTY(EditAnywhere)
		float FirstShotAccuracyBeginTime = 0.3f;


	UPROPERTY(EditAnywhere)
		int32 ShotgunShootingBulletCount;
    UPROPERTY(EditAnywhere)
		int32 DefaultAccuracyValue=50;
	UPROPERTY(EditAnywhere)
		int32 DefaultShootingAccuracyValueIncrease=15;
	UPROPERTY(EditAnywhere)
		int32 DefaultShootingWhileCrouchAccuracyValueIncrease = 10;

	UPROPERTY(EditAnywhere)
		int32 DefaultAimDownsightAccuracyValueDecrease = 35;
	UPROPERTY(EditAnywhere)
		int32 DefaultCrouchingAccuracyValueDecrease = 15;
	UPROPERTY(EditAnywhere)
		int32 DefaultAmmoQuantity = 30;
	UPROPERTY(EditAnywhere)
		int32 MinAccuracyValue = 3;

	int32 AccuracyValue;
	int32 ShootingAccuracyValueIncrease=0;
	int32 AimDownsightAccuracyValueDecrease = 0;
	int32 CrouchingAccuracyValueDecrease=0;
	int32 CurrentAmmo;

	float Damage;
	


	//bools
	//Properties
	UPROPERTY(EditAnywhere)
		bool AutomaticFire = false;
	UPROPERTY(EditAnywhere)
	    bool StandingFirstShotAccuracy = false;
	UPROPERTY(EditAnywhere)
		bool CrouchingStandingFirstShotAccuracy = false;


	//CodeBools
		bool bShootingAllowed = true;
		bool bShootingAccuracyInterp;
		bool bIsAimingDownsight=false;
		bool bOwnerIsCrouching=false;
		bool bUseCrosshairShrinkTimer=true;
		bool bCanUseWeapon = false;
		bool bFirstShotAccuracy = false;
		bool bCanUseFirstShotAccuracyTimer = true;
    //Time
		FTimerHandle ShootDelayHandle;
		FTimerHandle CrosshairRecoverAfterShootingHandle;
		FTimerHandle ReloadingHandle;
		FTimerHandle PullOutWeaponHandle;
		FTimerHandle FirstShotAccuracyHandle;

	//Other
	APlayerCharacter* GunPlayerOwner;
	ACharacterBaseClass* GunOwner;
	AController* OwnerController;
	FVector WeaponGripLocation;
	FRotator WeaponGripRotation;


//Action Functions
  void Shoot();
  void SpreadShoot();

  void Reload();

  void CalculateAccuracy();
  void CrosshairGrowth(float DeltaTime, int32& AccuracyLenght, int32  GoalAccuracyLenght, float Speed);
  void CrosshairShrink(float DeltaTime, int32& AccuracyLenght, int32 GoalAccuracyLenght, float Speed);
  void SetFALSEbShootingAccuracyInterp();
  void SetTRUEbFirstShotAccuracy();



 

  bool POVGunTrace(FHitResult& Hit, FVector& ShotDirection);
  bool GunTrace(FHitResult& Hit, FVector& ShotDirection);


 //Get Functions
  


  //Validation functions
  void AllowShooting();


};
