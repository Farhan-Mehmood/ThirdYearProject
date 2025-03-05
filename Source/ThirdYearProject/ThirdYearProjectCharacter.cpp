// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThirdYearProjectCharacter.h"
#include "ThirdYearProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/LocalPlayer.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AThirdYearProjectCharacter

AThirdYearProjectCharacter::AThirdYearProjectCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));



	GetCharacterMovement()->JumpZVelocity = 500.0f;
	GetCharacterMovement()->AirControl = 0.9f;  // Allow more control in air
	GetCharacterMovement()->GroundFriction = 0.5f;  // Reduce friction to maintain momentum
	GetCharacterMovement()->BrakingFrictionFactor = 0.2f; // Reduce braking friction
	GetCharacterMovement()->GravityScale = 1.0f; // Normal gravity

}


void AThirdYearProjectCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Call the parent class's Tick function

	// If sliding, check the velocity
	if (bIsSliding)
	{
		// If velocity falls below a certain threshold, stop the slide
		if (GetVelocity().Size() < 200.f) 
		{
			StopSlide();
		}
	}

	if (bIsWallRunning)
	{
		FVector WallNormal;
		if (!CanWallRun(WallNormal) || !GetCharacterMovement()->IsFalling()) // Check if we lost the wall
		{
			StopWallRun();
		}
	}
	else if (!bIsWallRunning && GetCharacterMovement()->IsFalling() && bCanWallRun) // Only check while airborne
	{
		FVector WallNormal;
		if (CanWallRun(WallNormal))
		{
			StartWallRun(WallNormal);
		}
	}
}

void AThirdYearProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

//////////////////////////////////////////////////////////////////////////// Input

void AThirdYearProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AThirdYearProjectCharacter::SlideJump);



		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThirdYearProjectCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThirdYearProjectCharacter::Look);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AThirdYearProjectCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AThirdYearProjectCharacter::StopSprint);

		//Slide
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AThirdYearProjectCharacter::StartSlide);
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AThirdYearProjectCharacter::StopSlide);


	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	
}


void AThirdYearProjectCharacter::Jump()
{
	if (JumpCount < MaxJumps)
	{
		if (bIsSliding)
		{
			// Jumping from slide - preserve forward momentum
			FVector SlideJumpDirection = (GetActorForwardVector() * 1.5f) + FVector(0, 0, 1); // Add upward component
			LaunchCharacter(SlideJumpDirection * 800.f, true, true);
			StopSlide(); // Stop sliding after jump
		}
		else if (bIsWallRunning)
		{
			StopWallRun();

			// Jump direction: Away from the wall and slightly forward
			FVector JumpAwayFromWall = WallRunNormal * 600.f + GetActorForwardVector() * 400.f + FVector(0, 0, 800.f);

			LaunchCharacter(JumpAwayFromWall, true, true);

			UE_LOG(LogTemp, Warning, TEXT("Wallrun Jump! Direction: %s"), *JumpAwayFromWall.ToString());
		}
		else if (JumpCount == 0)
		{
			// First jump (normal jump)
			Super::Jump();
		}
		else
		{
			// Second jump (double jump) - preserve some velocity
			FVector JumpVelocity = FVector(GetVelocity().X, GetVelocity().Y, GetCharacterMovement()->JumpZVelocity);
			LaunchCharacter(JumpVelocity, false, true);
		}

		JumpCount++;  // Track jumps before landing
		UE_LOG(LogTemp, Warning, TEXT("Jump Count: %d"), JumpCount);
	}
}

void AThirdYearProjectCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	JumpCount = 0;  // Reset jump count when landing
}



void AThirdYearProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// Check if movement input is provided
	bool bIsMoving = !MovementVector.IsZero();

	if (Controller != nullptr)
	{
		FVector DesiredDirection = GetActorForwardVector() * MovementVector.Y + GetActorRightVector() * MovementVector.X;

		// Get the current velocity of the character
		FVector CurrentVelocity = GetCharacterMovement()->Velocity;

		// Define the target velocity (based on the direction)
		FVector TargetVelocity = DesiredDirection.GetSafeNormal() * GetCharacterMovement()->MaxWalkSpeed;

		// Handle horizontal movement interpolation (speed up or slow down)
		if (bIsMoving)
		{
			// Smooth acceleration (speed up)
			FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, GetWorld()->DeltaTimeSeconds, AccelerationRate);
			CurrentVelocity = NewVelocity;
		}
		else
		{
			// Smooth deceleration (slow down when not moving)
			FVector DeceleratedVelocity = FMath::VInterpTo(CurrentVelocity, FVector(0.f, 0.f, 0.f), GetWorld()->DeltaTimeSeconds, DecelerationRate);
			CurrentVelocity = DeceleratedVelocity;
		}

		// Now make sure we preserve the vertical velocity (Z component) so gravity works normally
		FVector FinalVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, GetCharacterMovement()->Velocity.Z);

		// Apply the final velocity
		GetCharacterMovement()->Velocity = FinalVelocity;

		// Apply the movement input (this part would still be relevant for horizontal movement input processing)
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);

		// Log the current walk speed (using the current velocity size)
		float CurrentSpeed = GetCharacterMovement()->Velocity.Size();
		UE_LOG(LogTemp, Warning, TEXT("Current Speed: %f"), CurrentSpeed);
	}
}



void AThirdYearProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AThirdYearProjectCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = 900;
	UE_LOG(LogTemp, Warning, TEXT("SPRINTING"));
}

void AThirdYearProjectCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = 600;
	UE_LOG(LogTemp, Warning, TEXT("SPRINTING STOP"));
}

void AThirdYearProjectCharacter::StartSlide()
{
	if (!bIsSliding && GetCharacterMovement()->IsFalling() == false && GetCharacterMovement()->Velocity.Size() > 200.f)
	{
		bIsSliding = true;
		SlideSpeed = FMath::Max(GetCharacterMovement()->Velocity.Size(), 1200.f); // Maintain speed
		SlideDirection = GetVelocity().GetSafeNormal();
		GetCharacterMovement()->MaxWalkSpeed = SlideSpeed;

		// Lower capsule for sliding effect
		GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);
	}
}

void AThirdYearProjectCharacter::StopSlide()
{
	if (bIsSliding)
	{
		bIsSliding = false;
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
		GetCapsuleComponent()->SetCapsuleHalfHeight(96.f); // Reset capsule
	}
}

void AThirdYearProjectCharacter::SlideJump()
{
	if (bIsSliding)
	{
		// Preserve momentum when jumping out of slide
		FVector JumpDirection = GetActorForwardVector() * 1.2f + FVector(0, 0, 1);
		LaunchCharacter(JumpDirection * 800.f, true, true);
		StopSlide();
	}
}

void AThirdYearProjectCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AThirdYearProjectCharacter::GetHasRifle()
{
	return bHasRifle;
}


bool AThirdYearProjectCharacter::CanWallRun(FVector& OutWallNormal)
{
	FVector Start = GetActorLocation();
	FVector RightTraceEnd = Start + (GetActorRightVector() * 100.f); // Check right side
	FVector LeftTraceEnd = Start - (GetActorRightVector() * 100.f);  // Check left side

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = false;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, RightTraceEnd, ECC_Visibility, QueryParams))
	{
		OutWallNormal = HitResult.Normal;
		bHit = true;
		//UE_LOG(LogTemp, Warning, TEXT("Wall detected on RIGHT"));
	}
	else if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, LeftTraceEnd, ECC_Visibility, QueryParams))
	{
		OutWallNormal = HitResult.Normal;
		bHit = true;
		//UE_LOG(LogTemp, Warning, TEXT("Wall detected on LEFT"));
	}

	if (!bHit)
	{
		//UE_LOG(LogTemp, Warning, TEXT("No wall detected"));
	}

	return bHit;

}


void AThirdYearProjectCharacter::StartWallRun(FVector WallNormal)
{
    if (!bIsWallRunning && bCanWallRun)
    {
        bIsWallRunning = true;
        bCanWallRun = false; // Disable wallrunning after starting
        WallRunNormal = WallNormal; // Store the wall normal

        // Apply low gravity to the character during wall running
        GetCharacterMovement()->GravityScale = 0.3f;  
        GetCharacterMovement()->MaxWalkSpeed = 1000.0f;  // Start with a default wall run speed
        JumpCount = 0;

        // Get the direction to move along the wall (cross product with up vector)
        WallRunDirection = FVector::CrossProduct(WallNormal, FVector::UpVector);
        WallRunDirection *= (FVector::DotProduct(WallRunDirection, GetActorForwardVector()) > 0) ? 1 : -1;

        // Apply initial velocity based on the wall normal and run direction
        FVector WallRunVelocity = WallRunDirection * 800.f + FVector(0, 0, 300.f); // Add vertical lift for smooth wall running
        GetCharacterMovement()->Velocity = WallRunVelocity;  // Apply the calculated velocity to the character
        LaunchCharacter(WallRunDirection * 1200.0f, true, true);  // Initial jump boost along the wall

        UE_LOG(LogTemp, Warning, TEXT("Wallrun Started"));
    }
}


void AThirdYearProjectCharacter::StopWallRun()
{
	if (bIsWallRunning)
	{
		bIsWallRunning = false;
		GetCharacterMovement()->GravityScale = 1.0f;  // Restore normal gravity
		GetCharacterMovement()->MaxWalkSpeed = 600.f; // Reset speed

		UE_LOG(LogTemp, Warning, TEXT("Wallrun Stopped"));

		// Start the cooldown timer when stopping the wallrun
		GetWorld()->GetTimerManager().SetTimer(WallRunTimer, this, &AThirdYearProjectCharacter::ResetWallRun, WallRunCooldown, false);
	}
}

void AThirdYearProjectCharacter::ResetWallRun()
{
	bCanWallRun = true;
	UE_LOG(LogTemp, Warning, TEXT("Wallrun Ready Again"));
}


