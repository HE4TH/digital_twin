#!/usr/bin/env python3
# home_sim.py - 가상 스마트홈 센서 시뮬레이터
#
# 집 안의 거주자 상태를 시뮬레이션해서 MQTT로 발행한다.
# 실물 센서 없이 동작하므로 아무 PC 나 라즈베리파이에서 돌릴 수 있다.
#
#   발행 : home/vitals   1초마다  현재 상태 (늦게 접속한 구독자도 상황을 알 수 있음)
#          home/alert    전환 시점에 1회  (즉시 반응용)
#   구독 : home/trigger  수동 발생  {"type":"fall"} / {"type":"no_motion"} / {"type":"clear"}
#
#   실행 : python3 home_sim.py
#          python3 home_sim.py --auto     자동 시나리오 (시연용 아님, 테스트용)

import json, time, random, threading, argparse
import paho.mqtt.client as mqtt

BROKER        = 'localhost'
TOPIC_VITALS  = 'home/vitals'
TOPIC_ALERT   = 'home/alert'
TOPIC_TRIGGER = 'home/trigger'

VITALS_HZ = 1.0

# 상태 정의 --------------------------------------------------------------
#   normal     : 평상시
#   no_motion  : 일정 시간 움직임 없음 (주의)
#   fall       : 낙상 감지 (비상)
STATE_LEVEL = {'normal': 0, 'no_motion': 1, 'fall': 2}

state = {'name': 'normal', 'room': 'living', 'since': time.monotonic()}
lock = threading.Lock()
client = None


def vitals_payload():
    """현재 상태에 맞는 생체 신호를 만든다. 값은 시뮬레이션이다."""
    with lock:
        name = state['name']
        room = state['room']
        held = time.monotonic() - state['since']

    if name == 'fall':
        hr = random.randint(105, 130)      # 낙상 직후 심박 상승
        motion = False
    elif name == 'no_motion':
        hr = random.randint(58, 68)
        motion = False
    else:
        hr = random.randint(64, 78)
        motion = random.random() > 0.25

    return {
        'state':  name,
        'level':  STATE_LEVEL[name],
        'room':   room,
        'hr':     hr,
        'motion': motion,
        'held':   round(held, 1),          # 이 상태가 지속된 시간(초)
        't':      int(time.time() * 1000)
    }


def set_state(new_name, room=None):
    """상태를 바꾸고, 실제로 바뀐 경우에만 alert 를 1회 발행한다."""
    with lock:
        if state['name'] == new_name:
            return False
        state['name']  = new_name
        state['since'] = time.monotonic()
        if room:
            state['room'] = room
        cur_room = state['room']

    alert = {
        'type':  new_name,
        'room':  cur_room,
        'level': STATE_LEVEL[new_name],
        't':     int(time.time() * 1000)
    }
    if client:
        client.publish(TOPIC_ALERT, json.dumps(alert), qos=0, retain=False)

    mark = '!!' if new_name == 'fall' else ('!' if new_name == 'no_motion' else '  ')
    print(f'{mark} ALERT -> {new_name}  ({cur_room})')
    return True


def on_connect(c, userdata, flags, rc):
    print(f'[MQTT] connected (rc={rc}) -> subscribe "{TOPIC_TRIGGER}"')
    c.subscribe(TOPIC_TRIGGER)


def on_message(c, userdata, msg):
    try:
        d = json.loads(msg.payload.decode('utf-8'))
    except Exception:
        return

    t = str(d.get('type', '')).lower()
    room = d.get('room')

    if t in ('fall', 'no_motion'):
        set_state(t, room)
    elif t in ('clear', 'normal'):
        set_state('normal', room)


def auto_scenario():
    """테스트용 자동 시나리오. 시연에는 쓰지 말 것 (예측 불가)."""
    while True:
        time.sleep(random.randint(20, 40))
        set_state('no_motion')
        time.sleep(random.randint(8, 15))
        if random.random() < 0.5:
            set_state('fall')
            time.sleep(random.randint(10, 20))
        set_state('normal')


def main():
    global client

    parser = argparse.ArgumentParser()
    parser.add_argument('--auto', action='store_true', help='자동 시나리오 실행')
    parser.add_argument('--broker', default=BROKER)
    args = parser.parse_args()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id='home_sim')
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(args.broker)
    client.loop_start()

    if args.auto:
        threading.Thread(target=auto_scenario, daemon=True).start()
        print('[sim] 자동 시나리오 켜짐')

    print('[sim] 시작. 수동 발생:')
    print(f'  mosquitto_pub -h {args.broker} -t {TOPIC_TRIGGER} -m \'{{"type":"fall"}}\'')
    print(f'  mosquitto_pub -h {args.broker} -t {TOPIC_TRIGGER} -m \'{{"type":"clear"}}\'')
    print()

    # 콘솔은 조용하게 유지한다.
    # 매초 찍으면 정작 중요한 ALERT 줄이 묻혀서 시연 때 안 보인다.
    last_state = None
    last_beat = 0.0
    HEARTBEAT_SEC = 15

    try:
        while True:
            v = vitals_payload()
            client.publish(TOPIC_VITALS, json.dumps(v), qos=0, retain=False)

            now = time.monotonic()
            changed = (v['state'] != last_state)
            beat = (now - last_beat) >= HEARTBEAT_SEC

            if changed or beat:
                print(f"   {v['state']:<10} hr={v['hr']:>3}  motion={str(v['motion']):<5} "
                      f"room={v['room']}")
                last_state = v['state']
                last_beat = now

            time.sleep(1.0 / VITALS_HZ)
    except KeyboardInterrupt:
        print('\n[sim] 종료')
    finally:
        client.loop_stop()
        client.disconnect()


if __name__ == '__main__':
    main()
