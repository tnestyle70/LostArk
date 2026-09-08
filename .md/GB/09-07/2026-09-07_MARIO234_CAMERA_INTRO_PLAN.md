# 2·3·4마리오 입장 카메라와 구간 추적 확장

## G4. 13:34 사용자 최종 키 튜닝을 추적 카메라에 반영

현재 저장 revision 67의 `2Mario.k36`, `3Mario.k36`을 기준으로 한다.
첨부 화면에서 마지막 Key Eye가 바뀌었지만 상위 Eye는 이전 값인 점을 확인했고 저장 파일도 동일하다.
이번 변경은 camera-shots 데이터만 갱신하며 C++/Server/다른 시퀀스는 변경하지 않는다.

- `2Mario`, `3Mario`의 상위 Eye / Look At / FOV를 마지막 키에 일치시킨다.
- 첫 follow offset = 마지막 키의 Eye 또는 Look At - 기존 `MarioN_go` 입장 위치.
- 나머지 follow offset은 각 구간의 기존 상대 yaw 차이를 유지하여 첫 offset을 회전시킨다.
- 각 follow 샷의 정적 미리보기 Eye/Look도 box center + 새 offset으로 맞춘다.
- 2마리오 a~c, 3마리오 a~f의 FOV는 해당 마지막 키의 값을 사용한다.
- 사용자 cameraTrack 전체, box/전환시간/priority, 1·4마리오와 다른 샷은 보존한다.
- Map publisher Validate/Publish 후 저장값 보존, 종료 포즈 연속성, 회전 거리/높이 불변식과 JSON parse를 검사한다.
- 이후의 임의 마지막 키 편집까지 자동 연동하는 새 저장 계약은 이번 데이터 보정에 포함하지 않는다.

G3의 숫자는 최초 구현 시점 기록이며 G4의 변경 블록이 현재 튜닝값이다.

### G4 교체 필드 전체 블록

아래 shotId로 기존 샷을 찾고 표시된 필드만 교체한다. cameraTrack/box와 나머지 필드는 그대로 둔다.
revision은 67 → 68이다.

```json
[
  {
    "shotId": "2Mario",
    "eye": [
      -1441.39,
      -8.02,
      -1172.34
    ],
    "lookAt": [
      -1423.46,
      -7.17023,
      -1184.18
    ],
    "fovYDegrees": 50
  },
  {
    "shotId": "shot.mario2.a",
    "eye": [
      -1441.90001,
      -12.87019,
      -1196.37003
    ],
    "lookAt": [
      -1423.97001,
      -12.02042,
      -1208.21003
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -6.90001,
        1.00001,
        3.62997
      ],
      "lookAtOffset": [
        11.02999,
        1.84978,
        -8.21003
      ]
    }
  },
  {
    "shotId": "shot.mario2.b",
    "eye": [
      -1437.303691,
      -5.46042,
      -1192.551522
    ],
    "lookAt": [
      -1433.021655,
      -4.61065,
      -1213.607036
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -2.303691,
        1.00001,
        7.448478
      ],
      "lookAtOffset": [
        1.978345,
        1.84978,
        -13.607036
      ]
    }
  },
  {
    "shotId": "shot.mario2.c",
    "eye": [
      -1441.053598,
      0.909792,
      -1195.086674
    ],
    "lookAt": [
      -1425.795412,
      1.759562,
      -1210.214736
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -6.053598,
        1.00001,
        4.913326
      ],
      "lookAtOffset": [
        9.204588,
        1.84978,
        -10.214736
      ]
    }
  },
  {
    "shotId": "3Mario",
    "eye": [
      -1902.27,
      -9.66594,
      -1640.5
    ],
    "lookAt": [
      -1882.69,
      -9.66594,
      -1648.08
    ],
    "fovYDegrees": 50
  },
  {
    "shotId": "shot.mario3.a",
    "eye": [
      -1902.08006,
      -13.25944,
      -1650.79004
    ],
    "lookAt": [
      -1882.50006,
      -13.25944,
      -1658.37004
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -12.58006,
        1.86406,
        5.70996
      ],
      "lookAtOffset": [
        6.99994,
        1.86406,
        -1.87004
      ]
    }
  },
  {
    "shotId": "shot.mario3.b",
    "eye": [
      -1900.724393,
      -7.10294,
      -1648.445525
    ],
    "lookAt": [
      -1882.999384,
      -7.10294,
      -1659.699718
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -11.224393,
        1.86406,
        8.054475
      ],
      "lookAtOffset": [
        6.500616,
        1.86406,
        -3.199718
      ]
    }
  },
  {
    "shotId": "shot.mario3.c",
    "eye": [
      -1899.880862,
      0.52056,
      -1647.384121
    ],
    "lookAt": [
      -1883.344315,
      0.52056,
      -1660.321486
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -10.380862,
        1.86406,
        9.115879
      ],
      "lookAtOffset": [
        6.155685,
        1.86406,
        -3.821486
      ]
    }
  },
  {
    "shotId": "shot.mario3.d",
    "eye": [
      -1931.402975,
      1.80056,
      -1676.534072
    ],
    "lookAt": [
      -1917.708131,
      1.80056,
      -1692.448973
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -8.402975,
        1.86406,
        10.965928
      ],
      "lookAtOffset": [
        5.291869,
        1.86406,
        -4.948973
      ]
    }
  },
  {
    "shotId": "shot.mario3.e",
    "eye": [
      -1930.287669,
      -4.54294,
      -1675.763245
    ],
    "lookAt": [
      -1918.218694,
      -4.54294,
      -1692.943833
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -7.287669,
        1.86406,
        11.736755
      ],
      "lookAtOffset": [
        4.781306,
        1.86406,
        -5.443833
      ]
    }
  },
  {
    "shotId": "shot.mario3.f",
    "eye": [
      -1927.857912,
      -11.97944,
      -1674.56701
    ],
    "lookAt": [
      -1919.372604,
      -11.97944,
      -1693.772018
    ],
    "fovYDegrees": 50,
    "follow": {
      "eyeOffset": [
        -4.857912,
        1.86406,
        12.93299
      ],
      "lookAtOffset": [
        3.627396,
        1.86406,
        -6.272018
      ]
    }
  }
]
```

## G1. 기존 1마리오 계약 보존과 데이터 연결

현재 브랜치는 `codex/mario234-camera-intros`이며 시작점은 `0f05b7c5`다.
사용자가 검증한 `1Mario`, `shot.mario1.a/b/c`, `Mario1_Intro`와 기존 MapTool 미커밋 변경을 보존한다.
첨부 `마리오.txt`는 인계 자료이고 현재 코드·저장 데이터와 교차 확인하여 사용한다.

새 컷신은 기존 `CWorldSequencePlayer`의 3초 시계, camera-shots의 `cameraTrack`,
Server `playSequence` 트리거로 연결한다. 두 번째 카메라 런타임이나 Client 로컬 이동은 만들지 않는다.

