const el=id=>document.getElementById(id);
const modes={MANUAL:'수동 탐사',PREPARING_RETURN:'복귀 준비',RETURNING:'자동복귀',ARRIVED:'도착',STOPPED:'정지',PLANNING:'경로 계산',UNAVAILABLE:'복귀 불가'};
function status(id,text,good){el(id).textContent=(good?'● ':'○ ')+text;el(id).className=good?'ok':'warn';}
function render(s,started=performance.now()){
 el('server').textContent='대시보드 연결됨';
 status('broker',s.broker?'구독 연결됨':'연결 안 됨',s.broker);
 status('ue',s.quest?.connected?'Quest APK 연결됨':s.ue?(s.ue.play?'Play 실행 중':'에디터 연결됨'):'연결 안 됨',!!s.ue||s.quest?.connected);
 renderQuestVideo(s.quest,started);
 status('receiver',s.receiver?(s.receiver.dry_run?'모터 없는 시험':'상태 수신 중'):'상태 수신 없음',!!s.receiver);
 el('mode').textContent=s.quest?.control?.mode || (s.ue?(modes[s.ue.mode]||s.ue.mode):'상태 대기');
 el('speed').textContent=s.ue&&Number.isFinite(s.ue.speed)?s.ue.speed.toFixed(1)+' MPH':'—';
 const r=s.receiver;
 el('left').textContent=r?r.l.toFixed(2):'—';el('right').textContent=r?r.r.toFixed(2):'—';
 const motion=!r?'확인 불가':!r.l&&!r.r?'정지':r.l<0&&r.r>0?'좌회전':r.l>0&&r.r<0?'우회전':r.l+r.r<0?'후진':'전진 / 조향';
 el('motion').textContent=motion;el('drive').textContent=motion;
 const q=s.quest?.control;
 const toggle=el('send-toggle');toggle.dataset.action=q?.send_enabled?'send_off':'send_on';
 toggle.textContent=q?.send_enabled?'RC 명령 송신 끄기':'RC 명령 송신 켜기';
 el('send-state').textContent=q?`RC 송신 ${q.send_enabled?'켜짐':'꺼짐'} · Quest MQTT ${q.broker?'연결됨':'연결 안 됨'}`:'Quest 제어 연결 대기';
 document.querySelectorAll('[data-action]').forEach(b=>{
  const a=b.dataset.action;
  b.disabled=a==='receiver_start'?(!s.receiver_start_available||!!s.receiver||!!q?.send_enabled):
   a==='return'?(!q?.can_return||s.pending):(!q||(s.pending&&a!=='send_off'));
 });
 el('events').replaceChildren();
 for(const event of s.events){const li=document.createElement('li'),t=document.createElement('time'),text=document.createElement('span');t.textContent=event.time;text.textContent=event.text;li.append(t,text);el('events').append(li);}
 if(!s.events.length){const li=document.createElement('li');li.textContent='아직 수신한 이벤트가 없습니다.';el('events').append(li);}
}
async function poll(){try{const started=performance.now();const res=await fetch('/api/state',{signal:AbortSignal.timeout(2000)});if(!res.ok)throw Error();render(await res.json(),started);}catch{renderQuestVideo(null);el('server').textContent='대시보드 연결 끊김';for(const id of ['left','right','speed'])el(id).textContent='—';status('ue','상태 확인 불가',false);status('receiver','상태 확인 불가',false);status('broker','상태 확인 불가',false);el('mode').textContent='상태 확인 불가';el('motion').textContent='확인 불가';el('drive').textContent='확인 불가';document.querySelectorAll('[data-action]').forEach(b=>b.disabled=true);}finally{setTimeout(poll,500);}}
document.querySelectorAll('[data-action]').forEach(button=>button.addEventListener('click',async()=>{button.disabled=true;try{const response=await fetch('/api/action',{method:'POST',headers:{'Content-Type':'application/json','X-Dashboard':'local'},body:JSON.stringify({action:button.dataset.action}),signal:AbortSignal.timeout(10000)});const data=await response.json();if(!response.ok)throw Error(data.error);el('message').textContent='요청 접수됨. 적용 결과는 탐사 이벤트에서 확인하세요.';}catch(error){el('message').textContent='요청 실패: '+error.message;}}));
// Mirror the headset widget text, never infer events from motion or send commands.
const captions=document.createElement('div');captions.className='quest-captions';captions.setAttribute('aria-live','polite');captions.setAttribute('aria-atomic','true');el('capture-scene').append(captions);
let captionItems=[],captionBadge='',captionUntil=0,captionSignature='';
function renderCaptions(q,started){
 const active=!!q?.connected && (!!q?.video?.live || !!q?.streaming);
 captionItems=active?(q.notices||[]).map(n=>({...n,until:started+Math.max(0,n.remaining)*1000})):[];
 captionBadge=active?(q.notice_badge||''):'';captionUntil=started+Math.min(3,Math.max(0,q?.notice_lease||0))*1000;
 drawCaptions();
}
function drawCaptions(){
 const now=performance.now();const items=now<captionUntil?captionItems.filter(n=>n.until>now):[];const badge=now<captionUntil?captionBadge:'';
 const signature=JSON.stringify([items.map(n=>[n.kind,n.text]),badge]);if(signature===captionSignature)return;captionSignature=signature;
 captions.replaceChildren();
 for(const n of items){const box=document.createElement('div');box.className='quest-caption '+n.kind;box.textContent=n.text;captions.append(box);}
 if(badge){const box=document.createElement('div');box.className='quest-caption persistent';box.textContent=badge;captions.append(box);}
}
setInterval(drawCaptions,100);
const questImage=document.createElement('img');
questImage.alt='Quest 관전자 화면';
questImage.hidden=true;
questImage.style.cssText='position:absolute;inset:0;width:100%;height:100%;object-fit:contain;z-index:4;background:#061018';
el('capture-scene').append(questImage);
const questNotice=document.createElement('p');
questNotice.className='notice';questNotice.hidden=true;
el('pixel-state').parentElement.after(questNotice);
let questLive=false, questLoading=false, questRequestedAt=0;
function refreshQuestFrame(){if(!questLive||questLoading||document.hidden||Date.now()-questRequestedAt<100)return;questRequestedAt=Date.now();questLoading=true;questImage.src="/api/quest/frame?v="+Date.now();}
questImage.onload=()=>{questLoading=false;};
setInterval(refreshQuestFrame,100);
function renderQuestVideo(q,started=performance.now()){
 renderCaptions(q,started);
 const movie=!!q?.video?.live;QuestMovie.update(movie);
 const live=!!q?.streaming && !movie;questLive=live;
 questImage.hidden=!live;questNotice.hidden=!q?.connected;
 questNotice.textContent=movie?`Quest 연속 영상 · 수신 ${q.video.fps.toFixed(1)} fps · 목표 30 fps`:live?'Quest APK 영상 수신 중 · 16:9 관전자 화면':'Quest APK 연결됨 · 영상 대기';
 el('pixel-state').parentElement.hidden=live||movie; el('pixel-state').parentElement.style.display=live||movie?'none':'';
 if(live)refreshQuestFrame();
}
questImage.onerror=()=>{questLoading=false;questImage.hidden=true;questNotice.textContent='Quest 영상 수신 대기';};
poll();


