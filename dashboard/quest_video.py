"""Bounded AVC -> fragmented MP4 relay. No transcoding, files, commands or motor access."""
import base64
from collections import deque
import re
import struct
import time

def box(kind, data=b''):
    return struct.pack('>I4s',len(data)+8,kind.encode())+data

def full(kind, version, flags, data=b''):
    return box(kind,bytes([version])+flags.to_bytes(3,'big')+data)

def u32(*values):
    return struct.pack('>'+'I'*len(values),*values)

MATRIX=u32(0x10000,0,0,0,0x10000,0,0,0,0x40000000)

def init_segment(sps,pps,width,height):
    if len(sps)<4 or len(sps)>65535 or not pps or len(pps)>65535:
        raise ValueError('invalid AVC configuration')
    avcc=bytes([1,sps[1],sps[2],sps[3],255,225])+struct.pack('>H',len(sps))+sps+bytes([1])+struct.pack('>H',len(pps))+pps
    visual=b'\0'*6+struct.pack('>H',1)+b'\0'*16+struct.pack('>HH',width,height)+u32(0x480000,0x480000,0)+struct.pack('>H',1)+b'\0'*32+struct.pack('>HH',24,65535)
    stsd=full('stsd',0,0,u32(1)+box('avc1',visual+box('avcC',avcc)))
    stbl=box('stbl',stsd+full('stts',0,0,u32(0))+full('stsc',0,0,u32(0))+full('stsz',0,0,u32(0,0))+full('stco',0,0,u32(0)))
    dinf=box('dinf',full('dref',0,0,u32(1)+full('url ',0,1)))
    minf=box('minf',full('vmhd',0,1,b'\0'*8)+dinf+stbl)
    mdhd=full('mdhd',0,0,u32(0,0,1000000,0)+struct.pack('>HH',0x55c4,0))
    hdlr=full('hdlr',0,0,u32(0)+b'vide'+b'\0'*12+b'Quest video\0')
    tkhd=full('tkhd',0,7,u32(0,0,1,0,0)+b'\0'*8+struct.pack('>hhhh',0,0,0,0)+MATRIX+u32(width<<16,height<<16))
    trak=box('trak',tkhd+box('mdia',mdhd+hdlr+minf))
    mvhd=full('mvhd',0,0,u32(0,0,1000000,0,0x10000)+struct.pack('>HH',0x100,0)+b'\0'*8+MATRIX+b'\0'*24+u32(2))
    mvex=box('mvex',full('trex',0,0,u32(1,1,0,0,0)))
    return box('ftyp',b'isom'+u32(512)+b'isomiso6avc1mp41')+box('moov',mvhd+trak+mvex)

def fragment(sequence,pts,duration,data,key):
    def make(offset):
        trun=full('trun',0,0x701,u32(1,offset,duration,len(data),0x02000000 if key else 0x01010000))
        traf=box('traf',full('tfhd',0,0x020000,u32(1))+full('tfdt',1,0,struct.pack('>Q',pts))+trun)
        return box('moof',full('mfhd',0,0,u32(sequence))+traf)
    moof=make(0);return make(len(moof)+8)+box('mdat',data)

def nals(data):
    # MediaCodec AVC byte buffers use Annex B. Reject malformed/truncated streams.
    parts=re.split(b'\x00\x00\x00?\x01',data)
    if len(parts)<2 or parts[0]: raise ValueError('expected Annex B AVC')
    return [p for p in parts[1:] if p]

class QuestVideo:
    def __init__(self):
        self.session='';self.init=b'';self.codec='';self.frames=deque();self.bytes=0
        self.sps=b'';self.pps=b'';self.pending=None;self.base=0;self.sequence=0
        self.at=0;self.received=deque(maxlen=120);self.last_pts=-1

    def ingest(self,session,width,height,payload):
        if not re.fullmatch('[a-fA-F0-9]{32}',session) or width not in range(640,1281,32) or height!=width*9//16:
            raise ValueError('invalid video configuration')
        packets=[];offset=0
        while offset<len(payload):
            if len(payload)-offset<12: raise ValueError('truncated packet')
            pts,size=struct.unpack_from('<QI',payload,offset);offset+=12
            if not 1<=size<=524288 or size>len(payload)-offset or pts>2**63-1:raise ValueError('invalid packet')
            units=nals(payload[offset:offset+size]);offset+=size;packets.append((pts,units))
            if len(packets)>30:raise ValueError('too many frames')
        if not packets:raise ValueError('empty video')
        if self.session!=session:
            self.__init__();self.session=session
        for pts,units in packets:
            key=any(n[0]&31==5 for n in units)
            for unit in units:
                if unit[0]&31==7:self.sps=unit
                if unit[0]&31==8:self.pps=unit
            samples=[n for n in units if n[0]&31 in (1,5,6)]
            if not any(n[0]&31 in (1,5) for n in samples):continue
            if not self.init:
                if not key or not self.sps or not self.pps:continue
                self.init=init_segment(self.sps,self.pps,width,height);self.codec='avc1.'+self.sps[1:4].hex();self.base=pts
            if pts<=self.last_pts:raise ValueError('non-monotonic timestamp')
            if self.last_pts>=0 and pts-self.last_pts>1000000:
                # A reconnect must begin at a new keyframe; never append inter-frame gaps.
                self.init=b'';self.frames.clear();self.bytes=0;self.pending=None;self.sequence=0
                self.session='';return
            if self.pending:
                old_pts,data,old_key=self.pending;self.sequence+=1
                media=fragment(self.sequence,old_pts-self.base,pts-old_pts,data,old_key)
                self.frames.append((self.sequence,media,old_key));self.bytes+=len(media)
                while len(self.frames)>90 or self.bytes>4194304:
                    self.bytes-=len(self.frames.popleft()[1])
            data=b''.join(u32(len(n))+n for n in samples)
            self.pending=(pts,data,key);self.last_pts=pts;self.at=time.monotonic();self.received.append(self.at)

    def status(self):
        now=time.monotonic();times=[t for t in self.received if now-t<3]
        fps=(len(times)-1)/(times[-1]-times[0]) if len(times)>1 and times[-1]>times[0] else 0
        return dict(live=bool(self.init and self.frames and now-self.at<3),fps=round(fps,1),codec=self.codec)

    def read(self,session,after):
        if not self.status()['live']:return dict(live=False)
        frames=list(self.frames);reset=session!=self.session or after<frames[0][0]-1 or after>self.sequence
        if reset:
            keys=[i for i,f in enumerate(frames) if f[2]]
            if not keys:return dict(live=False)
            frames=frames[keys[-1]:]
        else:frames=[f for f in frames if f[0]>after]
        return dict(live=True,session=self.session,reset=reset,codec=self.codec,
                    init=base64.b64encode(self.init).decode() if reset else '',
                    frames=[dict(id=n,data=base64.b64encode(b).decode()) for n,b,_ in frames[:30]])
