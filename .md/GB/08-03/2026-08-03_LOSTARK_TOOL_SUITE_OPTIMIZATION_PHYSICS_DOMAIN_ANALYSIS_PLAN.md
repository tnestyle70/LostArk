# 2026-08-03 LostArk 툴 스위트·최적화·피직스 도메인 분석 (PLAN)

문서 유형: 도메인 분석 + 상위 설계 (개별 slice 구현 계획서 아님 — 각 툴/기능 착수 시 코드 우선 PLAN 별도 작성)
목표 선언: 참고팀 툴 스크린샷 9장(`C:\Users\user\Desktop\툴`)의 본질을 실측 분석하고, **같은 기능을 LostArk의 계약(안정 ID, publish 파이프라인, 서버 권위) 위에 "더 나은 형태"로 구현**하며, 컬링/LOD 최적화와 PhysX 본체인 클로스까지 도메인 전체를 하나의 영상+자막 기술소개로 닫는다.

이 문서는 기능 목록만 보관하지 않는다. 각 실제 구현 slice는
`2026-08-03_TOOL_SUITE_OVERVIEW_PLAN.md`의 학습·설계 완료 계약을 따라 파일 존재 이유,
H include, 자료구조와 대안, 함수 호출자와 상태 변화, 불변식, 실패 재현, 작은 변경까지 닫는다.
도메인 전체를 한 번에 구현하지 않고 CharacterSelect부터 하나씩 수직으로 검증한다.

관련 문서:
- [렌더링 원작 격차 조사](../08-01/2026-08-01_LOSTARK_RENDERING_ORIGINAL_LOOK_GAP_ANALYSIS_RESULT.md) (Tier 1~5)
- [고급 렌더링·월드 툴 견적](../08-02/2026-08-02_LOSTARK_ADVANCED_RENDERING_WORLDTOOL_INTEGRATION_SCOPE_PLAN.md) (W1~W8 공수)
- [북극성.md](../../../북극성.md) §19 툴 북극성, §20-21 단계 게이트

---

## 1. 참고팀 툴 9장 실측 분석 — 핵심 요약

9장 전수 분석 결과, 참고팀 툴 전체를 관통하는 **하나의 공통 패턴**이 있다:

```text
카테고리 선택 → 에셋 선택 → Create(씬 스폰) → 뷰포트 피킹/리스트 선택
→ 수치·드래그 편집 → 이름 붙여 Save/Load → "Parsing Save"(런타임 포맷 별도 출력)
```

즉 **툴 = 데이터 정본을 만드는 공장, 런타임 = 그 산출물의 소비자**이며, 작업본과 런타임본을 분리(`Save` vs `Parsing Save`)했다. 이건 우리 저장소의 `Data/Maps/Authoring → Publish-MapAuthoring.ps1 → DataFiles` 계약과 같은 사상이다. **아키텍처 사상은 우리가 이미 앞서 있고(안정 PlacementId, 트랜잭션 publish, rollback), 부족한 것은 서브툴의 폭이다.**

