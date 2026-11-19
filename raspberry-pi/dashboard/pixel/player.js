import {Config,PixelStreaming} from '@epicgames-ps/lib-pixelstreamingfrontend-ue5.7';
const parent=document.getElementById('pixel-player');
const message=document.getElementById('pixel-state');
const placeholder=document.getElementById('pixel-placeholder');
const config=new Config({useUrlParams:false,initialSettings:{
 ss:`ws://${location.hostname}:8889`,AutoConnect:true,AutoPlayVideo:true,StartVideoMuted:true,
 WaitForStreamer:true,MaxReconnectAttempts:100000,TimeoutIfIdle:false,
 KeyboardInput:false,MouseInput:false,TouchInput:false,GamepadInput:false,
 FakeMouseWithTouches:false,XRControllerInput:false,UseMic:false
}});
const player=new PixelStreaming(config,{videoElementParent:parent});
let live=false;
function waiting(text){live=false;placeholder.hidden=false;message.textContent=text;}
player.addEventListener('webRtcDisconnected',()=>waiting('Unreal 실행 대기 · 자동 재연결'));
player.addEventListener('webRtcFailed',()=>waiting('영상 연결 재시도 중 · 로컬 전송 서버를 확인하세요'));
player.addEventListener('playStreamRejected',()=>{message.textContent='브라우저가 자동 재생을 막았습니다. 영상 재생 버튼을 눌러주세요.';document.getElementById('pixel-play').hidden=false;});
parent.addEventListener('playing',()=>{live=true;placeholder.hidden=true;message.textContent='Unreal 영상 수신 중 · 관전자 화면';document.getElementById('pixel-play').hidden=true;},true);
document.getElementById('pixel-play').addEventListener('click',()=>{const video=parent.querySelector('video');if(video)video.play().catch(()=>{});});
// Do not present a frozen last frame as a running experience.
setInterval(()=>{const video=parent.querySelector('video');if(live&&(!video||video.readyState<2))waiting('영상 수신 대기');},2000);
window.addEventListener('pagehide',()=>player.disconnect());
