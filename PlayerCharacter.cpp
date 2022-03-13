// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Gun.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;


}
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

    DefaultTargetArmLength = CharacterSpringArm->TargetArmLength;
	DefaultSocketOffset = CharacterSpringArm->SocketOffset;
	
	PrimaryGun= GetWorld()->SpawnActor<AGun>(GunClass1);
	SecondaryGun= GetWorld()->SpawnActor<AGun>(GunClass2);

	if (PrimaryGun)
	{

		PrimaryGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Character_RightHand_WeaponSocket"));
		PrimaryGun->SetOwner(this);
		PrimaryGun->ChangeOwner();
        SecondaryGun->SetActorHiddenInGame(true);
		SecondaryGun->SetFALSEbCanUseWeapon();
	}

	if (SecondaryGun)
	{
		SecondaryGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Character_RightHand_WeaponSocket"));
		SecondaryGun-> SetOwner(this);
		SecondaryGun->ChangeOwner();
		
		SecondaryGun->SetFALSEbCanUseWeapon();
	}
	if (GunInHand)
	{
		GunInHand->SetTRUEbCanUseWeapon();
	}



}
void APlayerCharacter::CharacterCrouchAction()
{
	Super::CharacterCrouchAction();
	if (GunInHand)
	GunInHand->OwnerIsCrouching();
	

}
void APlayerCharacter::CharacterUncrouchAction()
{
	Super::CharacterUncrouchAction();
	if (GunInHand)
	GunInHand->OwnerIsUNCrouching();
	
}
/*
void APlayerCharacter::MoveForward(float AxisValue)
{
	Super::MoveForward(AxisValue);


	if(AxisValue!=0)bAutoRun = false;
	
}

void APlayerCharacter::MoveRight(float AxisValue)
{
	Super::MoveRight(AxisValue);


	if (AxisValue != 0)bAutoRun = false;


}
*/

void APlayerCharacter::AutoRun()
{

	bAutoRun = true;
	
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerCombatMode==EBT_Aiming)
	{
		CharacterSpringArm->TargetArmLength = FMath::FInterpConstantTo(CharacterSpringArm->TargetArmLength, AimingTargetArmLength, DeltaTime, 500.f);
		if (CharacterSpringArm->TargetArmLength == AimingTargetArmLength)   
		{
			bTargetDistanceReached = true;
		}

	}
	if (PlayerCombatMode != EBT_Aiming)
	{
		CharacterSpringArm->TargetArmLength = FMath::FInterpConstantTo(CharacterSpringArm->TargetArmLength, DefaultTargetArmLength, DeltaTime, 500.f);
		
		
		if (CharacterSpringArm->TargetArmLength == DefaultTargetArmLength)  
		{
			bTargetDistanceReached = true;
		}
	}

	float MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	float Velocity;

	Velocity = sqrt(pow(GetVelocity().X, 2) + pow(GetVelocity().Y, 2) + pow(GetVelocity().Z, 2));
	
	WeaponMovementAccuracyValue = int32 (Velocity);
	if (Velocity > MaxWalkSpeed - 2 && Velocity < MaxWalkSpeed)
		WeaponMovementAccuracyValue =int32( MaxWalkSpeed);

	if (GetCharacterMovement()->IsFalling() && bAimDownSightIsPressed)
	{
		PlayerCombatMode == EBT_Relaxed;
		bTargetDistanceReached = false;
		if (GunInHand)
			GunInHand->ReleaseAim();
	}
	else if (!GetCharacterMovement()->IsFalling() && bAimDownSightIsPressed) AimDownsight();

	if (bReceivesMovementInput)bAutoRun = false;
	if (bAutoRun)
	{
		MoveForward(1);
		bReceivesMovementInput = false;
	}


}
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Shoot"), EInputEvent::IE_Pressed, this, &APlayerCharacter::Shoot);
	PlayerInputComponent->BindAction(TEXT("Shoot"), EInputEvent::IE_Released, this, &APlayerCharacter::ReleaseGunTrigger);
	PlayerInputComponent->BindAction(TEXT("AimDowsight"), EInputEvent::IE_Pressed, this, &APlayerCharacter::AimDownsight);
	PlayerInputComponent->BindAction(TEXT("AimDowsight"), EInputEvent::IE_Released, this, &APlayerCharacter::ReleaseAimDownsight);
	PlayerInputComponent->BindAction(TEXT("ReloadWeapon"), EInputEvent::IE_Released, this, &APlayerCharacter::Reload);
	PlayerInputComponent->BindAction(TEXT("ChangeToPrimaryGun"), EInputEvent::IE_Pressed, this, &APlayerCharacter::ChangeToPrimaryGun);
	PlayerInputComponent->BindAction(TEXT("ChangeToSecondaryGun"), EInputEvent::IE_Pressed, this, &APlayerCharacter::ChangeToSecondaryGun);
	PlayerInputComponent->BindAction(TEXT("NoWeapon"), EInputEvent::IE_Pressed, this, &APlayerCharacter::NoWeapon);
	PlayerInputComponent->BindAction(TEXT("Auto-Run"), EInputEvent::IE_Pressed, this, &APlayerCharacter::AutoRun);

}




