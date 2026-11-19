#!/usr/bin/env python3
# rover_ambulance.py (v2)  -  rover_drive.py 를 대체한다
#
# 조종(수동)과 자동 출동을 한 프로그램에서 처리한다.
# GPIO 는 한 프로세스만 점유할 수 있으므로 두 개로 나눌 수 없다.
#
#   구독 : veh/cmd       {"th":-1..1, "st":-1..1}          사람의 수동 조종
#          home/alert    {"type":..., "level":0~2}          집에서 온 경보
#          veh/dispatch  {"action":"go"|"return"|"cancel"}  관제의 출동 지령
#   발행 : veh/state     {"mode":..., "l":.., "r":.., ...}
#
# 모드
#   idle        대기
#   standby     경보 수신. 출동 승인 대기 (아직 안 움직임)
#   dispatched  출동 중 (전진)
#   arrived     현장 도착 (정지)
#   returning   복귀 중 (후진)
#   manual      사람이 조종 중
#
# 설계 의도
#   경보(home/alert)는 "무슨 일이 일어났다"일 뿐, 출동 명령이 아니다.
#   출동은 사람이 관제 화면에서 승인해야 시작된다 (veh/dispatch).
#   실제 응급 시스템도 관제사가 판단해서 출동을 지시한다.
#
#   그리고 사람의 수동 조종은 언제나 자동 시퀀스를 이긴다.

import sys, os, json, time, atexit, threading
import paho.mqtt.client as mqtt

sys.path.insert(0, os.path.expanduser('~/pi-rover/ch06_basic_control'))
from motor_module import motor1, motor2, motor3, motor4, move_stop

# ---- 설정 -----------------------------------------------------------------
BROKER         = 'localhost'
TOPIC_CMD      = 'veh/cmd'
TOPIC_ALERT    = 'home/alert'
TOPIC_DISPATCH = 'veh/dispatch'
TOPIC_STATE    = 'veh/state'

MAX_SPEED   = 0.6     # 수동 조종 상한
CMD_TIMEOUT = 0.3     # 수동 조종 워치독

DISPATCH_SPEED   = 0.5    # 출동 주행 속도
DISPATCH_SECONDS = 3.0    # 전진 시간 (트랙 길이에 맞춰 조정)
RETURN_SECONDS   = 3.0    # 복귀 후진 시간

STATE_HZ = 5
# ---------------------------------------------------------------------------

lock = threading.Lock()
state = {
    'mode':     'idle',
    'alert':    0,         # 현재 경보 수준 0~2
    'th':       0.0,
    'st':       0.0,
    'cmd_at':   0.0,
    'phase_at': 0.0,
}
client = None


def clamp(v):
    return max(-1.0, min(1.0, v))


def apply(th, st):
    """아케이드 믹싱: 왼쪽 = th + st, 오른쪽 = th - st"""
    l = clamp(th + st)
    r = clamp(th - st)
    motor3.value = l
    motor4.value = l
    motor1.value = r
    motor2.value = r
    return l, r


def set_mode(new_mode):
    """호출자가 lock 을 잡고 있어야 한다."""
    if state['mode'] == new_mode:
        return
    state['mode'] = new_mode
    state['phase_at'] = time.monotonic()
    print(f'   mode -> {new_mode}')


# ---- MQTT -----------------------------------------------------------------

def on_connect(c, userdata, flags, rc):
    print(f'[MQTT] connected (rc={rc})')
    for t in (TOPIC_CMD, TOPIC_ALERT, TOPIC_DISPATCH):
        c.subscribe(t)
    print(f'[MQTT] subscribed to {TOPIC_CMD}, {TOPIC_ALERT}, {TOPIC_DISPATCH}')


def on_cmd(d):
    th = clamp(float(d.get('th', 0)))
    st = clamp(float(d.get('st', 0)))
    now = time.monotonic()

    with lock:
        state['th'] = th
        state['st'] = st
        state['cmd_at'] = now

        # 사람이 실제로 조종하면 자동 시퀀스를 중단한다.
        if (abs(th) > 0.05 or abs(st) > 0.05) and state['mode'] != 'manual':
            print('   [override] 사람 조종 감지 -> 자동 시퀀스 중단')
            set_mode('manual')


