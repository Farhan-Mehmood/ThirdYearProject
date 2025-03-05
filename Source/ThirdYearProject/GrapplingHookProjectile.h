//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
//#include "GrapplingHookProjectile.generated.h"
//
//UCLASS()
//class THIRDYEARPROJECT_API AGrapplingHookProjectile : public AActor
//{
//	GENERATED_BODY()
//	
//public:	
//	// Sets default values for this actor's properties
//	AGrapplingHookProjectile();
//
//protected:
//	// Called when the game starts or when spawned
//	virtual void BeginPlay() override;
//
//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;
//
//    UFUNCTION()
//    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
//
//	/** Sphere collision component */
//	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
//	USphereComponent* CollisionComp;
//
//	/** Projectile movement component */
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
//	UProjectileMovementComponent* ProjectileMovement;
//
//    UPROPERTY(EditDefaultsOnly, Category = "Grappling Hook")
//    float HookDistance = 1000.0f;
//
//    UPROPERTY(EditDefaultsOnly, Category = "Grappling Hook")
//    float RetractSpeed = 500.0f;
//
//    ACharacter* AttachedCharacter;
//
//    /** Returns CollisionComp subobject **/
//    USphereComponent* GetCollisionComp() const { return CollisionComp; }
//    /** Returns ProjectileMovement subobject **/
//    UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
//
//};
