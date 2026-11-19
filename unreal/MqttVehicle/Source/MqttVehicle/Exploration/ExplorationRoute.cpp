#include "ExplorationRoute.h"
#include "Engine/World.h"
#include "MqttVehiclePawn.h"

namespace
{
	bool ValidPose(const FVector& Position, const FVector& Forward)
	{
		return !Position.ContainsNaN() && !Forward.ContainsNaN() && Forward.SizeSquared2D() > 0.01;
	}
}

bool FExplorationRoute::Begin(const FVector& Position, const FVector& Forward, const FVector& Home)
{
	Invalidate();
	if (!ValidPose(Position, Forward) || Home.ContainsNaN()
		|| FVector::Dist2D(Position, Home) > 200.0 || FMath::Abs(Position.Z - Home.Z) > 200.0)
	{
		return false;
	}
	Points.Add({Position, Forward.GetSafeNormal2D()});
	LastSample = Position;
	bValid = true;
	return true;
}

bool FExplorationRoute::Record(const FVector& Position, const FVector& Forward, bool bForce)
{
	if (!bValid || Points.IsEmpty() || !ValidPose(Position, Forward)
		|| FVector::Dist(Position, LastSample) > 500.0)
	{
		Invalidate();
		return false;
	}
	LastSample = Position;
	const double Distance = FVector::Dist(Position, Points.Last().Position);
	const bool bTurn = FVector::DotProduct(Forward.GetSafeNormal2D(), Points.Last().Forward) < 0.978;
	if (Distance >= 75.0 || (bTurn && Distance >= 15.0) || (bForce && Distance >= 1.0))
	{
		if (Points.Num() >= 4000)
		{
			// Never silently discard the start of the route to make room.
			Invalidate();
			return false;
		}
		Points.Add({Position, Forward.GetSafeNormal2D()});
	}
	return true;
}

void FExplorationRoute::Invalidate()
{
	Points.Reset();
	bValid = false;
}

bool FExplorationRoute::TrimForManual(int32 TargetIndex, const FVector& Position, const FVector& Forward)
{
	if (!bValid || !Points.IsValidIndex(TargetIndex) || !Points.IsValidIndex(TargetIndex + 1)
		|| !ValidPose(Position, Forward)
		|| ExplorationRoute::DistanceToSegment3D(Position, Points[TargetIndex].Position, Points[TargetIndex + 1].Position) > 250.0)
	{
		Invalidate();
		return false;
	}
	Points.SetNum(TargetIndex + 1);
	LastSample = Points.Last().Position;
	return Record(Position, Forward, true);
}

float FExplorationRoute::RemainingDistance(int32 TargetIndex, const FVector& Position) const
{
	if (!bValid || !Points.IsValidIndex(TargetIndex)) return 0.0f;
	double Distance = FVector::Dist(Position, Points[TargetIndex].Position);
	for (int32 Index = TargetIndex; Index > 0; --Index)
	{
		Distance += FVector::Dist(Points[Index].Position, Points[Index - 1].Position);
	}
	return static_cast<float>(Distance);
}

float ExplorationRoute::ReturnDirection(const FExplorationRoutePoint& From, const FExplorationRoutePoint& To)
{
	return FVector::DotProduct(To.Position - From.Position, From.Forward) >= 0.0 ? 1.0f : -1.0f;
}

TArray<FExplorationRoutePoint> ExplorationRoute::PivotTurn(const FVector& Position, const FVector& Forward, float SignedDegrees)
{
	TArray<FExplorationRoutePoint> Result;
	if (!ValidPose(Position, Forward) || !FMath::IsFinite(SignedDegrees) || FMath::Abs(SignedDegrees) > 180.0f) return Result;
	const int32 Steps = FMath::Max(1, FMath::CeilToInt(FMath::Abs(SignedDegrees) / 2.0f));
	const double Heading = Forward.Rotation().Yaw;
	for (int32 I = 0; I <= Steps; ++I)
	{
		Result.Add({Position, FRotator(0, Heading + SignedDegrees * I / Steps, 0).Vector()});
	}
	return Result;
}

