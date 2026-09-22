# 쿠크 화염파동 불바닥·14개 편집 그룹 결과

## G00. 현재 완료 상태

후속 실제 반영 승인을 받아 불바닥 Effect와 14개 편집 그룹을 최신 저장본에 설치했다.
통합 revision 2194의 Kouku domain publish도 PASS했다. 아래 후보 생성·검증 기록은
설치 전 이력이며, 최종 상태는 [통합 결과](2026-09-22_KOUKU_GATE1_PATTERN_RESTORATION_RESULT.md)를 따른다.
실행 중 도구 Reload와 사용자의 최종 외형 판정은 수행하지 않았다.
계획과 연결 경계는 [구현 계획](2026-09-22_KOUKU_FLAME_WAVE_IMPLEMENTATION_PLAN.md)을 따른다.

`Tools/EffectPipeline/prepare_kouku_flame_wave_refinement.py`를 추가했다.
`build_kouku_flame_wave_groups.py::duration_ms`는 Codec 저장본에서 생략 가능한
`sourceScale.lifeTime`을 기본값 1로 처리하도록 한 줄 수정했다. 제품 C++·shader·
runtime schema와 프로젝트/filter는 변경하지 않았다.

## G01. 불바닥과 14개 그룹 후보

첨부 6번에는 바닥에 반복되는 검붉은 중심과 노란 외곽 화염이 보인다. 기존 바닥 문서는
WandDecal 13요소이며 FireWave의 지면 화염 sprite가 포함되지 않았다. 원본 FireWave에서
native2874의 `particlespriteemitter_16`과 native2873의 `particlespriteemitter_23`을
추가했다. texture·재질·EPAL_Z·크기·색·alpha·수명은 보존하고, 고정 초기 위치를
독립 저작 원점으로 옮기고 바닥 높이 0.04m를 적용했다. 원본 source 문서는 변경하지 않았다.

| 후보 asset suffix | 요소 수 | Playback 전체 수명 |
|---|---:|---:|
| `flame.wave.decal` | 15 | 7,000ms |
| `flame.wave.full` | 392 | 9,100ms |
| `flame.wave.full.koukusaydon` | 392 | 9,100ms |

full의 기존 260개 element ID를 모두 보존했다. 4행은 2/3/4/5개, 총 14지점이다.
각 `manual.flame-wave.rN.cN` 그룹은 전조·기둥·바닥 28요소를 소유한다. 좌우 간격은
5→3.5m, 행 전진은 4.330→3.031m, 첫 행 전방은 5→3.5m다. 원본 외형 배율을
전체 변경하는 대신 지점 사이의 배치만 축소했다.

후보와 stable-ID 변경 목록은 `out/KoukuFlameWave20260922/manifest.json`에 있다.
3개 Effect 후보의 Resources 34/58/58개와 유일 element ID, JSON parse를 확인했다.

## G02. 실제 Codec·Playback·그룹 저장 검사

기존 화염파동 console probe를 현행 Debug Product object와 재링크했다. Client/UI를
실행하지 않고 실제 Product staging과 Playback을 전체 시간에 걸쳐 검사했다.

- 3문서 219,335검사, failures 0. full peak particle 1,298, 마지막 양수 alpha 시각
  7.28333초, finite·capacity 검사가 통과했다. 9.1초는 보수적인 전체 active+tail 수명이다.
- root yaw 0/90/180도와 이동 대조의 최대 행렬 오차는 3.8147e-6m다.
- 새 바닥 28요소 모두에서 양수 alpha 표본이 나왔으며 총 8,372개다. 목표 중심 XZ
  오차는 최대 4e-7m, 입자 중심 높이는 0.0399999991m다. 이는 billboard 이전 중심
  검사이며 최종 GPU 법선이나 화면 외형 검사로 설명하지 않는다.
- 실제 `Build_AttachmentElementGroups`, `Translate_AttachmentElementGroup`,
  `Is_ManualElementGroupMember`, `ManualGroup_Label` 함수 원문을 별도 console TU로
  컴파일했다. 14그룹·각 28요소, 선택 그룹만 `[1.25,0.5,-2]m` 이동, 다른 요소 보존,
  실제 Codec `Save_Atomic → Load`의 일치를 포함한 426검사에서 failures 0이다.

