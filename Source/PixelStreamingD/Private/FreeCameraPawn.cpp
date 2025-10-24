// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"


// Sets default values
AFreeCameraPawn::AFreeCameraPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;
	SpringArm->bDoCollisionTest = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = false;

	VirtualDistance = SpringArm->TargetArmLength;
}

// Called when the game starts or when spawned
void AFreeCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	CachedPC = Cast<APlayerController>(GetController());
	if (CachedPC.IsValid())
	{
		CachedPC->bShowMouseCursor = true;
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		CachedPC->SetInputMode(Mode);
	}

	if (bUseFOVZoom)
	{
		VirtualDistance = SpringArm->TargetArmLength;
		UpdateFOVFromVirtualDistance();
	}
}

// Called every frame
void AFreeCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float DeltaSeconds = DeltaTime;

	if (!CachedPC.IsValid())
	{
		CachedPC = Cast<APlayerController>(GetController());
	}

	FVector2D MouseDelta(0.f, 0.f);
	if (CachedPC.IsValid())
	{
		float dx = 0.f, dy = 0.f;
		CachedPC->GetInputMouseDelta(dx, dy);
		MouseDelta = FVector2D(dx, dy);
	}

	if (bIsRotating && (MouseDelta.SizeSquared() > 0.f))
	{
		ApplyLook(MouseDelta, DeltaSeconds);
	}

	if (bIsPanning && (MouseDelta.SizeSquared() > 0.f))
	{
		ApplyPan(MouseDelta, DeltaSeconds);
	}

	if (bIsZoomingDrag && FMath::Abs(MouseDelta.Y) > KINDA_SMALL_NUMBER)
	{
		ApplyZoom(MouseDelta.Y, true, DeltaSeconds);
	}
}

// Called to bind functionality to input
void AFreeCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFreeCameraPawn::OnLMBPressed);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AFreeCameraPawn::OnLMBReleased);

	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AFreeCameraPawn::OnRMBPressed);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AFreeCameraPawn::OnRMBReleased);

	PlayerInputComponent->BindKey(EKeys::MiddleMouseButton, IE_Pressed, this, &AFreeCameraPawn::OnMMBPressed);
	PlayerInputComponent->BindKey(EKeys::MiddleMouseButton, IE_Released, this, &AFreeCameraPawn::OnMMBReleased);

	PlayerInputComponent->BindAxisKey(EKeys::MouseWheelAxis, this, &AFreeCameraPawn::OnMouseWheelAxis);
}

void AFreeCameraPawn::OnLMBPressed()
{
    bIsRotating = true;
    UpdateMouseCapture();
}

void AFreeCameraPawn::OnLMBReleased()
{
    bIsRotating = false;
    UpdateMouseCapture();
}

void AFreeCameraPawn::OnRMBPressed()
{
    bIsPanning = true;
    UpdateMouseCapture();
}

void AFreeCameraPawn::OnRMBReleased()
{
    bIsPanning = false;
    UpdateMouseCapture();
}

void AFreeCameraPawn::OnMMBPressed()
{
    bIsZoomingDrag = true;
    UpdateMouseCapture();
}

void AFreeCameraPawn::OnMMBReleased()
{
    bIsZoomingDrag = false;
    UpdateMouseCapture();
}

void AFreeCameraPawn::OnMouseWheelAxis(float Value)
{
    if (FMath::Abs(Value) < KINDA_SMALL_NUMBER)
        return;

    ApplyZoom(Value, false, 0.f);
}

void AFreeCameraPawn::ApplyLook(const FVector2D& MouseDelta, float DT)
{
    if (!Controller) return;

    const float ScaleY = bInvertY ? 1.f : -1.f;
    const float YawDelta = MouseDelta.X * LookSensitivity * DT * 60.f;
    const float PitchDelta = MouseDelta.Y * LookSensitivity * DT * 60.f * ScaleY;

    AddControllerYawInput(YawDelta);
    AddControllerPitchInput(PitchDelta);
    ClampControlPitch();
}

void AFreeCameraPawn::ApplyPan(const FVector2D& MouseDelta, float DT)
{
    FVector Fwd, Right;
    GetPlanarViewAxes(Fwd, Right);
	
	const float DistFactor = bScalePanByDistance ? GetDistanceFactor() : 1.f;

    // 屏幕像素 世界位移
    // const FVector Offset = (-MouseDelta.Y * Fwd + MouseDelta.X * Right) * PanSpeed * DT / 100.f;
    const FVector Offset = (-MouseDelta.Y * Fwd + MouseDelta.X * -Right) * PanSpeed * DistFactor * DT / 100.f;
    AddActorWorldOffset(Offset, true);
}

