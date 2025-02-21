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
		if (GetVelocity().Size() < 200.f) // Adjust this threshold as needed
		{
			StopSlide();
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

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
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



