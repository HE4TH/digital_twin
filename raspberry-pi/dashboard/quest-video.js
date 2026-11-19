/* Same-origin fragmented MP4 playback; bounded catch-up and no remote control. */
window.QuestMovie=(()=>{
 const video=document.createElement('video');video.muted=true;video.autoplay=true;video.playsInline=true;video.hidden=true;
 video.style.cssText='position:absolute;inset:0;width:100%;height:100%;object-fit:contain;z-index:5;background:#061018';
 document.getElementById('capture-scene').append(video);
 const play=document.createElement('button');play.textContent='관전자 영상 재생';play.hidden=true;
 document.getElementById('capture-scene').after(play);play.onclick=()=>video.play().then(()=>play.hidden=true).catch(()=>{});
 let active=false,session='',after=0,source=null,buffer=null,url='',lastTrim=0;
 const bytes=s=>Uint8Array.from(atob(s),c=>c.charCodeAt(0));
 function close(){video.pause();video.removeAttribute('src');video.load();video.hidden=true;play.hidden=true;if(url)URL.revokeObjectURL(url);url='';source=buffer=null;session='';after=0;}
 function completed(action){return new Promise((resolve,reject)=>{
  const b=buffer;if(!b){reject(Error('closed'));return;}
  const done=()=>{cleanup();resolve();},fail=()=>{cleanup();reject(Error('media append failed'));};
  const timeout=setTimeout(fail,3000);
  const cleanup=()=>{clearTimeout(timeout);b.removeEventListener('updateend',done);b.removeEventListener('error',fail);};
  b.addEventListener('updateend',done,{once:true});b.addEventListener('error',fail,{once:true});
  try{action(b);}catch(e){cleanup();reject(e);}
 });}
 async function open(codec,init){
  close();const mime=`video/mp4; codecs="${codec}"`;
  if(!window.MediaSource||!MediaSource.isTypeSupported(mime))throw Error('AVC MediaSource unavailable');
  source=new MediaSource();url=URL.createObjectURL(source);video.src=url;
  await new Promise((resolve,reject)=>{const timer=setTimeout(()=>reject(Error('open timeout')),3000);source.addEventListener('sourceopen',()=>{clearTimeout(timer);resolve();},{once:true});});
  buffer=source.addSourceBuffer(mime);buffer.mode='segments';
  await completed(b=>b.appendBuffer(bytes(init)));
 }
 async function poll(){
  try{
   if(!active||document.hidden){if(source)close();return;}
   const response=await fetch(`/api/quest/video?session=${session}&after=${after}`,{signal:AbortSignal.timeout(2000)});
   if(!response.ok)throw Error('video unavailable');const data=await response.json();
   if(!data.live)return;
   if(data.reset||session!==data.session){await open(data.codec,data.init);session=data.session;}
   for(const f of data.frames){await completed(b=>b.appendBuffer(bytes(f.data)));after=f.id;}
   if(video.buffered.length){
    const i=video.buffered.length-1,end=video.buffered.end(i),start=video.buffered.start(i);
    if(video.currentTime<start||end-video.currentTime>0.65)video.currentTime=Math.max(start,end-0.15);
    video.hidden=false;video.play().catch(()=>play.hidden=false);
    if(video.currentTime>3&&Date.now()-lastTrim>1000){lastTrim=Date.now();await completed(b=>b.remove(0,video.currentTime-2));}
   }
  }catch{close();}finally{setTimeout(poll,60);}
 }
 video.addEventListener('error',close);window.addEventListener('pagehide',()=>{active=false;close();});
 setTimeout(poll,60);
 return {update(value){active=value;if(!value&&source&&!buffer?.updating)close();}};
})();
