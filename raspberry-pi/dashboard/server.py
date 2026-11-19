"""Dashboard on loopback or an explicitly configured LAN host. MQTT is subscribe-only."""
import argparse
from collections import deque
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import math
import os
from pathlib import Path
import secrets
import subprocess
import threading
import time
from urllib.parse import urlsplit, parse_qs
from quest_video import QuestVideo

ROOT = Path(__file__).resolve().parent


class State:
    def __init__(self):
        self.lock = threading.RLock()
        self.connected = False
        self.receiver = None
        self.received = 0
        self.ue = None
        self.ue_at = 0
        self.pending = None
        self.events = deque(maxlen=30)
        self.last_detail = None
        self.video = QuestVideo()
        self.quest_frame = None
        self.quest_frame_at = 0
        self.quest_seen = 0
        self.quest_sequence = 0
        self.viewer_at = 0
        self.quest_control = None
        self.quest_control_at = 0
        self.start_receiver = None
        self.quest_notices = []
        self.quest_badge = ""

    def event(self, text):
        self.events.appendleft({'time': time.strftime('%H:%M:%S'), 'text': text})

    def snapshot(self):
        with self.lock:
            now = time.monotonic()
            fresh = self.connected and now-self.received < 1.5
            ue_fresh = now-self.ue_at < 2
            if self.pending and now-self.pending['created'] > 2:
                self.event('요청 만료: Unreal 적용 확인을 받지 못했습니다')
                self.pending = None
            return dict(broker=self.connected, receiver=self.receiver if fresh else None,
                        ue=self.ue if ue_fresh else None, pending=bool(self.pending),
                        events=list(self.events),
                        quest=dict(connected=now-self.quest_seen < 4,
                                   streaming=self.quest_frame is not None and now-self.quest_frame_at < 4,
                                   sequence=self.quest_sequence, video=self.video.status(),
                                   notices=[dict(kind=n['kind'],text=n['text'],remaining=round(n['until']-now,3)) for n in self.quest_notices if n['until']>now] if now-self.quest_control_at < 3 else [],
                                   notice_badge=self.quest_badge if now-self.quest_control_at < 3 else '',
                                   notice_lease=max(0,3-(now-self.quest_control_at)),
                                   control=self.quest_control if now-self.quest_control_at < 3 else None),
                        receiver_start_available=self.start_receiver is not None)

    def telemetry(self, data, retained=False):
        if retained or not isinstance(data, dict):
            return
        for key in ('l', 'r', 'th', 'st'):
            val = data.get(key)
            if type(val) not in (int, float) or not math.isfinite(val) or abs(val) > 1:
                return
        if type(data.get('dry_run')) is not bool or type(data.get('live')) is not bool:
            return
        with self.lock:
            self.receiver = {k: data[k] for k in ('l','r','th','st','live','dry_run')}
            self.receiver['reason'] = str(data.get('reason', ''))[:80]
            self.received = time.monotonic()

    def action(self, action):
        with self.lock:
            snap = self.snapshot()
            if action == 'receiver_start':
                if not self.start_receiver:
                    raise ValueError('Pi 수신기 서비스가 아직 설정되지 않았습니다')
                if self.pending:
                    raise ValueError('이전 요청 응답을 기다리는 중입니다')
                control = snap['quest']['control']
                if snap['quest']['connected'] and not control:
                    raise ValueError('Quest 제어 상태를 확인할 수 없습니다. 새 APK를 연결하세요')
                if control and control.get('send_enabled'):
                    raise ValueError('먼저 RC 명령 송신을 꺼주세요')
                self.start_receiver()
                self.event('RC 수신기 실행 확인 완료')
                return
            if action in ('send_on', 'send_off', 'return') and snap['quest']['control']:
                if self.pending and action != 'send_off':
                    raise ValueError('이전 요청 응답을 기다리는 중입니다')
                if action == 'return' and not snap['quest']['control'].get('can_return'):
                    raise ValueError('현재 자동복귀를 시작할 수 없습니다')
                self.pending = dict(id=secrets.token_hex(8), action=action, target='quest', created=time.monotonic())
                self.event({'send_on':'RC 송신 켜기', 'send_off':'RC 송신 끄기', 'return':'자동복귀 시작'}[action]+' 요청 대기')
                return
            if action not in ('manual', 'return', 'stop'):
                raise ValueError('알 수 없는 요청')
            if not snap['ue'] or not snap['ue'].get('play'):
                raise ValueError('Unreal Play 연결을 먼저 확인하세요')
            if self.pending and action != 'stop':
                raise ValueError('이전 요청 응답을 기다리는 중입니다')
            self.pending = dict(id=secrets.token_hex(8), action=action, created=time.monotonic())
            self.event({'manual':'수동 전환','return':'자동복귀','stop':'정지'}[action]+' 요청 전송 대기')

    def bridge(self, data):
        with self.lock:
            self.ue = dict(play=bool(data.get('play')), speed=data.get('speed'),
                           mode=str(data.get('mode',''))[:80], detail=str(data.get('detail',''))[:300])
            self.ue_at = time.monotonic()
            ack = data.get('ack')
            if self.pending and self.pending.get('target') != 'quest' and isinstance(ack, dict) and ack.get('id') == self.pending['id']:
                self.event('Unreal 응답: '+str(ack.get('message',''))[:300])
                self.pending = None
            detail = self.ue['detail']
            if detail and detail != self.last_detail:
                self.event(detail)
                self.last_detail = detail
            self.snapshot()
            return {'action': self.pending if self.pending and self.pending.get('target') != 'quest' else None}


    def quest_update(self, data):
        with self.lock:
            self.quest_seen = self.quest_control_at = time.monotonic()
            self.quest_control = dict(send_enabled=data.get('send_enabled') is True,
                                      broker=data.get('broker') is True,
                                      can_return=data.get('can_return') is True,
                                      mode=str(data.get('mode',''))[:80])
            self.quest_notices = []
            notices = data.get('notices', [])
            if isinstance(notices, list):
                for n in notices[:3]:
                    if not isinstance(n, dict) or n.get('kind') not in ('return','whale','danger') or not isinstance(n.get('text'),str): continue
                    ttl=n.get('remaining')
                    if type(ttl) not in (int,float) or not math.isfinite(ttl) or ttl<=0: continue
                    self.quest_notices.append(dict(kind=n['kind'],text=n['text'][:500],until=self.quest_control_at+min(ttl,5)))
            badge=data.get('notice_badge','')
            self.quest_badge=badge[:80] if isinstance(badge,str) else ''
            self.snapshot()  # Expire commands before returning them to a reconnecting headset.
            ack = data.get('ack')
            if self.pending and self.pending.get('target') == 'quest' and isinstance(ack,dict) and ack.get('id') == self.pending['id']:
                self.event('Quest 응답: '+str(ack.get('message',''))[:200])
                self.pending = None
            return dict(wanted=self.quest_seen-self.viewer_at < 3,
                        action=self.pending if self.pending and self.pending.get('target') == 'quest' else None)