| # | 화면 | 본질 한 문장 | 채택할 것 | 우리가 더 낫게 할 것 |
|---|---|---|---|---|
| 01 | 이펙트 툴(Texture+Detail) | 실게임 맵을 배경으로 스프라이트 이펙트의 채널·수치를 실시간 튜닝하고 프리셋으로 저장 | **5채널 합성 모델(Base/Noise/Mask/Emissive/Dissolve)**, 컬러 3종 분리(Clip/Mul/Offset)+Lerp 토글, UV 플립북 파라미터 세트, 이펙트 단위 Bloom/Distortion/Radial 강도 | 우리 `CEffect_Runtime`은 이미 모듈·커브·빔·트레일로 더 강력. 부족한 건 **씬 승격 + 5채널 셰이더 + 디스토션/소프트파티클**(견적 W6·⑭) |
| 02 | 이펙트 툴(Mesh+Emissive) | 메시 이펙트에 채널별 텍스처를 조합, Emissive는 블룸 적용 플래그 | Category→BaseMeshes 2단 브라우징, 채널탭↔슬롯 1:1 대응 UI | 에셋 ID 기반 카탈로그(문자열 이름 아님), `Update Textures` 대신 파일 워처 |
| 03 | 렌더 타깃 디버그 뷰 | 최종 화면 이상이 어느 중간 타깃(그림자/노멀/깊이/블룸 단계)에서 시작됐는지 한 화면에서 격리 | 전 타깃 그리드 표시, 블룸 체인 단계별 보존, 이펙트 전용 타깃 분리 노출 | **참고팀은 라벨이 없다.** 우리는 타깃 이름 라벨 + 클릭 시 단독 확대 + 채널(R/G/B/A/깊이 리니어라이즈) 선택 + 프레임 정지 검사까지 (`CTarget_Manager` 디버그 출력 확장) |
| 04 | 맵 툴(메시 배치+트랜스폼) | .mesh 라이브러리에서 배치→피킹→수치 정밀 편집→일반/보스맵 저장 | World/Cur 이원 커밋, 평면 구속 이동, 균등 스케일 | 이름 문자열 대신 **PlacementId 정본**(이미 우리 방식), Save/Parsing Save 대신 **validate→stage→commit publish**(이미 우리 방식), 기즈모+Undo 추가 |
| 05 | 맵 툴(애니메이션 메뉴) | 배치된 애니 모델을 툴 안에서 재생/정지/초기 프레임 확인 — 게임 재실행 왕복 제거 | 배치 상태 애니 프리뷰(Play/Stop/FirstFrame/클립 선택) | 클립을 인덱스 슬라이더가 아니라 **이름 목록**으로, `.wanim` 이벤트 마커 표시까지 |
| 06 | 맵 툴(Convenience: 클론·사각 인스턴싱·알파) | 반복 배치를 "개수·범위 입력→버튼 한 번"으로, 반투명 렌더 타입 즉시 전환 | **Clone, Rect 영역 N개 인스턴싱 생성**, 오브젝트별 렌더 타입 전환 | 렌더 모드는 우리 카탈로그 renderProfile이 정본(인스턴스 정수 속성 아님). 인스턴싱 배치는 랜덤 시드·간격·정렬 옵션 추가 |
| 07 | 맵 툴(콜라이더) | 메시마다 부모 구+자식 OBB 2계층 충돌 볼륨을 시각 편집해 저장 | 계층형(broad 구→narrow OBB) 콜라이더 저작, 와이어 시각화 | **서버 권위 계약 반영**: 판정 정본은 Server cook — 툴은 authoring만, publish가 서버 collision pack 생성 (북극성 §8) |
| 08 | 맵 툴(쿼드트리) | 컬링용 쿼드트리 루트 볼륨·깊이를 뷰포트에서 눈으로 잡고 맵 데이터에 굽기 | 트리 시각화(와이어 박스), 깊이 설정 | **손 저작 대신 자동 쿡**: placement bounds에서 publish 단계에 쿼드트리를 결정적으로 생성(C2 이동>계산). 툴은 시각화·검증만 |
| 09 | 카메라 툴(컷/포지션) | 실제 맵에서 카메라 포즈를 캡처해 컷 단위 경로를 만들고 즉시 재생·저장 | **Capture→PosList→Cut→파일** 마스터-디테일 구조, LookAt 더미 분리, Start/Stop 프리뷰 | 포즈 나열 대신 **키프레임+보간(ease) 트랙** — Winters `CSequencePlayer` 이식과 결합, CameraCue 계약(북극성 §17.2)의 저작 도구로 |

### 1.1 영상에 나온 기술 스택의 본질

