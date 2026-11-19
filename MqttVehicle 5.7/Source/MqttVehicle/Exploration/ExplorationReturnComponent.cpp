#include "ExplorationReturnComponent.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MqttVehicleBridgeSubsystem.h"
#include "MqttVehiclePawn.h"
#include "MqttVehicle.h"
#include "UnrealClient.h"
#include "WhaleObservationComponent.h"

#define LOCTEXT_NAMESPACE "ExplorationReturn"

UExplorationReturnComponent::UExplorationReturnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UExplorationReturnComponent::BeginPlay()
{
	Super::BeginPlay();
	const APlayerController* PC = Cast<APlayerController>(GetOwner());
	bActive = PC && PC->IsLocalPlayerController() && GetWorld()
		&& (GetWorld()->WorldType == EWorldType::Game || GetWorld()->WorldType == EWorldType::PIE);
	SetComponentTickEnabled(bActive);
	if (!bActive) return;
	Observation = GetOwner()->FindComponentByClass<UWhaleObservationComponent>();
	if (Observation.IsValid())
	{
		Observation->OnWhaleDiscovered.AddDynamic(this, &UExplorationReturnComponent::OnWhaleDiscovered);
		Observation->OnObservationZoneChanged.AddDynamic(this, &UExplorationReturnComponent::OnObservationZoneChanged);
		AddTickPrerequisiteComponent(Observation.Get());
	}
	RefreshHome();
	RefreshDangerZones();
	SetVehicle(Cast<AMqttVehiclePawn>(PC->GetPawn()));
}

void UExplorationReturnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bActive = false;
	if (Observation.IsValid())
	{
		Observation->OnWhaleDiscovered.RemoveDynamic(this, &UExplorationReturnComponent::OnWhaleDiscovered);
		Observation->OnObservationZoneChanged.RemoveDynamic(this, &UExplorationReturnComponent::OnObservationZoneChanged);
	}
	if (Vehicle.IsValid())
	{
		Vehicle->OnExplorationControlInterrupted.Remove(InterruptionHandle);
		if (IsAutomatic()) Vehicle->EndExplorationControl(true);
	}
	Super::EndPlay(EndPlayReason);
}

bool UExplorationReturnComponent::IsAutomatic() const
{
	return State.Mode == EExplorationReturnMode::PreparingReturn || State.Mode == EExplorationReturnMode::Returning || State.Mode == EExplorationReturnMode::Planning;
}

bool UExplorationReturnComponent::CanDriveNow() const
{
 return GetControlBlockReason().IsEmpty();
}

FText UExplorationReturnComponent::GetControlBlockReason() const
{
 const UWorld* World = GetWorld();
 const APlayerController* PC = Cast<APlayerController>(GetOwner());
 const UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
 if (!World || !PC || !PC->IsLocalPlayerController())
  return LOCTEXT("NoLocalControl", "탐사선 제어 준비 중입니다. 잠시 후 다시 요청하세요.");
 if (World->IsPaused() || PC->IsMoveInputIgnored())
  return LOCTEXT("PausedControl", "일시정지를 해제한 뒤 다시 복귀를 요청하세요.");
 if (!MqttVehicleControl::HasFocus(Viewport && Viewport->Viewport && Viewport->Viewport->HasFocus()))
  return LOCTEXT("NoFocusControl", "VR 앱 화면으로 돌아온 뒤 복귀를 다시 요청하세요.");
 const auto* Bridge = World->GetSubsystem<UMqttVehicleBridgeSubsystem>();
 if (Bridge && !Bridge->IsLocalReturnConnectionReady())
  return Bridge->bReceiveVehicleCommands
   ? LOCTEXT("RemoteControlConflict", "외부 차량 명령 수신 중에는 자동복귀를 시작할 수 없습니다.")
   : LOCTEXT("BrokerRequired", "RC 송신이 켜져 있지만 MQTT 연결이 없습니다. 연결을 복구하거나 대시보드에서 RC 송신을 끈 뒤 다시 요청하세요.");
 return FText::GetEmpty();
}

void UExplorationReturnComponent::SetVehicle(AMqttVehiclePawn* NewVehicle)
{
	if (Vehicle.IsValid())
	{
		Vehicle->OnExplorationControlInterrupted.Remove(InterruptionHandle);
		if (IsAutomatic()) Vehicle->EndExplorationControl(true);
	}
	Vehicle = NewVehicle;
	Navigator.Reset(); NavigationPath.Reset(); bNavigationReturn = bNavigationHomeValid = false;
	bTurningInPlace = false;
	bArrivalTurn = false;
	Route.Invalidate();
	for (auto& Pair : DangerZones) Pair.Value = FExplorationDangerZoneState();
	State.bInsideDangerZone = false;
	TargetIndex = INDEX_NONE;
	SwimAimIndex = INDEX_NONE;
	State.Mode = EExplorationReturnMode::Unavailable;
	State.Detail = LOCTEXT("NeedStart", "출발 패드에서 주행 기록을 시작하세요.");
	if (Vehicle.IsValid())
	{
		VehicleResetRevision = Vehicle->GetExplorationResetRevision();
		InterruptionHandle = Vehicle->OnExplorationControlInterrupted.AddUObject(this, &UExplorationReturnComponent::OnVehicleInterrupted);
		RecordPose();
	}
}

