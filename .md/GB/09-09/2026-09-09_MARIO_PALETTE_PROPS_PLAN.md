# G01. 마리오 공·폭탄 Map Assets Palette 등록

작성일: 2026-09-09. 사용자가 요청한 범위는 노란 공·파란 공·폭탄의 정적 외형 배치다.
공격 판정, 체력, 폭발, 스켈레탈 애니메이션, 자동 스폰은 이번 변경에 포함하지 않는다.

## 실제 호출 경로와 변경 위치

- `Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapassets`의 기존 323개 행 뒤에 3개를 추가하고 header count를 326으로 변경한다.
- `Data/Maps/MapCatalog.json`의 쿠크 Area `assetCount`를 실측 326으로 맞춘다. 기존 placement는 변경하지 않는다.
- 원본 glTF를 기존 `Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe`의 `--pretransform --no-auto-textures --scale 100`으로 정적 쿠킹한다. 원본 skeletal bind geometry를 정적 배치용으로 사용하며 원본 animation을 지원한다고 기록하지 않는다.
- WModel의 diffuse/normal/specular를 명시 remap한다. 노란 공은 `mn_ppcc_00a_d2/n1/s1`, 파란 공은 `mn_ppcc_00a_d3/n2/s2`, 폭탄은 `mn_rhcn_01_d_loc_int/n/s`다. `_s`는 ORM이 아니다.
- remap key는 glTF 실제 재질 이름을 사용한다. 두 공은 `mn_ppcc_00_mi`, 폭탄은 `mn_rhcn_01_mi`다. LookInfo의 variant 이름을 glTF key로 사용하지 않는다. 출력 WModel에는 `textures/<파일명>.dds`만 기록하고 해당 모델 옆 textures 폴더로 해석한다.
- 최종 리소스는 `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/MarioProps/{YellowBall,BlueBall,Bomb}/`에 WModel과 각 textures 폴더만 설치한다. glTF/UPK/props/조사 로그는 `out/`에 둔다.
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED`로 runtime catalog를 생성한다. runtime 문서를 직접 편집하지 않는다.

소비자는 기존 `CMapAssetCatalog -> CMapTool::Render_Palette/Select_Asset/Arm_SelectedAsset -> CMapAssetObject -> CModel -> CMaterial`이다. BottomCenter anchor로 바닥 클릭 위치를 모델 바닥에 맞춘다. Loader의 0.01 pretransform과 cooker의 100 배율이 상쇄된다. stable asset ID는 catalog에, 사용자가 만드는 placement ID/Transform은 기존 authoring에 저장된다. 원본 323개 정의 및 사용자 배치는 보존한다.

## 추가할 catalog 전체 행

```text
"MAP_MARIO_YELLOW_BALL" "Mario Yellow Ball (Triangle)" "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/YellowBall/YellowBall.wmodel" "Prototype_Component_Model_MAP_MARIO_YELLOW_BALL" 1 1 1 BottomCenter "mario_props" "Mario Props" "노란 공 / 삼각형. Static placement only. Source MN_PPCC_00-5 -> MN_PPCC_00_SK + MN_PPCC_00-2A_MI; diffuse d2, normal n1, specular s1." Opaque Back 1 1 0 0 1 0 1 50 1 1 1 1 1
"MAP_MARIO_BLUE_BALL" "Mario Blue Ball (Diamond)" "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/BlueBall/BlueBall.wmodel" "Prototype_Component_Model_MAP_MARIO_BLUE_BALL" 1 1 1 BottomCenter "mario_props" "Mario Props" "파란 공 / 마름모. Static placement only. Source MN_PPCC_00-6 -> MN_PPCC_00_SK + MN_PPCC_00-3A_MI; diffuse d3, normal n2, specular s2." Opaque Back 1 1 0 0 1 0 1 50 1 1 1 1 1
"MAP_MARIO_SKULL_BOMB" "Mario Skull Bomb" "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/Bomb/Bomb.wmodel" "Prototype_Component_Model_MAP_MARIO_SKULL_BOMB" 1 1 1 BottomCenter "mario_props" "Mario Props" "해골 폭탄. Static placement only; no explosion gameplay. Source MN_RHCN_01 -> MN_RHCN_01_SK + MN_RHCN_01_MI; diffuse d_loc_int, normal n, specular s." Opaque Back 1 1 0 0 1 0 1 50 1 1 1 1 1
```

## 프로젝트 등록과 검증

신규 C++/JSON 파일 및 shader 변경은 없다. 기존 mapassets와 MapCatalog.json은 이미 Client 프로젝트 Data 항목에 등록되어 있으므로 project/filter 변경은 필요 없다. C++ 재컴파일 없이 데이터 및 Resource 갱신 후 새 Client 세션에서 확인한다.

1. 기존 converter의 info와 WModel reader로 3개 정적 모델, 유한 bounds, UV/정점/삼각형, 실제 submesh의 material 슬롯을 검사한다.
2. 모든 참조 DDS를 열고 크기·경로와 원본 일치를 검사한다. 새로운 variant의 최종 Resource 상대 경로 이탈을 거부한다.
3. publisher Validate -> Publish -> Check, 변경 JSON parse, `git diff --check`를 실행한다. publisher가 변경할 기존 runtime 문서는 실행 전 byte 비교해 예상 범위를 확인한다.
4. 사용자 확인: Lobby Test -> F1 -> Map Tool -> KoukuSaydon -> Map Assets -> Palette 검색 `Mario`. 각 항목을 선택해 미리보기, 더블클릭 또는 Arm placement 후 바닥 클릭, Transform 조절, Save/Reload를 확인한다.
5. 실제 Client 실행·화면 판정은 사용자 전용이다. 에이전트는 이를 PASS로 기록하지 않는다. 다른 PC에는 `Map/LV_LUT_MIDNIGHTC_ED/MarioProps` 폴더를 Drive로 별도 전달한다.

독립 검토는 기존 NONANIM 타입 검사, BottomCenter, material-relative 경로, culling 소비 경로를 확인했다. C++ 확장 없이 기존 data 계약으로 등록한다.

# G02. 추가 네 외형과 트리거용 표시 모션

사용자 추가 요청은 빨간 별 공, 줄무늬 공, 광대 얼굴 공, 나팔 광대의 정적 외형이다. 불꽃, 공격, AI와 실제 트리거 위치 지정은 제외한다. 광대 몸체는 `MN_REUP_04_SK`, LookInfo의 부품은 `WP_MN_REUP_01_SK` / `WP_MN_REUP_01-1_MI`다. `wp_01` socket은 `bip001-prop1`, 위치 0, Pitch 16384(90도), Scale 1이다. 원본 bind pose에서 부품을 결합해 하나의 NONANIM 소품으로 쿠킹한다. 나팔을 부는 애니메이션 자세를 재현한다고 주장하지 않는다.

## 변경 계약

- `MarioProps/RedStarBall`, `StripedBall`, `ClownFaceBall`, `HornClown` 네 폴더를 추가한다. 각 폴더 이름과 같은 WModel을 둔다.
- 기존 mapassets header 바로 뒤에 `MAP_MARIO_RED_STAR_BALL`, `MAP_MARIO_STRIPED_BALL`, `MAP_MARIO_CLOWN_FACE_BALL`, `MAP_MARIO_HORN_CLOWN`을 같은 26-token Opaque/Back/BottomCenter 계약으로 추가한다. catalog count는 326에서 330으로 변경한다. 기존 326개 행의 상대 순서는 유지한다.
- diffuse/normal/specular는 각각 `mn_ppcc_00a_d1/n/s`, `mn_ppcc_00_c/n/s`, `mn_rhcn_00_d/n/s`, `mn_reup_04_d/n/s`다. 광대 나팔 부품은 `wp_mn_reup_01-1_d`와 `wp_mn_reup_01_n`만 명시 연결한다. 확인되지 않은 specular나 불꽃 텍스처를 붙이지 않는다.
- `worldsequences.json` revision 412를 413으로 올리고 기존 11 resources / 95 templates / 131 instances 배열 선두에 각각 네 개만 추가한다. 기존 항목과 사용자 위치/키는 보존한다.
- 각 소품의 suffix는 `red_star_ball`, `striped_ball`, `clown_face_ball`, `horn_clown`이다. object ID는 `world.object.mario.<suffix>`, template ID는 `sequence.mario.<suffix>.show`, instance ID는 `world.object.instance.mario.<suffix>.show`다.
- resource는 WORLD anchor, animated false, modelPreScale 0.01, scale [1,1,1], diffuse override 빈 문자열이며 위 WModel을 사용한다. defaultMotionInstanceId는 해당 show instance다.
- template은 durationMs 1000, LINEAR, category WorldObject, slot object의 0/1000ms 두 키가 visible true다. XZ는 모델 bounds 중앙을 0으로 옮기고 Y는 최하단을 0으로 옮기는 동일 positionOffset을 사용한다. 회전 quaternion [0,0,0,1], scaleMultiplier [1,1,1], 효과/애니메이션 track은 빈 배열이다. objectMotion의 모든 이동·회전·확산은 0, count 1, intervalMs 0, seed 1이다.
- instance는 enabled true, startDelayMs 0, playbackSpeed 1, WORLD anchor, position [0,0,0], motionEnd HOLD, nextMotionId 빈 문자열이다. binding은 slot object / OBJECT_RESOURCE / 해당 object ID다. 로드만으로 생성하지 않고 명시적인 Play 뒤에만 표시한다. 위치 0은 사용자가 수정할 기본값이며 특정 마리오 스폰 위치를 추정하지 않는다.

## 실제 소비자와 사용자 설정

F1 `World Object Tool`에서 Map 그룹의 네 소품/모션을 선택하고 `Map Position`을 조정한다. MapTool World Gameplay의 TriggerBox `Play Sequence Action`은 위 instance ID를 선택할 수 있다. Server는 기존 playSequence trigger 진입을 판정하고 Client `CWorldSequencePlayer -> CWorldSequenceObject -> CModel -> CMaterial`이 외형을 생성한다. HOLD는 모션 종료 후 외형을 유지한다. 실제 trigger 배치/Save 뒤에는 Map 및 WorldGameplay publisher와 Server 재시작이 필요하다. 이번 변경은 실제 TriggerBox나 몬스터 스폰을 임의 추가하지 않는다.

## 검증

기존 converter와 WModel reader로 메시/재질/리소스 경로를 검사하고 원본 삼각형·UV를 대조한다. 신규 resource-template-instance 참조 및 기존 항목 불변을 검사한다. Map publisher Validate/Publish/Check와 JSON parse, git diff --check를 실행한다. 기존 WorldSequence JSON은 이미 프로젝트 등록된 파일이며 새 C++/project/filter 변경은 없다. 사용자 Client 화면 판정은 별도로 남긴다.

독립 검토에서 WORLD/HOLD와 기존 trigger 연결을 확인했다. 기본 scale=1/회전 없음에서만 위 바닥 보정이 BottomCenter와 일치한다. World Object Tool에서 scale/회전을 바꾸면 키의 Position Offset도 조정한다. 일반 trigger는 duration override 0이므로 HOLD가 유지되지만 외부 cue에 durationMs가 지정되면 그 시간이 우선한다. 이 경로는 presentation이며 영구 server entity나 늦은 입장 동기화가 아니다.

## G02 정확한 추가 블록

mapassets header 바로 뒤에 삽입한다.

```text
"MAP_MARIO_RED_STAR_BALL" "Mario Red Ball (Star)" "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/RedStarBall/RedStarBall.wmodel" "Prototype_Component_Model_MAP_MARIO_RED_STAR_BALL" 1 1 1 BottomCenter "mario_props" "Mario Props" "빨간 별 공. Static placement only. Source MN_PPCC_00_SK + MN_PPCC_00-1A_MI." Opaque Back 1 1 0 0 1 0 1 50 1 1 1 1 1
"MAP_MARIO_STRIPED_BALL" "Mario Striped Circus Ball" "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/StripedBall/StripedBall.wmodel" "Prototype_Component_Model_MAP_MARIO_STRIPED_BALL" 1 1 1 BottomCenter "mario_props" "Mario Props" "줄무늬 서커스 공. Static placement only. Source MN_PPCC_00_SK + MN_PPCC_00_MI." Opaque Back 1 1 0 0 1 0 1 50 1 1 1 1 1
"MAP_MARIO_CLOWN_FACE_BALL" "Mario Clown Face Ball" "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/ClownFaceBall/ClownFaceBall.wmodel" "Prototype_Component_Model_MAP_MARIO_CLOWN_FACE_BALL" 1 1 1 BottomCenter "mario_props" "Mario Props" "광대 얼굴 공. Static placement only. Source WP_MN_RHCN_00.fm_d_rhcn_00 + MN_RHCN_00_MI." Opaque Back 1 1 0 0 1 0 1 50 1 1 1 1 1
"MAP_MARIO_HORN_CLOWN" "Mario Horn Clown (Static)" "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/HornClown/HornClown.wmodel" "Prototype_Component_Model_MAP_MARIO_HORN_CLOWN" 1 1 1 BottomCenter "mario_props" "Mario Props" "나팔 광대 / 정적 자세. Static placement only. Source MN_REUP_04_SK + WP_MN_REUP_01_SK at wp_01; static bind pose, no fire/AI." Opaque Back 1 1 0 0 1 0 1 50 1 1 1 1 1
```

worldsequences.json 각 동명 배열 선두에 다음 항목을 삽입한다. 기존 배열 원소는 유지한다.

### objectResources

```json
[
  {
    "objectId": "world.object.mario.red_star_ball",
    "displayName": "Mario Red Ball (Star) / 빨간 별 공",
    "modelAssetId": "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/RedStarBall/RedStarBall.wmodel",
    "anchorKind": "WORLD",
    "diffuseTextureAssetId": "",
    "modelPreScale": 0.01,
    "animated": false,
    "scale": [
      1,
      1,
      1
    ],
    "sequenceInstanceId": "",
    "defaultMotionInstanceId": "world.object.instance.mario.red_star_ball.show"
  },
  {
    "objectId": "world.object.mario.striped_ball",
    "displayName": "Mario Striped Circus Ball / 줄무늬 서커스 공",
    "modelAssetId": "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/StripedBall/StripedBall.wmodel",
    "anchorKind": "WORLD",
    "diffuseTextureAssetId": "",
    "modelPreScale": 0.01,
    "animated": false,
    "scale": [
      1,
      1,
      1
    ],
    "sequenceInstanceId": "",
    "defaultMotionInstanceId": "world.object.instance.mario.striped_ball.show"
  },
  {
    "objectId": "world.object.mario.clown_face_ball",
    "displayName": "Mario Clown Face Ball / 광대 얼굴 공",
    "modelAssetId": "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/ClownFaceBall/ClownFaceBall.wmodel",
    "anchorKind": "WORLD",
    "diffuseTextureAssetId": "",
    "modelPreScale": 0.01,
    "animated": false,
    "scale": [
      1,
      1,
      1
    ],
    "sequenceInstanceId": "",
    "defaultMotionInstanceId": "world.object.instance.mario.clown_face_ball.show"
  },
  {
    "objectId": "world.object.mario.horn_clown",
    "displayName": "Mario Horn Clown (Static) / 나팔 광대 / 정적 자세",
    "modelAssetId": "Map/LV_LUT_MIDNIGHTC_ED/MarioProps/HornClown/HornClown.wmodel",
    "anchorKind": "WORLD",
    "diffuseTextureAssetId": "",
    "modelPreScale": 0.01,
    "animated": false,
    "scale": [
      1,
      1,
      1
    ],
    "sequenceInstanceId": "",
    "defaultMotionInstanceId": "world.object.instance.mario.horn_clown.show"
  }
]
```

### templates

```json
[
  {
    "sequenceId": "sequence.mario.red_star_ball.show",
    "displayName": "Show Mario Red Ball (Star) / 빨간 별 공 표시 유지",
    "category": "WorldObject",
    "durationMs": 1000,
    "interpolation": "LINEAR",
    "objectMotion": {
      "velocity": [
        0,
        0,
        0
      ],
      "acceleration": [
        0,
        0,
        0
      ],
      "angularVelocityDegrees": [
        0,
        0,
        0
      ],
      "revolutionDegreesPerSecond": [
        0,
        0,
        0
      ],
      "revolutionOffset": [
        0,
        0,
        0
      ],
      "count": 1,
      "intervalMs": 0,
      "spreadDegrees": 0,
      "seed": 1
    },
    "tracks": [
      {
        "slotId": "object",
        "keys": [
          {
            "timeMs": 0,
            "positionOffset": [
              -5.722e-8,
              0.001468658447265625,
              0
            ],
            "rotationQuaternion": [
              0,
              0,
              0,
              1
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          },
          {
            "timeMs": 1000,
            "positionOffset": [
              -5.722e-8,
              0.001468658447265625,
              0
            ],
            "rotationQuaternion": [
              0,
              0,
              0,
              1
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          }
        ]
      }
    ],
    "animationTracks": [],
    "effectTracks": []
  },
  {
    "sequenceId": "sequence.mario.striped_ball.show",
    "displayName": "Show Mario Striped Circus Ball / 줄무늬 서커스 공 표시 유지",
    "category": "WorldObject",
    "durationMs": 1000,
    "interpolation": "LINEAR",
    "objectMotion": {
      "velocity": [
        0,
        0,
        0
      ],
      "acceleration": [
        0,
        0,
        0
      ],
      "angularVelocityDegrees": [
        0,
        0,
        0
      ],
      "revolutionDegreesPerSecond": [
        0,
        0,
        0
      ],
      "revolutionOffset": [
        0,
        0,
        0
      ],
      "count": 1,
      "intervalMs": 0,
      "spreadDegrees": 0,
      "seed": 1
    },
    "tracks": [
      {
        "slotId": "object",
        "keys": [
          {
            "timeMs": 0,
            "positionOffset": [
              -5.722e-8,
              0.001468658447265625,
              0
            ],
            "rotationQuaternion": [
              0,
              0,
              0,
              1
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          },
          {
            "timeMs": 1000,
            "positionOffset": [
              -5.722e-8,
              0.001468658447265625,
              0
            ],
            "rotationQuaternion": [
              0,
              0,
              0,
              1
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          }
        ]
      }
    ],
    "animationTracks": [],
    "effectTracks": []
  },
  {
    "sequenceId": "sequence.mario.clown_face_ball.show",
    "displayName": "Show Mario Clown Face Ball / 광대 얼굴 공 표시 유지",
    "category": "WorldObject",
    "durationMs": 1000,
    "interpolation": "LINEAR",
    "objectMotion": {
      "velocity": [
        0,
        0,
        0
      ],
      "acceleration": [
        0,
        0,
        0
      ],
      "angularVelocityDegrees": [
        0,
        0,
        0
      ],
      "revolutionDegreesPerSecond": [
        0,
        0,
        0
      ],
      "revolutionOffset": [
        0,
        0,
        0
      ],
      "count": 1,
      "intervalMs": 0,
      "spreadDegrees": 0,
      "seed": 1
    },
    "tracks": [
      {
        "slotId": "object",
        "keys": [
          {
            "timeMs": 0,
            "positionOffset": [
              -0.06464069366455079,
              0.3341184997558594,
              -4.3869e-7
            ],
            "rotationQuaternion": [
              0,
              0,
              0,
              1
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          },
          {
            "timeMs": 1000,
            "positionOffset": [
              -0.06464069366455079,
              0.3341184997558594,
              -4.3869e-7
            ],
            "rotationQuaternion": [
              0,
              0,
              0,
              1
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          }
        ]
      }
    ],
    "animationTracks": [],
    "effectTracks": []
  },
  {
    "sequenceId": "sequence.mario.horn_clown.show",
    "displayName": "Show Mario Horn Clown (Static) / 나팔 광대 / 정적 자세 표시 유지",
    "category": "WorldObject",
    "durationMs": 1000,
    "interpolation": "LINEAR",
    "objectMotion": {
      "velocity": [
        0,
        0,
        0
      ],
      "acceleration": [
        0,
        0,
        0
      ],
      "angularVelocityDegrees": [
        0,
        0,
        0
      ],
      "revolutionDegreesPerSecond": [
        0,
        0,
        0
      ],
      "revolutionOffset": [
        0,
        0,
        0
      ],
      "count": 1,
      "intervalMs": 0,
      "spreadDegrees": 0,
      "seed": 1
    },
    "tracks": [
      {
        "slotId": "object",
        "keys": [
          {
            "timeMs": 0,
            "positionOffset": [
              -0.268848876953125,
              0.0012882232666015625,
              0.02279958724975586
            ],
            "rotationQuaternion": [
              0,
              0,
              0,
              1
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          },
          {
            "timeMs": 1000,
            "positionOffset": [
              -0.268848876953125,
              0.0012882232666015625,
              0.02279958724975586
            ],
            "rotationQuaternion": [
              0,
              0,
              0,
              1
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          }
        ]
      }
    ],
    "animationTracks": [],
    "effectTracks": []
  }
]
```

### instances

```json
[
  {
    "instanceId": "world.object.instance.mario.red_star_ball.show",
    "templateId": "sequence.mario.red_star_ball.show",
    "enabled": true,
    "startDelayMs": 0,
    "playbackSpeed": 1,
    "anchorKind": "WORLD",
    "position": [
      0,
      0,
      0
    ],
    "motionEnd": "HOLD",
    "nextMotionId": "",
    "bindings": [
      {
        "slotId": "object",
        "targetKind": "OBJECT_RESOURCE",
        "targetId": "world.object.mario.red_star_ball"
      }
    ]
  },
  {
    "instanceId": "world.object.instance.mario.striped_ball.show",
    "templateId": "sequence.mario.striped_ball.show",
    "enabled": true,
    "startDelayMs": 0,
    "playbackSpeed": 1,
    "anchorKind": "WORLD",
    "position": [
      0,
      0,
      0
    ],
    "motionEnd": "HOLD",
    "nextMotionId": "",
    "bindings": [
      {
        "slotId": "object",
        "targetKind": "OBJECT_RESOURCE",
        "targetId": "world.object.mario.striped_ball"
      }
    ]
  },
  {
    "instanceId": "world.object.instance.mario.clown_face_ball.show",
    "templateId": "sequence.mario.clown_face_ball.show",
    "enabled": true,
    "startDelayMs": 0,
    "playbackSpeed": 1,
    "anchorKind": "WORLD",
    "position": [
      0,
      0,
      0
    ],
    "motionEnd": "HOLD",
    "nextMotionId": "",
    "bindings": [
      {
        "slotId": "object",
        "targetKind": "OBJECT_RESOURCE",
        "targetId": "world.object.mario.clown_face_ball"
      }
    ]
  },
  {
    "instanceId": "world.object.instance.mario.horn_clown.show",
    "templateId": "sequence.mario.horn_clown.show",
    "enabled": true,
    "startDelayMs": 0,
    "playbackSpeed": 1,
    "anchorKind": "WORLD",
    "position": [
      0,
      0,
      0
    ],
    "motionEnd": "HOLD",
    "nextMotionId": "",
    "bindings": [
      {
        "slotId": "object",
        "targetKind": "OBJECT_RESOURCE",
        "targetId": "world.object.mario.horn_clown"
      }
    ]
  }
]
```