| 영역 | 보이는 기능 | 실제로 증명해야 하는 본질 |
|---|---|---|
| AI | Behavior Tree/FSM, node graph, runtime trace | 저작 graph가 Server runtime의 동일 stable node ID와 실행 결과로 이어지는가 |
| Effect | sprite/mesh/particle/trail, curve, SubUV, emissive, distortion | 저작한 asset 하나가 runtime 단일 경로에서 같은 timing/material 결과를 내는가 |
| Map | asset browser, transform, instancing, collider, spatial partition | stable PlacementId 편집이 validate/publish를 거쳐 실제 world와 server cook에 반영되는가 |
| Physics | PhysX rigid body, joint solver, bone branch secondary motion | animation pose와 simulation pose의 소유권·고정 timestep·reset 경계가 닫혔는가 |
| Rendering | deferred targets, PBR, SSAO, outline/toon, fog, bloom | pass별 입력·출력을 render target에서 격리하고 최종 합성 오류의 최초 지점을 찾을 수 있는가 |
| Optimization | frustum/distance/occlusion culling, LOD, instancing | 측정된 병목과 정확성 기준을 바탕으로 CPU/GPU 비용이 실제로 줄었는가 |
| Camera | shot/cut list, sequencer | stable cue와 정렬된 keyframe이 동일 시간축으로 재생·저장·재로드되는가 |
| Editor | ImGui, save/load, runtime format conversion, debug rendering | UI command가 정본 데이터를 안전하게 변경하고 실패 시 기존 상태를 보존하는가 |

설득력은 “기능을 구현했다”가 아니라 저작 데이터와 실제 게임 결과가 같은 단일 경로를
소비하고, 제작 왕복 시간이 줄었다는 증거에서 나온다.

### 1.2 하나로 닫혀야 하는 제작자 시나리오

```text
CharacterSelect에서 캐릭터 선택
-> 장비·색상 변경
-> animation 재생
-> 무기/뼈 socket에 Effect 연결
-> 옷자락 secondary motion 조정
-> PBR/SSAO/outline 결과 확인
-> Render Target과 CPU/GPU 비용 확인
-> authoring 저장
-> validate/stage/commit publish
-> process 재시작 후 reload
-> 실제 Bern/Valtan/Training 경로에서 동일 결과 확인
```

영상과 결과 문서에는 최소한 첫 결과까지 걸린 시간, 한 번 수정 후 재확인 시간, publish 실패
rollback, reload 일치 여부, 실제 게임 경로의 asset/revision ID를 남긴다. 그래야 “툴을 만들었다”가
아니라 “제작 시간을 얼마나 단축했다”를 설명할 수 있다.

참고팀 대비 우리의 구조적 우위(이미 확보): 안정 ID 계약, publish 트랜잭션·rollback, 보스 판정 서버 권위, ImGui UI 규칙(계획서작성규칙 §9 — 참고팀 툴은 오타·내부 용어가 기본 화면에 노출됨).

---

## 2. LostArk 툴 스위트 설계 — 6종의 본질과 현재 보유량

각 툴의 계약: **ImGui는 선택과 command만, 정본은 데이터 파일, 런타임 반영은 publish/서비스 경유** (AGENTS.md).

### 2.1 맵 툴 (보유 → 확장)

- 본질: AssetId 카탈로그에서 PlacementId 인스턴스를 만들어 Transform과 레이어를 편집하고 publish로 런타임에 반영.
- 보유: 배치·피킹·기즈모·NavGrid 페인트·deploy prop·발탄 페이즈 오버레이 — 참고팀 대비 이미 우위.
- 확장(참고팀에서 채택): ① Clone+Rect 인스턴싱 배치(06) ② 배치 애니 프리뷰(05) ③ 콜라이더 authoring 레이어(07 — 단 판정 정본은 서버 cook) ④ 쿼드트리 **시각화**(08 — 생성은 publish 자동) ⑤ Undo/Redo(Winters EditorTransaction 이식).

### 2.2 이펙트 툴 (보유 → 승격+셰이더 확장)

