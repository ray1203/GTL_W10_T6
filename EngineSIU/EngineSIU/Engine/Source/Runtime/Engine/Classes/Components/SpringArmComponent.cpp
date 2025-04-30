#include "SpringArmComponent.h"

#include "GameFramework/Pawn.h"
#include "Math/JungleMath.h"
#include "Math/Quat.h"

const FName USpringArmComponent::SocketName(TEXT("SpringEndpoint"));

USpringArmComponent::USpringArmComponent() : Super()
{
    bEnableCameraRotationLag = true;
    bEnableCameraLag = true;
    bUsePawnControlRotation = false;
    // bDoCollisionTest = true;
    SocketOffset = FVector(10, 10, 10);
    TargetOffset = FVector(10, 10, 10);

    bInheritPitch = true;
    bInheritYaw = true;
    bInheritRoll = true;

    TargetArmLength = 300.0f;
    // ProbeSize = 12.0f;
    // ProbeChannel = ECC_Camera;

    // RelativeSocketRotation = FQuat::Identity;

    // bUseCameraLagSubstepping = true;
    CameraLagSpeed = 1.f;
    CameraRotationLagSpeed = 1.f;
    // CameraLagMaxTimeStep = 1.f / 60.f;
    CameraLagMaxDistance = 0.1f;
    // bClampToMaxPhysicsDeltaTime = false;

    UnfixedCameraPosition = FVector::ZeroVector;
}

void USpringArmComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    USceneComponent::GetProperties(OutProperties);

    OutProperties.Add(TEXT("TargetArmLength"), FString::Printf(TEXT("%f"), TargetArmLength));
    OutProperties.Add(TEXT("SocketOffset"), FString::Printf(TEXT("%s"), *SocketOffset.ToString()));
    OutProperties.Add(TEXT("TargetOffset"), FString::Printf(TEXT("%s"), *TargetOffset.ToString()));
    OutProperties.Add(TEXT("bUsePawnControlRotation"), bUsePawnControlRotation ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bInheritPitch"), bInheritPitch ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bInheritYaw"), bInheritYaw ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bInheritRoll"), bInheritRoll ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bEnableCameraLag"), bEnableCameraLag ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bEnableCameraRotationLag"), bEnableCameraRotationLag ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("CameraLagSpeed"), FString::Printf(TEXT("%f"), CameraLagSpeed));
    OutProperties.Add(TEXT("CameraRotationLagSpeed"), FString::Printf(TEXT("%f"), CameraRotationLagSpeed));
    OutProperties.Add(TEXT("CameraLagMaxDistance"), FString::Printf(TEXT("%f"), CameraLagMaxDistance));
}

void USpringArmComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    USceneComponent::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("TargetArmLength"));
    if (TempStr)
    {
        TargetArmLength = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("SocketOffset"));
    if (TempStr)
    {
        SocketOffset.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("TargetOffset"));
    if (TempStr)
    {
        TargetOffset.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("bUsePawnControlRotation"));
    if (TempStr)
    {
        bUsePawnControlRotation = (*TempStr == TEXT("true"));
    }
    TempStr = InProperties.Find(TEXT("bInheritPitch"));
    if (TempStr)
    {
        bInheritPitch = (*TempStr == TEXT("true"));
    }
    TempStr = InProperties.Find(TEXT("bInheritYaw"));
    if (TempStr)
    {
        bInheritYaw = (*TempStr == TEXT("true"));
    }
    TempStr = InProperties.Find(TEXT("bInheritRoll"));
    if (TempStr)
    {
        bInheritRoll = (*TempStr == TEXT("true"));
    }
    TempStr = InProperties.Find(TEXT("bEnableCameraLag"));
    if (TempStr)
    {
        bEnableCameraLag = (*TempStr == TEXT("true"));
    }
    TempStr = InProperties.Find(TEXT("bEnableCameraRotationLag"));
    if (TempStr)
    {
        bEnableCameraRotationLag = (*TempStr == TEXT("true"));
    }
    TempStr = InProperties.Find(TEXT("CameraLagSpeed"));
    if (TempStr)
    {
        CameraLagSpeed = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("CameraRotationLagSpeed"));
    if (TempStr)
    {
        CameraRotationLagSpeed = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("CameraLagMaxDistance"));
    if (TempStr)
    {
        CameraLagMaxDistance = FString::ToFloat(*TempStr);
    }
}

