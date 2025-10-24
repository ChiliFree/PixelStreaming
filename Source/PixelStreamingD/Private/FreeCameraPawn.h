// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FreeCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class PIXELSTREAMINGD_API AFreeCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFreeCameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UCameraComponent* Camera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Look")
    float LookSensitivity = 0.65f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Look")
    bool bInvertY = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Look")
    float MinPitch = -80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Look")
    float MaxPitch = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Pan")
    float PanSpeed = 100000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Zoom")
    float ZoomSpeed = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Zoom")
    float MinArmLength = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Zoom")
    float MaxArmLength = 100000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Zoom")
    bool bUseFOVZoom = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Zoom", meta=(EditCondition="bUseFOVZoom"))
    float FOVZoomSpeed = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Zoom", meta=(EditCondition="bUseFOVZoom"))
    float MinFOV = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Zoom", meta=(EditCondition="bUseFOVZoom"))
    float MaxFOV = 90.f;
	
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling")
    bool bScalePanByDistance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling")
    bool bScaleZoomByDistance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling")
    float ReferenceDistance = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling", meta=(ClampMin="0.1", ClampMax="4.0"))
    float DistanceScaleExponent = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling")
    float DistanceScaleMin = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling")
    float DistanceScaleMax = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling")
    UCurveFloat* DistanceScaleCurve = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling", meta=(EditCondition="DistanceScaleCurve!=nullptr"))
    float CurveDistanceMin = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceScaling", meta=(EditCondition="DistanceScaleCurve!=nullptr"))
    float CurveDistanceMax = 5000.f;

private:
    bool bIsRotating = false;
    bool bIsPanning = false;
    bool bIsZoomingDrag = false;
	
    float VirtualDistance = 800.f;

    // Cached controller
    TWeakObjectPtr<APlayerController> CachedPC;

    void OnLMBPressed();
    void OnLMBReleased();
    void OnRMBPressed();
    void OnRMBReleased();
    void OnMMBPressed();
    void OnMMBReleased();
    void OnMouseWheelAxis(float Value);

    void ApplyLook(const FVector2D& MouseDelta, float DT);
    void ApplyPan(const FVector2D& MouseDelta, float DT);
    void ApplyZoom(float Axis, bool bFromDrag, float DT);

    void GetPlanarViewAxes(FVector& OutForward, FVector& OutRight) const;
    void ClampControlPitch() const;
    void UpdateMouseCapture();

	float GetEffectiveDistance() const;
	float GetDistanceFactor() const;
	void UpdateFOVFromVirtualDistance();

	void CancelAllInteractions();

	bool ValidateDragState();
	
};
