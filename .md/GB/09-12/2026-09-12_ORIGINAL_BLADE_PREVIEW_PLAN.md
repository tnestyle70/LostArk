# G01. 원본_칼날 6개 교차 미리보기

## 소유자와 변경 경계

기존 World Object Tool의 월드오브젝트_칼날 아래 바닥_칼날 다음에 template/instance 한 쌍을 추가한다. 부모 resource와 기본 모션, 갈고리를 포함한 기존 모든 행은 보존한다. C++/프로젝트 등록과 제품 전투 패턴은 변경하지 않는다.

## 원본 근거와 저작값

원본 Projectile 421991301은 Par_V_RPCT_Cutting_pjt_01을 참조한다. 09-07 원본 연결 기록과 현재 source recipe의 cutting_pjt_02_loc_int mesh.objectpath는 fx_sm_01.fm_o_cngn_01이다. 설치 CuttingBlade와 FullRestore/Meshes/fm_o_cngn_01.wmodel의 28228개 정점 위치가 순서까지 정확히 같고, winding을 제외한 삼각형 연결도 같다. index와 전체 WMSH byte는 다르므로 완전한 binary identity를 주장하지 않는다. native animation은 0개다. 바탕화면 README의 해당 메시가 없다는 문구는 이 실물/typed 참조와 충돌하므로 이름 검색의 음성 결과만으로 모델을 교체하지 않는다.

이 프리뷰는 원본 본체 메시를 사용하는 저작 모션이다. 원본 파티클 전체 재생이나 원작 수치의 완전 복원이 아니다. 사용자 확인에 따라 1/3/5와 2/4/6의 반대 방향 경로를 엇갈려 놓는다. 간격2m, 전진2m/s, 거리28m, 14초 동시생성, yaw312/132는 PROJECT_TUNED다. 기존 바닥_칼날의 upright quaternion과 X자전1440deg/s를 재사용하며 source-exact spin이라고 주장하지 않는다. 모델 회전 반경을 실측해 floor1.2m 위에 놓는다. 발사/착탄 Effect, 사운드, 플레이어 피해, 서버 패턴 연결은 추가하지 않는다.

## 전체 삽입 블록

templates의 world.object.kouku.cutting_blade.state.1 바로 뒤, instances의 해당 기존 인스턴스 바로 뒤에 삽입한다. revision만 1 증가한다. 정본:
Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json

```json
{
  "template": {
    "sequenceId": "sequence.kouku.cutting_blade.original_preview",
    "displayName": "원본_칼날",
    "category": "KoukuGate3",
    "durationMs": 14000,
    "interpolation": "LINEAR",
    "objectMotion": {
      "velocity": [
        0,
        0,
        2
      ],
      "acceleration": [
        0,
        0,
        0
      ],
      "angularVelocityDegrees": [
        1440,
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
      "count": 6,
      "intervalMs": 0,
      "spreadDegrees": 0,
      "seed": 1,
      "spawnHalfExtents": [
        0,
        0,
        0
      ],
      "emissions": [
        {
          "positionOffset": [
            3.345653,
            0,
            3.715724
          ],
          "yawDegrees": 312,
          "startDelayMs": 0
        },
        {
          "positionOffset": [
            2.007392,
            0,
            2.229434
          ],
          "yawDegrees": 132,
          "startDelayMs": 0
        },
        {
          "positionOffset": [
            0.669131,
            0,
            0.743145
          ],
          "yawDegrees": 312,
          "startDelayMs": 0
        },
        {
          "positionOffset": [
            -0.669131,
            0,
            -0.743145
          ],
          "yawDegrees": 132,
          "startDelayMs": 0
        },
        {
          "positionOffset": [
            -2.007392,
            0,
            -2.229434
          ],
          "yawDegrees": 312,
          "startDelayMs": 0
        },
        {
          "positionOffset": [
            -3.345653,
            0,
            -3.715724
          ],
          "yawDegrees": 132,
          "startDelayMs": 0
        }
      ]
    },
    "tracks": [
      {
        "slotId": "object",
        "keys": [
          {
            "timeMs": 0,
            "positionOffset": [
              0,
              0.979919,
              -14
            ],
            "rotationQuaternion": [
              0,
              0,
              0.707106769,
              0.707106709
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          },
          {
            "timeMs": 13999,
            "positionOffset": [
              0,
              0.979919,
              -14
            ],
            "rotationQuaternion": [
              0,
              0,
              0.707106769,
              0.707106709
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": true
          },
          {
            "timeMs": 14000,
            "positionOffset": [
              0,
              0.979919,
              -14
            ],
            "rotationQuaternion": [
              0,
              0,
              0.707106769,
              0.707106709
            ],
            "scaleMultiplier": [
              1,
              1,
              1
            ],
            "visible": false
          }
        ]
      }
    ],
    "animationTracks": [],
    "effectTracks": []
  },
  "instance": {
    "instanceId": "world.sequence.instance.kouku.cutting_blade.original_preview",
    "templateId": "sequence.kouku.cutting_blade.original_preview",
    "enabled": true,
    "startDelayMs": 0,
    "playbackSpeed": 1,
    "bindings": [
      {
        "slotId": "object",
        "targetKind": "OBJECT_RESOURCE",
        "targetId": "world.object.kouku.cutting_blade"
      }
    ],
    "anchorKind": "WORLD",
    "position": [
      0,
      1.2,
      942.080017
    ],
    "motionEnd": "STOP",
    "nextMotionId": ""
  }
}
```

## 검증 및 사용자 확인

JSON parse, 기존 행 semantic equality, 정점/삼각형 기하 일치와 6개 평행 경로 간격, 공식 WorldSequences Validate/Publish/Check 및 scoped diff check를 실행한다. Publish 직전 동시 변경을 확인한다. C++ 변경이 없어 재빌드하지 않는다. 사용자는 미저장 편집 보존 후 Reload Source → 월드오브젝트_칼날 → 원본_칼날을 선택한다. Preview at Character를 끄면 (0,1.2,942.080017) 기준이다. 최종 화면 판정은 사용자 소유다.

## G02. 사용자 요청으로 비교 모션만 삭제

원본_칼날의 sequence.kouku.cutting_blade.original_preview template와 world.sequence.instance.kouku.cutting_blade.original_preview instance만 제거한다. 사용자가 편집한 바닥_칼날 및 6개 emissions, 공용 모델/텍스처와 다른 모션은 보존한다. revision1662→1663. 실제 삭제 블록은 로컬 original_blade_deleted_fragment.json에 보관한다. 제거 후 다른 모든 JSON 값의 semantic equality와 dangling reference 부재, 공식 Validate/Publish/Check를 확인한다.
