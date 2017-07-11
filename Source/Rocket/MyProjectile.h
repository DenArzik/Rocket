// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundWave.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "MyProjectile.generated.h"

UCLASS()
class ROCKET_API AMyProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Components")
		UCapsuleComponent *Collision;

	UPROPERTY(EditAnywhere, Category = "Components")
		UStaticMeshComponent *Projectile;

	//Describes movement for our projectile
	UPROPERTY(BlueprintReadWrite)
		USplineComponent *Trace;

	//Second step movement is described by this
	UPROPERTY(BlueprintReadWrite)
		UProjectileMovementComponent *MovementComponent;

	//Scale of function we take (range of x values)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1.0", ClampMax = "10.0"), Category = "Movement")
		float FunctionScale;

	//Speed of our projectile while moving upwards
	UPROPERTY(EditAnywhere, Category = "Movement")
		float FirstStepVelocity;

	//Speed of directional flight
	UPROPERTY(EditAnywhere, Category = "Movement")
		float SecondStepVelocity;

	//Another class should set it before spawning (don't know how to insert it in constructor)
	UFUNCTION(BlueprintCallable)
		void SetTarget(AActor *target);

	//We'll play it on hit
	UPROPERTY(EditAnywhere, Category = "Hit")
		USoundWave *Sound;

	//Same as upper one
	UPROPERTY(EditAnywhere, Category = "Hit")
		UParticleSystem *ExplosionEffect;

	//Curve to describe speed speed of moving across our trace
	UCurveFloat *Curve;

	//We get position on our spline during float of time with that variable
	UTimelineComponent *Timeline;

	//We bind function that uses our time value to that struct (maybe it is used for multiple functionst, but not in our case:) )
	FOnTimelineFloat InterpFunction{};

	//Function that recieves timeline value as parametr
	UFUNCTION()
		void TimelineFloatReturn(float val);

	UFUNCTION()
		void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//OnHit
	UPROPERTY(EditAnywhere, Category = "Hit")
		float ExplosionRadius;
	UPROPERTY(EditAnywhere, Category = "Hit")
		float ExplosionForce;

private:

	USceneComponent *SceneComp;

	//Describes how many points our spline will be consist of
	int SplinePoints;

	//Modifier that is used to scale the function value and world coordinates value
	float Scalar;

	//Step length for modifying x
	float delta_x;

	//Function to describe our spline
	float SplineFunction(const float&);

	//Length of our trace spline (x)
	float TraceLength;

	//Pointer to our projectile`s targer (will get it from weapon) 
	AActor *Target;

	//If our projectile has went through our trace
	bool PassedInitialTrace;

	//Nuff said
	FVector startPosition;
	FRotator startRotation;

	//To direct and rotate projectile towards ratget
	float decreasingVelocity;
	FRotator ToTargetRotation;

	bool kostil;
};