void UExplorationReturnComponent::RefreshHome()
{
	TArray<AActor*> Homes;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("ExplorationReturnAnchor"), Homes);
	AActor* Found = Homes.Num() == 1 ? Homes[0] : nullptr;
	const bool bChanged = Found != HomeAnchor.Get() || (Found && !Found->GetActorLocation().Equals(HomePosition, 1.0));
	HomeAnchor = Found;
	if (Found) HomePosition = Found->GetActorLocation();
	if (bChanged)
	{
		bNavigationHomeValid = false;
		if (IsAutomatic()) StopWithReason(LOCTEXT("HomeChanged", "출발점이 변경되어 복귀를 중단했습니다."), true);
		else Route.Invalidate();
	}
	if (!Found && !IsAutomatic())
	{
		Route.Invalidate();
		State.Mode = EExplorationReturnMode::Unavailable;
		State.Detail = LOCTEXT("MissingHome", "출발점 태그는 정확히 하나여야 합니다.");
	}
}

void UExplorationReturnComponent::RefreshDangerZones()
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("ExplorationDangerZone"), Found);
	for (auto It = DangerZones.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !Found.Contains(It.Key().Get())) It.RemoveCurrent();
	}
	for (AActor* Actor : Found)
	{
		if (Cast<ATriggerBox>(Actor)) DangerZones.FindOrAdd(TWeakObjectPtr<AActor>(Actor));
	}
}

void UExplorationReturnComponent::UpdateDangerZones()
{
	if (GetWorld()->IsPaused()) return;
	State.bInsideDangerZone = false;
	if (!Vehicle.IsValid()) return;
	bool bEntered = false;
	for (auto& Pair : DangerZones)
	{
		const auto* Trigger = Cast<ATriggerBox>(Pair.Key.Get());
		const auto* Box = Trigger ? Cast<UBoxComponent>(Trigger->GetCollisionComponent()) : nullptr;
		if (!Box || Box->GetComponentTransform().ContainsNaN()
			|| Box->GetComponentScale().GetAbsMin() < UE_DOUBLE_SMALL_NUMBER)
		{
			Pair.Value.BreakSample();
			continue;
		}
		// TriggerBoxes are fixed scene volumes. Geometry is queried even with collision disabled.
		const FVector Local = Box->GetComponentTransform().InverseTransformPosition(Vehicle->GetActorLocation());
		bEntered |= Pair.Value.Update(Local, Box->GetUnscaledBoxExtent());
		State.bInsideDangerZone |= Pair.Value.bInside;
	}
	// Multiple simultaneous entries produce one request; remaining inside never restarts a cancelled return.
	if (bEntered)
	{
		++State.DangerEncounterSerial;
		StartReturn(EExplorationReturnReason::DangerZone);
	}
}

void UExplorationReturnComponent::RecordPose(bool bForce)
{
	if (!Vehicle.IsValid() || !HomeAnchor.IsValid()) return;
	const FVector Position = Vehicle->GetActorLocation();
	const FVector Forward = Vehicle->GetActorForwardVector();
 if(Vehicle->IsUnderwaterMovementEnabled())
 {
  // Capture the actual departure pose once; no outbound history is required for A*.
  if(!bNavigationHomeValid && FVector::Dist2D(Position,HomePosition)<=500.0
   && FMath::Abs(Position.Z-HomePosition.Z)<=500.0)
  {
   NavigationHome=Position; bNavigationHomeValid=true;
   State.Mode=EExplorationReturnMode::Manual;
   State.Detail=LOCTEXT("NavigationReady","출발점을 저장했습니다. H로 경로를 계산해 복귀합니다.");
  }
  return;
 }
	if (!Route.bValid)
	{
		if (Route.Begin(Position, Forward, HomePosition))
		{
			State.Mode = EExplorationReturnMode::Manual;
			State.Detail = LOCTEXT("Recording", "수동 탐사 경로를 기록하고 있습니다.");
		}
	}
	else if (!Route.Record(Position, Forward, bForce))
	{
		StopWithReason(LOCTEXT("RouteInvalid", "경로 기록이 끊기거나 한도에 도달했습니다. 출발점에서 다시 시작하세요."), true);
	}
}

void UExplorationReturnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bActive) return;
	const APlayerController* PC = Cast<APlayerController>(GetOwner());
	AMqttVehiclePawn* Current = PC ? Cast<AMqttVehiclePawn>(PC->GetPawn()) : nullptr;
	if (Current != Vehicle.Get()) SetVehicle(Current);
	if (GetWorld()->GetTimeSeconds() >= NextHomeSearch)
	{
		RefreshHome();
		RefreshDangerZones();
		NextHomeSearch = GetWorld()->GetTimeSeconds() + 2.0;
	}
	if (Vehicle.IsValid() && VehicleResetRevision != Vehicle->GetExplorationResetRevision())
	{
		VehicleResetRevision = Vehicle->GetExplorationResetRevision();
		for (auto& Pair : DangerZones) Pair.Value.BreakSample();
		StopWithReason(LOCTEXT("VehicleReset", "차량 자세 복구로 경로 기록을 초기화했습니다. 출발점에서 다시 시작하세요."), true);
	}
	if (IsAutomatic() && (!Vehicle.IsValid() || !HomeAnchor.IsValid() || !CanDriveNow()
		|| !Vehicle->IsExplorationControlActive()))
	{
		StopWithReason(LOCTEXT("Interrupted", "포커스·일시정지·통신 또는 제어권 변경으로 복귀를 중단했습니다."));
	}
	UpdateDangerZones();
	if (bDiscoveryPending)
	{
		bDiscoveryPending = false;
		StartReturn(EExplorationReturnReason::WhaleDiscovery);
	}
	if (Vehicle.IsValid())
	{
		if ((State.Mode == EExplorationReturnMode::Stopped || State.Mode == EExplorationReturnMode::Arrived)
			&& Vehicle->IsManualDriveRequested())
		{
			State.Mode = EExplorationReturnMode::Manual;
			State.Detail = LOCTEXT("ManualDriving", "수동 탐사 중입니다.");
		}
		if (State.Mode == EExplorationReturnMode::PreparingReturn)
		{
			Vehicle->ApplyExplorationControl(0.0f, 0.0f, 1.0f, true);
			RecordPose();
			if (IsAutomatic())
			{
				Countdown = FMath::Max(0.0f, Countdown - DeltaTime);
				PreparationSeconds += DeltaTime;
				if (PreparationSeconds > 15.0f) StopWithReason(LOCTEXT("CannotStop", "차량 정지를 확인하지 못해 복귀를 중단했습니다."));
				else if (Countdown <= 0.0f && Vehicle->GetVelocity().Size() < 10.0)
				{
					RecordPose(true);
					if (IsAutomatic())
					{
						if(Vehicle->IsUnderwaterMovementEnabled()) { BeginNavigationPlan(); }
						else
						{
						TargetIndex = Route.Points.Num() - 2;
						State.Mode = EExplorationReturnMode::Returning;
						BestRemainingDistance = Route.RemainingDistance(TargetIndex, Vehicle->GetActorLocation());
						NoProgressSeconds = StoppedSeconds = 0.0f;
						}
					}
				}
			}
		}
		else if (State.Mode == EExplorationReturnMode::Planning) TickNavigationPlan(DeltaTime);
		else if (State.Mode == EExplorationReturnMode::Returning) FollowRoute(DeltaTime);
		else if (!GetWorld()->IsPaused())
		{
			SampleAccumulator += DeltaTime;
			if (SampleAccumulator >= 0.1f)
			{
				SampleAccumulator = 0.0f;
				RecordPose();
			}
		}
	}
	// Recover an event emitted before the controller/component finished BeginPlay.
	if (!bDiscoveryHandled && Observation.IsValid() && Observation->GetObservationState().bInObservationZone)
	{
		OnWhaleDiscovered(nullptr);
	}
	StateAccumulator += DeltaTime;
	if (StateAccumulator >= 0.1f) PublishState();
}

void UExplorationReturnComponent::OnObservationZoneChanged(bool bInside)
{
 if (!bActive) return;
 if (bInside) OnWhaleDiscovered(nullptr);
 else { bDiscoveryHandled = false; bDiscoveryPending = false; }
}

void UExplorationReturnComponent::OnWhaleDiscovered(AActor* Whale)
{
	if (bDiscoveryHandled || !bActive) return;
	bDiscoveryHandled = true;
	bDiscoveryPending = true;
}

void UExplorationReturnComponent::RequestReturn()
{
	StartReturn(EExplorationReturnReason::UserRequest);
}

