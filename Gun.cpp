// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterController.h"

// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	SetRootComponent(GunMesh);

	GunMesh->GetSocketLocation("Stock_Socket");

	CurrentAmmo = DefaultAmmo;

}





// Called when the game starts or when spawned
void AGun::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = DefaultAmmoQuantity;
	Damage = DefaultDamage;
//AccuracyValue = DefaultAccuracyValue;

}

// Called every frame
void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	int32 AccuracyValueToIncreaseAfterShooting;
	if (!bOwnerIsCrouching)AccuracyValueToIncreaseAfterShooting = DefaultShootingAccuracyValueIncrease;
	else AccuracyValueToIncreaseAfterShooting = DefaultShootingWhileCrouchAccuracyValueIncrease;

	if (bShootingAccuracyInterp && (ShootingAccuracyValueIncrease < AccuracyValueToIncreaseAfterShooting) )
	{
		GetWorldTimerManager().ClearTimer(CrosshairRecoverAfterShootingHandle);
		CrosshairGrowth(DeltaTime, ShootingAccuracyValueIncrease, AccuracyValueToIncreaseAfterShooting, ShotingCrosshairGrowingSpeed);
	 	

	}
	if ( (ShootingAccuracyValueIncrease >= AccuracyValueToIncreaseAfterShooting) && bUseCrosshairShrinkTimer)
	{
		
		GetWorldTimerManager().SetTimer(CrosshairRecoverAfterShootingHandle, this,
			&AGun::SetFALSEbShootingAccuracyInterp, CrosshairshrinkDelayAfterShooting);

		
       bUseCrosshairShrinkTimer = false;
	}


	if (!bShootingAccuracyInterp && ShootingAccuracyValueIncrease!=0)
		CrosshairShrink(DeltaTime, ShootingAccuracyValueIncrease, 0, ShotingCrosshairShrinkingSpeed);
		


	if (bIsAimingDownsight)
	{
	CrosshairGrowth(DeltaTime, AimDownsightAccuracyValueDecrease, DefaultAimDownsightAccuracyValueDecrease, AimingCrosshairGrowingSpeed);
	}
	else 
	{
	CrosshairShrink(DeltaTime, AimDownsightAccuracyValueDecrease, 0, AimingCrosshairShrinkingSpeed);
	}

	if (bOwnerIsCrouching)
	{
		CrosshairGrowth(DeltaTime, CrouchingAccuracyValueDecrease, DefaultCrouchingAccuracyValueDecrease, 300);

	}
	else
	{
		CrosshairShrink(DeltaTime, CrouchingAccuracyValueDecrease, 0, 1);
	}

    WeaponGripLocation=GunMesh->GetSocketLocation("Grip_Socket");
	WeaponGripRotation = GunMesh->GetSocketRotation("Grip_Socket");

   // UE_LOG(LogTemp, Error, TEXT("%f"), GetWorldTimerManager().GetTimerElapsed(CrosshairRecoverAfterShootinhHandle));

	CalculateAccuracy();

}
void AGun::RequestShooting()
{
//
	if (bCanUseWeapon && (bShootingAllowed || AutomaticFire) && !GetWorldTimerManager().IsTimerActive(ReloadingHandle) )
	{

		if (CurrentAmmo > 0)
		{
			if (WeaponType == EBT_AsaultRifle)
			Shoot();

			else if (WeaponType == EBT_Shotgun)
			{
					SpreadShoot();
			}
		}
		else RequestReloading();

	}

//	UE_LOG(LogTemp, Error, TEXT("%i"), CurrentAmmo);
}