bool ExplorationRoute::IsTurnClear(UWorld& World, const TArray<FExplorationRoutePoint>& Points, const AActor* Vehicle)
{
	if (Points.Num() < 2) return false;
	if (const auto* Pawn = Cast<AMqttVehiclePawn>(Vehicle); Pawn && Pawn->IsUnderwaterMovementEnabled())
	{
		float Angle = 0.0f;
		for (int32 Index=1; Index<Points.Num(); ++Index)
			Angle += FMath::FindDeltaAngleDegrees(Points[Index-1].Forward.Rotation().Yaw, Points[Index].Forward.Rotation().Yaw);
		return Pawn->IsUnderwaterTurnClear(FMath::Clamp(Angle,-180.0f,180.0f));
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ExplorationTurn), false, Vehicle);
	for (const auto& Point : Points)
	{
		if (!ValidPose(Point.Position, Point.Forward)) return false;
		// Root rests ~18 cm above the ground; include low objects and the hull roof.
		// A 10 cm margin covers the gap between adjacent 2-degree orientation samples.
		if (World.OverlapBlockingTestByChannel(Point.Position + FVector(0,0,170),
			Point.Forward.Rotation().Quaternion(), ECC_Vehicle,
			FCollisionShape::MakeBox(FVector(270,240,180)), Params)) return false;
	}
	return true;
}

double ExplorationRoute::DistanceToSegment3D(const FVector& Position, const FVector& A, const FVector& B)
{
	const FVector Delta = B - A;
	const double T = Delta.SizeSquared() > UE_SMALL_NUMBER
		? FMath::Clamp(FVector::DotProduct(Position - A, Delta) / Delta.SizeSquared(), 0.0, 1.0) : 0.0;
	return FVector::Dist(Position, A + T * Delta);
}

double ExplorationRoute::DistanceToSegment2D(const FVector& Position, const FVector& A, const FVector& B)
{
	const FVector2D P(Position.X, Position.Y), Start(A.X, A.Y), Delta(B.X - A.X, B.Y - A.Y);
	const double T = Delta.SizeSquared() > UE_SMALL_NUMBER
		? FMath::Clamp(FVector2D::DotProduct(P - Start, Delta) / Delta.SizeSquared(), 0.0, 1.0) : 0.0;
	return (P - (Start + T * Delta)).Size();
}

FExplorationDriveCommand ExplorationRoute::Follow(const FVector& Position, const FVector& Forward,
	const FVector& Target, float Direction, float ForwardSpeed, float DesiredSpeed)
{
	FExplorationDriveCommand Command;
	if (!ValidPose(Position, Forward) || Target.ContainsNaN() || !FMath::IsFinite(ForwardSpeed)
		|| !FMath::IsFinite(DesiredSpeed) || DesiredSpeed <= 0.0f || FMath::Abs(Direction) != 1.0f)
	{
		return Command;
	}
	const FVector Heading = Forward.GetSafeNormal2D();
	const FVector Right(-Heading.Y, Heading.X, 0.0);
	const FVector Offset = Target - Position;
	const double AlongTravel = FVector::DotProduct(Offset, Heading) * Direction;
	if (AlongTravel < -50.0 || ForwardSpeed * Direction < -20.0f) return Command;
	const double DistanceSquared = FMath::Max(Offset.SizeSquared2D(), 10000.0);
	// Pure-pursuit curvature in the vehicle's forward frame also works while reversing.
	const double Curvature = 2.0 * FVector::DotProduct(Offset, Right) / DistanceSquared;
	Command.Steering = FMath::Clamp(static_cast<float>(FMath::Atan(280.0 * Curvature) / FMath::DegreesToRadians(35.0)), -1.0f, 1.0f);
	// Small tracking corrections should not halve the requested cruise speed.
	// Only tight turns reduce cruise, smoothly down to 80%; arrival braking is separate.
	const float TightTurn = FMath::Clamp((FMath::Abs(Command.Steering) - 0.6f) / 0.4f, 0.0f, 1.0f);
	const float SpeedTarget = DesiredSpeed * (1.0f - 0.2f * TightTurn);
	const float SpeedError = SpeedTarget - ForwardSpeed * Direction;
	Command.Brake = SpeedError < -10.0f ? FMath::Clamp(-SpeedError / 150.0f, 0.0f, 1.0f) : 0.0f;
	// Low proportional input alone cannot launch the Chaos car while steering.
	// Supply rolling resistance compensation and a launch floor, then ease off
	// as speed approaches the target. Overspeed still takes priority over throttle.
	float Throttle = FMath::Clamp(0.18f + SpeedError / 180.0f, 0.0f, 0.65f);
	if (ForwardSpeed * Direction < 30.0f && SpeedError > 20.0f)
	{
		Throttle = FMath::Max(Throttle, 0.5f);
	}
	Command.Throttle = Command.Brake > 0.0f ? 0.0f : Direction * Throttle;
	return Command;
}
