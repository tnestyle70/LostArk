# Guardian 그랜드 피날레 용 문양 정렬 구현 계획

## G00. 저장본과 원본 소비 경로

대상은 `effect.guardianknight.skill.49150.clip.1.full.restore`의 차징과 clip2의 공격 시작 문양이다. 별도 광포화 S 용 ModelCue 재질 작업과 구분한다. 사용자 저장본의 Sprite Particle03은 `kouku.49150.e3c543e9ec6c981056b0`, 04는 `authored.copy.kouku.49150.e3c543e9ec6c981056b0.1`이다. 두 뿔 element 전체와 다른 수동 편집을 보존한다.

clip1 용 문양은 emitter52/3/6/2 네 겹이다. 저장본에서 차오르는 emitter3만 Element 회전을 따르고 나머지는 기존 fixed-axis 방향을 사용한다. 기존 `CEffectPlayback`의 SourceEmitterWorld와 `Make_ParticleSpriteWorld`의 `followEmitterAxisRotation` 소비 경로로 정렬한다. 공용 shader 또는 particle renderer를 새로 만들지 않는다.

## G01. 용 문양의 위치와 완성 시점

차오르는 emitter3의 수동 TRS를 기준으로 나머지 완성·전이 문양을 정렬한다. 네 겹은 같은 source location, StartRotation, pivot과 활성 SizeLife를 사용한다. 실제 설치 GuardianKnight.wmodel의 b_effectworldzero 본 및 제품 scale normalization을 포함해 최종 quad 중심·법선·방향을 대조한다. source texture의 UV 회전과 geometry 회전을 구분한다.

emitter3의 원본 ColorScaleOverLife R이 끝에 도달하는 시간은 `0.25 + 1.5 / 1.05263162 = 1.674999944초`다. 이 UV 진행은 보존하고 emitter52/6/2의 생성 지연을 1.675초로 맞춘다. 완성된 문양52의 alpha scale만 기존 최고값0.6 상수로 사용하여 생성 순간부터 표시한다. 다른 flash·transition 곡선과 source 수명은 유지한다. 이 시점 변경은 원본 복원이라고 표기하지 않고 사용자 요청에 따른 저작 조정으로 기록한다.

공격 clip2에서 시작하는 완성 문양 emitter30/15도 같은 TRS와 회전 추종 설정을 사용한다. clip2의 시작0초, 수명0.4초 및 공격 시점은 유지한다. source 수명은 생성 시점을 기준으로 진행하므로 지연을 옮긴 clip1 잔존 효과와 clip2의 실제 화면은 사용자 확인 항목으로 구분한다.

## G02. 두 뿔 주변 이펙트

fx_j_normal_03을 사용하는 emitter54/24는 사용자03, emitter53/25는 사용자04의 배치에 맞춘다. 04는 원래 음의 Z 쪽 뿔을 복제한 것이므로 원본 양의 Z 쪽 emitter53/25에는 local Z -2m의 변환된 위치 보정을 적용한다. source 위치·방출·색·크기 곡선은 변경하지 않는다. 카메라를 향하는 원본 billboard의 성질은 보존하고 생성 위치와 진행 방향을 뿔의 배치에 맞춘다.

## G03. 후보 검증과 최종 반영

후보는 `out/GuardianGrandFinale20260930`에 준비한다. 실제 Codec, Playback, 최종 sprite geometry와 설치 WModel 본을 사용한 수치 검증 및 변경 문서의 공식 validator를 실행한다. Client/UI를 자동 실행하지 않으며 수치 검증을 화면 확인으로 기록하지 않는다.

최종 교체 시 최신 디스크를 다시 읽고 stable ID와 변경 필드만 병합한다. 두 뿔 전체와 무관한 element·root 설정은 유지한다. 변경 필드의 실제 충돌은 덮어쓰지 않는다. 교체 직전 SHA 확인, 백업, 원자 교체를 사용하고 미저장 메모리나 Reload를 자동 조작하지 않는다. 신규 C++ 파일과 프로젝트 등록은 필요 없다. 최종 Debug/Release Product 빌드는 진행 중인 발탄·쿠크 작업과 함께 수행한다.

## G04. 후속 편집 중 선택한 이펙트의 Local Space