FRotator USpringArmComponent::GetTargetRotation() const
{
    FRotator DesiredRot = GetDesiredRotation();

    if (bUsePawnControlRotation)
    {
        if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
        {
            const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
            if (DesiredRot != PawnViewRotation)
            {
                DesiredRot = PawnViewRotation;
            }
        }
    }

    // If inheriting rotation, check options for which components to inherit
    // if (!IsUsingAbsoluteRotation())
    {
        const FRotator LocalRelativeRotation = GetRelativeRotation();
        if (!bInheritPitch)
        {
            DesiredRot.Pitch = LocalRelativeRotation.Pitch;
        }

        if (!bInheritYaw)
        {
            DesiredRot.Yaw = LocalRelativeRotation.Yaw;
        }

        if (!bInheritRoll)
        {
            DesiredRot.Roll = LocalRelativeRotation.Roll;
        }
    }

    return DesiredRot;
}

void USpringArmComponent::InitializeComponent()
{
    USceneComponent::InitializeComponent();
    
    //CameraLagMaxTimeStep = FMath::Max(CameraLagMaxTimeStep, 1.f / 200.f);
    CameraLagSpeed = FMath::Max(CameraLagSpeed, 0.f);

    // Set initial location (without lag).
    UpdateDesiredArmLocation(false, false, 0.f);
    GetOwner()->SetActorTickInEditor(true);
}

void USpringArmComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    UpdateDesiredArmLocation(bEnableCameraLag, bEnableCameraRotationLag, DeltaTime);
}

// void USpringArmComponent::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
// {
//     Super::ApplyWorldOffset(InOffset, bWorldShift);
//     PreviousDesiredLoc += InOffset;
//     PreviousArmOrigin += InOffset;
// }

FRotator USpringArmComponent::GetDesiredRotation() const
{
    return GetWorldRotation();
}

void USpringArmComponent::UpdateDesiredArmLocation(bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
    FRotator DesiredRot = GetTargetRotation();

	// If our viewtarget is simulating using physics, we may need to clamp deltatime
	// if (bClampToMaxPhysicsDeltaTime)
	// {
	// 	// Use the same max timestep cap as the physics system to avoid camera jitter when the viewtarget simulates less time than the camera
	// 	DeltaTime = FMath::Min(DeltaTime, UPhysicsSettings::Get()->MaxPhysicsDeltaTime);
	// }

	// Apply 'lag' to rotation if desired
	if(bDoRotationLag)
	{
		// if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.f)
		// {
		// 	const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
		// 	FRotator LerpTarget = PreviousDesiredRot;
		// 	float RemainingTime = DeltaTime;
		// 	while (RemainingTime > UE_KINDA_SMALL_NUMBER)
		// 	{
		// 		const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
		// 		LerpTarget += ArmRotStep * LerpAmount;
		// 		RemainingTime -= LerpAmount;
		//
		// 		DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(LerpTarget), LerpAmount, CameraRotationLagSpeed));
		// 		PreviousDesiredRot = DesiredRot;
		// 	}
		// }
		// else
		{
			DesiredRot = FRotator(JungleMath::QInterpTo(PreviousDesiredRot.ToQuaternion(), DesiredRot.ToQuaternion(), DeltaTime, CameraRotationLagSpeed));
		}
	}
	PreviousDesiredRot = DesiredRot;

	// Get the spring arm 'origin', the target we want to look at
	FVector ArmOrigin = GetWorldLocation() + TargetOffset;
	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = ArmOrigin;
	if (bDoLocationLag)
	{
		// if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
		// {
		// 	const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
		// 	FVector LerpTarget = PreviousDesiredLoc;
		//
		// 	float RemainingTime = DeltaTime;
		// 	while (RemainingTime > KINDA_SMALL_NUMBER)
		// 	{
		// 		const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
		// 		LerpTarget += ArmMovementStep * LerpAmount;
		// 		RemainingTime -= LerpAmount;
		//
		// 		DesiredLoc = JungleMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
		// 		PreviousDesiredLoc = DesiredLoc;
		// 	}
		// }
		// else
		{
		    DesiredLoc = JungleMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}		

// #if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
// 		if (bDrawDebugLagMarkers)
// 		{
// 			DrawDebugSphere(GetWorld(), ArmOrigin, 5.f, 8, FColor::Green);
// 			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);
//
// 			const FVector ToOrigin = ArmOrigin - DesiredLoc;
// 			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc, DesiredLoc + ToOrigin * 0.5f, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
// 			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc + ToOrigin * 0.5f, ArmOrigin,  7.5f, bClampedDist ? FColor::Red : FColor::Green);
// 		}
// #endif
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	// Now offset camera position back along our rotation
	DesiredLoc -= DesiredRot.Vector() * TargetArmLength;
	// Add socket offset in local space
	DesiredLoc += FMatrix::TransformVector(SocketOffset, FRotationMatrix(DesiredRot, FVector::ZeroVector));

	// Do a sweep to ensure we are not penetrating the world
	FVector ResultLoc;
	// if (bDoTrace && (TargetArmLength != 0.0f))
	// {
	// 	// bIsCameraFixed = true;
	// 	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
	//
	// 	FHitResult Result;
	// 	GetWorld()->SweepSingleByChannel(Result, ArmOrigin, DesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);
	// 	
	// 	UnfixedCameraPosition = DesiredLoc;
	//
	// 	ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);
	//
	// 	if (ResultLoc == DesiredLoc) 
	// 	{	
	// 		bIsCameraFixed = false;
	// 	}
	// }
	// else
	{
		ResultLoc = DesiredLoc;
		bIsCameraFixed = false;
		UnfixedCameraPosition = ResultLoc;
	}

	// // Form a transform for new world transform for camera
	// FTransform WorldCamTM(DesiredRot, ResultLoc);
	// // Convert to relative to component
	// FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentTransform());
	//
	// // Update socket location/rotation
	// RelativeSocketLocation = RelCamTM.GetLocation();
	// RelativeSocketRotation = RelCamTM.GetRotation();

	// UpdateChildTransforms();

    for (auto* Child : AttachChildren)
    {
        Child->SetWorldLocation(ResultLoc);
        Child->SetWorldRotation(DesiredRot);
    } 
}

FVector USpringArmComponent::BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime)
{
    return bHitSomething ? TraceHitLocation : DesiredArmLocation;
}

