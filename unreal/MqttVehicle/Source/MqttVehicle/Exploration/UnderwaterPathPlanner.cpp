#include "UnderwaterPathPlanner.h"
#include "HAL/PlatformTime.h"
#include "Algo/Reverse.h"

void FUnderwaterPathPlanner::Reset()
{
 Status=EStatus::Idle; Lookup.Reset(); Nodes.Reset(); Open.Reset(); RawPath.Reset(); Path.Reset();
 ExpandedCount=SmoothFrom=SmoothTo=0; GraphDistance=0;
}
void FUnderwaterPathPlanner::Push(const FOpen& Item)
{
 int32 I=Open.Add(Item);
 while(I>0) { const int32 P=(I-1)/2; if(Open[P].F<=Open[I].F) break; Open.Swap(P,I); I=P; }
}
FUnderwaterPathPlanner::FOpen FUnderwaterPathPlanner::Pop()
{
 const FOpen Result=Open[0]; Open[0]=Open.Last(); Open.Pop(EAllowShrinking::No);
 int32 I=0;
 while(I*2+1<Open.Num())
 {
  int32 C=I*2+1; if(C+1<Open.Num() && Open[C+1].F<Open[C].F) ++C;
  if(Open[I].F<=Open[C].F) break; Open.Swap(I,C); I=C;
 }
 return Result;
}
void FUnderwaterPathPlanner::Begin(const FVector& Start,const FVector& End,const FSettings& InSettings,FClearance Clear)
{
 Reset(); Settings=InSettings; Origin=Start; Goal=End; Status=EStatus::Failed;
 if(Start.ContainsNaN() || End.ContainsNaN() || !Settings.Bounds.IsValid
  || Settings.Bounds.Min.ContainsNaN() || Settings.Bounds.Max.ContainsNaN()
  || !FMath::IsFinite(Settings.CellSize) || Settings.CellSize<25 || Settings.MaxNodes<2
  || !Settings.Bounds.IsInsideOrOn(Start) || !Settings.Bounds.IsInsideOrOn(End)) return;
 if(!Clear(Start,Start) || !Clear(End,End)) return;
 if(Clear(Start,End)) { Path={Start,End}; GraphDistance=FVector::Dist(Start,End); Status=EStatus::Succeeded; return; }
 FNode First; First.Cell=FIntVector::ZeroValue; First.Position=Start; First.G=0;
 Nodes.Add(First); Lookup.Add(First.Cell,0);
 FNode Last; Last.Position=End; Nodes.Add(Last);
 Push({FVector::Dist(Start,End),0,0}); Status=EStatus::Searching;
}
void FUnderwaterPathPlanner::BuildPath()
{
 GraphDistance=Nodes[1].G;
 for(int32 I=1; I!=INDEX_NONE; I=Nodes[I].Parent) RawPath.Add(Nodes[I].Position);
 Algo::Reverse(RawPath); Path.Add(RawPath[0]); SmoothFrom=0; SmoothTo=RawPath.Num()-1;
 Status=EStatus::Smoothing;
}
void FUnderwaterPathPlanner::Step(FClearance Clear,int32 MaxExpansions,double Milliseconds)
{
 const double EndTime=FPlatformTime::Seconds()+FMath::Max(0.1,Milliseconds)/1000.0;
 for(int32 Work=0; Work<FMath::Max(1,MaxExpansions) && FPlatformTime::Seconds()<EndTime; ++Work)
 {
  if(Status==EStatus::Smoothing)
  {
   if(Clear(RawPath[SmoothFrom],RawPath[SmoothTo]))
   {
    Path.Add(RawPath[SmoothTo]); SmoothFrom=SmoothTo; SmoothTo=RawPath.Num()-1;
    if(SmoothFrom==RawPath.Num()-1) Status=EStatus::Succeeded;
   }
   else if(--SmoothTo<=SmoothFrom) { Status=EStatus::Failed; Path.Reset(); }
   continue;
  }
  if(Status!=EStatus::Searching) return;
  if(Open.IsEmpty()) { Status=EStatus::Failed; return; }
  const FOpen Item=Pop();
  if(Nodes[Item.Index].Closed || Item.G>Nodes[Item.Index].G+0.001) continue;
  Nodes[Item.Index].Closed=true; ++ExpandedCount;
  if(Item.Index==1) { BuildPath(); continue; }
  const FNode Current=Nodes[Item.Index];
  auto Relax=[&](int32 Next,double Cost)
  {
   const double G=Current.G+Cost;
   if(G+0.001<Nodes[Next].G)
   {
    Nodes[Next].G=G; Nodes[Next].Parent=Item.Index; Nodes[Next].Closed=false;
    Push({G+FVector::Dist(Nodes[Next].Position,Goal),G,Next});
   }
  };
  const double GoalDistance=FVector::Dist(Current.Position,Goal);
  if(GoalDistance<=Settings.CellSize*1.733 && Clear(Current.Position,Goal)) Relax(1,GoalDistance);
  for(int32 X=-1; X<=1; ++X) for(int32 Y=-1; Y<=1; ++Y) for(int32 Z=-1; Z<=1; ++Z)
  {
   if(X==0 && Y==0 && Z==0) continue;
   const FIntVector Cell=Current.Cell+FIntVector(X,Y,Z);
   const FVector Position=Origin+FVector(Cell)*Settings.CellSize;
   if(!Settings.Bounds.IsInsideOrOn(Position)) continue;
   const int32* Found=Lookup.Find(Cell);
   if(Found && Nodes[*Found].Closed) continue;
   if(!Clear(Current.Position,Position)) continue;
   int32 Next;
   if(Found) Next=*Found;
   else
   {
    if(Nodes.Num()>=Settings.MaxNodes) { Status=EStatus::Failed; return; }
    FNode New; New.Cell=Cell; New.Position=Position; Next=Nodes.Add(New); Lookup.Add(Cell,Next);
   }
   Relax(Next,FVector::Dist(Current.Position,Position));
  }
 }
}
double FUnderwaterPathPlanner::Length(const TArray<FVector>& Points)
{
 double Result=0; for(int32 I=1; I<Points.Num(); ++I) Result+=FVector::Dist(Points[I-1],Points[I]); return Result;
}
