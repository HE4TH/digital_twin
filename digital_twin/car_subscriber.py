# -*- coding: utf-8 -*-
# 실물 RC카 노드 (전시회 프로젝트, 라즈베리파이4에 배포) — 전체 흐름 뼈대
#
# car/control (steering, throttle) 구독 -> 실제 모터 구동 (motor_module.py 재사용)
# car/telemetry 주기 발행 -> 언리얼 동기화용 (아직 위치 센서 없어 상태값만 우선 발행)
# car/mode 발행 -> 목적지 도착(arrived) / 자율주행 시작(autonomous), 상태 전환 시 1회씩
#
# 자율주행(복귀)은 이 RC카가 스스로 인식/판단하지 않는다 — 실제 바닥엔 차선이 없고,
# 가상 도로/차선은 언리얼 쪽에만 존재하기 때문. 언리얼이 가상 환경에서 자율주행(경로
# 추종/차선인식 등)을 계산해 그 결과를 방문객 조작 때와 동일하게 car/control 로 계속
# 발행하면, 이 RC카는 그걸 그대로 따라간다(= 가상 자율주행 결과를 실물에 미러링).
# 즉 AUTONOMOUS 상태에서도 car/control 을 계속 받아 모터에 반영한다.
#
# TODO (하드웨어/로직 준비되면 교체할 부분):
#   - ARRIVAL: 초음파/IR 등 실제 도착 감지 센서로 교체 (지금은 키보드 'a' 또는
#     MQTT test/trigger/arrival 토픽으로 임시 트리거)
#   - TELEMETRY: 위치 추정(오도메트리/비전 등) 붙이면 x/y도 같이 발행
#   - HOME(복귀완료): 언리얼이 가상 원위치 도착을 감지해 신호를 보내야 하는데 아직
#     미구현 -> 지금은 MQTT test/trigger/home 토픽으로 임시 트리거

import json
import signal
import sys
import threading
import time

import paho.mqtt.client as mqtt
import motor_module as motor

BROKER_HOST = "localhost"
BROKER_PORT = 1883

TOPIC_CONTROL = "car/control"
TOPIC_TELEMETRY = "car/telemetry"
TOPIC_MODE = "car/mode"
TOPIC_TRIGGER_ARRIVAL = "test/trigger/arrival"
TOPIC_TRIGGER_HOME = "test/trigger/home"  # 언리얼의 "가상 원위치 도착" 신호 임시 대체

SPEED = 0.3
TELEMETRY_HZ = 2

STATE_MANUAL = "manual"
STATE_ARRIVED = "arrived"
STATE_AUTONOMOUS = "autonomous"

state = STATE_MANUAL
last_action = "stop"
lock = threading.Lock()


def steering_throttle_to_action(steering, throttle):
    # TODO: motor_module이 속도 인자를 받으므로, 필요하면 throttle 크기로 continuous speed control 가능
    if throttle > 0.2:
        return "forward"
    if throttle < -0.2:
        return "backward"
    if steering < -0.2:
        return "left"
    if steering > 0.2:
        return "right"
    return "stop"


CURVE_INNER_RATIO = 0.6

ACTIONS = {
    "forward": motor.move_forward,
    "backward": motor.move_backward,
    "left": lambda speed: motor.move_curve_left(speed, inner_ratio=CURVE_INNER_RATIO),
    "right": lambda speed: motor.move_curve_right(speed, inner_ratio=CURVE_INNER_RATIO),
    "stop": lambda _speed: motor.move_stop(),
}


def publish_mode(client, mode):
    client.publish(TOPIC_MODE, json.dumps({"mode": mode, "ts": int(time.time() * 1000)}))
    print(f"[car] published {TOPIC_MODE}: mode={mode}")


def on_connect(client, userdata, flags, reason_code, properties=None):
    print(f"[car] connected, subscribing to {TOPIC_CONTROL}, {TOPIC_TRIGGER_ARRIVAL}, {TOPIC_TRIGGER_HOME}")
    client.subscribe(TOPIC_CONTROL)
    client.subscribe(TOPIC_TRIGGER_ARRIVAL)
    client.subscribe(TOPIC_TRIGGER_HOME)


