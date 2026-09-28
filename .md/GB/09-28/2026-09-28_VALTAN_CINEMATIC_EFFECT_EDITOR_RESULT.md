# 발탄 등장 연출 Effect Editor 구현 결과

## G00. 적용 범위

대상은 사용자가 확인한 **발탄 본체 등장**이다. 기존 `entrance`와 `entrance.colorless` WorldSequence에 연결된 24,708ms 연출, Animation 2개, Effect occurrence 14개를 기존 WorldSequencePlayer로 편집한다. 13개 공유 Effect body에는 84개 고유 Element와 반복 사용을 포함한 88개 Element 발생이 있다. 별도 actor64와 루가루 데이터는 연결하지 않았다.

작업은 `codex/valtan-all-effects-editing`의 기존 미커밋 변경을 보존한 상태에서 진행했다. 원래 Effect 데이터·Resources·렌더링 옵션은 이 작업에서 수정하지 않았다. 09-28에 시작해 09-29 KST까지 검증했다.

## G01. 사용자 기능

`F1 > Open Effect Tool V1 > All Effects > Valtan > CINEMATIC EFFECTS > 진입 / Entrance` 그룹 상단의 `Open Editor`로 연출 전체를 연다.

- `Model View > All Cinematic Elements`에서 모든 공유 Effect와 Element를 검색·선택한다. 개별 리소스 행의 Open Editor는 기존 단일 body 편집을 유지한다.
- `Play All`, Pause/Resume, Movie time, playback rate는 본체·무채색 모델과 Effect의 같은 장면 시계를 사용한다.
- Element `Solo`와 `Repeat Selection`은 원래 발생 시각과 모델·본 부착을 유지하며 선택한 Element만 표시한다.
- 반복 사용되는 Effect는 `Use occurrence`로 특정 발생 하나를 고르고 Solo할 수 있다. `Solo: All occurrences`는 전체 발생 선택을 복구한다.
- 수정·삭제는 `Save Changes`로 원래 공유 body에 저장한다. 반복 occurrence도 같은 원본을 사용하므로 저장 결과를 공유한다. 별도 복제 문서를 만들지 않는다.
- 마지막 Element를 지운 경우도 구조적으로 완전히 빈 authored body에 한해 저장할 수 있다. 숨김 또는 잘못된 carrier를 전부 허용하는 우회가 아니다.
- 미저장 전환 보호, stable ID, 디스크 기준본 비교, 원자적 저장과 실패 rollback은 유지한다. 첫 자원 준비가 진행 중이면 상태 메시지 뒤 Play All로 재시도한다.

Stop은 장면 객체를 정리하되 Solo 선택을 유지해 이후 Seek와 UI 표시가 일치한다. End/Clear가 선택을 초기화한다. 원래 runtime cinematic이 시작되면 편집 재생이 양보한다.

## G02. 구현 연결

| 소비 경로 | 변경 |
|---|---|
| `Effect_Tool_ValtanWorld`, `Effect_Tool_Workspace`, `Effect_Tool` | Entrance 그룹 Open, 전체 Element 목록, 원래 문서 선택, 연출 전용 controls와 Level context |
| `MainApp`, `Level_ValtanArena` | 기존 Movie callback을 `valtan.entrance`로 전달하고 Level이 단독으로 장면 시계를 소유 |
| `WorldSequencePlayer`, `WorldSequencePlayer_Objects` | 기존 world-root preview의 full/selection 교체와 occurrence draw 격리, 실패 시 이전 상태 유지 |
| `Effect_Tool_Editing`, `CatalogPreview`, `Playback`, `Detail`, `TransformHistory` | Full Restore source-aware Solo, 그룹 선택 전체 전달, 올바른 source 시작 시점, 선택 history와 실제 follow carrier 처리 |
| `Effect_Tool_DocumentIo`, `Effect_DocumentRenderer_PreparedDocument` | 구조적으로 완전히 빈 authored Product body 저장·준비 허용; 나머지 drawable 검증 유지 |

WorldSequence 편집은 임시 preview만 기존 renderer/service에 교체한다. 원래 actor animation, TRS, socket/bone, particle history는 계속 기존 sequence 경로가 소유한다. 빈 source는 loop/fit 계산 전에 건너뛰고 이전 draw handle을 제거한다. 신규 C++ 파일이나 project/filter 등록은 없다.

## G03. 피자 검기와 공통 Solo 원인

현재 피자/모아치기 주 대상은 `effect.valtan.action.420620.stage005.full.restore`다. 기존 경로는 Solo에서 제외한 형제 trail의 baked-edge history를 남긴 문서를 먼저 World Preview에 stage하여 `Authored runtimeExtensions contain an unreferenced baked-edge history.`로 실패했다. 현재 문서의 재생 입장이 허용된 10개 중 9개에서 이전 실패를 재현했다.

Full Restore Solo를 Complete와 같은 source-aware sequencer에 먼저 전달한다. 공유 선택 문서 생성에서도 실제 선택 및 dependency closure가 참조하지 않는 history만 제거한다. 남은 trail/light의 history와 source projection은 유지한다. 여러 Element의 일회 그룹 재생도 첫 Element만 전달하던 분기를 수정했다.

legacy `effect.valtan.red-blade-wave.active`의 실제 projectile offset은 1700ms다. World Solo 시작·반복은 선택 Element source 시작을 owner timeline으로 변환한 시점으로 맞춘다. 2833ms/28600ms는 추가 시간 변환 fixture이며 실제 projectile binding으로 기록하지 않는다.

