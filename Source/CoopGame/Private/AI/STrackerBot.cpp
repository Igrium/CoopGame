// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Components/SHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"


// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetVisibility(false);
	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;

	ExplosionDamage = 40;
	ExplosionRadius = 200;

	SelfDamageInterval = 0.25f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		// Find initial move to
		NextPathPoint = GetNextPathPoint();
	}
}

// Get the next point in the path to the player
FVector ASTrackerBot::GetNextPathPoint()
{
	// Hack to get player location
	APawn* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath && NavPath->PathPoints.Num() > 1)
	{
		// Return next point in the path
		return NavPath->PathPoints[1];
	}

	// Failed to find path
	return GetActorLocation();
}



void ASTrackerBot::HandleTakeDamage(USHealthComponent * OwningHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	// Make material flash
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	// Explode on hitpoints == 0
	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetSimulatePhysics(false);
	bExploded = true;
	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// Apply Damage!
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		// Delete actor
		SetLifeSpan(2.0f);
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
		}
		else
		{
			// Keep moving towards next target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
	}

}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn)
		{
			if (Role == ROLE_Authority)
			{
				//Start self destruction sequence
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}