void APlayerCharacter::Shoot()
{
	if(GunInHand)
	GunInHand->RequestShooting();
	SetSteadyModeOn();
	
	GetWorldTimerManager().SetTimer(SteadyModeOffTimerHandle, this, &APlayerCharacter::SetRelaxedModeOn, SteadyModeOffDelay);
	//bHasShot = true;
}

void APlayerCharacter::ReleaseGunTrigger()
{
	if (GunInHand)
	GunInHand->ReleaseTrigger();
	//bHasShot = false;
	


}

void APlayerCharacter::AimDownsight()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		PlayerCombatMode = EBT_Aiming;
		bTargetDistanceReached = false;
		if (GunInHand)
			GunInHand->AimDownSight();

		GetWorldTimerManager().ClearTimer(SteadyModeOffTimerHandle);
	}
	


	bAimDownSightIsPressed = true;

}

void APlayerCharacter::ReleaseAimDownsight()
{
	SetSteadyModeOn();
	bTargetDistanceReached = false;
	if (GunInHand)
	GunInHand->ReleaseAim();
	bAimDownSightIsPressed = false;
	GetWorldTimerManager().SetTimer(SteadyModeOffTimerHandle, this, &APlayerCharacter::SetRelaxedModeOn, SteadyModeOffDelay);
	
}

void APlayerCharacter::Reload()
{
	if (GunInHand)
	GunInHand->RequestReloading();
}

void APlayerCharacter::ChangeToPrimaryGun()
{
    SecondaryGun->SetFALSEbCanUseWeapon();
	GunInHand = PrimaryGun;
	PrimaryGun->SetActorHiddenInGame(false);
	SecondaryGun->SetActorHiddenInGame(true);
	if (GunInHand)
	GunInHand->ActivebCanUseWeaponTimer();
	PlayerEquipmentState = EBT_Rifle;
	ChangeWeapon.Broadcast();

}

void APlayerCharacter::ChangeToSecondaryGun()
{
    PrimaryGun->SetFALSEbCanUseWeapon();
	GunInHand =SecondaryGun;
	SecondaryGun->SetActorHiddenInGame(false);
	PrimaryGun->SetActorHiddenInGame(true);
	if (GunInHand)
	GunInHand->ActivebCanUseWeaponTimer();
	PlayerEquipmentState = EBT_Rifle;
	ChangeWeapon.Broadcast();

}

void APlayerCharacter::NoWeapon()
{
	SecondaryGun->SetFALSEbCanUseWeapon();
	PrimaryGun->SetFALSEbCanUseWeapon();
	GunInHand = nullptr;
	PrimaryGun->SetActorHiddenInGame(true);
	SecondaryGun->SetActorHiddenInGame(true);
	PlayerEquipmentState = EBT_NoWeapon;
}

  //bools
bool APlayerCharacter::GetHasShot()
{
	return bHasShot;
}


  //floats

float APlayerCharacter::GetWeaponMovementAccuracyValue()
{
	

	return float(WeaponMovementAccuracyValue)/ GetCharacterMovement()->MaxWalkSpeed;


}

  //others
AGun* APlayerCharacter::GetGun()const
{
	return GunInHand;
}

EPlayerEquipmentState APlayerCharacter::GetPlayerEquipmentState()
{
	return PlayerEquipmentState;
}

