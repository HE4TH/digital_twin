# 수업용 오리지널 고래 모델

이 폴더의 고래는 이 과제를 위해 Blender 4.5에서 직접 만든 저용량 **정적 메시**입니다. 외부 마켓 에셋이 아닙니다.

- `Blue_Whale_Original.blend`: 수정 가능한 Blender 원본
- `Blue_Whale_Original.fbx`: Unreal Engine에서 가져올 기본 파일
- `Blue_Whale_Original.glb`: 미리보기·다른 도구용 교환 파일
- `Blue_Whale_Preview.png`: 모델 확인 이미지
- `make_whale.py`, `render_preview.py`: 생성·미리보기 스크립트

모델은 **+X 방향**을 바라보고, Blender 단위는 **미터**, 전체 길이는 약 **11 m**입니다. Unreal의 FBX 임포트에서는 Static Mesh로 가져오고 머티리얼을 함께 가져오세요. 현재 뼈대·수영 애니메이션은 없습니다. 고래를 움직이려면 학생이 Sequencer에서 모델 전체의 위치를 움직이거나, 지느러미·꼬리용 리깅과 애니메이션을 추가하면 됩니다.

탐사 과제에서는 먼저 해저 관찰 지점에 정지한 고래로 배치하고, 실물 로버와 UE 차량이 같은 MQTT 명령으로 주행·정지하는지 검증한 뒤 움직임을 붙이는 순서를 권장합니다.