void UExplorationReturnComponent::StartReturn(EExplorationReturnReason Reason)
{
	if (!bActive) return;
	// Explicit stop stays stopped; an event in the same frame must not undo a user takeover.
	if (Reason != EExplorationReturnReason::UserRequest
		&& (State.Mode == EExplorationReturnMode::Stopped || LastUserInterruptionFrame == GFrameCounter)) return;
	const FText Detail = Reason == EExplorationReturnReason::DangerZone
		? LOCTEXT("DangerReason", "지정 위험 구역에 진입했습니다. 정지 후 출발점까지 경로를 계산합니다.")
		: Reason == EExplorationReturnReason::WhaleDiscovery
			? LOCTEXT("WhaleReason", "고래를 발견했습니다. 관찰 후 출발점까지 경로를 계산합니다.")
			: LOCTEXT("ManualReason", "출발점까지 안전한 복귀 경로를 계산합니다.");
	if (IsAutomatic())
	{
		// Danger takes precedence without resetting the countdown, route cursor or vehicle control.
		if (Reason == EExplorationReturnReason::DangerZone)
		{
			State.Reason = Reason;
			State.Detail = State.Mode == EExplorationReturnMode::Returning
				? LOCTEXT("DangerWhileReturning", "복귀 경로에서 지정 위험 구역에 진입했습니다. 기존 복귀를 계속합니다.") : Detail;
			PublishState();
		}
		return;
	}
	State.Reason = Reason;
	RecordPose(true);
 FText BlockReason;
 if (!Vehicle.IsValid()) BlockReason = LOCTEXT("MissingVehicle", "탐사선을 찾지 못했습니다. 앱을 다시 시작하세요.");
 else if (!HomeAnchor.IsValid()) BlockReason = LOCTEXT("MissingReturnAnchor", "맵의 출발점 표시를 찾지 못했습니다.");
 else if (Vehicle->IsUnderwaterMovementEnabled() ? !bNavigationHomeValid : (!Route.bValid || Route.Points.Num()<2))
  BlockReason = LOCTEXT("HomeNotSaved", "저장된 출발점이 없습니다. 출발 패드 가까이 돌아온 뒤 다시 요청하세요.");
 else BlockReason = GetControlBlockReason();
 if (!BlockReason.IsEmpty())
 {
  const auto* Bridge = GetWorld()->GetSubsystem<UMqttVehicleBridgeSubsystem>();
  UE_LOG(LogMqttVehicle, Log, TEXT("RETURN_REJECTED: reason=%s home_valid=%d position=%s vr_focus=%d/%d rc_output=%d broker=%d"),
   *BlockReason.ToString(), bNavigationHomeValid, Vehicle.IsValid()?*Vehicle->GetActorLocation().ToString():TEXT("none"),
   FApp::UseVRFocus(),FApp::HasVRFocus(),Bridge && Bridge->bDashboardOutputEnabled,Bridge && Bridge->IsBrokerConnected());
  StopWithReason(BlockReason); return;
 }
	ReplanCount=0;
	Vehicle->BeginExplorationControl();
	State.Mode = EExplorationReturnMode::PreparingReturn;
	State.Detail = Detail;
	Countdown = Reason == EExplorationReturnReason::UserRequest ? 0.0f : 5.0f;
	PreparationSeconds = StoppedSeconds = NoProgressSeconds = 0.0f;
	TargetIndex = Route.Points.Num() - 2;
	SwimAimIndex = INDEX_NONE;
	PublishState();
}