사용자가 후속 이미지에서 선택한 문서는 `effect.guardianknight.skill.49260.clip.2.full.restore`, 블레이즈 스텝 ddk_sk_eurosloof_03이다. b_effectroot의9개 요소, .4초 시작 및 mesh·texture 목록으로 식별했다. 사용자 확인에 따라 이 그룹만이 아니라 해당 문서의 모든 particle을 대상으로 한다. 현재42개 particle 중 true인39개의 `detail.particle.localSpace`만 false로 변경한다. 위치·회전·소켓·material·source 모듈과 다른 스킬은 변경하지 않는다.

현재 화면에 보이는 소켓 편집은 디스크와 다를 수 있으므로 사용자 저장 완료 이후 최신 디스크에서 false 변경만 다시 생성·검증한다. 별도 renderer 변경이나 실행 중 draft 직접 조작은 하지 않는다. 이 후속 요청도 G03의 최종 교체 경계를 따른다.

## G05. 완성·전이 용 문양 3개를 자기 중심에서 시계 방향 회전

후속 사용자 요청은 clip1의 emitter52/6/2만이다. 차오르는 emitter3, clip2와 다른 사용자의 편집은 보존한다. 현재 세 요소는 같은 위치(-1.60000026,-1.20637143,-0.0740000159), 회전(-90,0,0), source fixed-axis 회전 추종을 사용한다. 원본 StartLocation(1,.1,0)m과 pivot(.5,.9)이 있으므로 Element 원점은 실제 문양 중심이 아니다.

기존 Element Transform 회전만 (0,90,-90)도로 교체한다. DirectX row-vector 기준 기존 Rx(-90)에 Rz(-90)를 뒤에서 합성한 값이며, 문양의 right를 -up으로, up을 right로 돌린다. 위치에는 source location과 pivot의 회전 차이(2.12,2.12,0)m를 보상하여 (0.51999974,0.91362857,-0.0740000159)로 저장한다. source 곡선·pivot·크기·socket·재질·생성 시점은 그대로 사용한다.

기존 actual Codec/Playback/최종 sprite geometry probe와 설치 GuardianKnight.wmodel 본을 사용해 중심·법선·크기 보존, 시계 방향90도와 세 겹의 일치를 확인한다. 최신 저장본에서 stable ID3개의 position/rotationDegrees6필드만 bytes 보존 패치하며 hash 재확인·백업·원자 교체를 사용한다. C++·shader·project/filter 변경 및 Product 재빌드는 필요 없다. 도구 미저장 draft와 최종 화면은 사용자 저장·Reload·확인 경계로 유지한다.

## G06. 차오르는 문양 정렬·완성 잔상 고정·뿔 주변 높이

사용자 화면 확인 뒤 범위를 보완한다. clip1의 차오르는 emitter3는 현재 완성52의
position과 rotationDegrees에 맞춘다. 생성 시각·UV 진행·크기는 유지하고 Local Space는 켠다.
완성 문양과 다른 위치였던 사용자 offset도 이번 동일 위치 요청에 따라 맞춘다.

사용자가 완성·찍기5개 모두 Local Space OFF를 확정했다. clip1의52/6/2와 clip2의30/15는
detail.particle.localSpace만 false로 바꾼다. 본 attachment follow와 fixed-axis rotation 추종은
유지하며, 생성 시점의 본 basis를 고정하는 기존 world particle 경로를 사용한다.
clip2 문양의 회전·위치에는 추가 변경을 하지 않는다.

뿔 주변 Sprite Particle은 검은 skintrail54/53/24/25의 원본4개와 복제3개뿐 아니라
동반 chargingcontrol69/70/66/5도 포함한다. 총11개 stable ID의 실제 높이를
사용자가 지정한0.1m만큼 낮춘다. 실제 b_effectworldzero 본에서 Element Y는 수평으로
향하므로 최초 Y-0.1 후보를 보완해 Y를 원래 값으로 복구하고 local Z+0.1로 환산한다.
제품 CModel의 Scale(.0001)*Ry(-90), 본 정규화, owner yaw를 포함해 world Y-0.1을 검증한다.
뿔 본체64와 복제64, 용 문양, 무기 notify022,
screenPost/light는 높이 조정에서 제외한다. 모든 다른 사용자 필드는 byte 단위로 보존한다.

최신 디스크로 후보2개를 만들고 field allowlist·JSON·공통 validator·actual Codec 및
기존 Playback/Geometry 검사로 정렬과 birth-space 소비를 확인한다. 승인된 변경은
최신 SHA 재확인·백업·원자 교체한다. 실행 중 draft/Reload를 조작하지 않는다.
direct-authored Data만 바꾸며 C++·shader·Resources·EXE 재빌드는 필요 없다.