- 본질: EffectAssetId 정본(.weffect)을 편집하고 `CEffect_Runtime`으로 즉시 재생 확인.
- 보유: 모듈·커브·SubUV·빔·트레일·워밍업 — 파라미터 모델은 참고팀보다 강력.
- 확장: ① 5채널(Base/Noise/Mask/Emissive/Dissolve) 셰이더 도입 ② Bloom/Distortion 강도(HDR 체인 연동, 견적 W1 선행) ③ 씬 승격 + EffectCue 소비(견적 W6) ④ 프리셋 브라우저(01의 Saved Data UX).

### 2.3 카메라/시퀀스 툴 (신규 — 참고팀 09 + Winters Sequencer)

- 본질: CameraCueId가 가리키는 컷신(카메라 트랙+이펙트/사운드 마커)을 저작·프리뷰. gameplay는 cue 의미만 보내고 재생 여부는 클라 결정.
- 구성: Capture 기반 키프레임 수집(09 채택) + 트랙 보간·이벤트(Winters `CSequencePlayer` 이식) + 발탄 등장/에스더 컷신이 첫 소비자.

### 2.4 렌더 타깃 디버그 툴 (신규 — 참고팀 03의 상위 호환)

- 본질: 파이프라인 각 단계(G-Buffer 5장, LightAcc, 섀도맵, 향후 HDR/블룸 체인)를 라벨과 함께 나열·격리 검사해 "어느 타깃부터 틀렸는지"를 즉시 판정.
- 기반: `CTarget_Manager`의 _DEBUG 타깃 표시가 이미 존재 — 그리드 전체 뷰 + 클릭 확대 + 채널 선택 + 깊이 리니어라이즈 + 스크린샷 저장으로 확장. **견적 W1~W3의 모든 작업이 이 툴 위에서 검증되므로 렌더링 트랙 착수 직전(0.5~1일 선행)이 최적 시점.**
- 영상 챕터의 중심 장면이기도 하다(참고팀도 이 화면으로 렌더러를 소개).

### 2.5 밸런스 툴 (신규 — 참고 이미지 없음, 우리 계약에서 도출)

- 본질: `Data/Balance/*.json`(Player/Skills/Damage/Boss)의 스키마 검증 편집기. 정본은 JSON이며 툴은 편집·검증·publish 트리거만.
- 흐름: JSON 로드 → 스키마·참조 검증(스킬 ID↔서버 계약 존재 여부) → 저장 → `Publish-GameplayBalance.ps1` 실행 → Server 재기동 + smoke 안내. Hot Reload는 `.md/TEAM/BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`의 수직 슬라이스가 닫히기 전 금지(기존 계약 준수).
- 가치: 기획 역할(팀원)이 코드 없이 수치를 만지고, 잘못된 ID·범위를 저장 전에 차단.

### 2.6 AI(행동트리) 툴 (신규 — 시기 조건부)

- 본질: 서버에서만 실행되는 BT(Selector/Sequence/Condition/Action/Wait)의 노드 그래프를 저작·검증하고, 고정 seed 시뮬레이션으로 프리뷰.
- **착수 조건(중요)**: AGENTS.md가 소비자 없는 계약을 금지하므로, **찬영의 서버 BT 런타임(P4 카오스 던전)이 먼저 존재해야 한다.** 툴은 그 후. 그 전에 할 수 있는 것은 BT 자산 JSON 스키마 합의뿐.
- 프리뷰는 클라 재생이 아니라 **서버 하네스 실행 결과(행동 로그·타임라인)를 시각화** — 이중 truth 금지.

---

## 3. 컬링·LOD 최적화 도메인

### 3.1 현재 보유 실측

- `CFrustum` 월드/로컬 컬링 판정 존재. `CMapStaticBatchObject`가 인스턴스별 프러스텀 컬링 후 동적 인스턴스 버퍼 재구성(발탄 13,103 placement 중 Opaque 배치 분).
- `CLevelRegistry`의 `MAP_LOAD_SCOPE`가 거리 로딩의 상위 계약이다. 외부 LevelCatalog parser는
  폐기하고 Bern/Valtan/Test의 고정 진입·전투 범위를 LEVEL descriptor에 둔다.

