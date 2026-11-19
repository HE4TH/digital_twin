# -*- coding: utf-8 -*-
# 고령자 집 낙상 센서 시뮬레이션 (이벤트 전용 — 주기적 vitals 스트림 없음)
#
# 위험 판정 기준: 낙상 센서 값이 False -> True로 바뀌는 순간을 낙상으로 판정한다.
# 오탐(false positive) 사례:
#   1. 사람이 침대/바닥에 눕는 정상 동작을 센서가 낙상으로 오인하는 경우
#   2. 문을 세게 닫는 소리나 물건 낙하 진동을 센서가 낙상으로 오인하는 경우
#
# 데모에서는 실제 센서 대신 두 가지 방법으로 낙상 발생을 트리거한다:
#   - 터미널에서 Enter 키 입력
#   - MQTT test/trigger/fall 토픽 발행 (테스트 대시보드에서 원격으로 트리거할 때 사용)

import json
import sys
import threading
import time

import paho.mqtt.client as mqtt

BROKER_HOST = "localhost"
BROKER_PORT = 1883
TOPIC_ALERT = "home/alert"
TOPIC_TRIGGER_FALL = "test/trigger/fall"
TOPIC_CAR_MODE = "car/mode"

fall_detected = False
lock = threading.Lock()


def trigger_fall(client, force=False):
    # force=True: 테스트 대시보드 버튼 등 사람이 명시적으로 누른 트리거는
    # 매번 새로운 낙상 사건으로 취급해 항상 발행한다(이전 상태와 무관).
    global fall_detected
    with lock:
        if fall_detected and not force:
            print("이미 위험 상태입니다 (home/alert는 상태 전환 시 1번만 발행됨)")
            return
        fall_detected = True

    payload = json.dumps({"type": "risk_start", "ts": int(time.time() * 1000)})
    client.publish(TOPIC_ALERT, payload)
    print(f"[home_sim] published {TOPIC_ALERT}: {payload}")


def on_connect(client, userdata, flags, reason_code, properties=None):
    print(f"[home_sim] connected, subscribing to {TOPIC_TRIGGER_FALL}, {TOPIC_CAR_MODE}")
    client.subscribe(TOPIC_TRIGGER_FALL)
    client.subscribe(TOPIC_CAR_MODE)


def on_message(client, userdata, msg):
    global fall_detected
    if msg.topic == TOPIC_TRIGGER_FALL:
        trigger_fall(client, force=True)
    elif msg.topic == TOPIC_CAR_MODE:
        try:
            mode = json.loads(msg.payload.decode("utf-8")).get("mode")
        except Exception:
            return
        if mode == "autonomous":
            # 자율주행 복귀 시작 = 상황 종료 -> 다음 데모 사이클을 위해 위험 상태 리셋
            with lock:
                fall_detected = False
            print("[home_sim] car/mode: autonomous 수신 -> 위험 상태 리셋, 다음 낙상 트리거 대기")


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER_HOST, BROKER_PORT, keepalive=30)
client.loop_start()

if sys.stdin.isatty():
    print("Enter: 낙상 이벤트 시뮬레이션 발생 / q + Enter: 종료")
    print(f"(또는 MQTT {TOPIC_TRIGGER_FALL} 토픽으로 원격 트리거 가능)")
    while True:
        line = input()
        if line.strip().lower() == "q":
            break
        trigger_fall(client)
    client.loop_stop()
    client.disconnect()
else:
    # 데몬/원격 실행 시 stdin이 없으므로, MQTT test/trigger/fall만으로 동작
    print(f"stdin 없음 — MQTT {TOPIC_TRIGGER_FALL} 트리거로만 낙상 이벤트를 받습니다")
    threading.Event().wait()
