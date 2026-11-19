// Epic's UE5.7 signalling implementation, bound only to this computer.
const http=require('http');
const {SignallingServer}=require('@epicgames-ps/lib-pixelstreamingsignalling-ue5.7');
const bind=process.env.DMIX_BIND||'127.0.0.1';
const publicHost=process.env.DMIX_PUBLIC_HOST||'127.0.0.1';
const origins=new Set(['http://127.0.0.1:5050','http://localhost:5050',`http://${publicHost}:5050`]);
const server=http.createServer((req,res)=>{
 res.writeHead(200,{'Content-Type':'application/json'});
 res.end(JSON.stringify({service:'underwater-pixel-local',streamers:signal.streamerRegistry.streamers.map(s=>s.streamerId)}));
});
const signal=new SignallingServer({httpServer:server,streamerPort:8888,
 streamerWsOptions:{host:bind},peerOptions:{iceServers:[]},maxSubscribers:4,
 playerWsOptions:{verifyClient:info=>!info.origin||origins.has(info.origin)}});
server.listen(8889,bind,()=>console.log(`Pixel signalling: ${bind}, streamer 8888 / viewer 8889`));