### 3.2 기법별 본질과 채택 판단

| 기법 | 본질 | 채택 판단 |
|---|---|---|
| 프러스텀 컬링 | 카메라 절두체 밖 = 그리지 않는다 | 보유. **쿼드트리 결합으로 O(N) 순회→O(logN+K) 개선**이 다음 단계 |
| 쿼드트리 | 공간을 4분할 트리로 나눠 "노드째 기각" | 채택. 단 참고팀처럼 손 저작하지 않고 **publish 단계에서 placement bounds로 자동 생성**(결정적 cook). 툴은 시각화·검증 |
| 디스턴스 컬링 | 카메라 거리 기준 소형 프롭 생략 | 채택(저비용). 카탈로그에 cullDistance 필드 — 쿼터뷰는 카메라 거리가 거의 일정하므로 효과는 제한적, 소품류에만 |
| 오클루전 컬링 | 가려진 것은 그리지 않는다 (HW occlusion query / HiZ / SW rasterizer) | **profiler gate 뒤로 유보.** 쿼터뷰 부감 카메라는 가림 자체가 적어 이득이 작다. 면접 스토리가 목적이면 HiZ 컴퓨트 방식이 GTAO 인프라(견적 W3)와 시너지 |
| LOD | 먼 것은 싼 버전으로 | 유보. `.wmodel`에 LOD 체인이 없어 포맷 확장 필요 — World Partition HLOD(견적 W5 C컷)와 함께 판단 |
| 배칭/인스턴싱 | 드로우콜 병합 | 보유(`CMapStaticBatchObject`). 참고팀 렌더러 컴포넌트식 자동 분류는 이미 우리가 상위 호환 |

원칙: **측정 → 병목 → 기법 선택.** 발탄 아레나에서 GPU 타임스탬프/드로우콜 카운트를 먼저 찍고(Winters GPU 마커 패턴 이식), 프러스텀+쿼드트리 → 디스턴스 → (증거 시) 오클루전 순.

## 4. PhysX 본체인 클로스 도메인

### 4.1 질문에 대한 확인

**맞다.** PhysX SDK 안에 전부 있다: rigid body(`PxRigidDynamic`/`PxRigidStatic`), 관절(`PxSphericalJoint` 포함 PxJoint 계열), 그리고 **solver**(반복 제약 솔버 — PGS 기본, TGS 선택 가능)가 `PxScene::simulate()` 내부에서 돈다. 참고팀 설명은 PhysX의 표준 사용법이다.

### 4.2 기법의 본질 — "본 체인 = 관절로 묶인 강체 사슬"

```text
애니메이션이 소유한 뼈(골반/어깨 등 루트) = kinematic 앵커 (애니 행렬을 매 프레임 주입)
시뮬레이션할 뼈 집합(치맛자락·코트 자락) = branch
branch의 각 뼈 = PxRigidDynamic (캡슐/구 지오메트리)
이웃 뼈 사이 = PxSphericalJoint (콘 리밋으로 과회전 방지)
매 프레임: 애니 갱신 → 앵커 kinematic target 설정 → simulate(고정 스텝) → fetchResults
→ branch 뼈의 시뮬 결과 pose를 본 행렬에 역주입 → 스키닝
```

- 이것은 진짜 천 시뮬(cloth mesh solver)이 아니라 **관절 강체 사슬 근사**다 — 옷자락·머리카락·장식 술에 적합하고 값싸며, 참고팀(건슬링어 코트)도 이 방식.
- 핵심 계약: ① **고정 timestep**(가변 dt를 그대로 넣으면 폭발) ② 앵커는 kinematic, 나머지만 dynamic ③ joint limit + damping + 최대 속도 클램프 ④ 순간이동/텔레포트 시 branch 리셋 ⑤ **표현 전용** — 서버 판정과 무관한 Client visual only(북극성 §20 PhysX cloth = P6 stretch와 일치).

### 4.3 툴 연동 설계 (질문의 "툴에서 애니메이션을 적용"의 정체)

