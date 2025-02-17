// : LeaveMeAlone Game by Netologiya. All RightsReserved.

#include "Player/LMADefaultCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/DecalComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/LMAHealthComponent.h"
#include "Components/LMAStaminaComponent.h"



// Sets default values
ALMADefaultCharacter::ALMADefaultCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->TargetArmLength = ArmLength;
	SpringArmComponent->SetRelativeRotation(FRotator(YRotation, 0.0f, 0.0f));
	SpringArmComponent->bDoCollisionTest = false;
	SpringArmComponent->bEnableCameraLag = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetFieldOfView(FOV);
	CameraComponent->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	StaminaComponent = CreateDefaultSubobject<ULMAStaminaComponent>("StaminaComponent");

}

// Called when the game starts or when spawned
void ALMADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
	}

	OnStaminaChanged(StaminaComponent->GetStamina());
	StaminaComponent->OnStaminaChanged.AddDynamic(this, &ALMADefaultCharacter::OnStaminaChanged);
}

// Called every frame
void ALMADefaultCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		FHitResult ResultHit;
		PC->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);  
		if (ResultHit.bBlockingHit)  
		{
			float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
			SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f))); 

			if (CurrentCursor)
			{
				CurrentCursor->SetWorldLocation(ResultHit.Location); 
			}
		}
	}
}

// Called to bind functionality to input
void ALMADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ALMADefaultCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ALMADefaultCharacter::MoveRight);

	PlayerInputComponent->BindAxis("CameraZoom", this, &ALMADefaultCharacter::CameraZoom);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ALMADefaultCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ALMADefaultCharacter::StopSprint);
}

void ALMADefaultCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		Sprint(Value);
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ALMADefaultCharacter::MoveRight(float Value)
{
	if ( Value != 0.0f)
	{
		Sprint(Value);
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ALMADefaultCharacter::CameraZoom(float Value)
{
	float tmpArmLength = ArmLength + ZoomSpeed * Value;
	if (tmpArmLength > ArmLengthMax)
	{
		tmpArmLength = ArmLengthMax;
	}
	else if (tmpArmLength < ArmLengthMin)
	{
		tmpArmLength = ArmLengthMin;
	}
	ArmLength = tmpArmLength;
	SpringArmComponent->TargetArmLength = ArmLength;
}




void ALMADefaultCharacter::StartSprint()
{
	IsSprint = true;
}

void ALMADefaultCharacter::StopSprint()
{
	IsSprint = false;
}
void ALMADefaultCharacter::OnStaminaChanged(float NewStamina)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Stamina = %f"), NewStamina));
}

void ALMADefaultCharacter::Sprint(float Value)
{
	if (IsSprint && Value != 0.0f)
	{
		if (!StaminaComponent->IsStaminaEmpty())
		{
			GetCharacterMovement()->MaxWalkSpeed = 600.0f;
			StaminaComponent->StaminaDecrease();
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = 300.0f;
			StaminaComponent->StaminaIncrease();
		}
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		StaminaComponent->StaminaIncrease();

	}
}