void AFreeCameraPawn::ApplyZoom(float Axis, bool bFromDrag, float DT)
{
    // Axis>0 表示拉近（向前）
    const float BaseAmount  = bFromDrag ? (Axis * ZoomSpeed * DT / 10.f) : (Axis * ZoomSpeed);
	const float DistFactor = bScaleZoomByDistance ? GetDistanceFactor() : 1.f;
	const float Amount = BaseAmount * DistFactor;

	if (!bUseFOVZoom)
    {
        const float NewArm = FMath::Clamp(SpringArm->TargetArmLength - Amount, MinArmLength, MaxArmLength);
        SpringArm->TargetArmLength = NewArm;
        VirtualDistance = SpringArm->TargetArmLength;
    }
    else
    {
        VirtualDistance = FMath::Clamp(VirtualDistance - Amount, MinArmLength, MaxArmLength);
        UpdateFOVFromVirtualDistance();
    }

	
    // if (bUseFOVZoom)
    // {
    //     const float NewFOV = FMath::Clamp(Camera->FieldOfView - Amount * (FOVZoomSpeed / FMath::Max(ZoomSpeed, 1.f)), MinFOV, MaxFOV);
    //     Camera->SetFieldOfView(NewFOV);
    //     return;
    // }
    //
    // const float NewArm = FMath::Clamp(SpringArm->TargetArmLength - Amount, MinArmLength, MaxArmLength);
    // SpringArm->TargetArmLength = NewArm;
}

void AFreeCameraPawn::GetPlanarViewAxes(FVector& OutForward, FVector& OutRight) const
{
    const FRotator ViewRot = GetControlRotation();
    const FRotationMatrix RM(ViewRot);

    FVector Fwd = RM.GetScaledAxis(EAxis::X);
    FVector Right = RM.GetScaledAxis(EAxis::Y);

    // 投影到水平面，避免 Pitch 造成上下漂移
    Fwd.Z = 0.f; Right.Z = 0.f;
    Fwd = Fwd.GetSafeNormal();
    Right = Right.GetSafeNormal();

    OutForward = Fwd;
    OutRight = Right;
}

void AFreeCameraPawn::ClampControlPitch() const
{
    if (!Controller) return;

    FRotator R = Controller->GetControlRotation();
    R.Pitch = FMath::ClampAngle(R.Pitch, MinPitch, MaxPitch);
    R.Roll = 0.f;
    Controller->SetControlRotation(R);
}

void AFreeCameraPawn::UpdateMouseCapture()
{
    if (!CachedPC.IsValid()) return;

    const bool bAnyDrag = bIsRotating || bIsPanning || bIsZoomingDrag;
    if (bAnyDrag)
    {
        FInputModeGameOnly Mode;
        CachedPC->SetInputMode(Mode);
        CachedPC->bShowMouseCursor = false;
        CachedPC->SetIgnoreLookInput(false);
    }
    else
    {
        FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(false);
        CachedPC->SetInputMode(Mode);
        CachedPC->bShowMouseCursor = true;
    }
}

float AFreeCameraPawn::GetEffectiveDistance() const
{
	return bUseFOVZoom ? VirtualDistance : SpringArm->TargetArmLength;
}

float AFreeCameraPawn::GetDistanceFactor() const
{
	const float Dist = FMath::Max(GetEffectiveDistance(), 1.f);

	if (DistanceScaleCurve)
	{
		const float Alpha = FMath::GetRangePct(CurveDistanceMin, CurveDistanceMax, Dist);
		const float CurveValue = DistanceScaleCurve->GetFloatValue(FMath::Clamp(Alpha, 0.f, 1.f));
		return FMath::Clamp(CurveValue, DistanceScaleMin, DistanceScaleMax);
	}

	// 连续幂函数：factor = clamp( (Dist/ReferenceDistance)^Exponent )
	const float Ratio = Dist / FMath::Max(ReferenceDistance, 1.f);
	const float Raw = FMath::Pow(Ratio, DistanceScaleExponent);
	return FMath::Clamp(Raw, DistanceScaleMin, DistanceScaleMax);
}

void AFreeCameraPawn::UpdateFOVFromVirtualDistance()
{
	const float Alpha = FMath::GetRangePct(MinArmLength, MaxArmLength, VirtualDistance);
	const float NewFOV = FMath::Lerp(MinFOV, MaxFOV, Alpha);
	Camera->SetFieldOfView(NewFOV);
}
