// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThirdYearProjectProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"

AThirdYearProjectProjectile::AThirdYearProjectProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AThirdYearProjectProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AThirdYearProjectProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    FVector ExplosionOrigin = GetActorLocation();
    float ExplosionRadius = 500.0f;  // Radius of the effect
    float ExplosionForce = 2000.0f;  // Strength of the force applied

    // Find all actors in the explosion radius
    TArray<FOverlapResult> OverlapResults;
    FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(ExplosionRadius);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // Ignore the projectile itself

    GetWorld()->OverlapMultiByChannel(OverlapResults, ExplosionOrigin, FQuat::Identity, ECC_PhysicsBody, CollisionSphere, QueryParams);

    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* AffectedActor = Result.GetActor();
        if (AffectedActor && AffectedActor != this)
        {
            UPrimitiveComponent* AffectedComponent = Cast<UPrimitiveComponent>(Result.GetComponent());
            if (AffectedComponent)
            {
                FVector Direction = AffectedComponent->GetComponentLocation() - ExplosionOrigin;
                float Distance = Direction.Size();
                Direction.Normalize();

                float ScaledForce = ExplosionForce * (1.0f - (Distance / ExplosionRadius)); // Scale force based on distance

                // Apply force to physics objects
                if (AffectedComponent->IsSimulatingPhysics())
                {
                    AffectedComponent->AddImpulse(Direction * ScaledForce, NAME_None, true);
                }

                // Apply force to characters
                ACharacter* AffectedCharacter = Cast<ACharacter>(AffectedActor);
                if (AffectedCharacter)
                {
                    AffectedCharacter->LaunchCharacter(Direction * ScaledForce, true, true);
                }
            }
        }
    }

    // Destroy the projectile after applying effects
    Destroy();
}