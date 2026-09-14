# G01. 원본_갈고리 비교용 모션 추가

기존 World Object Tool의 template/instance 한 쌍을 갈고리 아래에 추가한다. 부모 resource, Default Motion, 기존 18개 모션과 Composition/Server 판정은 변경하지 않는다. 새 C++/프로젝트 등록은 없다.

원본 설치 WModel의 respawn/attack1/attack2/attack3 4클립을 0/1334/8334/9834ms에 재생한다. Count=1, Lifetime=11334ms. 전진은 원본 b_hook_root_01 이동이며 21.25m × 1.15 = 24.4375m. 전진 종료 시 다음 클립의 root reset만 보정하는 8333→8334ms 위치 키를 사용한다. 단일 프리뷰를 중심 기준 ±12.21875m에 놓는다. 이 위치·4클립 연결은 비교용 저작값이며 원작 서버 경로 복원 증거가 아니다. 14단계 연속기·사운드·플레이어 잡기/피해는 이번에 연결하지 않는다.

소유자: Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json. 기존 hook_diagonal 바로 뒤에 다음 블록을 각각 templates/instances에 넣고 revision 672→673. 공식 WorldSequences publisher만 runtime 파일을 갱신한다.

```json
{
  "template": {
    "sequenceId": "sequence.kouku.hook.original_preview",
    "displayName": "원본_갈고리",
    "category": "KoukuGate3",
    "durationMs": 11334,
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
              -12.21875,
              0,
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
            "timeMs": 8333,
            "positionOffset": [
              -12.21875,
              0,
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
            "visible": false
          },
          {
            "timeMs": 8334,
            "positionOffset": [
              12.21875,
              0,
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
            "timeMs": 11333,
            "positionOffset": [
              12.21875,
              0,
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
            "timeMs": 11334,
            "positionOffset": [
              12.21875,
              0,
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
            "visible": false
          }
        ]
      }
    ],
    "animationTracks": [
      {
        "slotId": "object",
        "clipName": "Hook_respawn_1",
        "displayName": "Hook_respawn_1",
        "startMs": 0,
        "playbackRate": 1,
        "loop": false,
        "holdLastFrame": true
      },
      {
        "slotId": "object",
        "clipName": "Hook_att_battle_1_01",
        "displayName": "Hook_att_battle_1_01",
        "startMs": 1334,
        "playbackRate": 1,
        "loop": false,
        "holdLastFrame": true
      },
      {
        "slotId": "object",
        "clipName": "Hook_att_battle_2_01",
        "displayName": "Hook_att_battle_2_01",
        "startMs": 8334,
        "playbackRate": 1,
        "loop": false,
        "holdLastFrame": true
      },
      {
        "slotId": "object",
        "clipName": "Hook_att_battle_3_01",
        "displayName": "Hook_att_battle_3_01",
        "startMs": 9834,
        "playbackRate": 1,
        "loop": false,
        "holdLastFrame": true
      }
    ],
    "effectTracks": []
  },
  "instance": {
    "instanceId": "world.sequence.instance.kouku.hook.original_preview",
    "templateId": "sequence.kouku.hook.original_preview",
    "enabled": true,
    "startDelayMs": 0,
    "playbackSpeed": 1,
    "bindings": [
      {
        "slotId": "object",
        "targetKind": "OBJECT_RESOURCE",
        "targetId": "world.object.kouku.hook"
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

검증: native clip/길이/root 이동, 기존 모든 row 불변, JSON parse, 공식 Validate/Publish/Check, git diff --check. C++ 무변경으로 재빌드하지 않는다. 사용자가 Reload Source → 원본_갈고리 → Play로 화면을 판단한다.

경계 보정: 8333~8334ms의 1ms 구간은 숨겨, 선형 키 보간 중 원본 root와 이동 끝 위치가 겹쳐 보이는 것을 방지한다. 원작 clip 자체를 수정하지 않는다.

## 클립 표시 이름 정정

사용자가 확인한 동작은 유지한다. revision 673→674에서 위 네 animationTracks의 displayName만 실제 clipName과 같게 맞춘다. 전체 모션 이름 원본_갈고리, timing, Transform, emission, instance 및 runtime 게시본은 변경하지 않는다.
