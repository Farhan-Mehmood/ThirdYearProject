//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "GrapplingHookProjectile.h"
//
//// Sets default values
//AGrapplingHookProjectile::AGrapplingHookProjectile()
//{
// 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
//	PrimaryActorTick.bCanEverTick = true;
//
//	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
//	CollisionComp->InitSphereRadius(5.0f);
//	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
//	RootComponent = CollisionComp;
//
//	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
//	ProjectileMovement->InitialSpeed = 3000.f;
//	ProjectileMovement->MaxSpeed = 3000.f;
//
//	CollisionComp->OnComponentHit.AddDynamic(this, &AGrapplingHookProjectile::OnHit);
//}
//
//// Called when the game starts or when spawned
//void AGrapplingHookProjectile::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}
//
//// Called every frame
//void AGrapplingHookProjectile::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
//
//
//void AGrapplingHookProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
//{
//    // Attach the character to the hook
//    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
//    {
//        AttachedCharacter = Character;
//
//        //pulls character towards location of hook
//        FVector HookLocation = Hit.Location;
//        Character->LaunchCharacter(FVector(0, 0, 0), true, true); // Reset velocity
//        Character->SetActorLocation(HookLocation);
//
//        
//    }
//
//    Destroy(); // Destroy the grappling hook after it hits
//}