`effect.valtan.pattern.420633.active`는 현재 baked trail 3개·follow particle 5개·baked light 1개다. baked carrier를 live bone-follow로 오인하던 준비 경로를 분리하고, 고정된 옛 carrier 개수 대신 실제 follow 집합의 동일 bone/socket을 검증한다. 모델·clip·generation 및 실제 bone 검증은 유지한다.

## G04. 실행한 검증

### 완료된 검증

- 앞선 정상 Debug Product 빌드 2회 PASS. 두 번째 receipt는 `out/BuildPipeline/runs/20260928T145951787Z-debug-product.json`이다. 최종 후속 수정 전 결과라는 경계를 유지한다.
- 13개 Effect JSON, 현재 연결된 sequence, 프로젝트/filters XML 및 기존 C++ 14개 등록 확인. 수치는 14개 Effect track/84개 Element/88개 발생/2개 Animation이다.
- 실제 설치 WModel과 clip을 사용하는 창 없는 WARP probe에서 full→exact occurrence Solo→Clear, 잘못된 track/source의 rollback, 마지막 Element draft 삭제, 정방향·역방향 seek, Stop 통과.
- Solo에서 EffectObject 9개를 유지하고 정확히 1개만 inspection-visible이었다. 설치된 head bone은 같은 16초에서 Solo 전후 동일하며, 16→18초 actor-relative head transform은 최대 0.174266 변해 실제 애니메이션 진행을 확인했다.
- 피자 stage005의 입장 허용 Element 10개에서 선택별 CPU playback packet 생성 확인. 숨김/미지원 Element admission은 해제하지 않았다.
- legacy 검기 실제 1700ms와 추가 시간 fixture를 합쳐 15개 선택/24개 CPU packet 확인.

증거는 `out/ValtanCinematicEditor20260928/sequence-preview-receipt.json`, `sequence-run.log`, `structure-validation.json`, `out/ValtanSoloPlayback20260928/native.log`에 있다. 검증 산출물은 Git 제외 out에 둔다.

### 최종 인계 시 구분

빈 문서 저장·추가 Solo 보완과 아래 에테르 구슬 후속 수정을 포함한 최종 Debug Product 빌드가 통과했다. receipt는 `out/BuildPipeline/runs/20260928T151923322Z-debug-product.json`이며 최종 Client EXE가 링크·배포됐다. 이 빌드는 사용자 EXE 종료 이후 실행했고 데이터 게시를 수행하지 않았다.

저장된 빈 source의 분리된 native 검증과 420633 추가 검증 결과는 G05에 기록한다.

Client/UI를 에이전트가 실행하거나 조작하지 않았다. 수치 검증은 실제 화면·GPU 출력의 시각 판정을 대신하지 않는다. 창 없는 fixture는 audio 초기화를 하지 않아 오디오 판정도 제외했다. 실제 화면에서 끝까지 Play All, Solo, 편집·저장 후 표현 확인은 사용자가 수행한다.

## G05. 최종 후속 검증

`out/ValtanSoloPlayback20260928/receipt.json`의 최종 검사에서 420633은 설치된 `MN_RPBF_01.wmodel`과 AnimSet, 정본 preScale 0.0001, 실제 `b_effectroot` index 83을 매 fixed-step으로 샘플했다. 9개 Element 모두 여러 시각에서 nonempty CPU packets를 만들었다. 전체 9개 그룹 71 packets, particle 5개 그룹 67 packets를 포함해 총 605 packets였다. 기존 선택 경로의 particle 5개 orphan history 실패와 full source의 follow 분류 실패를 재현했다. 누락된 attachment/history, 잘못된 bone 및 실제 모델에 없는 bone은 계속 거부했다.

창 없는 sequence probe는 Git 제외 `out/ValtanCinematicEditor20260928/saved-empty-data`의 13개 원본 복사본과 분리된 catalog를 사용했다. 완전한 빈 body의 `Save_AtomicIfUnchanged → parse → Reload_SelectedProductEffect`를 통과했고 준비된 duration은 0이었다. Preview override를 Clear한 뒤 catalog의 빈 눈 Effect를 실제 sequence로 소비했을 때 native-loop root가 생성되지 않았고 본 자세와 다른 Effect의 catalog identity는 유지됐다. 실제 원본 저작 파일에는 쓰지 않았다.

잘못된 nonempty 후보는 원자적 저장 전 Codec에서 `Authored runtimeCarrier target is not a visible bounded drawable.`로 거부됐다. 구조 검증을 끄지 않았다.

최종 전체 `git diff --check`는 exit 0이며 whitespace 오류가 없다. Renderer/WSP 후속 변경은 out의 분리된 native probe에서 컴파일했다. 마지막 보완을 포함한 정상 Product 전체 빌드 결과는 G04의 최종 receipt로 확인했다.

## G06. 사용자 재진입 검사에서 발견한 에테르 구슬 오류

사용자 화면의 `level-valtan.initialize / Map Effect source-loop clone/attach rejected: effect.valtan.ether.orb`를 실제 failure log와 대조했다. 구슬의 nonempty 6개 Element 중 portable Cascade Ribbon이 기존 owner-sustained의 sprite/mesh-only 조건에 걸렸다. 이펙트 데이터를 변경하지 않고 기존 ribbon 재생 능력과 해당 허용 검사를 일치시켰다. 상세 원인과 재검증은 같은 날짜의 `2026-09-28_VALTAN_WORLD_ETHER_RESULT.md` G04를 따른다.