void UExplorationReturnComponent::FollowRoute(float DeltaTime)
{
	if(bArrivalTurn) { FollowTurnaround(DeltaTime); return; }
	if(bNavigationReturn) { FollowNavigationPath(DeltaTime); return; }
	if (bTurningInPlace) { FollowTurnaround(DeltaTime); return; }
	if (!Route.bValid || !Route.Points.IsValidIndex(TargetIndex + 1) || !Route.Points.IsValidIndex(TargetIndex))
	{
		StopWithReason(LOCTEXT("NoRoute", "복귀 경로가 유효하지 않아 정지했습니다."), true);
		return;
	}
	const FVector Position = Vehicle->GetActorLocation();
	// Consume points only in reverse sequence, never jump to a nearby crossing of the route.
	const bool bSwimming = Vehicle->IsUnderwaterMovementEnabled();
	const auto Distance = [bSwimming](const FVector& A, const FVector& B) { return bSwimming ? FVector::Dist(A,B) : FVector::Dist2D(A,B); };
	const auto SegmentDistance = [bSwimming](const FVector& P, const FVector& A, const FVector& B)
	{
		return bSwimming ? ExplorationRoute::DistanceToSegment3D(P,A,B) : ExplorationRoute::DistanceToSegment2D(P,A,B);
	};
	const float ArrivalRadius = bSwimming ? 25.0f : 100.0f;
	while (TargetIndex > 0 && (Distance(Position, Route.Points[TargetIndex].Position) <= ArrivalRadius
		|| (SegmentDistance(Position, Route.Points[TargetIndex].Position, Route.Points[TargetIndex + 1].Position) < (bSwimming ? 50.0 : 200.0)
			&& FVector::DotProduct(Position - Route.Points[TargetIndex].Position,
				Route.Points[TargetIndex].Position - Route.Points[TargetIndex + 1].Position) > 0.0)))
	{
		--TargetIndex;
	}
	const FVector Target = Route.Points[TargetIndex].Position;
	const FVector From = Route.Points[TargetIndex + 1].Position;
	if (Position.ContainsNaN() || (!bSwimming && FMath::Abs(Position.Z - Target.Z) > 200.0)
		|| SegmentDistance(Position, From, Target) > 250.0
		|| FVector::DotProduct(Vehicle->GetActorUpVector(), FVector::UpVector) < 0.5)
	{
		StopWithReason(LOCTEXT("OffRoute", "경로 이탈 또는 차량 기울기로 복귀를 중단했습니다."), true);
		return;
	}
	const float Remaining = Route.RemainingDistance(TargetIndex, Position);
	if (TargetIndex == 0 && Distance(Position, Target) <= ArrivalRadius && Distance(Position, HomePosition) <= 200.0)
	{
		Vehicle->ApplyExplorationControl(0.0f, 0.0f, 1.0f, true);
		NoProgressSeconds += DeltaTime;
		StoppedSeconds = Vehicle->GetVelocity().Size() < 10.0 ? StoppedSeconds + DeltaTime : 0.0f;
		if (StoppedSeconds >= 0.5f)
		{
			LeaveAutomaticControl(true);
			Route.Begin(Position, Vehicle->GetActorForwardVector(), HomePosition);
			State.Mode = EExplorationReturnMode::Arrived;
			State.Detail = LOCTEXT("Arrived", "출발점에 도착해 정지했습니다. M으로 수동 탐사를 재개하세요.");
		}
		else if (NoProgressSeconds >= 8.0f)
		{
			StopWithReason(LOCTEXT("ArrivalNotSettled", "출발점에서 정지를 확인하지 못해 자동 제어를 종료했습니다."));
		}
		return;
	}
	StoppedSeconds = 0.0f;
	if (Remaining < BestRemainingDistance - 30.0f)
	{
		BestRemainingDistance = Remaining;
		NoProgressSeconds = 0.0f;
	}
	else NoProgressSeconds += DeltaTime;
	// Revisit recorded positions using forward travel; a rearward target requires a checked turn.
	// Hold a selected swim aim until its point is consumed. Re-evaluating the
	// lookahead every frame makes the heading alternate around distance/corridor
	// thresholds; the swimmer then brakes to turn on each frame and never launches.
	const bool bKeepSwimAim = bSwimming && Route.Points.IsValidIndex(SwimAimIndex) && SwimAimIndex <= TargetIndex;
	FVector Aim = bKeepSwimAim ? Route.Points[SwimAimIndex].Position : Target;
	if (bSwimming && !bKeepSwimAim) SwimAimIndex = TargetIndex;
	float LookDistance = static_cast<float>(Distance(Position, Aim));
	for (int32 Index = TargetIndex; !bKeepSwimAim && Index > 0 && LookDistance < 300.0f; --Index)
	{
		const FVector ThisSegment = Route.Points[Index].Position - Route.Points[Index + 1].Position;
		const FVector NextSegment = Route.Points[Index - 1].Position - Route.Points[Index].Position;
		const FVector ThisDirection = bSwimming ? ThisSegment.GetSafeNormal() : ThisSegment.GetSafeNormal2D();
		const FVector NextDirection = bSwimming ? NextSegment.GetSafeNormal() : NextSegment.GetSafeNormal2D();
		if (FVector::DotProduct(ThisDirection, NextDirection) < (bSwimming ? 0.7 : 0.0)) break;
		if (bSwimming)
		{
			// A locally gentle bend can still make a long shortcut miss the active
			// waypoint. Keep every skipped point inside its arrival radius so the
			// reverse cursor can advance, including on paths that change depth.
			const FVector Candidate = Route.Points[Index - 1].Position;
			bool bWithinRecordedPath = true;
			for (int32 Skipped = TargetIndex; Skipped >= Index; --Skipped)
			{
				if (ExplorationRoute::DistanceToSegment3D(Route.Points[Skipped].Position, Position, Candidate) > 20.0)
				{
					bWithinRecordedPath = false;
					break;
				}
			}
			if (!bWithinRecordedPath || !Vehicle->IsUnderwaterPathClear(Candidate)) break;
		}
		LookDistance += static_cast<float>(Distance(Aim, Route.Points[Index - 1].Position));
		Aim = Route.Points[Index - 1].Position;
		if (bSwimming) SwimAimIndex = Index - 1;
	}
	if (NoProgressSeconds >= 8.0f)
	{
		UE_LOG(LogMqttVehicle, Log, TEXT("Return stalled: index=%d position=%s target=%s aim=%s yaw=%.2f speed=%.2f remaining=%.2f best=%.2f"),
			TargetIndex, *Position.ToString(), *Target.ToString(), *Aim.ToString(), Vehicle->GetActorRotation().Yaw,
			Vehicle->GetVelocity().Size(), Remaining, BestRemainingDistance);
		StopWithReason(bSwimming && !Vehicle->IsUnderwaterPathClear(Aim)
			? LOCTEXT("BlockedRoute", "복귀 경로가 장애물로 막혀 정지했습니다. M으로 수동 전환해 이동하세요.")
			: LOCTEXT("Stuck", "차량이 진행하지 못해 정지했습니다. M으로 수동 조작하세요."));
		return;
	}
	if (FVector::DotProduct(Aim - Position, Vehicle->GetActorForwardVector()) < -50.0)
	{
		if (!BeginTurnaround()) StopWithReason(LOCTEXT("TurnBlocked", "제자리 회전 공간이 부족해 정지했습니다. M으로 수동 전환하세요."));
		return;
	}
	const float DesiredSpeed = FMath::Clamp(Remaining * 0.6f, 50.0f, Vehicle->GetReturnCruiseSpeed());
	if (bSwimming)
	{
		Vehicle->ApplyUnderwaterReturnTarget(Aim, DesiredSpeed);
		return;
	}
	const auto Command = ExplorationRoute::Follow(Position, Vehicle->GetActorForwardVector(), Aim, 1.0f,
		Vehicle->GetChaosVehicleMovement()->GetForwardSpeed(), DesiredSpeed);
	Vehicle->ApplyExplorationControl(Command.Throttle, Command.Steering, Command.Brake, false);
}