캐릭터/애니메이션 프리뷰 툴에 **본 브랜치 저작 모드**를 추가한다: 스켈레톤 트리에서 시뮬 대상 뼈 집합을 branch로 지정 → 뼈별 질량·반경·joint limit·damping 편집 → 애니 재생하며 즉석 시뮬 확인 → 모델별 데이터(`Data/Animation/Authored/<AssetId>/physics.json` 류)로 저장 → 런타임 로더가 소비. `vector<Branch>` 보관 구조는 참고팀 설명 그대로 채택하되, 뼈 참조는 인덱스가 아닌 **본 이름 해시**(우리 WSKL 계약)로 저장한다.

### 4.4 공수 (견적 증분)

PhysX 통합(라이브러리 링크·씬·고정 스텝 루프) 2~3일 + branch 저작 툴 2~3일 + 캐릭터 1종 튜닝·리셋 처리 2일 = **6~8일** (P6 stretch, C컷).

## 5. 렌더링 도메인 연결 (기존 문서에 추가되는 것)

기존 격차 조사(08-01)와 견적(08-02)에 이미 있는 PBR/SSAO/포그 외에 이번 요청에서 새로 확정된 것:

- **외곽선(Outline) 셰이딩**: 원작에도 옵션이 있는 요소. 디퍼드라 노멀/깊이 불연속 검출 포스트 패스가 정석(1~2일) — 톤맵 체인(W1)에 동거. 캐릭터 한정이면 스텐실 마킹 후 검출.
- **렌더 타깃 디버그 툴**: §2.4 — W1 착수 직전 선행(0.5~1일).
- **환경 안개**: W3의 거리/높이 포그 = 즉시, froxel 볼류메트릭 = C컷 유지.

## 6. 견적 증분 요약 (08-02 견적에 더해지는 것)

| 항목 | 일수 | 배치 |
|---|---:|---|
| 렌더 타깃 디버그 툴 (라벨·확대·채널) | 0.5~1 | W1 직전 |
| 외곽선 포스트 패스 | 1~2 | W1 체인 |
| 쿼드트리 자동 쿡 + 프러스텀 결합 + 툴 시각화 | 3~4 | W3 이후, profiler와 함께 |
| 디스턴스 컬링(카탈로그 필드) | 0.5 | 상동 |
| 맵 툴 Convenience(클론·Rect 인스턴싱) | 1~2 | 아무 때나(독립) |
| 배치 애니 프리뷰 | 1~2 | 상동 |
| 콜라이더 authoring(+서버 cook 연동) | 3~5 | P4~P5 서버 충돌 계약과 동시 |
| 카메라 툴(Capture/Cut) + Sequencer 이식 | 4~6 | W6 항목 확장(기존 3~5일 대체) |
| 밸런스 툴 | 2~3 | 독립(팀 가치 즉시) |
| AI(BT) 툴 | 4~6 | **P4 BT 런타임 이후** |
| PhysX 본체인 클로스 + 저작 툴 | 6~8 | P6 stretch (C컷) |
| **증분 합계** | **26~40일** | 오클루전·LOD는 profiler gate 뒤 별도 |

08-02 총계와 합치면 "전부 다"는 **약 115~155 집중 작업일**이다. 컷라인 원칙 유지: A컷(렌더링 코어+디버그 툴+맵 툴 확장+밸런스 툴) 우선, AI 툴은 P4 게이트, PhysX·오클루전·LOD는 C컷.

## 7. 영상·기술소개 마감 전략

참고팀 영상의 챕터 구조(툴별 챕터 + 자막 해설 + 렌더 타깃 뷰로 렌더러 소개)를 채택하되:

1. 챕터당 "문제 → 툴/기능 시연 → 내부 구조 한 컷(디버그 뷰·로그) → 자막 요약" 4박자.
2. 렌더 타깃 챕터는 우리 디버그 툴의 **라벨·격리 검사**를 그대로 보여줌 — 참고팀과의 차별점이 화면에 드러난다.
3. 각 챕터의 자막 문구는 기술소개서의 해당 절 요약과 동일 문장 사용(영상↔문서 교차 검증 가능하게).
4. 캡처는 기능 완료 즉시(견적 W8 원칙 유지).

