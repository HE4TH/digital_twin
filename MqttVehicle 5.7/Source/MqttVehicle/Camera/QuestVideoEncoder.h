#pragma once
#include "CoreMinimal.h"

/** Worker-thread-only Android hardware AVC encoder. No UObject or network ownership. */
struct FQuestVideoPacket
{
 int64 TimestampUs = 0;
 bool bKeyFrame = false;
 TArray<uint8> Data;
};
class FQuestVideoEncoder
{
public:
 FQuestVideoEncoder(int32 Width, int32 Height, int32 FPS);
 ~FQuestVideoEncoder();
 bool Encode(const TArray<FColor>& Pixels, int64 TimestampUs, bool bRequestKeyFrame, TArray<FQuestVideoPacket>& Packets);
private:
 void* Codec = nullptr;
 int32 Width, Height, FPS, Stride = 0, SliceHeight = 0;
 bool bStarted = false;
 TArray<uint8> Configuration;
 bool Start();
};