| 사용자 이름 | 원본 자료 | 인트로 | 레일 | 입장점의 정본 |
|---|---|---|---|---|
| 2마리오 | 마리오2 카드박스 | efseqact_matinee_21 | 11, 12, 13 | Mario2_go.targetPosition |
| 3마리오 | 마리오4 조명 | efseqact_matinee_9 | 14~19 | Mario3_go.targetPosition |
| 4마리오 | 마리오3 카드 | efseqact_matinee_20 | 0~5 | Mario4_go.targetPosition |

새 샷 ID는 `2Mario`, `3Mario`, `4Mario`, 새 인스턴스는 사용자 번호 기준
`world.sequence.instance.mario_m2_intro`, `mario_m3_intro`, `mario_m4_intro`다.
기존 소품 시퀀스의 m3/m4 이름과 `Mario4_Tigger_*`는 변경하지 않는다.

### 파일과 책임

- `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json`:
  원본 부모/자식 카메라 키 합성, 3개 인트로 샷, 각 층의 follow 샷. 마지막 키와 첫 follow 포즈를 일치시킨다.
- 같은 폴더의 `.worldsequences.json`: 다른 시퀀스/자체 모션이 사용하지 않는 배치에 항등 트랙을 연결한 3초 시계 3개.
- `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`: 기존 go 목적지에 `Mario2_Intro`~`Mario4_Intro`.
  `triggerOnce=false`로 재입장을 허용하고 기존 보행·점프·소품 트리거는 보존한다.
- `Client/Private/Level_KakulSaydonArena.cpp`, `Get_DebugGates`: 세 gate의 placeholder 좌표를
  기존 go 목적지로 교체하고 보류 사유를 해제한다. 기존 typed command → Server navigation 승인 → snapshot 경로를 유지한다.
- `Client/Private/MapTool.cpp`: 여러 intro를 번갈아 미리볼 때 선택한 intro가 카메라를 소유하는지 확인한다.
  필요한 수정은 기존 preview 선택/중단 경계 내부에 한정한다.

### 데이터 불변식

원본 UE cm를 `(X/100, Z/100, -Y/100)`으로 변환하고 부모 자세와 자식 로컬 자세를 합성한다.
동일 그룹의 중복 트랙과 `cam_a`는 실제 키/actor 연결로 구분한다.
영상은 장면 순서와 전환 방식의 참고이며 우리 Client의 visual PASS 증거가 아니다.
층별 박스는 높이와 경로로 분리하고 entry, 이동 목적지, 소품 트리거에서 선택 결과를 수치 검사한다.

## G2. 런타임 배포와 검증

신규 C++ 파일과 신규 데이터 형식은 없다. 기존 파일 등록을 사용하므로 vcxproj/filters 추가가 필요 없다.

1. 정확한 source 키와 새 JSON 블록을 확정한 후 같은 문서에 보존한다.
2. Map publisher로 카메라/시퀀스를, World publisher로 서버 트리거를 생성한다. 생성물을 직접 편집하지 않는다.
3. 세 입장점의 Server 세부 navgrid walkability·높이, ID 참조, 키 시간·유한값·마지막 포즈 연속성을 검사한다.
4. 변경 C++의 Debug 컴파일/링크와 JSON parse, `git diff --check`를 확인한다.
5. 사용자가 Client/Server 재시작 후 F1의 2·3·4마리오와 Test → MapTool → Camera의 개별 키/전체 intro를 확인한다.
   자동 검증과 사용자 화면 검증은 RESULT에서 분리한다.

이번 범위는 카메라 연출·편집·빠른 입장이다. 장애물 피해, 제한시간, 패턴 클리어 규칙의 신규 구현은 포함하지 않는다.

## G3. 확정한 데이터와 호환 경계

원본의 활성 트랙만 사용한다. 2마리오는 parent `move9` + child `move38`,
3마리오는 parent `move39` + child `move4` + grandchild `move5`,
4마리오는 relative-to-initial parent `move45` + child `move3`이다.
Director는 각각 `director4/cam`, `director9/cam_a`, `director6/cam`을 가리킨다.
같은 그룹에 남은 disabled track은 사용하지 않는다.

원본 Hermite 곡선을 원본 키 시점과 100ms 간격으로 샘플링하여 기존 LINEAR 스키마로 저장한다.
각 인트로는 3초, 키 수는 37/37/33개로 기존 64개 제한 이내다.
이것은 원본 track의 데이터 변환이지 사용자 화면 검증 완료를 뜻하지 않는다.

원본 camera volume의 8개 FVector도 UPK에서 복구했다. 원본 영역은 현재 trigger/착지점과 맞지만,
프로젝트의 중간 점프 궤적은 영역 밖을 통과한다. 따라서 제품 샷은 원본 rail 방향 및 층 구조를 따르는
넓은 저작 박스를 사용한다. 원본 OBB를 그대로 복사했다고 표현하지 않는다.
3마리오 두 타워의 경계 X는 -1904로 두어 타워 이동 중 중층 샷을 잠깐 거치는 현상을 피한다.
영역 중심/반크기, follow offset, 전환 시간은 기존 MapTool Camera에서 다시 편집할 수 있다.

원본 roll과 fade/postprocess는 기존 camera-shots 계약에 없으므로 이번에 새 구현하지 않는다.
FOVAngle은 기존 1마리오와 동일하게 fovYDegrees에 옮긴다. 원본과 화면비별 동일한 FOV라는 검증은 없다.
2마리오 첫 키의 높은 위치/상향 시선은 활성 원본 수치다. 두 번의 독립 계산에서 단위/행렬 부호 오류는
찾지 못했지만, 원작 첫 키 이전 처리와 fade를 포함한 화면 일치는 미확인이다. 사용자가 키를 조정할 수 있다.

기존 서버 sequence event는 같은 방 전체에 broadcast된다. 이번 변경으로 개인용 camera event 계약을
새로 만들지는 않는다. 여러 사용자가 동시에 서로 다른 마리오를 F1 재생하는 경우는 이번 시각 확인 범위와 구분한다.

### 새 시계·트리거 전체 데이터

