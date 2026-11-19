# file name : mqtt_4_motor.py
# 모터 제어 프로그램

import paho.mqtt.client as mqtt
from motor_module import * # 작성해둔 AI-Rover 모터 제어 모듈 불러오기

# 1. 우체국에 연결 성공했을 때 실행될 행동
def on_connect(client, userdata, flags, rc):
    print('✅ 우체국 연결 완료! 가상 세계의 명령을 기다립니다...')
    # 통신이 끊겼다가 재연결되어도 다시 구독할 수 있도록 설정
    client.subscribe("rover/command/move")

# 2. 우체국에서 편지가 도착했을 때 실행될 행동 (콜백 함수)
def on_message(client, userdata, message):
    # 만약의 오타나 띄어쓰기를 방지하기 위해 대문자로 변환하고 공백 제거
    command = str(message.payload.decode("utf-8")).strip().upper()
    print(f'[수신] 명령 도착: {command}')
    
    # 3. 받은 명령에 따라 실제 motor_module.py의 함수 호출
    if command == "FORWARD":
        move_forward()      # 전진 함수 호출 (모듈에 정의된 이름에 맞게 수정)
    elif command == "BACKWARD":
        move_backward()     # 후진 함수 호출
    elif command == "LEFT":
        move_turn_left()       # 좌회전 함수 호출
    elif command == "RIGHT":
        move_turn_right()      # 우회전 함수 호출
    elif command == "STOP":
        move_stop()      # 정지 함수 호출
    else:
        print('Unknown command.')

# 4. 통신 준비 (MQTT 세팅)
broker_address = "192.168.0.100" # 브로커(우체국)가 설치된 PC의 IP 주소
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id="AI_Rover_Motor")

client.on_connect = on_connect
client.on_message = on_message 

# 5. 프로그램 실행 및 무한 대기
try:
    print('🚀 AI-Rover 수신 시스템을 가동합니다...')
    move_stop() # 시작할 때 혹시 모를 급발진 방지를 위해 정지 상태로 초기화
    
    client.connect(broker_address)
    client.loop_forever() # 편지가 올 때까지 계속 대기

except KeyboardInterrupt:
    print('Stopped. Ctrl+C pressed.')
except Exception as err:
    print(f'Error : {err}')

finally:
    # [매주 중요] 프로그램이 꺼질 때 모터가 계속 도는 것(Runaway) 방지
    move_stop() 
    client.disconnect()
    print('terminated.')