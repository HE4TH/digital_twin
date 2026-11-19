#include "QuestVideoEncoder.h"
#if PLATFORM_ANDROID
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <dlfcn.h>
#endif

FQuestVideoEncoder::FQuestVideoEncoder(int32 W, int32 H, int32 Rate):Width(W),Height(H),FPS(Rate) {}
FQuestVideoEncoder::~FQuestVideoEncoder()
{
#if PLATFORM_ANDROID
 if (Codec) { if(bStarted) AMediaCodec_stop(static_cast<AMediaCodec*>(Codec)); AMediaCodec_delete(static_cast<AMediaCodec*>(Codec)); }
#endif
}
bool FQuestVideoEncoder::Start()
{
#if PLATFORM_ANDROID
 auto* C=AMediaCodec_createEncoderByType("video/avc"); if(!C) return false; Codec=C;
 char* Name=nullptr;
 const auto GetName=reinterpret_cast<media_status_t(*)(AMediaCodec*,char**)>(dlsym(RTLD_DEFAULT,"AMediaCodec_getName"));
 const auto ReleaseName=reinterpret_cast<void(*)(AMediaCodec*,char*)>(dlsym(RTLD_DEFAULT,"AMediaCodec_releaseName"));
 const auto GetInputFormat=reinterpret_cast<AMediaFormat*(*)(AMediaCodec*)>(dlsym(RTLD_DEFAULT,"AMediaCodec_getInputFormat"));
 if(!GetName || !ReleaseName || !GetInputFormat || GetName(C,&Name)!=AMEDIA_OK || !Name) return false;
 const FString CodecName(UTF8_TO_TCHAR(Name)); ReleaseName(C,Name);
 // Do not silently consume headset CPU with the Android software encoder.
 if(CodecName.Contains(TEXT("android")) || CodecName.Contains(TEXT("google"))) return false;
 auto* F=AMediaFormat_new();
 AMediaFormat_setString(F,AMEDIAFORMAT_KEY_MIME,"video/avc");
 AMediaFormat_setInt32(F,AMEDIAFORMAT_KEY_WIDTH,Width); AMediaFormat_setInt32(F,AMEDIAFORMAT_KEY_HEIGHT,Height);
 AMediaFormat_setInt32(F,AMEDIAFORMAT_KEY_BIT_RATE,2500000);
 AMediaFormat_setInt32(F,AMEDIAFORMAT_KEY_FRAME_RATE,FPS);
 AMediaFormat_setInt32(F,AMEDIAFORMAT_KEY_I_FRAME_INTERVAL,1);
 AMediaFormat_setInt32(F,AMEDIAFORMAT_KEY_COLOR_FORMAT,21); // Standard NV12 byte-buffer input.
 AMediaFormat_setInt32(F,"profile",1); // AVC baseline: no B-frame reordering.
 AMediaFormat_setInt32(F,"max-bframes",0);
 auto Result=AMediaCodec_configure(C,F,nullptr,nullptr,AMEDIACODEC_CONFIGURE_FLAG_ENCODE); AMediaFormat_delete(F);
 if(Result!=AMEDIA_OK || AMediaCodec_start(C)!=AMEDIA_OK) return false;
 bStarted=true; Stride=Width; SliceHeight=Height;
 F=GetInputFormat(C); int32 Value=0;
 if(F)
 {
  if(AMediaFormat_getInt32(F,"stride",&Value) && Value>=Width) Stride=Value;
  if(AMediaFormat_getInt32(F,"slice-height",&Value) && Value>=Height) SliceHeight=Value;
  AMediaFormat_delete(F);
 }
 if(Stride>4096 || SliceHeight>4096 || (Stride&1) || (SliceHeight&1)) return false;
 UE_LOG(LogTemp,Log,TEXT("QUEST_VIDEO_ENCODER: %s %dx%d target=%dfps NV12 stride=%d slice=%d"),*CodecName,Width,Height,FPS,Stride,SliceHeight);
 return true;
#else
 return false;
#endif
}
bool FQuestVideoEncoder::Encode(const TArray<FColor>& Pixels,int64 TimestampUs,bool bRequestKeyFrame,TArray<FQuestVideoPacket>& Packets)
{
#if PLATFORM_ANDROID
 if(!bStarted && !Start()) return false;
 auto* C=static_cast<AMediaCodec*>(Codec);
 if(Pixels.Num()!=Width*Height) return false;
 if(bRequestKeyFrame)
 {
  auto* Params=AMediaFormat_new(); AMediaFormat_setInt32(Params,"request-sync",0);
  AMediaCodec_setParameters(C,Params); AMediaFormat_delete(Params);
 }
 const auto InputIndex=AMediaCodec_dequeueInputBuffer(C,0);
 if(InputIndex>=0)
 {
  size_t Capacity=0; uint8* Input=AMediaCodec_getInputBuffer(C,InputIndex,&Capacity);
  // Codec2 exposes through the final UV pixel, excluding trailing padding rows.
  const int32 YSize=Stride*SliceHeight, Size=YSize+(Height/2-1)*Stride+Width;
  if(!Input || Capacity<static_cast<size_t>(Size)) { UE_LOG(LogTemp,Warning,TEXT("QUEST_VIDEO_INPUT: capacity=%llu required=%d"),static_cast<unsigned long long>(Capacity),Size); return false; }
  FMemory::Memset(Input,16,YSize); FMemory::Memset(Input+YSize,128,Size-YSize);
  for(int32 Y=0;Y<Height;Y++) for(int32 X=0;X<Width;X++)
  {
   const FColor& P=Pixels[Y*Width+X];
   Input[Y*Stride+X]=FMath::Clamp(((66*P.R+129*P.G+25*P.B+128)>>8)+16,0,255);
   if(!(X&1) && !(Y&1))
   {
    int32 R=0,G=0,B=0;
    for(int32 DY=0;DY<2;DY++) for(int32 DX=0;DX<2;DX++)
    { const auto& Q=Pixels[(Y+DY)*Width+X+DX]; R+=Q.R; G+=Q.G; B+=Q.B; }
    R/=4;G/=4;B/=4;
    Input[YSize+(Y/2)*Stride+X]=FMath::Clamp(((-38*R-74*G+112*B+128)>>8)+128,0,255);
    Input[YSize+(Y/2)*Stride+X+1]=FMath::Clamp(((112*R-94*G-18*B+128)>>8)+128,0,255);
   }
  }
  if(AMediaCodec_queueInputBuffer(C,InputIndex,0,Size,TimestampUs,0)!=AMEDIA_OK) return false;
 }
 for(int32 I=0;I<8;I++)
 {
  AMediaCodecBufferInfo Info{}; auto Index=AMediaCodec_dequeueOutputBuffer(C,&Info,0);
  if(Index==AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED || Index==AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) continue;
  if(Index==AMEDIACODEC_INFO_TRY_AGAIN_LATER) break;
  if(Index<0) { UE_LOG(LogTemp,Warning,TEXT("QUEST_VIDEO_OUTPUT: status=%d"),static_cast<int32>(Index)); return false; }
  size_t Capacity=0; uint8* Output=AMediaCodec_getOutputBuffer(C,Index,&Capacity);
  const bool Valid=Output && Info.offset>=0 && Info.size>=0 && static_cast<size_t>(Info.offset)+Info.size<=Capacity;
  if(!Valid) { AMediaCodec_releaseOutputBuffer(C,Index,false); return false; }
  if(Info.flags&AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG)
  { Configuration.Reset(); Configuration.Append(Output+Info.offset,Info.size); }
  else if(Info.size>0)
  {
   auto& Packet=Packets.AddDefaulted_GetRef(); Packet.TimestampUs=Info.presentationTimeUs; Packet.bKeyFrame=(Info.flags&1)!=0;
   if(Packet.bKeyFrame) Packet.Data.Append(Configuration);
   Packet.Data.Append(Output+Info.offset,Info.size);
  }
  AMediaCodec_releaseOutputBuffer(C,Index,false);
 }
 return true;
#else
 return false;
#endif
}