```json
{
  "templates": [
    {
      "sequenceId": "sequence.LV_LUT_MIDNIGHTC_ED.mario_m2_intro",
      "displayName": "mario_m2_intro",
      "category": "World",
      "durationMs": 3000,
      "interpolation": "LINEAR",
      "tracks": [
        {
          "slotId": "obj01",
          "keys": [
            {
              "timeMs": 0,
              "positionOffset": [
                0,
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
              "timeMs": 3000,
              "positionOffset": [
                0,
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
            }
          ]
        }
      ],
      "animationTracks": []
    },
    {
      "sequenceId": "sequence.LV_LUT_MIDNIGHTC_ED.mario_m3_intro",
      "displayName": "mario_m3_intro",
      "category": "World",
      "durationMs": 3000,
      "interpolation": "LINEAR",
      "tracks": [
        {
          "slotId": "obj01",
          "keys": [
            {
              "timeMs": 0,
              "positionOffset": [
                0,
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
              "timeMs": 3000,
              "positionOffset": [
                0,
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
            }
          ]
        }
      ],
      "animationTracks": []
    },
    {
      "sequenceId": "sequence.LV_LUT_MIDNIGHTC_ED.mario_m4_intro",
      "displayName": "mario_m4_intro",
      "category": "World",
      "durationMs": 3000,
      "interpolation": "LINEAR",
      "tracks": [
        {
          "slotId": "obj01",
          "keys": [
            {
              "timeMs": 0,
              "positionOffset": [
                0,
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
              "timeMs": 3000,
              "positionOffset": [
                0,
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
            }
          ]
        }
      ],
      "animationTracks": []
    }
  ],
  "instances": [
    {
      "instanceId": "world.sequence.instance.mario_m2_intro",
      "templateId": "sequence.LV_LUT_MIDNIGHTC_ED.mario_m2_intro",
      "enabled": true,
      "startDelayMs": 0,
      "playbackSpeed": 1,
      "bindings": [
        {
          "slotId": "obj01",
          "targetKind": "MAP_PLACEMENT",
          "targetId": "11284422400836074445"
        }
      ]
    },
    {
      "instanceId": "world.sequence.instance.mario_m3_intro",
      "templateId": "sequence.LV_LUT_MIDNIGHTC_ED.mario_m3_intro",
      "enabled": true,
      "startDelayMs": 0,
      "playbackSpeed": 1,
      "bindings": [
        {
          "slotId": "obj01",
          "targetKind": "MAP_PLACEMENT",
          "targetId": "13607882261321843072"
        }
      ]
    },
    {
      "instanceId": "world.sequence.instance.mario_m4_intro",
      "templateId": "sequence.LV_LUT_MIDNIGHTC_ED.mario_m4_intro",
      "enabled": true,
      "startDelayMs": 0,
      "playbackSpeed": 1,
      "bindings": [
        {
          "slotId": "obj01",
          "targetKind": "MAP_PLACEMENT",
          "targetId": "17873503318011911402"
        }
      ]
    }
  ],
  "triggers": [
    {
      "placementId": "Mario2_Intro",
      "kind": "triggerBox",
      "position": [
        -1434.48999,
        -9.02000999,
        -1175.96997
      ],
      "yawDegrees": 0,
      "enabled": true,
      "halfExtents": [
        3,
        2,
        3
      ],
      "triggerOnce": false,
      "events": [
        {
          "type": "playSequence",
          "sequenceInstanceId": "world.sequence.instance.mario_m2_intro"
        }
      ]
    },
    {
      "placementId": "Mario3_Intro",
      "kind": "triggerBox",
      "position": [
        -1889.68994,
        -11.5299997,
        -1646.20996
      ],
      "yawDegrees": 0,
      "enabled": true,
      "halfExtents": [
        3,
        2,
        3
      ],
      "triggerOnce": false,
      "events": [
        {
          "type": "playSequence",
          "sequenceInstanceId": "world.sequence.instance.mario_m3_intro"
        }
      ]
    },
    {
      "placementId": "Mario4_Intro",
      "kind": "triggerBox",
      "position": [
        -1632.57,
        -20.49,
        -1400.92
      ],
      "yawDegrees": 0,
      "enabled": true,
      "halfExtents": [
        3,
        2,
        3
      ],
      "triggerOnce": false,
      "events": [
        {
          "type": "playSequence",
          "sequenceInstanceId": "world.sequence.instance.mario_m4_intro"
        }
      ]
    }
  ]
}
```

### 새 샷 전체 데이터

실제 저장 sceneId는 `<shotId>.<원본샘플ID>`로 구별한다.