증거는 같은 out 폴더의 `native_result.log`, `native_source_receipt.json`,
`ground-native-measurements.json`, `group_result.log`, `group-helper-source.json`,
`verification.json`이다. source text·Codec/Playback 검증과 사용자 화면 판정은 별개다.

## G03. 최신 Composition 후보와 필드 보존

최초 검토는 디스크 revision 2186에서 revision 2187 후보를 생성했다. 변경은 resource
기본 수명 3곳, P49/P59 effect occurrence 수명 2곳, 두 pattern 전체 수명 2곳,
revision 1곳으로 총 8개 scalar다. JSON token 범위만 교체해 무관한 필드·숫자 표기·
공백을 보존했다. P58은 최신 저장본 전체를 유지했다.

| 대상 | 후보 값 |
|---|---:|
| 바닥 resource 기본 수명 | 7,000ms |
| full·alias resource 기본 수명 | 9,100ms |
| P49 effect occurrence / pattern 전체 | 9,100 / 12,604ms |
| P59 effect occurrence / pattern 전체 | 9,100 / 11,004ms |

후보 경로는 `out/KoukuFlameWave20260922/candidate/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`이다.
최종 재확인 때 사용자가 P100을 추가 저장하여 디스크 revision이 2188로 증가했다.
해당 변경을 보존하며 같은 field guard를 다시 통과해 후보 revision 2189를 생성했다.
적용 기준 latest-before SHA256은
`26c3fa2462f2d234ef5b9b8293d46bc46b502cea4d6c42efd2d5222496645aa4`,
후보 SHA256은
`7024e979c9390de9e20b4f375b795ea439a2e9a29f05b2a9136135aa8dcb3e7b`다.

예상 duration을 고의로 다르게 만든 patch는 `Concurrent duration edit`로 거부됐고,
기존 후보와 라이브 저장본은 그대로 유지됐다. `composition-minimal.diff`와
`composition-merge-validation.json`에 비교·충돌·보존 결과를 기록했다.

## G04. 기존 projector 검증

라이브 Data/Resources는 읽기 전용 junction으로 참조하고 Composition만 후보 복사본을
사용하는 out의 격리 repository에서 다음 명령을 실행했다.

```powershell
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --repository-root out/KoukuFlameWave20260922/validation-repository --mode validate
```

명령은 구조 검사와 실제 projection 생성 뒤 기존
`Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`의
`projected Product is stale`로 exit 1을 반환했다. 최초 revision 2187 후보와 최신
revision 2189 후보에서 모두 같은 결과다. 게시하지 않은 후보와 기존 projected output의
불일치이며, 전체 validate 성공으로 기록하지 않는다. 최신 CLI 로그는
`projector-validate-2189-error.log`에 남겼다.

개별 Pattern이 admission에서 제외된 뒤 전체 검사가 진행되는 경우도 구분하기 위해
P49/P58/P59 각각의 실제 dependency closure에 대해 현행
`validate_document → validate_publishable → projected_outputs`를 실행했다.
최신 revision 2189로 다시 검사한 세 Pattern 모두 admission 성공이며
Encounter/patternbindings 총 6개 결과를 out에
생성했다. 이들은 검증용 개별 projection이며 제품 전체 게시본으로 설치하지 않는다.
`projection-validation.json`에 각 결과의 hash와 크기를 남겼다.

Python compile, 후보 JSON parse와 변경 파일 whitespace 검사를 통과했다. 최종 반영 시에는
현재 저장본 hash와 같은 필드의 변경을 다시 검사해야 한다. 준비 후 추가 저장이 있었다면
파일 전체 교체 대신 stable-ID patch를 최신본에 다시 병합한다.

## G05. 남은 사용자 확인

Effect Tool V1에서 기존 화염파동 saved Effect를 열고 `Current Effect → Group by Anchor`
아래 14개 manual 그룹을 확인한다. 한 그룹의 `Group Center`를 수정한 뒤
`Save Changes`로 저장하며, `Play Group`과 전체 재생으로 바닥·기둥의 밀도와 간격을
확인한다. 설치·publish 전에는 실행 중 도구에 후보가 자동 반영되지 않는다.

불바닥 색·형태·밀도와 첨부 이미지의 시각 일치는 사용자 확인 전이다. GPU first pixel,
visual PASS 또는 원본 action의 disabled notify 복구 완료로 기록하지 않았다.