def on_alert(d):
    """경보는 '무슨 일이 일어났다'일 뿐 출동 명령이 아니다."""
    a_type = str(d.get('type', '')).lower()
    room   = d.get('room', '?')
    level  = int(d.get('level', 0))

    with lock:
        state['alert'] = level

        if level >= 1:
            if state['mode'] == 'idle':
                print(f'   [ALERT] {a_type} @ {room} -> 출동 대기 (승인 필요)')
                set_mode('standby')
        else:
            # 상황 종료. 나가 있으면 알아서 복귀한다.
            if state['mode'] in ('dispatched', 'arrived'):
                print('   [CLEAR] 상황 종료 -> 복귀')
                set_mode('returning')
            elif state['mode'] == 'standby':
                print('   [CLEAR] 상황 종료 -> 대기')
                set_mode('idle')


def on_dispatch(d):
    """관제의 출동 지령. 여기서만 실제로 움직이기 시작한다."""
    action = str(d.get('action', '')).lower()

    with lock:
        if action == 'go':
            if state['mode'] in ('idle', 'standby', 'returning'):
                print('   [DISPATCH] 출동 승인 -> 출발')
                set_mode('dispatched')
        elif action in ('return', 'back'):
            if state['mode'] in ('dispatched', 'arrived'):
                print('   [DISPATCH] 복귀 지시')
                set_mode('returning')
        elif action == 'cancel':
            print('   [DISPATCH] 출동 취소')
            set_mode('idle' if state['alert'] == 0 else 'standby')


def on_message(c, userdata, msg):
    try:
        d = json.loads(msg.payload.decode('utf-8'))
    except Exception:
        return
    if msg.topic == TOPIC_CMD:
        on_cmd(d)
    elif msg.topic == TOPIC_ALERT:
        on_alert(d)
    elif msg.topic == TOPIC_DISPATCH:
        on_dispatch(d)


# ---- 제어 루프 -------------------------------------------------------------

def control_loop():
    last_print = None
    last_state = 0.0

    while True:
        now = time.monotonic()

        with lock:
            mode    = state['mode']
            th, st  = state['th'], state['st']
            cmd_at  = state['cmd_at']
            phase_t = now - state['phase_at']
            alert   = state['alert']

            cmd_live = (cmd_at != 0.0) and (now - cmd_at <= CMD_TIMEOUT)

            if mode == 'manual':
                if not cmd_live:
                    th = st = 0.0
                    set_mode('standby' if alert >= 1 else 'idle')
                else:
                    th, st = th * MAX_SPEED, st * MAX_SPEED

            elif mode == 'dispatched':
                th, st = DISPATCH_SPEED, 0.0
                if phase_t >= DISPATCH_SECONDS:
                    set_mode('arrived')
                    th = st = 0.0

            elif mode == 'returning':
                th, st = -DISPATCH_SPEED, 0.0
                if phase_t >= RETURN_SECONDS:
                    set_mode('standby' if alert >= 1 else 'idle')
                    th = st = 0.0

            else:  # idle, standby, arrived  — 전부 정지 상태
                th = st = 0.0

            cur_mode = state['mode']

        l, r = apply(th, st)

        tag = (round(th, 2), round(st, 2), cur_mode)
        if tag != last_print:
            print(f'[{cur_mode:<10}] th={th:+.2f} st={st:+.2f}  ->  L={l:+.2f} R={r:+.2f}')
            last_print = tag

        if client is not None and (now - last_state) >= (1.0 / STATE_HZ):
            last_state = now
            payload = json.dumps({
                'mode':  cur_mode,
                'alert': alert,
                'l': round(l, 3), 'r': round(r, 3),
                'th': round(th, 3), 'st': round(st, 3),
                'live': cmd_live
            })
            try:
                client.publish(TOPIC_STATE, payload, qos=0, retain=False)
            except Exception:
                pass

        time.sleep(0.05)


def cleanup():
    try:
        move_stop()
    except Exception:
        pass
    print('motors stopped.')


def main():
    global client

    atexit.register(cleanup)
    threading.Thread(target=control_loop, daemon=True).start()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id='rover_ambulance')
    client.on_connect = on_connect
    client.on_message = on_message

    print(f'출동 설정: 전진 {DISPATCH_SECONDS:.1f}초 @ 속도 {DISPATCH_SPEED:.1f}')
    print('경보를 받아도 바로 움직이지 않습니다. 관제의 출동 승인을 기다립니다.')
    print()

    try:
        client.connect(BROKER)
        client.loop_forever()
    except KeyboardInterrupt:
        print('\nstopped by user')
    finally:
        cleanup()


if __name__ == '__main__':
    main()