def on_message(client, userdata, msg):
    global state, last_action

    if msg.topic == TOPIC_TRIGGER_ARRIVAL:
        threading.Thread(target=run_arrival_sequence, args=(client,), daemon=True).start()
        return

    if msg.topic == TOPIC_TRIGGER_HOME:
        with lock:
            if state != STATE_AUTONOMOUS:
                print("자율주행 상태가 아닙니다 (복귀 완료 신호 무시)")
                return
            state = STATE_MANUAL
        motor.move_stop()
        last_action = "stop"
        print("[car] 원위치 복귀 완료(언리얼 신호), MANUAL 대기 상태로 리셋")
        return

    with lock:
        if state == STATE_ARRIVED:
            return  # 도착 직후 짧은 전환 구간에만 조작 무시
        # MANUAL: 방문객이 직접 조작 / AUTONOMOUS: 언리얼의 가상 자율주행 결과를 그대로 반영

    try:
        payload = json.loads(msg.payload.decode("utf-8"))
        steering = float(payload.get("steering", 0.0))
        throttle = float(payload.get("throttle", 0.0))
    except Exception:
        return

    action = steering_throttle_to_action(steering, throttle)
    with lock:
        last_action = action
    ACTIONS[action](SPEED)


def telemetry_loop(client):
    while True:
        time.sleep(1.0 / TELEMETRY_HZ)
        with lock:
            payload = {"status": last_action, "mode": state}
        client.publish(TOPIC_TELEMETRY, json.dumps(payload))


def run_arrival_sequence(client):
    # TODO: 실제 도착 감지 센서(초음파 등) 붙이면 이 함수를 센서 폴링에서 호출
    global state, last_action

    with lock:
        if state != STATE_MANUAL:
            print("이미 도착/자율주행 상태입니다")
            return
        state = STATE_ARRIVED

    motor.move_stop()
    last_action = "stop"
    publish_mode(client, "arrived")

    time.sleep(1.0)  # TODO: 실제 "위험 끝" UI 처리 대기 시간으로 조정

    with lock:
        state = STATE_AUTONOMOUS
    publish_mode(client, "autonomous")
    # 여기서부터는 이 함수가 더 할 일이 없다 — 언리얼이 가상 자율주행 결과를
    # car/control 로 계속 발행하면 on_message 가 그대로 받아 모터에 반영한다.
    # 복귀 완료(원위치 도착)는 언리얼의 신호(test/trigger/home, 추후 실제 토픽으로 교체)로
    # on_message 에서 MANUAL로 리셋한다.


def keyboard_trigger_loop(client):
    print(f"도착 임시 트리거: 'a' + Enter (또는 MQTT {TOPIC_TRIGGER_ARRIVAL} 토픽) / q + Enter: 종료")
    while True:
        line = input()
        if line.strip().lower() == "q":
            motor.cleanup_motor()
            sys.exit(0)
        if line.strip().lower() == "a":
            run_arrival_sequence(client)


def shutdown(*_args):
    print("[car] shutting down, stopping motors")
    motor.cleanup_motor()
    sys.exit(0)


signal.signal(signal.SIGTERM, shutdown)
signal.signal(signal.SIGINT, shutdown)

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER_HOST, BROKER_PORT, keepalive=30)

threading.Thread(target=telemetry_loop, args=(client,), daemon=True).start()
client.loop_start()

try:
    if sys.stdin.isatty():
        keyboard_trigger_loop(client)
    else:
        # 데몬/원격 실행 시 stdin이 없으므로, MQTT test/trigger/arrival만으로 동작
        print(f"stdin 없음 — MQTT {TOPIC_TRIGGER_ARRIVAL} 트리거로만 도착 이벤트를 받습니다")
        threading.Event().wait()
finally:
    motor.cleanup_motor()
