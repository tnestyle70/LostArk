# 마리오 작은 오망성 바닥 연결 결과

## G00. 반영 상태

`effect.kouku.gate3.mario.boss.pentagram.full.restore`에 기존 별 그리기 mesh 8개와 바닥 decal 5개를 연결했다. 효과 문서, Resource Tree, 재설치 생성기의 표시 이름은 `작은 오망성`이다. stable asset ID와 기존 별 그리기 8개 요소는 보존했다.

현재 중앙 마리오 V1은 바닥 해골·연기 sprite 10개이며 별 바닥 decal을 포함하지 않는다. 재사용한 바닥 정본은 `Data/Effects/V2/Groups/boss.kouku.disarm.effectv2group.json`의 `boss.kouku.disarm.star.decal_1` 다섯 배치다. 이 원본 그룹과 leaf는 수정하지 않았다. 이를 중앙 마리오 V1에서 직접 가져온 원본이라고 기록하지 않는다.

## G01. 크기와 수명

설치된 `fm_d_rpct_07/08.wmodel`의 UV V=0.5 중심선에 geometryPreScale, 원본 StartSize, 원본 MeshRotation을 적용했다. geometryPreScale은 약 0.01이고 StartSize는 2이며, MeshRotation lookupTable의 첫 두 값은 bounds이므로 실제 roll/pitch/yaw는 그 뒤에서 읽는다.

| 항목 | 반영값 |
|---|---:|
| 작은 별 중심선 반경 | 4.6999998 m |
| 기존 바닥 별의 배치 중심으로 산출한 기준 반경 | 10.0124973 m |
| 위치·decal 폭·길이 배율 | 0.4694133433 |
| 바닥 yaw 보정 | +17.999976° |
| 각 바닥 strip 크기 | 0.4694133 × 9.3882669 m |
| projection depth | 기존 0.2 m |
| 한 번의 바닥 표시 시간 | 0~2.29999995 s |
| 등장 alpha 상승 완료 | 약 0.1794 s |
| 퇴장 alpha 감소 | 2.0~2.3 s |

기존 바닥 strip의 끝 여유 길이도 함께 축소했으므로 strip 끝 반경은 약 4.91 m다. 별의 선 중심 반경 4.7 m에 원본 바닥 균열의 끝 여유가 남는다. 다섯 요소는 같은 시점에 한 번 생성되고 2.3초에 제거된다. 원본 V2의 1초 간격 순차 재생·9초 수명을 복사하지 않았다.

V2의 dissolve-in 구간 비율 0.078을 등장 alpha 구간에 사용하고, 작은 별의 최종 flash lifetime 0.3초를 퇴장 alpha 구간으로 사용했다. V2 dissolve mask의 공간 진행을 완전히 재현했다는 판정은 하지 않는다. 기존 V1 `alphaScaleKeys` 경로로 명시적인 등장·퇴장을 연결했다. 전체 문서의 기존 CPU playback duration 4초와 Composition 등록 duration 3000 ms는 변경하지 않았다.

## G02. 변경 파일과 재생성

- `Data/Effects/Authored/effect.kouku.gate3.mario.boss.pentagram.full.restore.effect.json`: 기존 8개를 보존하고 바닥 5개 추가, 이름 정리.
- `Data/Effects/EffectResourceTree.json`: 같은 asset의 표시 이름 정리.
- `Tools/EffectPipeline/build_kouku_gate3_slam_mario_restore.py`: 작은 별 완성 함수, 신규 투영 시 연결, 기존 문서에 대한 명시적인 `--complete-small-pentagram` 적용 추가.
- `Tools/EffectPipeline/install_kouku_effect_library.py`: 재등록 시 작은 별의 표시 이름을 동일하게 유지.

재생성 명령은 다음과 같다. 현재 입력에 두 번 적용해도 바닥 요소가 늘어나지 않는 것을 확인했다.

```powershell
python Tools/EffectPipeline/build_kouku_gate3_slam_mario_restore.py --complete-small-pentagram --evidence-root out/KoukuSmallPentagram20260912
```

새 shader, C++, project/filter 항목, Resources binary를 추가하지 않았다. 참조하는 기존 바닥 파일은 `Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_decal_020.dds`와 `Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_atypical_076_2_cl.dds`다. Resources는 기존 Drive 관리 입력을 그대로 사용한다.

Composition은 같은 요청의 Parent 구현자가 `kakulsaydon.effect.7e765b32937e215a2fef`의 표시 이름을 `작은 오망성`으로 반영하도록 전달했다. 이 문서 작업에서 Composition을 직접 수정하지 않았다.

## G03. 실행한 검증과 남은 확인

- 현재 `Effect_DocumentCodec.cpp`, `Effect_Playback.cpp`를 out 전용 probe로 다시 컴파일하고 링크했다. DirectXTK debug PDB 부재 경고 외 컴파일·링크 오류는 없다.
- 실제 codec load와 CPU playback stage에서 13개 요소를 수용했다. 기존 범용 probe의 최대 CPU draw row는 10개였다.
- 현재 코드의 60 Hz probe로 0~4초 241개 frame을 검사했다. 2.3초 전에는 바닥 5개, 이후에는 0개이며 모든 바닥 transform이 finite였다.
- 실제 평가 alpha는 0초 0, 0.1초 0.557414, 0.2초 1, 2.15초 0.5, 2.28333초 0.0555555였다.
- 원본 8개 요소가 Git HEAD와 구조적으로 동일하고, 13개 stable element ID가 중복되지 않으며, 직접 참조하는 Resource 4개가 모두 설치되어 있다.
- Python syntax, JSON parse, 생성기 재적용 동일성, 변경 파일 `git diff --check`를 통과했다.

증거는 `out/KoukuSmallPentagram20260912/asset_numeric_validation.json`, `floor_timing_probe.json`, `small_pentagram_floor_fit.json`에 있다. 처음 재사용한 과거 probe는 alphaScaleKeys 반영 전 객체여서 alpha를 무시했으며, 최종 곡선 검증은 현재 두 번역 단위를 다시 컴파일한 실행 파일로 수행했다.

Client/UI 실행·조작·캡처와 GPU 육안 판정은 하지 않았다. 사용자가 F1의 Effect Tool V1에서 KoukuSaydon → 3관문 → 패턴 → 세이튼 → 마리오 → `작은 오망성`을 재생해 바닥의 실제 크기·방향·색상과 퇴장 시점을 확인해야 한다. 현재 상태는 데이터 반영 및 CPU 검증 완료이며 최종 visual 판정은 `USER_PENDING`이다.