void AGun::Shoot()
{
	
	FHitResult POV_Hit;
	FHitResult Gun_Hit;
	FVector POV_ShotDirection;
	FVector GunShotDirection;
    FVector Start = GunMesh->GetSocketLocation("Muzzle_Socket");


	bShootingAccuracyInterp = true;
	bUseCrosshairShrinkTimer = true;

	if (POVGunTrace(POV_Hit, POV_ShotDirection))
    {
		
	GunShotDirection = POV_Hit.Location;
	
		//GunShotDirection = ShotDirection;
	   GunTrace(Gun_Hit, GunShotDirection);
	   AActor* HitActor = Gun_Hit.GetActor();
	   if (HitActor)
	   {
		  
		   FVector ShotDirectionVector = Gun_Hit.Location - Start;
		   FVector FallofStartPoint = Start +
	       ( ShotDirectionVector /sqrt(pow(ShotDirectionVector.X,2)+ pow(ShotDirectionVector.Y, 2) + pow(ShotDirectionVector.Z, 2) ) )
		   * NoDamageFalloffRange;

		   float DistaceFromBarellToHit = sqrt(pow((Gun_Hit.Location.X - Start.X), 2) + pow((Gun_Hit.Location.Y - Start.Y), 2));
		  
		   if (DistaceFromBarellToHit <= NoDamageFalloffRange )
		   {
			   Damage = DefaultDamage;
		   }
		   else 
		   {
			   float DistanceFalloffStartToHit =DistaceFromBarellToHit- NoDamageFalloffRange;

			 //  UE_LOG(LogTemp, Warning, TEXT("%f"), DistanceFalloffStartToHit);
			   Damage =int32( DefaultDamage - DefaultDamage*DamageFalloffRate* (  DistanceFalloffStartToHit / (MaxRange-NoDamageFalloffRange) ));

			 //  UE_LOG(LogTemp, Warning, TEXT("%f"), Damage);

		   }
			//////

		 DrawDebugLine(GetWorld(), Start, FallofStartPoint, FColor::Blue, true, 1, 0, 1);
         DrawDebugLine(GetWorld(), FallofStartPoint, GunShotDirection, FColor::Green, true, 1, 0, 1);

			 FPointDamageEvent DamageEvent(Damage, Gun_Hit, GunShotDirection, nullptr);
			 if (OwnerController)
				 HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
			 if(APlayerCharacterController* PlayerController = Cast<APlayerCharacterController>( GetWorld()->GetFirstPlayerController() ) )
			 {
				 PlayerController->AddDamageIndicatorToViewport(Damage, Gun_Hit.Location);
			 }
		//	 IndicateDamageDelegate.Broadcast(Damage, Gun_Hit.Location);
		 

		//	UE_LOG(LogTemp, Warning, TEXT("%s"),*HitActor->GetName());

	   }

    }

else
	{
	
		GunShotDirection = Start + POV_ShotDirection * MaxRange;
		GunTrace(Gun_Hit, GunShotDirection);
		/////
		DrawDebugLine(GetWorld(), Start, GunShotDirection, FColor::Green, true, 1, 0, 1);

	}
	
	CurrentAmmo--;
	bShootingAllowed = false;
	GetWorldTimerManager().SetTimer(ShootDelayHandle, this, &AGun::AllowShooting, ShootingDelay);



}
void AGun::SpreadShoot()
{
	FHitResult POV_Hit;
	FHitResult Gun_Hit;
	FVector POV_ShotDirection;
	FVector GunShotDirection;
	FVector Start = GunMesh->GetSocketLocation("Muzzle_Socket");
	AActor* HitActor=nullptr;
	
	float DamageAccumulation=0;

	bShootingAccuracyInterp = true;
	bUseCrosshairShrinkTimer = true;

	for (int i = 0; i < ShotgunShootingBulletCount; i++)
	{

		if (POVGunTrace(POV_Hit, POV_ShotDirection))
		{

			GunShotDirection = POV_Hit.Location;

			//GunShotDirection = ShotDirection;
			GunTrace(Gun_Hit, GunShotDirection);
			HitActor = Gun_Hit.GetActor();
			if (HitActor)
			{

				FVector ShotDirectionVector = Gun_Hit.Location - Start;
				FVector FallofStartPoint = Start +
					(ShotDirectionVector / sqrt(pow(ShotDirectionVector.X, 2) + pow(ShotDirectionVector.Y, 2) + pow(ShotDirectionVector.Z, 2)))
					* NoDamageFalloffRange;

				float DistaceFromBarellToHit = sqrt(pow((Gun_Hit.Location.X - Start.X), 2) + pow((Gun_Hit.Location.Y - Start.Y), 2));

				if (DistaceFromBarellToHit <= NoDamageFalloffRange)
				{
					Damage = DefaultDamage;
				}
				else
				{
					float DistanceFalloffStartToHit = DistaceFromBarellToHit - NoDamageFalloffRange;

					
					Damage =int32( DefaultDamage - DefaultDamage * DamageFalloffRate * (DistanceFalloffStartToHit / (MaxRange - NoDamageFalloffRange)));

					 

				}
				//////
				DamageAccumulation = DamageAccumulation + Damage;
				DrawDebugLine(GetWorld(), Start, FallofStartPoint, FColor::Blue, true, 1, 0, 1);
				DrawDebugLine(GetWorld(), FallofStartPoint, GunShotDirection, FColor::Green, true, 1, 0, 1);

				
				//	UE_LOG(LogTemp, Warning, TEXT("%s"),*HitActor->GetName());

			}

		}
		else
		{

			GunShotDirection = Start + POV_ShotDirection * MaxRange;
			GunTrace(Gun_Hit, GunShotDirection);
			/////
			DrawDebugLine(GetWorld(), Start, GunShotDirection, FColor::Green, true, 1, 0, 1);

		}

    }//Loop End
	if (HitActor)
	{
		FPointDamageEvent DamageEvent(DamageAccumulation, Gun_Hit, GunShotDirection, nullptr);
		if (OwnerController)
			HitActor->TakeDamage(DamageAccumulation, DamageEvent, OwnerController, this);
		if (APlayerCharacterController* PlayerController = Cast<APlayerCharacterController>(GetWorld()->GetFirstPlayerController()))
		{
			
			PlayerController->AddDamageIndicatorToViewport(DamageAccumulation, Gun_Hit.Location);
		}
		//IndicateDamageDelegate.Broadcast(DamageAccumulation, Gun_Hit.Location);

	//	UE_LOG(LogTemp, Warning, TEXT("%f"), DamageAccumulation);
	}

	CurrentAmmo--;

	bShootingAllowed = false;

	GetWorldTimerManager().SetTimer(ShootDelayHandle, this, &AGun::AllowShooting, ShootingDelay);
}
void AGun::RequestReloading()
{
	if (CurrentAmmo<DefaultAmmoQuantity && bCanUseWeapon)
	GetWorldTimerManager().SetTimer(ReloadingHandle, this, &AGun::Reload, ReloadTime);
	//UE_LOG(LogTemp, Error, TEXT("%i"), CurrentAmmo);
	
}
void AGun::Reload()
{
	CurrentAmmo = DefaultAmmoQuantity;
}

