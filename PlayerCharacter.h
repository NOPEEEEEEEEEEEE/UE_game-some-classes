// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBaseClass.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UENUM(BlueprintType, Category = "PlayerEquipmentState")
enum EPlayerEquipmentState
{
    EBT_NoWeapon  UMETA(DisplayName = "No Weapon"),
	EBT_Rifle UMETA(DisplayName = "Rifle")

};

class AGun;
UCLASS()
class BIGHEADGUYGAME_API APlayerCharacter : public ACharacterBaseClass
{
	GENERATED_BODY()
public:
	APlayerCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	
	//Get functions
	UFUNCTION(BlueprintPure)
	AGun* GetGun()const;
	UFUNCTION(BlueprintPure)
		EPlayerEquipmentState GetPlayerEquipmentState();
	UPROPERTY(BlueprintReadWrite)
		TEnumAsByte<EPlayerEquipmentState> PlayerEquipmentState = EBT_NoWeapon;

	float GetWeaponMovementAccuracyValue();

	  //bools
	UFUNCTION(BlueprintPure)
	    bool GetHasShot();
	

	  //floats
	 

protected:
	
	virtual void BeginPlay() override;

	virtual void CharacterCrouchAction()override;
	virtual void CharacterUncrouchAction()override;
	//virtual void MoveForward(float AxisValue)override;
//	virtual void MoveRight(float AxisValue)override;
   


private:
	//Properties

	int32 WeaponMovementAccuracyValue;

	

	//Functions 


	
	//Weapons
   UPROPERTY(EditDefaultsOnly)
   TSubclassOf<AGun> GunClass1;
   UPROPERTY(EditDefaultsOnly)
   TSubclassOf<AGun> GunClass2;


    UPROPERTY()
    AGun* GunInHand=nullptr;
	UPROPERTY()
	AGun* PrimaryGun = nullptr;
	UPROPERTY()
	AGun* SecondaryGun = nullptr;
	///////Input Functions
	void Shoot();
	void ReleaseGunTrigger();
	void AimDownsight();
	void ReleaseAimDownsight();
	void Reload();
	void ChangeToPrimaryGun();
	void ChangeToSecondaryGun();
	void NoWeapon();
	void AutoRun();


	//Camera

    UPROPERTY(EditAnywhere)
		float AimingTargetArmLength;
	UPROPERTY(EditAnywhere)
		FVector AimingSocketOffset;

	



	//Bools
		
    bool bHasShot = false;

   
	
	bool bTargetDistanceReached = true;
	bool bAutoRun = false;
	

};
