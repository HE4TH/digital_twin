#pragma once
#include "CoreMinimal.h"

/** Incremental 26-neighbour 3D A*. All edges, including shortcuts, require hull clearance. */
class FUnderwaterPathPlanner
{
public:
 enum class EStatus { Idle, Searching, Smoothing, Succeeded, Failed };
 struct FSettings
 {
  double CellSize = 150.0;
  int32 MaxNodes = 30000;
  FBox Bounds = FBox(ForceInit);
 };
 using FClearance = TFunctionRef<bool(const FVector&, const FVector&)>;
 void Begin(const FVector& Start, const FVector& Goal, const FSettings& Settings, FClearance Clear);
 void Step(FClearance Clear, int32 MaxExpansions = 48, double Milliseconds = 2.0);
 void Reset();
 EStatus GetStatus() const { return Status; }
 const TArray<FVector>& GetPath() const { return Path; }
 int32 GetExpandedCount() const { return ExpandedCount; }
 double GetGraphDistance() const { return GraphDistance; }
 static double Length(const TArray<FVector>& Points);
private:
 struct FNode { FIntVector Cell; FVector Position; double G = TNumericLimits<double>::Max(); int32 Parent = INDEX_NONE; bool Closed = false; };
 struct FOpen { double F; double G; int32 Index; };
 void Push(const FOpen& Item);
 FOpen Pop();
 void BuildPath();
 FSettings Settings;
 FVector Origin = FVector::ZeroVector, Goal = FVector::ZeroVector;
 EStatus Status = EStatus::Idle;
 TMap<FIntVector,int32> Lookup;
 TArray<FNode> Nodes;
 TArray<FOpen> Open;
 TArray<FVector> RawPath, Path;
 int32 ExpandedCount = 0, SmoothFrom = 0, SmoothTo = 0;
 double GraphDistance = 0;
};