bool AGun::POVGunTrace(FHitResult& Hit, FVector& ShotDirection)
{
	if (!OwnerController )return false;

	FVector Start;
	FVector End;
	FVector POV_ShotLocation;
	FRotator Rotation;
	int32 ViewportSizeX, ViewportSizeY;
    
	OwnerController->GetPlayerViewPoint(Start, Rotation);
	
    if(APlayerController* PlayerController=Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{ 

		   PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	       auto ScreenLocation = FVector2D(ViewportSizeX, ViewportSizeY);
		   int32 RandomGeneratedAddX=0;
		   int32 RandomGeneratedAddY=0;
	       FVector2D ScreenCenter = ScreenLocation / 2;
		 
		   float DistanceCenterToMaxAccVal= AccuracyValue+1;  


		   
		   if(WeaponType== EBT_AsaultRifle)
		   while (DistanceCenterToMaxAccVal>=AccuracyValue)
		   {
            RandomGeneratedAddX = FMath::RandRange(-AccuracyValue, AccuracyValue);
		    RandomGeneratedAddY =FMath::RandRange(-AccuracyValue, AccuracyValue);
		    DistanceCenterToMaxAccVal= sqrt(pow((RandomGeneratedAddX ), 2) + pow((RandomGeneratedAddY ), 2));

		   }
		   else if (WeaponType == EBT_Shotgun)
		   {
			   RandomGeneratedAddX = FMath::RandRange(-AccuracyValue, AccuracyValue);
			   RandomGeneratedAddY = FMath::RandRange(-AccuracyValue, AccuracyValue);
		   }
		  FVector2D ShotLandingLocation;
		   ShotLandingLocation.X= ScreenCenter.X + RandomGeneratedAddX;
		   ShotLandingLocation.Y = ScreenCenter.Y + RandomGeneratedAddY;


	        if (PlayerController->DeprojectScreenPositionToWorld(ShotLandingLocation.X, ShotLandingLocation.Y, POV_ShotLocation, ShotDirection))
	        {
			  End = Start + ShotDirection * MaxRange;
		     
	        }
			else
			{
				ShotDirection = -Rotation.Vector();
				End = Start + Rotation.Vector() * MaxRange;
			}
    }
	else
	{
		ShotDirection = -Rotation.Vector();
        End = Start + Rotation.Vector() * MaxRange;
	}

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(GetOwner());

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, true, 1, 0, 1);

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_GameTraceChannel1, CollisionParams);

}

