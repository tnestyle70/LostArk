# 2026-08-16 LanceMaster visual evidence manifest

이 문서는 사용자가 제공한 LanceMaster 원본 화면을 스킬 커맨드와 타수별 시각 근거로 고정한다.
이미지는 `창술사원본/`과 `C:/Users/user/Desktop/로스트아크이펙트이미지/창술사/`에 같은
파일명과 같은 바이트로 보존한다. 이 화면은 source occurrence 후보와 재생 타임라인을 좁히는
참고 근거이며, 자동 visual PASS나 다른 스킬의 material/resource 근거로 승격하지 않는다.

| 파일 | SHA-256 | 관찰 역할 |
|---|---|---|
| `LanceMaster_Q00.png` | `2a501aaa4a8f244b7b6eb5a0eebcf58e67b80560ad5d8f0e85cfd3e07ae508ec` | 1타: BA 계열의 백색/청색 검격 경계와 여러 겹의 emissive arc |
| `LanceMaster_Q01.png` | `21735eeb4a886aeeb0acab7d8a9e56680b8e8451e48f69541ce53e1ab0a0e02f` | 2타: 전방으로 길게 형성되는 창 형태의 백색 mesh particle |
| `LanceMaster_W00.png` | `d991b044a86640c006f8f697de0848caced5c209681f3170b71c1b580478dbf8` | 바닥 충돌의 백색 emissive, 검은 dust와 초기 crack/decal 역할 |
| `LanceMaster_W01.png` | `e9a3f55406030f97ad81f6d41203e3491b2dcc5351745a70bc45b06cbcf66b55` | 후속 BA 계열 검격, 잔류 바닥 흔적·파편과 작은 백색 particle |
| `LanceMaster_E00.png` | `7db94ab0818c94723a7ea4ad8fd8f7db7daa5565dcdf7c3f7fafab585d857dca` | 1타: 돌진 전방의 캐릭터 크기 백색 보호막형 mesh와 작은 발광 입자 |
| `LanceMaster_E01.png` | `4b39b946a90af211c295c129c95f385749a72b3a7d25df0184745b42325dc97b` | 2타: 회전 창을 따르는 반투명 원형 mesh와 백색/검은 검격 몸체 |
| `LanceMaster_E02.png` | `aff4568a5aec902b8e27032ffca23c4b2149b1b4b7ede372ba6062575eb13bf8` | 3타 초기: 백색 대형 검격과 지면 충돌 emissive |
| `LanceMaster_E03.png` | `4e88819d6bdc2f5d86e28628a1e90828f920a03506443fe787d79ab8fd70adea` | 3타 잔류: crack/decal, 검은 흔적과 벽돌형 파편의 소멸 단계 |
| `LanceMaster_R00.png` | `76717d7a03da8c1dd3a05282a7ffda79dcbb7fdf7bbdd614f1edb39db47725ba` | 1타: 바닥 decal과 캐릭터 위 BA 계열 반투명 검격 |
| `LanceMaster_R01.png` | `38fac8e2b661c0bf2b3a84b25297841c5baf5465b204b0f502c46aa464fcdda6` | 후속 회전/3타: 전방에 겹쳐 생성되는 대형 백색·청색 검격 |
| `LanceMaster_A00.png` | `f32e8182cd6015414a66180a816d5c010990f880e2b2bd526fc3fb31d704fb33` | 전방 검격과 퍼져 소멸하는 검은 연기 sprite; 후속 타의 공통 검격 기준 |
| `LanceMaster_S00.png` | `bf9a4cb429b2c0c4d32f3be7fc273076fbed9eee37f2dab96a95c0d434da21bf` | 1타의 작은 검은 검격, 바늘형 방사 입자와 검은 dust |
| `LanceMaster_D00.png` | `59da801fa85059f6d1caa3c6a4163d722ddfe234e92ccb9522fdf911aedc1fb5` | 전방 백색 emissive/바늘 입자, 검은 검격 mesh와 잔류 파편 |
| `LanceMaster_F00.png` | `551a306d7b74cc015646bc84475ef7dd00093337969480ea27741e72f19ca105` | 1타: 캐릭터 주변의 노란 발광 sprite particle |
| `LanceMaster_F01.png` | `3a5135b5d4df4fefc25c2f8b08748994e5732a3b5873535bf8d47062c7cb29f0` | 2타: 거대해진 창과 창을 감싸는 백색·금색 emissive mesh |
| `LanceMaster_F02.png` | `fe3582a5b3c6facf247bc807541c03fbc1be5d44230b5016d96a53f1a5d429be` | 후속: 중앙이 투명한 대형 금색 원형 mesh와 주변 dust |

## 타수와 누락 경계

- Q는 `Q00=1타`, `Q01=2타`로 분리한다.
- E는 `E00=1타`, `E01=2타`, `E02/E03=3타의 초기·잔류 단계`로 묶는다. E02와 E03을
  서로 다른 gameplay stage로 임의 추가하지 않는다.
- R은 `R00=1타`, `R01=후속 회전·3타의 대표 화면`이다. 실제 clip/stage 수와 재생 시점은
  현재 `PlayerSkills -> skillbindings -> animevent` 정본을 따른다.
- A와 S의 후속 타는 사용자 설명상 BA 계열 검격을 공유하지만, 다른 타의 source occurrence를
  이름만으로 복사하지 않는다. exact clip-local source evidence가 있는 행만 각 타임라인에 둔다.
- BA는 현재 복원된 visual을 기준으로 사용하지만 이번 첨부에는 별도 BA 원본 화면이 없다.
  golden BA1 문서를 자동 materialize하거나 다른 스킬의 donor로 사용하지 않는다.
- V는 사용자 결정에 따라 보류한다. 이번 후보 선택, authored 수정, cue 연결 범위에서 제외한다.

## 적용 경계

- 현재 `PlayerSkills -> LanceMaster.skillbindings -> 기존 clip` 연결을 유지한다.
- 해당 clip의 exact source system 안에서 위 역할과 일치하는 stable occurrence만 Visible 후보로
  선택한다. 비선택 source element는 삭제하지 않고 `visible:false`와 provenance를 보존한다.
- Q/W/E/R/A/S의 기존 unified 문서는 새 스킬별 renderer를 만들지 않고 공용 typed
  profile/runtime 경로를 사용한다.
- 현재 corpus에서 누락된 D 34110과 F 34150은 오래된 slot 표기가 아니라 현재
  `PlayerSkills`, binding, animnotify를 다시 join해 intake한다. 다른 class 문서나 DDS/WModel을
  fallback으로 복사하지 않는다.
- Full, `AUTHORING_APPROXIMATE`, Hard/fail-closed는 source 계약으로 분류한다. Approximate는
  표시를 유지한 사용자 조정 후보가 될 수 있지만 Hard는 Visible/Product로 승격하지 않는다.
- 사용자 직접 Effect Tool 육안 확인 전에는 visual PASS로 기록하지 않는다.
