# file name : mqtt_3_subscribe_try.py

import paho.mqtt.client as mqtt

# 1. 우체국에 연결 성공했을 때 실행될 행동 (콜백 함수)
def on_connect(client, userdata, flags, rc):
    print('우체국 연결 완료! 가상 세계의 명령을 기다립니다...')
    # 통신이 끊겼다가 재연결되어도 다시 구독할 수 있도록 여기에 작성합니다.
    client.subscribe("rover/command/move")

# 2. 우체국에서 편지가 도착했을 때 실행될 행동 (콜백 함수)
def on_message(client, userdata, message):
    command = str(message.payload.decode("utf-8"))
    print(f'[수신] 가상 세계로부터 명령이 도착했습니다: {command}')
    
    # 받은 명령에 따라 모터 제어 (뼈대 로직)
    if command == "FORWARD":
        print('-> 붕붕! AI-Rover 전진합니다!\n')
        # motor_forward() 
    elif command == "STOP":
        print('-> 끼익! AI-Rover 정지합니다!\n')
        # motor_stop()

# 3. 통신 준비 (최신 paho-mqtt 2.0 버전에 맞춘 API 버전 명시)
broker_address = "192.168.0.100"
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id="AI_Rover_Motor")

# 4. 편지가 오거나 연결될 때 실행할 함수 예약
client.on_connect = on_connect
client.on_message = on_message 

# 5. 우체국 연결 및 대기
try:
    client.connect(broker_address)
    client.loop_forever() # 프로그램이 종료되지 않고 계속 편지를 기다림
except KeyboardInterrupt:
    print("\n통신을 종료합니다.")
    client.disconnect()