bool AGun::GunTrace(FHitResult& Hit, FVector& ShotDirection)
{

	FHitResult HitOut;
	if (!GunMesh)return false;
     
	FVector Start = GunMesh->GetSocketLocation("Muzzle_Socket");
	




	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(GetOwner());
   

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, ShotDirection, ECollisionChannel::ECC_GameTraceChannel1, CollisionParams);

}
void AGun::CalculateAccuracy()
{
	int32 MovementAccuracyValueAdd=0;
	float Velocity=0;
	if (GunPlayerOwner)
	{
		Velocity = GunPlayerOwner->GetWeaponMovementAccuracyValue();
		MovementAccuracyValueAdd = int32(Velocity * MovementAccuracyValueMultiplier);

	}
	

	AccuracyValue = DefaultAccuracyValue + MovementAccuracyValueAdd+ 
		            ShootingAccuracyValueIncrease- AimDownsightAccuracyValueDecrease- CrouchingAccuracyValueDecrease;
	if (AccuracyValue < MinAccuracyValue)AccuracyValue = MinAccuracyValue;

	if ((bOwnerIsCrouching && CrouchingStandingFirstShotAccuracy && Velocity == 0 && bIsAimingDownsight) ||
		(!bOwnerIsCrouching && StandingFirstShotAccuracy && Velocity == 0 && bIsAimingDownsight))
	{
		if (bCanUseFirstShotAccuracyTimer)
		{
			bCanUseFirstShotAccuracyTimer=false;
			GetWorldTimerManager().SetTimer(FirstShotAccuracyHandle, this, &AGun::SetTRUEbFirstShotAccuracy, FirstShotAccuracyBeginTime);
		}
	}
	else 
	{
		bFirstShotAccuracy = false;
		bCanUseFirstShotAccuracyTimer = true;
	} 
	if(bFirstShotAccuracy)AccuracyValue = MinAccuracyValue + ShootingAccuracyValueIncrease;

}

void AGun::CrosshairGrowth(float DeltaTime, int32& AccuracyLenght, int32 GoalAccuracyLenght,float Speed)
{
	AccuracyLenght = int32(FMath::FInterpConstantTo(float(AccuracyLenght), float(GoalAccuracyLenght), DeltaTime, Speed));
	//UE_LOG(LogTemp, Warning, TEXT("%i"), AccuracyLenght);
}

void AGun::CrosshairShrink(float DeltaTime, int32& AccuracyLenght, int32 GoalAccuracyLenght, float Speed)
{
	AccuracyLenght = int32(FMath::FInterpConstantTo(float(AccuracyLenght),0.f, DeltaTime, Speed));
	//UE_LOG(LogTemp, Error, TEXT("%i"), AccuracyLenght);
}

void AGun::SetFALSEbShootingAccuracyInterp()
{
	bShootingAccuracyInterp = false;
	
	
//	UE_LOG(LogTemp, Error, TEXT("Confirm"));
}

void AGun::SetTRUEbFirstShotAccuracy()
{
	bFirstShotAccuracy=true;
}



void AGun::AllowShooting()
{
    bShootingAllowed = true;
	if (AutomaticFire && CurrentAmmo>0)
	Shoot();
}

void AGun::ReleaseTrigger()
{
	if(AutomaticFire)
	GetWorldTimerManager().ClearTimer(ShootDelayHandle);

}
void AGun::AimDownSight()
{

	bIsAimingDownsight = true;

}
void AGun::ReleaseAim()
{

	bIsAimingDownsight = false;

}
void AGun::OwnerIsCrouching()
{
	bOwnerIsCrouching = true;
}
void AGun::OwnerIsUNCrouching()
{
	bOwnerIsCrouching = false;
}
void AGun::ChangeOwner()
{
	GunPlayerOwner = Cast<APlayerCharacter>(GetOwner());
    GunOwner = Cast<ACharacterBaseClass>(GetOwner());
	if(GunOwner)
	OwnerController = Cast<AController>(GunOwner->GetOwner());


}

void AGun::ActivebCanUseWeaponTimer()
{
	GetWorldTimerManager().SetTimer(PullOutWeaponHandle, this, &AGun::SetTRUEbCanUseWeapon, PullOutTime);
}

void AGun::SetTRUEbCanUseWeapon()
{
	bCanUseWeapon = true;
	//UE_LOG(LogTemp, Error, TEXT("True"));
}
void AGun::SetFALSEbCanUseWeapon()
{
	bCanUseWeapon = false;
	GetWorldTimerManager().ClearTimer(ReloadingHandle);
	//UE_LOG(LogTemp, Error, TEXT("false"));
}


//Get Functions//////////////////////////////////////
int32 AGun::GetGunAccuracyValue()const
{
	return AccuracyValue;
}

int32 AGun::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

float AGun::GetReloadingTimer()
{
	return GetWorldTimerManager().GetTimerElapsed(ReloadingHandle)/ReloadTime;
}

EWeaponType AGun::GetWeaponType()
{
	return WeaponType;
}

FVector AGun::GetWeaponGripLocation()
{
	return WeaponGripLocation;
}

FRotator AGun::GetWeaponGripRotation()
{
	return WeaponGripRotation;
}