## 8. 위험·경계

- **AI 툴 선행 금지**: BT 런타임(소비자) 없는 에디터는 AGENTS.md 위반. 스키마 합의까지만.
- **쿼드트리 손 저작 금지**: 참고팀 방식을 그대로 옮기면 C2(이동>계산) 위반 — publish 자동 생성이 정본.
- **콜라이더 이중 truth 금지**: 툴이 만든 콜라이더는 authoring이고 서버 cook이 판정 정본. Client Collider는 시각화/예측용.
- **모든 신규 툴 코드는 Framework.sln 안에서 빌드**(Winters 에디터 rot 부채 #11 재발 방지).
- 각 slice 착수 시 코드 우선 PLAN + `.vcxproj/.filters` 등록 + smoke를 한 변경 단위로.

## 9. 도메인별 구현 전에 답할 자료구조·함수 질문

아래 형태는 확정 구현이 아니라 조사 출발점이다. 실제 코드의 owner, 호출자, 데이터 정본을
확인한 뒤 현재 수직 슬라이스에 필요한 최소 구조만 선택한다.

| 영역 | 우선 검토할 자료구조 | 반드시 비교할 대안·불변식 |
|---|---|---|
| CharacterSelect | 고정 class `array`, stable class ID, entry state enum, optional pending world | `vector` 전환 조건, UI index 저장 금지, 승인 대기 상태의 단일성 |
| AI | stable node ID map + child ID list + 실행 trace ring buffer | pointer graph/variant/상속 node 비교, cycle 금지, Server 실행 순서와 trace 일치 |
| Effect | emitter 목록, curve keyframe의 시간 정렬 vector, particle pool, material channel descriptor | AoS/SoA, CPU/GPU simulation, keyframe 중복 시간 처리, runtime asset 단일성 |
| Map | PlacementId lookup map + draw batch vector + spatial tree | vector index 저장 금지, map/unordered_map 검색 비용, placement와 catalog 분리 |
| Physics | `vector<Branch>`, branch별 bone binding, actor/joint handle 집합, fixed-step accumulator | bone index/이름 hash 비교, teleport reset, actor/joint 생성·파괴 순서 |
| Rendering | render target descriptor registry + pass 입력/출력 view | tag map/enum/handle 비교, read-write hazard 금지, resize 후 view 일관성 |
| Optimization | bounds array + quadtree/BVH node 배열 + visible set | O(N) 순회와 tree 비용, false negative 금지, LOD hysteresis |
| Camera | track별 시간 정렬 keyframe vector + stable cue ID | list/vector 비교, 같은 timestamp 정책, interpolation 범위와 loop 종료 |
| Balance | staged document + stable ID lookup + validation error list | live object 직접 수정 금지, 중복 ID, 전체 검증 성공 전 commit 금지 |
| Editor 공통 | command/transaction + undo/redo stack + selection stable ID | snapshot/command diff 비교, 실행 실패 command를 history에 넣지 않음 |

각 함수는 계획서에 다음 순서로 기록한다.

```text
직접 호출자와 thread
-> 입력과 선행 불변식
-> 읽는 owner 상태
-> 지역 staging 상태
-> 외부 계약 호출
-> 성공 시 변경하는 상태
-> 실패 시 rollback하고 보존하는 상태
-> 반환값과 다음 소비자
```

각 헤더 include는 “필요한 symbol”만으로 끝내지 않고 public signature/member에 완전한 타입이
필요한지 확인한다. pointer/reference만 노출한다면 forward 선언으로 CPP에 숨길 수 있는지 먼저
검토한다. 새 Manager, Catalog, Service는 이름 때문에 만들지 않고 현재 소비자와 owner가 확인될
때만 추가한다.