```json
[
  {
    "shotId": "shot.mario2.a",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1435,
        -13.870217,
        -1200
      ],
      "halfExtents": [
        35,
        6.129783,
        40
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1449.546424,
      -12.020438,
      -1191.12134
    ],
    "lookAt": [
      -1423.96891,
      -12.020438,
      -1208.211693
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -14.546424,
        1.849779,
        8.87866
      ],
      "lookAtOffset": [
        11.03109,
        1.849779,
        -8.211693
      ]
    }
  },
  {
    "shotId": "shot.mario2.b",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1435,
        -6.460435,
        -1200
      ],
      "halfExtents": [
        35,
        1.28,
        40
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1438.988631,
      -4.610656,
      -1183.431359
    ],
    "lookAt": [
      -1433.02206,
      -4.610656,
      -1213.60899
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -3.988631,
        1.849779,
        16.568641
      ],
      "lookAtOffset": [
        1.97794,
        1.849779,
        -13.60899
      ]
    }
  },
  {
    "shotId": "shot.mario2.c",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1435,
        -0.090218,
        -1200
      ],
      "halfExtents": [
        35,
        5.090217,
        40
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1447.52147,
      1.759561,
      -1188.439643
    ],
    "lookAt": [
      -1425.794654,
      1.759561,
      -1210.216577
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -12.52147,
        1.849779,
        11.560357
      ],
      "lookAtOffset": [
        9.205346,
        1.849779,
        -10.216577
      ]
    }
  },
  {
    "shotId": "2Mario",
    "sequenceInstanceId": "world.sequence.instance.mario_m2_intro",
    "box": {
      "center": [
        -1434.48999,
        -9.02000999,
        -1175.96997
      ],
      "halfExtents": [
        1,
        3,
        1
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1449.0364136482542,
      -7.170231475830078,
      -1167.0913097469245
    ],
    "lookAt": [
      -1423.4589003629453,
      -7.170231475830078,
      -1184.181662661703
    ],
    "fovYDegrees": 50,
    "blendInMs": 0,
    "blendOutMs": 900,
    "priority": 30,
    "cameraTrack": {
      "durationMs": 3000,
      "interpolation": "LINEAR",
      "easing": "LINEAR",
      "keyframes": [
        {
          "sceneId": "2Mario.k00",
          "timeMs": 0,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k01",
          "timeMs": 100,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k02",
          "timeMs": 200,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k03",
          "timeMs": 300,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k04",
          "timeMs": 400,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k05",
          "timeMs": 500,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k06",
          "timeMs": 600,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k07",
          "timeMs": 700,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k08",
          "timeMs": 792,
          "eye": [
            -1489.25526295409,
            89.36102645874024,
            -1142.0074094841923
          ],
          "lookAt": [
            -1442.8590970376538,
            128.43308822272735,
            -1189.3381582151655
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k09",
          "timeMs": 800,
          "eye": [
            -1489.2552660785443,
            89.27868208816133,
            -1142.0074074441468
          ],
          "lookAt": [
            -1442.8397360788047,
            128.30374628501966,
            -1189.3579459719658
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k10",
          "timeMs": 900,
          "eye": [
            -1489.2557491730684,
            76.54682246523772,
            -1142.0070920179141
          ],
          "lookAt": [
            -1440.1572191506912,
            108.16171370173842,
            -1192.100509209554
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k11",
          "timeMs": 1000,
          "eye": [
            -1489.256757780733,
            49.96516912252247,
            -1142.0064334691194
          ],
          "lookAt": [
            -1436.49897792612,
            65.37882335178192,
            -1195.8463329167328
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k12",
          "timeMs": 1100,
          "eye": [
            -1489.2578638519983,
            20.814882268348843,
            -1142.005711283549
          ],
          "lookAt": [
            -1435.4398346038856,
            18.47086860216193,
            -1196.9422483481947
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k13",
          "timeMs": 1167,
          "eye": [
            -1489.2584463153967,
            5.464176177978516,
            -1142.0053309765308
          ],
          "lookAt": [
            -1435.9504715299386,
            -5.322053249231162,
            -1196.4288517758428
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k14",
          "timeMs": 1200,
          "eye": [
            -1489.2585294922087,
            3.272067997988224,
            -1142.0052766680099
          ],
          "lookAt": [
            -1435.7581736873194,
            -5.346129315628291,
            -1196.626299187039
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k15",
          "timeMs": 1250,
          "eye": [
            -1489.2584463153967,
            5.464176177978516,
            -1142.0053309765308
          ],
          "lookAt": [
            -1435.4208123136548,
            4.799799311526443,
            -1196.9695948727244
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k16",
          "timeMs": 1300,
          "eye": [
            -1489.2373135910912,
            5.464176177978516,
            -1142.0258224574054
          ],
          "lookAt": [
            -1435.4681550767855,
            2.1533559589239775,
            -1196.92017791814
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k17",
          "timeMs": 1333,
          "eye": [
            -1489.2584463153967,
            5.464176177978516,
            -1142.0053309765308
          ],
          "lookAt": [
            -1435.5418989430736,
            0.264280795667152,
            -1196.845974331497
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k18",
          "timeMs": 1400,
          "eye": [
            -1489.3662200915142,
            5.464176177978516,
            -1141.900827447444
          ],
          "lookAt": [
            -1435.5448976350267,
            0.253741027119637,
            -1196.8484384558471
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k19",
          "timeMs": 1417,
          "eye": [
            -1489.3935656764993,
            5.464176177978516,
            -1141.874311626631
          ],
          "lookAt": [
            -1435.5456585090012,
            0.25106651256537216,
            -1196.8490636693439
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k20",
          "timeMs": 1500,
          "eye": [
            -1489.5270764737795,
            5.464176177978516,
            -1141.7448520308965
          ],
          "lookAt": [
            -1435.5493734330948,
            0.23800716499482455,
            -1196.8521161120514
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k21",
          "timeMs": 1600,
          "eye": [
            -1489.6879328560444,
            5.464176177978516,
            -1141.5888766143491
          ],
          "lookAt": [
            -1435.5538493969789,
            0.222269872581764,
            -1196.8557935989693
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k22",
          "timeMs": 1700,
          "eye": [
            -1489.8487892383098,
            5.464176177978516,
            -1141.4329011978016
          ],
          "lookAt": [
            -1435.5583255267074,
            0.20652914988316606,
            -1196.8594709165727
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k23",
          "timeMs": 1800,
          "eye": [
            -1490.009645620575,
            5.464176177978516,
            -1141.2769257812545
          ],
          "lookAt": [
            -1435.5628018223074,
            0.19078499690174233,
            -1196.8631480648337
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k24",
          "timeMs": 1900,
          "eye": [
            -1490.1705020028403,
            5.464176177978516,
            -1141.120950364707
          ],
          "lookAt": [
            -1435.5672782838067,
            0.17503741364020087,
            -1196.866825043723
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k25",
          "timeMs": 2000,
          "eye": [
            -1490.3313583851052,
            5.464176177978516,
            -1140.9649749481596
          ],
          "lookAt": [
            -1435.5717549112326,
            0.1592864001012586,
            -1196.870501853214
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k26",
          "timeMs": 2100,
          "eye": [
            -1490.4922147673706,
            5.464176177978516,
            -1140.808999531612
          ],
          "lookAt": [
            -1435.5762317046133,
            0.1435319562876236,
            -1196.874178493277
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k27",
          "timeMs": 2200,
          "eye": [
            -1490.653071149636,
            5.464176177978516,
            -1140.6530241150647
          ],
          "lookAt": [
            -1435.5807086639763,
            0.12777408220200925,
            -1196.8778549638846
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k28",
          "timeMs": 2300,
          "eye": [
            -1490.813927531901,
            5.464176177978516,
            -1140.4970486985171
          ],
          "lookAt": [
            -1435.585185789349,
            0.11201277784712982,
            -1196.8815312650083
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k29",
          "timeMs": 2400,
          "eye": [
            -1490.9747839141664,
            5.464176177978516,
            -1140.3410732819698
          ],
          "lookAt": [
            -1435.589663080759,
            0.09624804322569602,
            -1196.8852073966204
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k30",
          "timeMs": 2500,
          "eye": [
            -1491.1356402964313,
            5.464176177978516,
            -1140.1850978654224
          ],
          "lookAt": [
            -1435.594140538234,
            0.08047987834042392,
            -1196.888883358692
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k31",
          "timeMs": 2600,
          "eye": [
            -1491.2964966786967,
            5.464176177978516,
            -1140.029122448875
          ],
          "lookAt": [
            -1435.5986181618018,
            0.06470828319402688,
            -1196.8925591511952
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k32",
          "timeMs": 2700,
          "eye": [
            -1491.4573530609619,
            5.464176177978516,
            -1139.8731470323276
          ],
          "lookAt": [
            -1435.6030959514899,
            0.0489332577892192,
            -1196.896234774102
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k33",
          "timeMs": 2750,
          "eye": [
            -1491.5377812520944,
            5.464176177978516,
            -1139.7951593240539
          ],
          "lookAt": [
            -1435.6053349086376,
            0.04104445874076035,
            -1196.8980725219476
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k34",
          "timeMs": 2800,
          "eye": [
            -1486.2350528137863,
            3.872962821044922,
            -1143.1844169521617
          ],
          "lookAt": [
            -1433.2140176716853,
            -0.6000791574552187,
            -1194.5684164489724
          ],
          "fovYDegrees": 50.00000000000001
        },
        {
          "sceneId": "2Mario.k35",
          "timeMs": 2900,
          "eye": [
            -1463.3349553891742,
            -2.9308462023925768,
            -1157.8959082861709
          ],
          "lookAt": [
            -1425.8737316293236,
            -4.053675180247251,
            -1186.8742319908754
          ],
          "fovYDegrees": 50
        },
        {
          "sceneId": "2Mario.k36",
          "timeMs": 3000,
          "eye": [
            -1449.0364136482542,
            -7.170231475830078,
            -1167.0913097469245
          ],
          "lookAt": [
            -1423.4589003629453,
            -7.170231475830078,
            -1184.181662661703
          ],
          "fovYDegrees": 50
        }
      ]
    }
  },
  {
    "shotId": "shot.mario3.a",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1889.5,
        -15.1235,
        -1656.5
      ],
      "halfExtents": [
        14.5,
        4.8765,
        21.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1905.976547,
      -13.259442,
      -1648.642402
    ],
    "lookAt": [
      -1882.501039,
      -13.259442,
      -1658.366275
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -16.476547,
        1.864058,
        7.857598
      ],
      "lookAtOffset": [
        6.998961,
        1.864058,
        -1.866275
      ]
    }
  },
  {
    "shotId": "shot.mario3.b",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1889.5,
        -8.967,
        -1656.5
      ],
      "halfExtents": [
        14.5,
        1.28,
        21.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1904.127013,
      -7.102942,
      -1645.578969
    ],
    "lookAt": [
      -1882.999614,
      -7.102942,
      -1659.695845
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -14.627013,
        1.864058,
        10.921031
      ],
      "lookAtOffset": [
        6.500386,
        1.864058,
        -3.195845
      ]
    }
  },
  {
    "shotId": "shot.mario3.c",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1889.5,
        -1.3435,
        -1656.5
      ],
      "halfExtents": [
        14.5,
        6.3435,
        21.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1902.986132,
      0.520558,
      -1644.197858
    ],
    "lookAt": [
      -1883.344163,
      0.520558,
      -1660.317605
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -13.486132,
        1.864058,
        12.302142
      ],
      "lookAtOffset": [
        6.155837,
        1.864058,
        -3.817605
      ]
    }
  },
  {
    "shotId": "shot.mario3.d",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1923,
        -0.0635,
        -1687.5
      ],
      "halfExtents": [
        19,
        5.0635,
        17.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1933.826971,
      1.800558,
      -1672.803227
    ],
    "lookAt": [
      -1917.707223,
      1.800558,
      -1692.445195
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -10.826971,
        1.864058,
        14.696773
      ],
      "lookAtOffset": [
        5.292777,
        1.864058,
        -4.945195
      ]
    }
  },
  {
    "shotId": "shot.mario3.e",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1923,
        -6.407,
        -1687.5
      ],
      "halfExtents": [
        19,
        1.28,
        17.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1932.334301,
      -4.542942,
      -1671.812767
    ],
    "lookAt": [
      -1918.217423,
      -4.542942,
      -1692.940165
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -9.334301,
        1.864058,
        15.687233
      ],
      "lookAtOffset": [
        4.782577,
        1.864058,
        -5.440165
      ]
    }
  },
  {
    "shotId": "shot.mario3.f",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1923,
        -13.8435,
        -1687.5
      ],
      "halfExtents": [
        19,
        6.1565,
        17.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1929.094517,
      -11.979442,
      -1670.293161
    ],
    "lookAt": [
      -1919.370643,
      -11.979442,
      -1693.768668
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -6.094517,
        1.864058,
        17.206839
      ],
      "lookAtOffset": [
        3.629357,
        1.864058,
        -6.268668
      ]
    }
  },
  {
    "shotId": "3Mario",
    "sequenceInstanceId": "world.sequence.instance.mario_m3_intro",
    "box": {
      "center": [
        -1889.68994,
        -11.5299997,
        -1646.20996
      ],
      "halfExtents": [
        1,
        3,
        1
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1906.1664871145313,
      -9.665941528320312,
      -1638.3523615407657
    ],
    "lookAt": [
      -1882.690978860756,
      -9.665941433661775,
      -1648.0762354416802
    ],
    "fovYDegrees": 50,
    "blendInMs": 0,
    "blendOutMs": 900,
    "priority": 30,
    "cameraTrack": {
      "durationMs": 3000,
      "interpolation": "LINEAR",
      "easing": "LINEAR",
      "keyframes": [
        {
          "sceneId": "3Mario.k00",
          "timeMs": 0,
          "eye": [
            -1975.0309848716392,
            20.344169880532547,
            -1607.7265630102602
          ],
          "lookAt": [
            -1906.8376765955747,
            12.035382895506944,
            -1674.439879903536
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k01",
          "timeMs": 100,
          "eye": [
            -1974.8166882867963,
            20.344169865842286,
            -1607.9426745084975
          ],
          "lookAt": [
            -1906.8496038349592,
            12.025486344982959,
            -1674.4491345906795
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k02",
          "timeMs": 167,
          "eye": [
            -1974.673109574955,
            20.34416985600073,
            -1608.0874692123152
          ],
          "lookAt": [
            -1906.8575668635733,
            12.01898584222864,
            -1674.4553105912325
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k03",
          "timeMs": 200,
          "eye": [
            -1974.6023914574982,
            20.344169852650925,
            -1608.158785414451
          ],
          "lookAt": [
            -1906.863160217043,
            12.022766472195086,
            -1674.4609328626784
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k04",
          "timeMs": 300,
          "eye": [
            -1974.3880913921084,
            20.344169860911755,
            -1608.3748884241506
          ],
          "lookAt": [
            -1906.8988510924369,
            12.112183954711393,
            -1674.5068453349973
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k05",
          "timeMs": 400,
          "eye": [
            -1974.173787854807,
            20.344169896700866,
            -1608.5909827900211
          ],
          "lookAt": [
            -1906.958306831549,
            12.300538994827155,
            -1674.58899905099
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k06",
          "timeMs": 500,
          "eye": [
            -1973.959481680636,
            20.344169958918553,
            -1608.8070703425897
          ],
          "lookAt": [
            -1907.0360161441968,
            12.56452714002996,
            -1674.6981271830577
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k07",
          "timeMs": 600,
          "eye": [
            -1973.7451737054416,
            20.344170045389326,
            -1609.0231529120138
          ],
          "lookAt": [
            -1907.1265245014492,
            12.881130718091956,
            -1674.8250810172005
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k08",
          "timeMs": 700,
          "eye": [
            -1973.530864765872,
            20.34417015286196,
            -1609.239232328082
          ],
          "lookAt": [
            -1907.2243005576825,
            13.227603793266821,
            -1674.9609567739403
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k09",
          "timeMs": 800,
          "eye": [
            -1973.3165556993781,
            20.34417027700946,
            -1609.4553104202118
          ],
          "lookAt": [
            -1907.3236531284515,
            13.581464091501083,
            -1675.097178326557
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k10",
          "timeMs": 900,
          "eye": [
            -1973.102247344215,
            20.344170412429104,
            -1609.6713890174503
          ],
          "lookAt": [
            -1907.4186969397874,
            13.92049121165707,
            -1675.2255359399487
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k11",
          "timeMs": 1000,
          "eye": [
            -1972.8879405394428,
            20.34417055264243,
            -1609.8874699484709
          ],
          "lookAt": [
            -1907.5033649205773,
            14.222729953581961,
            -1675.3381806717105
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k12",
          "timeMs": 1100,
          "eye": [
            -1972.673636124926,
            20.34417069009525,
            -1610.1035550415743
          ],
          "lookAt": [
            -1907.5714646972683,
            14.46649747078551,
            -1675.4275740021296
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k13",
          "timeMs": 1200,
          "eye": [
            -1972.4593349413356,
            20.34417081615766,
            -1610.319646124687
          ],
          "lookAt": [
            -1907.6167771766789,
            14.630393185651986,
            -1675.4863925619622
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k14",
          "timeMs": 1208,
          "eye": [
            -1972.4421910114224,
            20.344170825467295,
            -1610.336933724846
          ],
          "lookAt": [
            -1907.6192333764716,
            14.639420123378637,
            -1675.4895603264356
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k15",
          "timeMs": 1300,
          "eye": [
            -1972.2450386271373,
            20.34417090374792,
            -1610.5357439868783
          ],
          "lookAt": [
            -1907.6349602188286,
            14.704104882570395,
            -1675.5100792369615
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k16",
          "timeMs": 1400,
          "eye": [
            -1972.0307475035845,
            20.344170941972006,
            -1610.7518470422815
          ],
          "lookAt": [
            -1907.6296308145772,
            14.705225921077655,
            -1675.5040513409044
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k17",
          "timeMs": 1500,
          "eye": [
            -1971.8164598153176,
            20.344170957467064,
            -1610.9679538500068
          ],
          "lookAt": [
            -1907.6065905293515,
            14.651610210317049,
            -1675.4755879579202
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k18",
          "timeMs": 1600,
          "eye": [
            -1971.6021737975188,
            20.344170976694247,
            -1611.1840629743333
          ],
          "lookAt": [
            -1907.5716272557352,
            14.56084866923213,
            -1675.4318091606576
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k19",
          "timeMs": 1700,
          "eye": [
            -1971.3878876847125,
            20.344171025767015,
            -1611.400172980339
          ],
          "lookAt": [
            -1907.5304380521745,
            14.450302518216262,
            -1675.3797593066963
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k20",
          "timeMs": 1800,
          "eye": [
            -1971.1735997107708,
            20.34417113045107,
            -1611.6162824339006
          ],
          "lookAt": [
            -1907.4885808163276,
            14.337098857514919,
            -1675.3264555377812
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k21",
          "timeMs": 1900,
          "eye": [
            -1970.9593081089113,
            20.34417131616434,
            -1611.832389901695
          ],
          "lookAt": [
            -1907.451455078211,
            14.23812862588297,
            -1675.278906612524
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k22",
          "timeMs": 2000,
          "eye": [
            -1970.7450111116977,
            20.344171607976918,
            -1612.048493951197
          ],
          "lookAt": [
            -1907.4243114598203,
            14.170047147282528,
            -1675.2441029861782
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k23",
          "timeMs": 2042,
          "eye": [
            -1970.6550043386865,
            20.344171768136036,
            -1612.1392562882265
          ],
          "lookAt": [
            -1907.4171265749592,
            14.15462245980449,
            -1675.2349575058215
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k24",
          "timeMs": 2100,
          "eye": [
            -1970.5307029292264,
            20.34417208031334,
            -1612.2645904912927
          ],
          "lookAt": [
            -1907.409210141937,
            14.146056568810963,
            -1675.2255765379953
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k25",
          "timeMs": 2200,
          "eye": [
            -1970.3163714773589,
            20.344172891254928,
            -1612.4806718651923
          ],
          "lookAt": [
            -1907.397368324707,
            14.156253681628364,
            -1675.2136666428814
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k26",
          "timeMs": 2250,
          "eye": [
            -1970.2092010912938,
            20.34417337933096,
            -1612.5887099015815
          ],
          "lookAt": [
            -1907.3934602375698,
            14.16999860324723,
            -1675.2105831862025
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k27",
          "timeMs": 2300,
          "eye": [
            -1970.102030294887,
            20.344173892939946,
            -1612.6967481467257
          ],
          "lookAt": [
            -1907.3916293947823,
            14.187471380976126,
            -1675.2099507837518
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k28",
          "timeMs": 2400,
          "eye": [
            -1969.8876955622918,
            20.344174909856402,
            -1612.912831187946
          ],
          "lookAt": [
            -1907.3963843674599,
            14.227553065586442,
            -1675.217630516905
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k29",
          "timeMs": 2500,
          "eye": [
            -1969.6733834654099,
            20.344175771055724,
            -1613.1289328336593
          ],
          "lookAt": [
            -1907.4159662496563,
            14.264506569465677,
            -1675.2398587517937
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k30",
          "timeMs": 2542,
          "eye": [
            -1969.583383029261,
            20.34417604614185,
            -1613.2197039204725
          ],
          "lookAt": [
            -1907.4296430417342,
            14.276243408899635,
            -1675.2542807225136
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k31",
          "timeMs": 2600,
          "eye": [
            -1969.4591134855518,
            20.34417635602214,
            -1613.3450683147294
          ],
          "lookAt": [
            -1907.4603052337393,
            14.292543498691517,
            -1675.2859812714344
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k32",
          "timeMs": 2700,
          "eye": [
            -1969.2448952517923,
            20.344176781868274,
            -1613.561247303747
          ],
          "lookAt": [
            -1907.5444983093894,
            14.32715964944061,
            -1675.3724812686555
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k33",
          "timeMs": 2750,
          "eye": [
            -1969.1377959490733,
            20.34417694866333,
            -1613.6693450456835
          ],
          "lookAt": [
            -1907.5937822188882,
            14.344852961175397,
            -1675.4228433017342
          ],
          "fovYDegrees": 90
        },
        {
          "sceneId": "3Mario.k34",
          "timeMs": 2800,
          "eye": [
            -1961.5598357470155,
            16.73274664303943,
            -1616.639726308293
          ],
          "lookAt": [
            -1902.644065582242,
            11.917522193296161,
            -1670.4436797413282
          ],
          "fovYDegrees": 85.84
        },
        {
          "sceneId": "3Mario.k35",
          "timeMs": 2900,
          "eye": [
            -1927.5606849455405,
            0.52983673089467,
            -1629.9664423976271
          ],
          "lookAt": [
            -1887.4094165780748,
            -0.5492195739268863,
            -1653.3317064438486
          ],
          "fovYDegrees": 64.08000000000001
        },
        {
          "sceneId": "3Mario.k36",
          "timeMs": 3000,
          "eye": [
            -1906.1664871145313,
            -9.665941528320312,
            -1638.3523615407657
          ],
          "lookAt": [
            -1882.690978860756,
            -9.665941433661775,
            -1648.0762354416802
          ],
          "fovYDegrees": 50
        }
      ]
    }
  },
  {
    "shotId": "shot.mario4.a",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1635,
        -23.605218,
        -1430
      ],
      "halfExtents": [
        30,
        4.394782,
        40
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1641.833396,
      -21.998647,
      -1424.348865
    ],
    "lookAt": [
      -1640.84477,
      -21.931612,
      -1425.368297
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -6.833396,
        1.606571,
        5.651135
      ],
      "lookAtOffset": [
        -5.84477,
        1.673606,
        4.631703
      ]
    }
  },
  {
    "shotId": "shot.mario4.b",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1635,
        -17.930435,
        -1406.5
      ],
      "halfExtents": [
        30,
        1.28,
        16.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1634.446713,
      -16.323864,
      -1397.649887
    ],
    "lookAt": [
      -1634.690034,
      -16.256829,
      -1399.048965
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        0.553287,
        1.606571,
        8.850113
      ],
      "lookAtOffset": [
        0.309966,
        1.673606,
        7.451035
      ]
    }
  },
  {
    "shotId": "shot.mario4.c",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1635,
        -17.930435,
        -1446.5
      ],
      "halfExtents": [
        30,
        1.28,
        23.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1642.835995,
      -16.323864,
      -1450.650639
    ],
    "lookAt": [
      -1641.515832,
      -16.256829,
      -1450.127387
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -7.835995,
        1.606571,
        -4.150639
      ],
      "lookAtOffset": [
        -6.515832,
        1.673606,
        -3.627387
      ]
    }
  },
  {
    "shotId": "shot.mario4.d",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1635,
        -15.370435,
        -1416.5
      ],
      "halfExtents": [
        30,
        1.28,
        26.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1640.599612,
      -13.763864,
      -1409.624321
    ],
    "lookAt": [
      -1639.828864,
      -13.696829,
      -1410.817036
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -5.599612,
        1.606571,
        6.875679
      ],
      "lookAtOffset": [
        -4.828864,
        1.673606,
        5.682964
      ]
    }
  },
  {
    "shotId": "shot.mario4.e",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1635,
        -15.370435,
        -1456.5
      ],
      "halfExtents": [
        30,
        1.28,
        13.5
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1638.362591,
      -13.763864,
      -1448.294903
    ],
    "lookAt": [
      -1637.971257,
      -13.696829,
      -1449.659997
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -3.362591,
        1.606571,
        8.205097
      ],
      "lookAtOffset": [
        -2.971257,
        1.673606,
        6.840003
      ]
    }
  },
  {
    "shotId": "shot.mario4.f",
    "sequenceInstanceId": "",
    "box": {
      "center": [
        -1635,
        -7.045217,
        -1430
      ],
      "halfExtents": [
        30,
        7.045217,
        40
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1641.875679,
      -5.438646,
      -1435.599612
    ],
    "lookAt": [
      -1640.682964,
      -5.371611,
      -1434.828864
    ],
    "fovYDegrees": 50,
    "blendInMs": 900,
    "blendOutMs": 900,
    "priority": 20,
    "follow": {
      "eyeOffset": [
        -6.875679,
        1.606571,
        -5.599612
      ],
      "lookAtOffset": [
        -5.682964,
        1.673606,
        -4.828864
      ]
    }
  },
  {
    "shotId": "4Mario",
    "sequenceInstanceId": "world.sequence.instance.mario_m4_intro",
    "box": {
      "center": [
        -1632.57,
        -20.49,
        -1400.92
      ],
      "halfExtents": [
        1,
        3,
        1
      ],
      "yawDegrees": 0
    },
    "eye": [
      -1639.4033961904363,
      -18.883429199218753,
      -1395.2688651163821
    ],
    "lookAt": [
      -1638.4147697685366,
      -18.816394483861238,
      -1396.2882971860795
    ],
    "fovYDegrees": 50,
    "blendInMs": 0,
    "blendOutMs": 900,
    "priority": 30,
    "cameraTrack": {
      "durationMs": 3000,
      "interpolation": "LINEAR",
      "easing": "LINEAR",
      "keyframes": [
        {
          "sceneId": "4Mario.k00",
          "timeMs": 0,
          "eye": [
            -1625.02424365861,
            -7.8497257854298415,
            -1442.5611261887918
          ],
          "lookAt": [
            -1596.0780187258345,
            -0.3618655976465144,
            -1472.3228207657048
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k01",
          "timeMs": 100,
          "eye": [
            -1625.176727171943,
            -7.8947094080587314,
            -1442.372387403158
          ],
          "lookAt": [
            -1596.4046307149526,
            -0.4517282593182106,
            -1471.972717503725
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k02",
          "timeMs": 200,
          "eye": [
            -1625.3292106852762,
            -7.939693030687622,
            -1442.1836486175243
          ],
          "lookAt": [
            -1596.7311468349246,
            -0.5415673177256286,
            -1471.6225098456773
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k03",
          "timeMs": 300,
          "eye": [
            -1625.4816941986094,
            -7.984676653316515,
            -1441.9949098318905
          ],
          "lookAt": [
            -1597.057567036791,
            -0.6313827725459733,
            -1471.2721978347736
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k04",
          "timeMs": 400,
          "eye": [
            -1625.6341777119424,
            -8.029660275945405,
            -1441.8061710462568
          ],
          "lookAt": [
            -1597.3838912716094,
            -0.7211746234565615,
            -1470.9217815142467
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k05",
          "timeMs": 500,
          "eye": [
            -1625.7866612252756,
            -8.074643898574296,
            -1441.617432260623
          ],
          "lookAt": [
            -1597.710119490457,
            -0.810942870134836,
            -1470.5712609273503
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k06",
          "timeMs": 600,
          "eye": [
            -1625.9391447386088,
            -8.119627521203189,
            -1441.4286934749894
          ],
          "lookAt": [
            -1598.0362516444266,
            -0.9006875122583535,
            -1470.2206361173596
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k07",
          "timeMs": 700,
          "eye": [
            -1626.091628251942,
            -8.164611143832078,
            -1441.2399546893555
          ],
          "lookAt": [
            -1598.3622876846302,
            -0.9904085495047781,
            -1469.8699071275698
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k08",
          "timeMs": 800,
          "eye": [
            -1626.244111765275,
            -8.209594766460969,
            -1441.0512159037216
          ],
          "lookAt": [
            -1598.688227562197,
            -1.080105981551907,
            -1469.519074001298
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k09",
          "timeMs": 900,
          "eye": [
            -1626.3965952786082,
            -8.254578389089861,
            -1440.862477118088
          ],
          "lookAt": [
            -1599.0140712282741,
            -1.169779808077644,
            -1469.1681367818815
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k10",
          "timeMs": 1000,
          "eye": [
            -1626.5490787919414,
            -8.299562011718752,
            -1440.6737383324541
          ],
          "lookAt": [
            -1599.3398186340264,
            -1.2594300287600113,
            -1468.8170955126786
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k11",
          "timeMs": 1100,
          "eye": [
            -1635.97526472139,
            -11.080299065576842,
            -1429.0071196799952
          ],
          "lookAt": [
            -1619.2873291109745,
            -6.747656533951123,
            -1446.9154440765158
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k12",
          "timeMs": 1200,
          "eye": [
            -1645.4014506508386,
            -13.861036119434932,
            -1417.3405010275362
          ],
          "lookAt": [
            -1638.8556091218034,
            -12.154904495996297,
            -1424.6292434779887
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k13",
          "timeMs": 1292,
          "eye": [
            -1654.0735417059313,
            -16.419314208984375,
            -1406.607211867274
          ],
          "lookAt": [
            -1651.6330015230612,
            -15.780626973663123,
            -1409.4189882246217
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k14",
          "timeMs": 1300,
          "eye": [
            -1654.0864086365534,
            -16.419314208984375,
            -1406.5926280930269
          ],
          "lookAt": [
            -1651.6333866006362,
            -15.77756567560025,
            -1409.41895313697
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k15",
          "timeMs": 1400,
          "eye": [
            -1654.2472452693262,
            -16.419314208984375,
            -1406.4103309149405
          ],
          "lookAt": [
            -1651.6382516174365,
            -15.739489861217963,
            -1409.418600548942
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k16",
          "timeMs": 1500,
          "eye": [
            -1654.4080819020987,
            -16.419314208984375,
            -1406.228033736854
          ],
          "lookAt": [
            -1651.6432123465904,
            -15.70176670558145,
            -1409.4184070083174
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k17",
          "timeMs": 1600,
          "eye": [
            -1654.5689185348715,
            -16.419314208984375,
            -1406.0457365587674
          ],
          "lookAt": [
            -1651.648269155189,
            -15.664396273351121,
            -1409.4183721888153
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k18",
          "timeMs": 1700,
          "eye": [
            -1654.7297551676445,
            -16.419314208984375,
            -1405.8634393806808
          ],
          "lookAt": [
            -1651.6534224099253,
            -15.627378628804966,
            -1409.4184957636653
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k19",
          "timeMs": 1800,
          "eye": [
            -1654.8905918004173,
            -16.419314208984375,
            -1405.6811422025942
          ],
          "lookAt": [
            -1651.658672477094,
            -15.590713835838482,
            -1409.4187774056065
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k20",
          "timeMs": 1900,
          "eye": [
            -1655.0514284331898,
            -16.419314208984375,
            -1405.4988450245075
          ],
          "lookAt": [
            -1651.6640197225913,
            -15.554401957964634,
            -1409.4192167868885
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k21",
          "timeMs": 2000,
          "eye": [
            -1655.2122650659628,
            -16.419314208984375,
            -1405.316547846421
          ],
          "lookAt": [
            -1651.669464511915,
            -15.518443058313787,
            -1409.4198135792722
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k22",
          "timeMs": 2100,
          "eye": [
            -1655.3731016987356,
            -16.419314208984375,
            -1405.1342506683345
          ],
          "lookAt": [
            -1651.675007210161,
            -15.482837199633643,
            -1409.4205674540299
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k23",
          "timeMs": 2200,
          "eye": [
            -1655.5339383315084,
            -16.419314208984375,
            -1404.951953490248
          ],
          "lookAt": [
            -1651.6806481820254,
            -15.447584444289191,
            -1409.4214780819466
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k24",
          "timeMs": 2300,
          "eye": [
            -1655.694774964281,
            -16.419314208984375,
            -1404.7696563121613
          ],
          "lookAt": [
            -1651.6863877918017,
            -15.412684854262652,
            -1409.4225451333198
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k25",
          "timeMs": 2400,
          "eye": [
            -1655.855611597054,
            -16.419314208984375,
            -1404.5873591340746
          ],
          "lookAt": [
            -1651.6922264033826,
            -15.378138491153415,
            -1409.423768277961
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k26",
          "timeMs": 2500,
          "eye": [
            -1656.0164482298267,
            -16.419314208984375,
            -1404.405061955988
          ],
          "lookAt": [
            -1651.6981643802565,
            -15.34394541617798,
            -1409.4251471851958
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k27",
          "timeMs": 2600,
          "eye": [
            -1656.1772848625994,
            -16.419314208984375,
            -1404.2227647779014
          ],
          "lookAt": [
            -1651.7042020855088,
            -15.310105690169914,
            -1409.4266815238639
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k28",
          "timeMs": 2700,
          "eye": [
            -1656.3381214953724,
            -16.419314208984375,
            -1404.040467599815
          ],
          "lookAt": [
            -1651.7103398818203,
            -15.27661937357978,
            -1409.4283709623207
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k29",
          "timeMs": 2750,
          "eye": [
            -1656.4185398117586,
            -16.419314208984375,
            -1403.9493190107717
          ],
          "lookAt": [
            -1651.71344642735,
            -15.2600087626025,
            -1409.4292737401986
          ],
          "fovYDegrees": 100
        },
        {
          "sceneId": "4Mario.k30",
          "timeMs": 2800,
          "eye": [
            -1654.648964875141,
            -16.675582167968752,
            -1403.0465518057554
          ],
          "lookAt": [
            -1650.2987272024632,
            -15.690551015959679,
            -1408.0490814636123
          ],
          "fovYDegrees": 94.80000000000001
        },
        {
          "sceneId": "4Mario.k31",
          "timeMs": 2900,
          "eye": [
            -1645.3927267451418,
            -18.01606072265625,
            -1398.3243848872073
          ],
          "lookAt": [
            -1643.0217633473992,
            -17.71371503769573,
            -1400.8759290975654
          ],
          "fovYDegrees": 67.60000000000001
        },
        {
          "sceneId": "4Mario.k32",
          "timeMs": 3000,
          "eye": [
            -1639.4033961904363,
            -18.883429199218753,
            -1395.2688651163821
          ],
          "lookAt": [
            -1638.4147697685366,
            -18.816394483861238,
            -1396.2882971860795
          ],
          "fovYDegrees": 50
        }
      ]
    }
  }
]
```

## G5. 2026-09-08 3관문 마리오 입구 네 곳 비활성화

사용자 요청으로 Gameplay.world.json의 Mario1_go, Mario2_go, Mario3_go, Mario4_go 네
triggerBox의 enabled만 false로 변경한다. 원래 위치·목적지·movePlayer 시간·arcHeight,
각 마리오 내부 소품/점프/인트로 Trigger와 F1 Debug 진입은 유지한다.
정본 World publisher의 Validate/Publish로 Server bootstrap에 반영하고 disabled 네 행과
그 외 모든 JSON 의미 보존을 확인한다. C++/프로젝트 등록 변경은 없다.
현재 실행 중 Server에는 자동 적용하지 않으며 새 bootstrap을 읽는 Server 재시작이 필요하다.