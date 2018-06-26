// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGrenadeLauncher : public ASWeapon
{
	GENERATED_BODY()

public:
	// sets default values
	ASGrenadeLauncher();
	
protected:
	//Properties
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AActor> GrenadeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float SpawnVelocity;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire() override;
	
};