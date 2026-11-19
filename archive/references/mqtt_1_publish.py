file name : mqtt_1_publish.py

import paho.mqtt.client as mqtt
import time
import random

# 1. 우체국(MQTT 브로커) 주소 설정 (라즈베리파이나 PC의 IP 주소)
broker_address = "192.168.0.100" 
client = mqtt.Client(client_id="AI_Rover_Sensor")

# 2. 우체국에 연결
client.connect(broker_address)
print("우체국 연결 완료! 데이터 전송을 시작합니다.")

try:
    while True:
        # 3. 초음파 센서 데이터라고 가정 (예시를 위해 랜덤 값 사용)
        # 실제로는 여기서 초음파 센서의 거리 값을 읽어옵니다.
        distance = random.uniform(10.0, 50.0) 
        
        # 4. 'rover/sensor/ultrasonic' 이라는 주제(Topic)로 우체국에 데이터 발행
        client.publish("rover/sensor/ultrasonic", f'{distance:.2f}')
        print(f'[발행] 현재 장애물 거리: {distance:.2f} cm')
        
        time.sleep(1) # 1초마다 보냄

except KeyboardInterrupt:
    print('통신을 종료합니다.')
    client.disconnect()

