// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProjectile.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/Components/PrimitiveComponent.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
AMyProjectile::AMyProjectile()
	:FunctionScale(3.f)
	, FirstStepVelocity(0.5f)
	, SecondStepVelocity(5000.f)
	, SplinePoints(5)
	, TraceLength(1000.f)
	, PassedInitialTrace(false)
	, Target(nullptr)
	, ExplosionRadius(1000.f)
	, ExplosionForce(100000000.f)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComp;

	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AMyProjectile::OnBeginOverlap);				//OnComponentHit.AddDynamic(this, &AMyProjectile::OnHit);
	Collision->AttachToComponent(SceneComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	Projectile = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile"));
	Projectile->AttachToComponent(SceneComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	Trace = CreateDefaultSubobject<USplineComponent>(TEXT("Trace"));
	Trace->RemoveSplinePoint(1);
	Trace->RemoveSplinePoint(0);

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovemetComponent"));
	MovementComponent->bRotationFollowsVelocity = false;

	Sound = CreateDefaultSubobject<USoundWave>(TEXT("Sound"));

	ExplosionEffect = CreateDefaultSubobject<UParticleSystem>(TEXT("ExplosionEffect"));

	Timeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));
	InterpFunction.BindUFunction(this, TEXT("TimelineFloatReturn"));

	static ConstructorHelpers::FObjectFinder<UCurveFloat> curve(TEXT("/Game/Geometry/MyCurve"));
	if (curve.Object) {
		Curve = curve.Object;
	}
	InitialLifeSpan = 5.0f;
}



// Called when the game starts or when spawned
void AMyProjectile::BeginPlay()
{
	Super::BeginPlay();

	FRotator temp = this->GetActorRotation();
	temp.Pitch = 0.f;
	this->SetActorRotation(temp);

	startPosition = this->GetActorLocation();
	startRotation = this->GetActorRotation();


	Timeline->AddInterpFloat(Curve, InterpFunction, TEXT("FloatValue"));
	Timeline->SetPlayRate(FirstStepVelocity);
	Timeline->PlayFromStart();


	Scalar = FunctionScale / TraceLength;
	float x(0), z(0);
	delta_x = (TraceLength / SplinePoints) * Scalar;

	float X = startPosition.X;
	float Y = startPosition.Y;
	float Z = startPosition.Z;

	for (int i(0); i < SplinePoints; i++)
	{
		Trace->AddSplinePoint(startRotation.RotateVector(FVector((x / Scalar), 0.f, (z / Scalar))), ESplineCoordinateSpace::World);
		x += delta_x;
		z = SplineFunction(x);
	}

	//Additional points to make spline smoother
	Trace->AddSplinePoint(Trace->GetLocationAtSplinePoint(SplinePoints - 1, ESplineCoordinateSpace::World) + startRotation.RotateVector(FVector(TraceLength / 5, 0, TraceLength / 5)), ESplineCoordinateSpace::World);
	Trace->AddSplinePoint(Trace->GetLocationAtSplinePoint(SplinePoints, ESplineCoordinateSpace::World) + startRotation.RotateVector(FVector(TraceLength / 5, 0, 0)), ESplineCoordinateSpace::World);

	//It offsets for some reason at random distantion so i repeatedly set it back
	Trace->SetWorldLocation(startPosition);
}

// Called every frame
void AMyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PassedInitialTrace)
	{
		if (Target)
		{
			//Some kind of interpolation
			//It is for smoother targeted flight
			if (decreasingVelocity > 0)
				decreasingVelocity -= SecondStepVelocity * DeltaTime;
			MovementComponent->Velocity = startRotation.RotateVector(FVector(decreasingVelocity, 0.f, 0.f));


			MovementComponent->HomingAccelerationMagnitude = SecondStepVelocity * 100;
		}
		else
		{
			//Just load it forward
			MovementComponent->Velocity = startRotation.RotateVector(FVector(SecondStepVelocity, 0.f, 0.f));
		}
	}
}


// Called when hit
void AMyProjectile::OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!(OtherActor->GetClass()->IsChildOf(this->GetClass())))
	{
		TArray<UPrimitiveComponent*> objectsToAdjustImpulse;
		UGameplayStatics::PlaySoundAtLocation(this, Sound, OtherActor->GetActorLocation(), 2);

		for (TObjectIterator<UPrimitiveComponent> CompItr; CompItr; ++CompItr)
		{
			if (FVector::Distance(Collision->GetComponentLocation(), CompItr->GetComponentLocation()) <= ExplosionRadius)
				objectsToAdjustImpulse.Add(*CompItr);
		}

		for (int i(0); i < objectsToAdjustImpulse.Num(); i++)
			objectsToAdjustImpulse[i]->AddRadialForce(Collision->GetComponentLocation(), ExplosionRadius, ExplosionForce, RIF_Linear);

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, FTransform(Collision->GetComponentLocation()), true);
		this->Destroy();
	}
}


float AMyProjectile::SplineFunction(const float &x)
{
	return x*x;
}


void AMyProjectile::SetTarget(AActor *target)
{
	Target = target;
	if (Target)
	{
		FVector MyPosition(this->GetActorLocation());
		FVector TargetPosition(Target->GetActorLocation());

		TraceLength = FVector::DistXY(MyPosition, TargetPosition) / 10;

		MovementComponent->bIsHomingProjectile = true;
		MovementComponent->HomingAccelerationMagnitude = 0.f;
		MovementComponent->HomingTargetComponent = Target->GetRootComponent();
		decreasingVelocity = SecondStepVelocity;
	}
}

void AMyProjectile::TimelineFloatReturn(float val)
{
	//I took local position and rotation at spline by time
	FVector vector = Trace->GetLocationAtTime(val, ESplineCoordinateSpace::World);
	FRotator rotator = Trace->GetRotationAtTime(val, ESplineCoordinateSpace::Local);

	//And set it to my projectile lile offset
	SceneComp->SetWorldLocation(vector+FVector(10.f,10.f,0));
	SceneComp->SetWorldRotation(rotator);

	//If we went through spline
	if (1 == val)
	{
		PassedInitialTrace = true;
		Trace->DestroyComponent();
		MovementComponent->bRotationFollowsVelocity = true;
	}
}
