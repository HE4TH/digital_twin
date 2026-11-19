#pragma once

#include "CoreMinimal.h"

class UWorld;
class AActor;

struct FExplorationRoutePoint
{
	FVector Position = FVector::ZeroVector;
	FVector Forward = FVector::ForwardVector;
};

/** Bounded history of actual poses, in outbound order. Never uses the scene's guide markers. */
struct FExplorationRoute
{
	TArray<FExplorationRoutePoint> Points;
	bool bValid = false;
	FVector LastSample = FVector::ZeroVector;

	bool Begin(const FVector& Position, const FVector& Forward, const FVector& Home);
	bool Record(const FVector& Position, const FVector& Forward, bool bForce = false);
	void Invalidate();
	bool TrimForManual(int32 TargetIndex, const FVector& Position, const FVector& Forward);
	float RemainingDistance(int32 TargetIndex, const FVector& Position) const;
};

struct FExplorationDriveCommand
{
	float Throttle = 0.0f;
	float Steering = 0.0f;
	float Brake = 1.0f;
};

namespace ExplorationRoute
{
	/** Sample a stationary yaw without translating; signed degrees select left/right. */
	TArray<FExplorationRoutePoint> PivotTurn(const FVector& Position, const FVector& Forward, float SignedDegrees);
	/** Check the rotated hull footprint, including low obstacles just above the floor. */
	bool IsTurnClear(UWorld& World, const TArray<FExplorationRoutePoint>& Points, const AActor* Vehicle);
	/** Reverse the original segment travel, retaining the original vehicle orientation. */
	float ReturnDirection(const FExplorationRoutePoint& From, const FExplorationRoutePoint& To);
	double DistanceToSegment2D(const FVector& Position, const FVector& A, const FVector& B);
	double DistanceToSegment3D(const FVector& Position, const FVector& A, const FVector& B);
	FExplorationDriveCommand Follow(const FVector& Position, const FVector& Forward,
		const FVector& Target, float Direction, float ForwardSpeed, float DesiredSpeed);
}
