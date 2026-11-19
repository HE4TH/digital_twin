#pragma once

#include "CoreMinimal.h"

/** Local-space box entry policy. No collision callbacks, world, network or vehicle output. */
struct FExplorationDangerZoneState
{
	bool bInside = false;
	bool bLatched = false;
	bool bHasPrevious = false;
	FVector Previous = FVector::ZeroVector;

	// Emit once per entry. Exit needs 100 cm clearance; continuous samples also detect pass-through.
	bool Update(const FVector& Position, const FVector& Extent);
	void BreakSample() { bHasPrevious = false; bInside = false; }
};
