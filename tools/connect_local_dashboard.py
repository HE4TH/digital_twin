"""Editor-only local dashboard bridge. Does not enable MQTT or operate GPIO."""
import builtins
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import time
import urllib.request
import unreal as ue

root = Path(ue.Paths.project_dir()).resolve().parent
config_path = root/'dashboard/.runtime/remote-bridge.json'
if not config_path.exists():
    config_path = root/'dashboard/.runtime/bridge.json'
config = json.loads(config_path.read_text(encoding='utf-8'))
from urllib.parse import urlsplit
endpoint = urlsplit(config['url'])
if endpoint.scheme != 'http' or endpoint.hostname not in ('127.0.0.1', '192.168.0.6') or endpoint.username or endpoint.password:
    raise RuntimeError('Dashboard must be loopback or the configured Pi')
old = getattr(builtins, '_local_dashboard', None)
if old:
    ue.unregister_slate_post_tick_callback(old['handle'])
    old['pool'].shutdown(wait=False, cancel_futures=True)
state = {'pool':ThreadPoolExecutor(max_workers=1), 'future':None, 'next':0, 'ack':None, 'seen':None}
builtins._local_dashboard = state


def exchange(payload):
    req = urllib.request.Request(config['url']+'/api/bridge', json.dumps(payload).encode(),
        {'Content-Type':'application/json', 'X-Bridge-Token':config['token']})
    with urllib.request.urlopen(req, timeout=0.8) as response:
        return time.monotonic(), json.load(response)


def tick(delta):
    now = time.monotonic()
    if now < state['next']:
        return
    state['next'] = now+0.25
    world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_game_world()
    pc = ue.GameplayStatics.get_player_controller(world, 0) if world else None
    if state['future']:
        if not state['future'].done():
            return
        try:
            finished, response = state['future'].result()
            state['ack'] = None
            action = response.get('action')
            if action and action['id'] != state['seen']:
                state['seen'] = action['id']
                try:
                    if not pc or now-finished > 1:
                        raise RuntimeError('Play 연결 또는 요청 유효시간 확인 실패')
                    methods={'manual':'resume_manual_exploration','return':'request_exploration_return','stop':'stop_exploration'}
                    getattr(pc, methods[action['action']])()
                    state['ack']={'id':action['id'],'message':{'manual':'수동 전환 처리','return':'복귀 요청 처리','stop':'정지 처리'}[action['action']]}
                except Exception as exc:
                    state['ack']={'id':action['id'],'message':'실패: '+str(exc)}
        except Exception:
            pass
        state['future'] = None
    payload = {'play':bool(pc), 'speed':None, 'mode':'대기', 'detail':'', 'ack':state['ack']}
    if pc:
        try:
            pawn=ue.GameplayStatics.get_player_pawn(world, 0)
            if pawn:
                payload['speed']=pawn.get_velocity().length()/44.704
            component=pc.get_component_by_class(ue.ExplorationReturnComponent)
            if component:
                snapshot=component.get_return_state()
                payload['mode']=str(snapshot.mode).split('.')[-1].split(':')[0].strip('> ')
                payload['detail']=str(snapshot.detail)
        except Exception as exc:
            payload['detail']='상태 읽기 오류: '+str(exc)
    state['future']=state['pool'].submit(exchange,payload)


state['handle']=ue.register_slate_post_tick_callback(tick)
ue.log('DASHBOARD_CONNECTED: '+config['url']+'; MQTT defaults unchanged')