// Display-only crop: headset rendering and remote input stay with Unreal.
(()=>{
const parent=document.getElementById('pixel-player');
const view=document.getElementById('pixel-view');
try{if(localStorage.getItem('dmix-video-view-v2')==='vr-left')view.value='vr-left';}catch{}
function fitVideo(){
 const video=parent.querySelector('video');
 if(!video)return;
 const single=view.value==='vr-left'&&video.videoWidth>0&&video.videoHeight>0;
 video.toggleAttribute('data-single-eye',single);
 if(!single)return;
 // Fit one eye without stretching it; clip the other eye before compositing.
 const eyeWidth=video.videoWidth/2;
 const scale=Math.min(parent.clientWidth/eyeWidth,parent.clientHeight/video.videoHeight);
 video.style.setProperty('--eye-video-width',`${video.videoWidth*scale}px`);
 video.style.setProperty('--eye-video-height',`${video.videoHeight*scale}px`);
 video.style.setProperty('--eye-video-left',`${(parent.clientWidth-eyeWidth*scale)/2}px`);
 video.style.setProperty('--eye-video-top',`${(parent.clientHeight-video.videoHeight*scale)/2}px`);
}
view.addEventListener('change',()=>{
 try{localStorage.setItem('dmix-video-view-v2',view.value);}catch{}
 fitVideo();
});
const videoLayoutObserver=new ResizeObserver(fitVideo);
videoLayoutObserver.observe(parent);
for(const event of ['loadedmetadata','resize','playing'])parent.addEventListener(event,fitVideo,true);
fitVideo();
window.addEventListener('pagehide',()=>videoLayoutObserver.disconnect());
})();


