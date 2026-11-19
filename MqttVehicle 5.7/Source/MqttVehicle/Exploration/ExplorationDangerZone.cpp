#include "ExplorationDangerZone.h"

namespace
{
	bool Inside(const FVector& P, const FVector& E)
	{
		return FMath::Abs(P.X) <= E.X && FMath::Abs(P.Y) <= E.Y && FMath::Abs(P.Z) <= E.Z;
	}

	bool CrossesBox(const FVector& From, const FVector& To, const FVector& Extent)
	{
		double Enter = 0.0, Leave = 1.0;
		for (int32 Axis = 0; Axis < 3; ++Axis)
		{
			const double Delta = To[Axis] - From[Axis];
			if (FMath::Abs(Delta) < UE_DOUBLE_SMALL_NUMBER)
			{
				if (FMath::Abs(From[Axis]) > Extent[Axis]) return false;
				continue;
			}
			const double A = (-Extent[Axis] - From[Axis]) / Delta;
			const double B = (Extent[Axis] - From[Axis]) / Delta;
			Enter = FMath::Max(Enter, FMath::Min(A, B));
			Leave = FMath::Min(Leave, FMath::Max(A, B));
			if (Enter > Leave) return false;
		}
		return true;
	}
}

bool FExplorationDangerZoneState::Update(const FVector& Position, const FVector& Extent)
{
	if (Position.ContainsNaN() || Extent.ContainsNaN() || Extent.GetMin() <= 0.0)
	{
		BreakSample(); // An invalid sample must not re-arm a cancelled entry.
		return false;
	}
	const bool bInner = Inside(Position, Extent);
	const bool bOuter = Inside(Position, Extent + FVector(100.0));
	const bool bCrossed = bHasPrevious && FVector::DistSquared(Position, Previous) <= FMath::Square(500.0)
		&& CrossesBox(Previous, Position, Extent);
	const bool bEntered = !bLatched && (bInner || bCrossed);

	bInside = bLatched ? bOuter : bInner;
	if (bEntered) bLatched = true;
	else if (!bOuter) bLatched = false;
	Previous = Position;
	bHasPrevious = true;
	return bEntered;
}