bool UExplorationReturnComponent::BeginTurnaround()
{
	Vehicle->ApplyExplorationControl(0.0f, 0.0f, 1.0f, true);
	for (float Side : {1.0f, -1.0f})
	{
		const auto Arc = ExplorationRoute::PivotTurn(Vehicle->GetActorLocation(), Vehicle->GetActorForwardVector(), Side * 180.0f);
		if (Vehicle->bAllowStationaryTurn && ExplorationRoute::IsTurnClear(*GetWorld(), Arc, Vehicle.Get()))
		{
			bTurningInPlace = true;
			TurnRemainingDegrees = Side * 180.0f;
			TurnSeconds = NoProgressSeconds = 0.0f;
			TurnStartPosition = Vehicle->GetActorLocation();
			State.Detail = bArrivalTurn ? LOCTEXT("ArrivalPivoting", "출발점에 도착했습니다. 제자리에서 180도 회전합니다.") : LOCTEXT("Pivoting", "정지 후 제자리에서 180도 회전합니다. 회전 뒤 기록 경로로 복귀합니다.");
			return true;
		}
	}
	return false;
}

void UExplorationReturnComponent::FollowTurnaround(float DeltaTime)
{
	Vehicle->ApplyExplorationControl(0.0f, 0.0f, 1.0f, true);
	TurnSeconds += DeltaTime;
	const FVector Position = Vehicle->GetActorLocation();
	if (TurnSeconds > 15.0f || Position.ContainsNaN() || FVector::Dist2D(Position, TurnStartPosition) > 50.0)
	{
		StopWithReason(LOCTEXT("PivotInterrupted", "제자리 회전 중 이동 또는 시간 초과로 정지했습니다."));
		return;
	}
	if (Vehicle->GetVelocity().Size() >= 10.0f) return;
	const float Step = FMath::Sign(TurnRemainingDegrees) * FMath::Min(FMath::Abs(TurnRemainingDegrees),
		Vehicle->StationaryTurnRate * FMath::Min(DeltaTime, 0.1f));
	if (!Vehicle->PivotInPlace(Step))
	{
		StopWithReason(LOCTEXT("PivotBlocked", "회전할 공간이 막혔거나 차체가 기울어 정지했습니다. M으로 수동 전환하세요."));
		return;
	}
	TurnRemainingDegrees -= Step;
	if (FMath::Abs(TurnRemainingDegrees) <= 0.1f)
	{
		bTurningInPlace = false;
		if (bArrivalTurn)
		{
			LeaveAutomaticControl(true);
			State.Mode = EExplorationReturnMode::Arrived;
			State.Detail = LOCTEXT("ArrivalTurnComplete", "출발점에서 180도 회전을 마치고 정지했습니다.");
			return;
		}
		NoProgressSeconds = 0.0f;
		BestRemainingDistance = Route.RemainingDistance(TargetIndex, Position);
		State.Detail = LOCTEXT("PivotComplete", "제자리 회전을 마쳤습니다. 기록 경로를 전진으로 따라갑니다.");
	}
}

void UExplorationReturnComponent::LeaveAutomaticControl(bool bRequireRelease)
{
	bArrivalTurn = false;
	bTurningInPlace = false;
	SwimAimIndex = INDEX_NONE;
	if (!bNavigationReturn && State.Mode == EExplorationReturnMode::Returning && Vehicle.IsValid())
	{
		Route.TrimForManual(TargetIndex, Vehicle->GetActorLocation(), Vehicle->GetActorForwardVector());
	}
	Navigator.Reset(); NavigationPath.Reset(); bNavigationReturn=false;
	State.Mode = EExplorationReturnMode::Stopped;
	if (Vehicle.IsValid()) Vehicle->EndExplorationControl(bRequireRelease);
}

void UExplorationReturnComponent::StopWithReason(const FText& Reason, bool bInvalidateRoute)
{
	LeaveAutomaticControl(true);
	if (bInvalidateRoute) Route.Invalidate();
	State.Detail = Reason;
	Countdown = 0.0f;
	PublishState();
}

void UExplorationReturnComponent::StopExploration()
{
	LastUserInterruptionFrame = GFrameCounter;
	if (bActive) StopWithReason(LOCTEXT("UserStopped", "사용자 요청으로 정지했습니다. M: 수동 전환 / H: 복귀 요청"));
}

