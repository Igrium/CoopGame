// Fill out your copyright notice in the Description page of Project Settings.

#include "SGrenadeLauncher.h"
#include "Kismet/GameplayStatics.h"



void ASGrenadeLauncher::Fire()
{


	if (GrenadeClass)
	{
		APawn* MyOwner = Cast<APawn>(GetOwner());
		if (MyOwner)
		{

			//Firing Code
			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);


			// Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			ActorSpawnParams.Instigator = MyOwner;
	

			// Get the projectile spawn vector
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		
			// spawn the projectile at the muzzle
			AActor* Projectile = GetWorld()->SpawnActor<AActor>(GrenadeClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
			Projectile->Instigator = MyOwner;
			

		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Grenade class not set! Not Firing!"));
	}
}
