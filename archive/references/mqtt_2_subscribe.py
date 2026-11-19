file name : mqtt_2_subscribe.py

import paho.mqtt.client as mqtt

# 1. 우체국에서 편지가 도착했을 때 실행될 행동 (콜백 함수)
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

# 2. 통신 준비 및 우체국 연결
broker_address = "192.168.0.100"
client = mqtt.Client(client_id="AI_Rover_Motor")

# 3. 편지가 오면 'on_message' 함수를 실행하라고 예약
client.on_message = on_message 
client.connect(broker_address)

# 4. 'rover/command/move' 주제(Topic)를 구독 시작
client.subscribe("rover/command/move")
print('우체국 연결 완료! 가상 세계의 명령을 기다립니다...')

# 5. 프로그램이 종료되지 않고 계속 편지를 기다림
client.loop_forever()