void UExplorationReturnComponent::ResumeManual()
{
	if (!bActive) return;
	LastUserInterruptionFrame = GFrameCounter;
	LeaveAutomaticControl(false);
	State.Mode = EExplorationReturnMode::Manual;
	State.Detail = LOCTEXT("Manual", "수동 탐사로 전환했습니다. H로 복귀를 다시 요청할 수 있습니다.");
	PublishState();
}

void UExplorationReturnComponent::OnVehicleInterrupted(bool bManualOverride)
{
	if (!bActive || !IsAutomatic()) return;
	LastUserInterruptionFrame = GFrameCounter;
	if (bManualOverride) ResumeManual();
	else StopWithReason(LOCTEXT("ControlStopped", "정지 요청 또는 통신·제어 상태 변경으로 복귀를 중단했습니다."));
}

void UExplorationReturnComponent::PublishState()
{
	StateAccumulator = 0.0f;
	State.RecordedPoints = Route.Points.Num();
	State.SecondsUntilReturn = State.Mode == EExplorationReturnMode::PreparingReturn ? Countdown : 0.0f;
	State.bCanReturn = Vehicle.IsValid() && HomeAnchor.IsValid()
  && (Vehicle->IsUnderwaterMovementEnabled() ? bNavigationHomeValid : (Route.bValid && Route.Points.Num() >= 2)) && CanDriveNow();
	State.RemainingMeters = Vehicle.IsValid() ? Route.RemainingDistance(
		State.Mode == EExplorationReturnMode::Returning ? TargetIndex : Route.Points.Num() - 1,
		Vehicle->GetActorLocation()) / 100.0f : 0.0f;
	if(Vehicle.IsValid() && Vehicle->IsUnderwaterMovementEnabled())
  State.RemainingMeters = (bNavigationReturn && !NavigationPath.IsEmpty() ? NavigationRemaining()
   : bNavigationHomeValid ? FVector::Dist(Vehicle->GetActorLocation(),NavigationHome) : 0.0)/100.0f;
	OnReturnStateChanged.Broadcast(State);
}

bool UExplorationReturnComponent::IsNavigationSegmentClear(const FVector& From, const FVector& To) const
{
 if(!Vehicle.IsValid() || !Vehicle->IsUnderwaterNavigationSegmentClear(From,To)) return false;
 const FBox Hull=Vehicle->GetUnderwaterNavigationEnvelope();
 for(const auto& Pair : DangerZones)
 {
  const auto* Trigger=Cast<ATriggerBox>(Pair.Key.Get());
  const auto* Box=Trigger ? Cast<UBoxComponent>(Trigger->GetCollisionComponent()) : nullptr;
  if(!Box) continue;
  // Minkowski expansion: positions of the vehicle origin that would enter the hazard.
  const FBox WorldBox=Box->Bounds.GetBox();
  const FBox Forbidden(WorldBox.Min-Hull.Max,WorldBox.Max-Hull.Min);
  if(Forbidden.IsInsideOrOn(From) && Forbidden.IsInsideOrOn(NavigationStart))
  {
   if(From.Equals(To,0.01) || !Forbidden.IsInsideOrOn(To)) continue;
   auto Depth=[&](const FVector& P) { return FMath::Min((P-Forbidden.Min).GetMin(),(Forbidden.Max-P).GetMin()); };
   if(Depth(To)<Depth(From)-0.01) continue; // Escape is permitted; re-entry is not.
   return false;
  }
  if(FMath::LineBoxIntersection(Forbidden,From,To,To-From)) return false;
 }
 return true;
}

void UExplorationReturnComponent::BeginNavigationPlan(bool bReplan)
{
 if(!Vehicle.IsValid() || !bNavigationHomeValid) { StopWithReason(LOCTEXT("NoNavigationHome","저장된 출발점이 없어 정지했습니다.")); return; }
 if(bReplan && ++ReplanCount>3) { StopWithReason(LOCTEXT("ReplanLimit","장애물 변화가 반복되어 정지했습니다. M으로 수동 전환하세요.")); return; }
 Vehicle->ApplyExplorationControl(0,0,1,true);
 bNavigationReturn=true; NavigationPath.Reset(); NavigationIndex=0; PlanningSeconds=0;
 NavigationStart=Vehicle->GetActorLocation();
 State.Mode=EExplorationReturnMode::Planning;
 State.Detail=LOCTEXT("PlanningPath","출발점까지 3차원 장애물 회피 경로를 계산하고 있습니다.");
 if(FVector::Dist(NavigationStart,NavigationHome)>20000.0)
 { StopWithReason(LOCTEXT("NavigationRange","출발점이 탐색 거리 한도 밖에 있어 정지했습니다.")); return; }
 FUnderwaterPathPlanner::FSettings Settings;
 FVector Min=NavigationStart.ComponentMin(NavigationHome)-FVector(4000,4000,1500);
 FVector Max=NavigationStart.ComponentMax(NavigationHome)+FVector(4000,4000,3000);
 Max.Z=FMath::Min(Max.Z,Vehicle->GetUnderwaterCeiling());
 Settings.Bounds=FBox(Min,Max);
 Navigator.Begin(NavigationStart,NavigationHome,Settings,[this](const FVector& A,const FVector& B) { return IsNavigationSegmentClear(A,B); });
 UE_LOG(LogMqttVehicle,Log,TEXT("A* return planning: start=%s home=%s replan=%d"),*NavigationStart.ToString(),*NavigationHome.ToString(),ReplanCount);
}