// FTransform USpringArmComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
// {
//     FTransform RelativeTransform(RelativeSocketRotation, RelativeSocketLocation);
//
//     switch(TransformSpace)
//     {
//     case RTS_World:
//         {
//             return RelativeTransform * GetComponentTransform();
//             break;
//         }
//     case RTS_Actor:
//         {
//             if( const AActor* Actor = GetOwner() )
//             {
//                 FTransform SocketTransform = RelativeTransform * GetComponentTransform();
//                 return SocketTransform.GetRelativeTransform(Actor->GetTransform());
//             }
//             break;
//         }
//     case RTS_Component:
//         {
//             return RelativeTransform;
//         }
//     }
//     return RelativeTransform;
// }

// bool USpringArmComponent::HasAnySockets() const
// {
//     return true;
// }
//
// void USpringArmComponent::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
// {
//     new (OutSockets) FComponentSocketDescription(SocketName, EComponentSocketType::Socket);
// }

FVector USpringArmComponent::GetUnfixedCameraPosition() const
{
    return UnfixedCameraPosition;
}

bool USpringArmComponent::IsCollisionFixApplied() const
{
    return bIsCameraFixed;
}

FMatrix USpringArmComponent::FRotationMatrix(const FRotator& Rot, const FVector& Origin) const
{
    // #if PLATFORM_ENABLE_VECTORINTRINSICS && (!PLATFORM_HOLOLENS || !PLATFORM_CPU_ARM_FAMILY)
    //
    //     const TVectorRegisterType<T> Angles = MakeVectorRegister(Rot.Pitch, Rot.Yaw, Rot.Roll, 0.0f);
    //     const TVectorRegisterType<T> HalfAngles = VectorMultiply(Angles, GlobalVectorConstants::DEG_TO_RAD);
    //
    //     union { TVectorRegisterType<T> v; T f[4]; } SinAngles, CosAngles;
    //     VectorSinCos(&SinAngles.v, &CosAngles.v, &HalfAngles);
    //
    //     const T SP	= SinAngles.f[0];
    //     const T SY	= SinAngles.f[1];
    //     const T SR	= SinAngles.f[2];
    //     const T CP	= CosAngles.f[0];
    //     const T CY	= CosAngles.f[1];
    //     const T CR	= CosAngles.f[2];
    //
    // #else

    float SP, SY, SR;
    float CP, CY, CR;
    FMath::SinCos(&SP, &CP, (float)FMath::DegreesToRadians(Rot.Pitch));
    FMath::SinCos(&SY, &CY, (float)FMath::DegreesToRadians(Rot.Yaw));
    FMath::SinCos(&SR, &CR, (float)FMath::DegreesToRadians(Rot.Roll));

    // #endif // PLATFORM_ENABLE_VECTORINTRINSICS
    FMatrix M;
    M[0][0]	= CP * CY;
    M[0][1]	= CP * SY;
    M[0][2]	= SP;
    M[0][3]	= 0.f;

    M[1][0]	= SR * SP * CY - CR * SY;
    M[1][1]	= SR * SP * SY + CR * CY;
    M[1][2]	= - SR * CP;
    M[1][3]	= 0.f;

    M[2][0]	= -( CR * SP * CY + SR * SY );
    M[2][1]	= CY * SR - CR * SP * SY;
    M[2][2]	= CR * CP;
    M[2][3]	= 0.f;

    M[3][0]	= Origin.X;
    M[3][1]	= Origin.Y;
    M[3][2]	= Origin.Z;
    M[3][3]	= 1.f;

    return M;
}
