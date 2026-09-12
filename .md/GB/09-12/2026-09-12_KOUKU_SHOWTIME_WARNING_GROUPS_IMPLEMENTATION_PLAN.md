# 세이튼 노란 예고 장판과 공격 그룹 구현 계획

## G00. 원본 연결과 범위

사용자가 첨부한 이미지의 노란 원형·도넛·부채꼴 예고 장판을 기존 공격 표현 앞에 연결해 독립 V1 그룹 세 종류를 만든다. 현재 쇼타임 생성기는 Projectile의 ParticleSystem 폭발 leaf를 가져오지만 FixedArea의 SkillDecal 예고 설정을 포함하지 않는다.

원형 Projectile 421991207은 SkillEffect 421991218의 반경 400 cm, SkillDecal 2112를 가리킨다. 도넛 421991208은 SkillEffect 421991220의 내반경 400 cm·외반경 800 cm, SkillDecal 2116을 가리킨다. 원본 FixedArea 후미의 1.5초 timer와 경고 구간을 보존한다.

SkillDecal의 `GR_Mon_Circle_cond_EX_01`과 `GR_Mon_Donut_cond_EX_01`을 data3.lpk의 ParticleSoundNew GroundEffect로 연결하면 각각 `FX_M_MI_O_00.FX_MI.FX_O_De_CondCircle_02_01_Tr`, `FX_O_De_CondMonDonut_02_01_Tr`이다. 같은 계열의 FanShape는 SkillDecal 2111과 `FX_O_De_CondMonFan_02_01_Tr`이다. 이 원본 material의 수식과 shape 입력을 기존 LocalDecal 경로로 투영한다.

부채꼴의 쇼타임 damage cone은 SkillEffect 421991212의 반경 1100 cm·45°이나, 해당 action에 2111 PlayDecalEffect 호출은 없다. 세 번째 독립 그룹의 이 결합은 원본 material·원본 범위를 이용한 프로젝트 저작 구성으로 구분하며 원본 쇼타임 occurrence 전체를 그대로 복원했다고 기록하지 않는다.

## G01. 구현

전용 생성기에서 원본 GroundEffect material과 FixedArea/SkillEffect 근거, 현재 설치된 폭발 문서를 읽는다. 예고 장판과 폭발의 수명·시작 시점을 한 V1 문서에서 소유하고 반복 발생을 만들지 않는다. 기존 쇼타임 leaf와 작은 오망성 변경은 보존한다.

현재 native 프로그램에 없는 원본 material 세 종류는 out에서 3600~3602 후보 contract와 HLSL을 생성한다. 기존 64개 단위 bucket 3584와 LocalDecal carrier를 사용하며 2304 단일 거대 셰이더로 되돌리지 않는다. 필요한 공유 material 표·shader 최소 설치는 Parent 작업자와 조정한다. 원본 shader 바이너리를 그대로 제품 runtime에 복사하지 않는다.

원본 GroundEffect가 참조하는 `EngineResources.DefaultTexture`의 실제 물리 입력은 설치 원본에서 확인되지 않는다. 원본 PS에서는 이 입력의 유한 차분을 caustic UV 흔들림에만 사용한다. 기존 원본 흰색 텍스처를 명시적으로 바인딩해 차분을 0으로 만들고, 실제 caustic·경계 선 텍스처와 원본 도형·색·시간 수식은 유지한다. 이 입력의 UV 흔들림은 미복원으로 기록한다. GroundEffect는 Cascade emitter가 아니므로 한 번 생성하는 LocalDecal adapter의 ID도 `project.groundeffect.adapter.*`로 구분한다.

세 V1 문서는 EffectCatalog와 ResourceTree의 쇼타임 장판 분류에 등록하고 필요한 Data project 항목만 추가한다. Composition 신규 resource는 Parent 작업자가 최신 Composition에 마지막으로 연결하며 이 생성기는 Composition을 수정하지 않는다. V2 Independent는 V1 그룹을 저장하지 않으므로 임의로 혼합하지 않는다.

## G02. 검증

JSON parse, 생성기 재실행 동일성, actual codec/CPU playback의 예고→공격 순서와 종료, native 입력 참조와 물리 Resource 존재, 변경 diff를 검사한다. 필요한 최소 shader compile은 out으로 한정한다. Client/UI 실행·조작·캡처는 하지 않으며 첨부 이미지와의 실제 GPU 표시 비교는 사용자 판정으로 남긴다.