void UExplorationReturnComponent::TickNavigationPlan(float DeltaTime)
{
 Vehicle->ApplyExplorationControl(0,0,1,true);
 PlanningSeconds+=DeltaTime;
 if(PlanningSeconds>15.0f) { StopWithReason(LOCTEXT("PlanningTimeout","제한 시간 안에 경로를 찾지 못해 정지했습니다.")); return; }
 Navigator.Step([this](const FVector& A,const FVector& B) { return IsNavigationSegmentClear(A,B); });
 if(Navigator.GetStatus()==FUnderwaterPathPlanner::EStatus::Failed)
 {
  UE_LOG(LogMqttVehicle,Log,TEXT("A* return failed: expanded=%d"),Navigator.GetExpandedCount());
  StopWithReason(LOCTEXT("NavigationNoPath","선체가 통과할 복귀 경로를 탐색 범위 안에서 찾지 못해 정지했습니다. M으로 수동 전환하세요.")); return;
 }
 if(Navigator.GetStatus()!=FUnderwaterPathPlanner::EStatus::Succeeded) return;
 NavigationPath=Navigator.GetPath(); NavigationIndex=1;
 if(NavigationPath.Num()<2) { StopWithReason(LOCTEXT("NavigationEmpty","계산된 경로가 없어 정지했습니다.")); return; }
 State.Mode=EExplorationReturnMode::Returning; NoProgressSeconds=StoppedSeconds=0;
 BestRemainingDistance=NavigationRemaining();
 State.Detail=LOCTEXT("FollowingPlan","계산한 장애물 회피 경로로 출발점에 복귀합니다.");
 UE_LOG(LogMqttVehicle,Log,TEXT("A* return ready: points=%d length=%.1f graph=%.1f expanded=%d seconds=%.3f"),
  NavigationPath.Num(),BestRemainingDistance,Navigator.GetGraphDistance(),Navigator.GetExpandedCount(),PlanningSeconds);
}

float UExplorationReturnComponent::NavigationRemaining() const
{
 if(!Vehicle.IsValid() || !NavigationPath.IsValidIndex(NavigationIndex)) return 0;
 double Remaining=FVector::Dist(Vehicle->GetActorLocation(),NavigationPath[NavigationIndex]);
 for(int32 I=NavigationIndex+1; I<NavigationPath.Num(); ++I) Remaining+=FVector::Dist(NavigationPath[I-1],NavigationPath[I]);
 return static_cast<float>(Remaining);
}

void UExplorationReturnComponent::FollowNavigationPath(float DeltaTime)
{
 if(!Vehicle.IsValid() || !NavigationPath.IsValidIndex(NavigationIndex))
 { StopWithReason(LOCTEXT("InvalidNavigation","계산된 경로가 유효하지 않아 정지했습니다.")); return; }
 const FVector Position=Vehicle->GetActorLocation();
 if(Position.ContainsNaN()) { StopWithReason(LOCTEXT("InvalidPosition","차량 위치가 유효하지 않아 정지했습니다.")); return; }
 if(FVector::Dist(Position,NavigationHome)<=25.0)
 {
  Vehicle->ApplyExplorationControl(0,0,1,true);
  StoppedSeconds=Vehicle->GetVelocity().Size()<10 ? StoppedSeconds+DeltaTime : 0;
  if(StoppedSeconds>=0.5f)
  {
   bArrivalTurn=true;
   if(!BeginTurnaround()) StopWithReason(LOCTEXT("ArrivalTurnBlocked","출발점에 도착했지만 회전 공간이 부족해 정지했습니다."));
  }
  return;
 }
 while(NavigationIndex<NavigationPath.Num()-1 && FVector::Dist(Position,NavigationPath[NavigationIndex])<=8.0) ++NavigationIndex;
 const FVector Aim=NavigationPath[NavigationIndex];
 // Recheck the entire next segment every tick. Stop before replanning; never drive blind.
 if(!IsNavigationSegmentClear(Position,Aim)) { BeginNavigationPlan(true); return; }
 const float Remaining=NavigationRemaining();
 if(Remaining<BestRemainingDistance-30.0f) { BestRemainingDistance=Remaining; NoProgressSeconds=0; }
 else NoProgressSeconds+=DeltaTime;
 if(NoProgressSeconds>=8.0f) { BeginNavigationPlan(true); return; }
 Vehicle->ApplyUnderwaterReturnTarget(Aim,FMath::Clamp(Remaining*0.6f,50.0f,Vehicle->GetReturnCruiseSpeed()));
}

#undef LOCTEXT_NAMESPACE