def start_rc_receiver():
    # Fixed service name; no user-supplied commands or shell execution.
    env = os.environ.copy()
    env['XDG_RUNTIME_DIR'] = '/run/user/'+str(os.getuid())
    try:
        subprocess.run(['systemctl','--user','start','dmix-rc-session.service'],
                       env=env, check=True, capture_output=True, timeout=5)
        subprocess.run(['systemctl','--user','is-active','--quiet','dmix-rc-session.service'],
                       env=env, check=True, capture_output=True, timeout=2)
    except (OSError, subprocess.SubprocessError) as exc:
        raise ValueError('RC 수신기 실행 실패: Pi 서비스 설치 상태를 확인하세요') from exc


def handler(state, token, port, public_host='127.0.0.1'):
    hosts = {f'127.0.0.1:{port}', f'localhost:{port}', f'{public_host}:{port}'}
    origins = {'http://'+host for host in hosts}
    sockets = ' '.join('ws://'+host+':8889' for host in {'127.0.0.1','localhost',public_host})
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args):
            pass

        def send(self, code, data, content='application/json; charset=utf-8'):
            body = data if isinstance(data, bytes) else json.dumps(data, ensure_ascii=False).encode()
            self.send_response(code)
            self.send_header('Content-Type', content)
            self.send_header('Content-Length', str(len(body)))
            self.send_header('Cache-Control', 'no-store')
            self.send_header('X-Content-Type-Options', 'nosniff')
            self.send_header('Content-Security-Policy', f"default-src 'self'; style-src 'self' 'unsafe-inline'; script-src 'self'; connect-src 'self' {sockets}; media-src 'self' blob:; frame-ancestors 'none'")
            self.end_headers()
            self.wfile.write(body)

        def valid_host(self):
            return self.headers.get('Host') in hosts

        def do_GET(self):
            if not self.valid_host():
                return self.send(403, {'error':'configured host required'})
            if self.path == '/api/state':
                with state.lock:
                    state.viewer_at = time.monotonic()
                return self.send(200, state.snapshot())
            if urlsplit(self.path).path == '/api/quest/video':
                try:
                    query=parse_qs(urlsplit(self.path).query)
                    after=int(query.get('after',['0'])[0]);session=query.get('session',[''])[0]
                    if after<0 or len(session)>32: raise ValueError('invalid cursor')
                    with state.lock:
                        state.viewer_at=time.monotonic()
                        result=state.video.read(session,after)
                    return self.send(200,result)
                except ValueError:
                    return self.send(400,{'error':'invalid video cursor'})
            if self.path == '/api/quest/demand':
                if self.headers.get('X-Quest-Stream') != '1':
                    return self.send(403, {'error':'quest header required'})
                with state.lock:
                    state.quest_seen = time.monotonic()
                    wanted = state.quest_seen-state.viewer_at < 3
                return self.send(200, b'1' if wanted else b'0', 'text/plain')
            if self.path.split('?',1)[0] == '/api/quest/frame':
                with state.lock:
                    frame = state.quest_frame if time.monotonic()-state.quest_frame_at < 4 else None
                return self.send(200, frame, 'image/jpeg') if frame else self.send(503, {'error':'no live Quest frame'})
            if self.path == '/pixel-player.js':
                bundle = ROOT/'.runtime/pixel-player.js'
                return self.send(200, bundle.read_bytes(), 'text/javascript; charset=utf-8') if bundle.exists() else self.send(503, {'error':'Run setup_pixel.ps1 first'})
            if self.path in ('/', '/app.js', '/quest-video.js'):
                name = 'index.html' if self.path == '/' else self.path[1:]
                return self.send(200, (ROOT/name).read_bytes(), 'text/html; charset=utf-8' if name.endswith('html') else 'text/javascript; charset=utf-8')
            self.send(404, {'error':'not found'})

        def do_POST(self):
            if not self.valid_host():
                return self.send(403, {'error':'configured host required'})
            origin = self.headers.get('Origin')
            if origin and origin not in origins:
                return self.send(403, {'error':'origin rejected'})
            if self.path == '/api/quest/video':
                if self.headers.get('X-Quest-Stream') != '1':
                    return self.send(403,{'error':'quest header required'})
                try:
                    length=int(self.headers.get('Content-Length','0'))
                    if self.headers.get('Content-Type')!='application/octet-stream' or not 12<length<=1100000:
                        raise ValueError('invalid video size')
                    self.connection.settimeout(2);payload=self.rfile.read(length)
                    if len(payload)!=length:raise ValueError('truncated video')
                    with state.lock:
                        state.video.ingest(self.headers.get('X-Video-Session',''),int(self.headers.get('X-Video-Width','0')),int(self.headers.get('X-Video-Height','0')),payload)
                        state.quest_seen=time.monotonic()
                    return self.send(200,{'ok':True})
                except (ValueError,OSError):
                    return self.send(400,{'error':'invalid AVC video'})
            if self.path == '/api/quest/frame':
                if self.headers.get('X-Quest-Stream') != '1':
                    return self.send(403, {'error':'quest header required'})
                try:
                    length = int(self.headers.get('Content-Length','0'))
                    if self.headers.get('Content-Type') != 'image/jpeg' or not 4 <= length <= 524288:
                        raise ValueError('expected JPEG up to 512 KiB')
                    self.connection.settimeout(3)
                    frame = self.rfile.read(length)
                    if len(frame) != length or not frame.startswith(b'\xff\xd8') or not frame.endswith(b'\xff\xd9'):
                        raise ValueError('invalid JPEG envelope')
                    with state.lock:
                        state.quest_frame = frame
                        state.quest_frame_at = state.quest_seen = time.monotonic()
                        state.quest_sequence += 1
                    return self.send(200, {'ok':True})
                except (ValueError, OSError):
                    return self.send(400, {'error':'invalid Quest frame'})
            if self.path == '/api/quest/control':
                if self.headers.get('X-Quest-Stream') != '1':
                    return self.send(403, {'error':'quest header required'})
            elif self.path == '/api/bridge':
                if not secrets.compare_digest(self.headers.get('X-Bridge-Token',''), token):
                    return self.send(403, {'error':'bridge token required'})
            elif self.path == '/api/action':
                if self.headers.get('X-Dashboard') != 'local':
                    return self.send(403, {'error':'dashboard header required'})
            else:
                return self.send(404, {'error':'not found'})
            try:
                length = int(self.headers.get('Content-Length','0'))
                if not 0 < length < 16384:
                    raise ValueError('invalid body size')
                self.connection.settimeout(3)
                data = json.loads(self.rfile.read(length))
                if not isinstance(data, dict):
                    raise ValueError('object required')
                if self.path == '/api/quest/control':
                    return self.send(200, state.quest_update(data))
                if self.path == '/api/bridge':
                    return self.send(200, state.bridge(data))
                state.action(data.get('action'))
                self.send(202, {'queued':True})
            except (ValueError, TypeError, AttributeError) as exc:
                self.send(400, {'error':str(exc)})
    return Handler


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--broker', default='localhost')
    parser.add_argument('--bind', default='127.0.0.1')
    parser.add_argument('--public-host', default='127.0.0.1')
    parser.add_argument('--mqtt-port', type=int, default=1883)
    parser.add_argument('--topic', default='codex/ue-bench/20261019/state')
    parser.add_argument('--port', type=int, default=5050)
    parser.add_argument('--no-mqtt', action='store_true')
    parser.add_argument('--rc-service', action='store_true', help='Enable the fixed Pi user service start button')
    args = parser.parse_args()
    state = State()
    if args.rc_service or os.environ.get('DMIX_RC_SERVICE') == '1':
        state.start_receiver = start_rc_receiver
    import ipaddress
    ipaddress.IPv4Address(args.bind)
    ipaddress.IPv4Address(args.public_host)
    runtime = ROOT/'.runtime'
    runtime.mkdir(mode=0o700, exist_ok=True)
    token_path = runtime/'bridge-token'
    try:
        fd = os.open(token_path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    except FileExistsError:
        token = token_path.read_text(encoding='utf-8').strip()
        if len(token) < 32:
            raise RuntimeError('Invalid bridge-token file')
    else:
        token = secrets.token_urlsafe(32)
        with os.fdopen(fd, 'w', encoding='utf-8') as token_file:
            token_file.write(token)
    server = ThreadingHTTPServer((args.bind, args.port), handler(state, token, args.port, args.public_host))
    (runtime/'bridge.json').write_text(json.dumps({'url':f'http://{args.public_host}:{args.port}', 'token':token, 'process_id':os.getpid()}), encoding='utf-8')
    (runtime/'bridge.json').chmod(0o600)
    client = None
    if not args.no_mqtt:
        import paho.mqtt.client as mqtt
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id='underwater-dashboard-'+secrets.token_hex(4))
        def connected(c, u, f, reason, p):
            with state.lock:
                state.connected = False
                if reason == 0:
                    c.subscribe(args.topic, qos=0)
                else:
                    state.event('MQTT 연결 거부')
        def subscribed(c,u,mid,reasons,p):
            with state.lock:
                state.connected = bool(reasons) and all(not r.is_failure for r in reasons)
                state.event('MQTT 구독 완료' if state.connected else 'MQTT 구독 실패')
        def disconnected(*_):
            with state.lock:
                state.connected = False
                state.received = 0
                state.event('MQTT 연결 끊김')
        def message(c,u,msg):
            try:
                if msg.topic == args.topic and len(msg.payload) < 4096:
                    state.telemetry(json.loads(msg.payload), msg.retain)
            except (ValueError, UnicodeError):
                pass
        client.on_connect=connected
        client.on_subscribe=subscribed
        client.on_disconnect=disconnected
        client.on_message=message
        client.reconnect_delay_set(1,5)
        client.connect_async(args.broker,args.mqtt_port,keepalive=5)
        client.loop_start()
    print(f'Dashboard: http://{args.public_host}:{args.port}', flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        if client:
            client.disconnect()
            client.loop_stop()
        server.server_close()


if __name__ == '__main__':
    main()
