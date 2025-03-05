	// Copyright Epic Games, Inc. All Rights Reserved.

	#pragma once

	#include "CoreMinimal.h"
	#include "GameFramework/Character.h"
	#include "Logging/LogMacros.h"
	#include "ThirdYearProjectCharacter.generated.h"

	class UInputComponent;
	class USkeletalMeshComponent;
	class UCameraComponent;
	class UInputAction;
	class UInputMappingContext;
	struct FInputActionValue;

	DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

	UCLASS(config=Game)
	class AThirdYearProjectCharacter : public ACharacter
	{
		GENERATED_BODY()

		/** Pawn mesh: 1st person view (arms; seen only by self) */
		UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
		USkeletalMeshComponent* Mesh1P;

		/** First person camera */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FirstPersonCameraComponent;

		/*Movement*/
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		UInputMappingContext* DefaultMappingContext;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		UInputAction* JumpAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		UInputAction* MoveAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		UInputAction* LookAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		UInputAction* SprintAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		UInputAction* SlideAction;
	
	public:
		AThirdYearProjectCharacter();

	protected:
		virtual void BeginPlay();
		virtual void Tick(float DeltaTime) override;
	public:
		

		/** Bool for AnimBP to switch to another animation set */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
		bool bHasRifle;

		/** Setter to set the bool */
		UFUNCTION(BlueprintCallable, Category = Weapon)
		void SetHasRifle(bool bNewHasRifle);

		/** Getter for the bool */
		UFUNCTION(BlueprintCallable, Category = Weapon)
		bool GetHasRifle();



	protected:
		/** Called for movement input */
		void Move(const FInputActionValue& Value);

		/** Called for looking input */
		void Look(const FInputActionValue& Value);

	protected:
		int JumpCount = 0;
		const int MaxJumps = 2;  // Allows one extra jump (double jump)

	protected:
		virtual void Jump() override;  // Override Jump
		virtual void Landed(const FHitResult& Hit) override;  // Override Landed

		void SlideJump();

		void StartSprint();
		void StopSprint();

		void StartSlide();
		void StopSlide();

	protected:
		// APawn interface
		virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
		// End of APawn interface

	public:
		/** Returns Mesh1P subobject **/
		USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
		/** Returns FirstPersonCameraComponent subobject **/
		UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


	private:
		float WalkSpeed = 600;
		bool  bIsSprinting = false;
		float SprintSpeed = 900.0f;
		bool  bIsSliding = false;
		float SlideSpeed;
		FVector SlideDirection;
	
		float AccelerationRate = 5.0f;
		float DecelerationRate =25.0f;

	protected:
		bool bIsWallRunning = false;
		FVector WallRunDirection;
		FVector WallRunNormal;
		FTimerHandle WallRunTimer;
		bool bCanWallRun = true;  // Prevents immediate reactivation
		float WallRunCooldown = 0.3f; // Delay before reactivating wallrun

		void StartWallRun(FVector WallNormal);
		void StopWallRun();
		bool CanWallRun(FVector& OutWallNormal);
		void ResetWallRun(); // Function to reset the cooldown
	

	
	};

