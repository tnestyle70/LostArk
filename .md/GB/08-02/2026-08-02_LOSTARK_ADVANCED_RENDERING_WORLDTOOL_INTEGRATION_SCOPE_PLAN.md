# 2026-08-02~08-03 LostArk 제작자 툴·렌더링·물리·최적화 통합 마스터 계획 (SCOPE PLAN)

문서 유형: 범위·공수 견적서 + 통합 마스터 설계서 (개별 slice 구현 계획서 아님 — 각 항목 착수 시 별도 PLAN 작성)
최종 갱신: 2026-08-03 — `C:\Users\user\Desktop\툴` 레퍼런스 9장, 현재 LostArk 코드·데이터·팀 계약, 밸런스/AI/이펙트/맵/카메라/렌더 타깃/물리/컬링·LOD 범위를 통합했다.
목표 선언: **Winters와 발표 레퍼런스의 UI를 그대로 복제하는 데서 끝내지 않고, 제작자가 안전하게 찾고 선택하고 편집하고 검증하고 publish하고 재현할 수 있는 LostArk 제작 파이프라인으로 다시 설계한다. PBR·SSAO·외곽선·환경 안개, 렌더 타깃 디버깅, 컬링·LOD, 물리 기반 secondary motion까지 실제 제품 경로에서 작동하게 하고, 각 기능을 측정값·영상·게임 자막·기술소개서로 닫는다.**

```text
C1~C8: OFF
문제 해결 ①~⑤: OFF
자료구조·알고리즘: OFF
```

근거 문서:
- Winters 실측: 렌더러 보유/미구현 전수 조사 (2026-08-02, 아래 §2 표)
- LostArk 실측: [렌더링 원작 격차 조사](../08-01/2026-08-01_LOSTARK_RENDERING_ORIGINAL_LOOK_GAP_ANALYSIS_RESULT.md)
- 순서 계약: [북극성.md](../../../북극성.md) §20-21 (고급 렌더는 P6, 항목별 profiler gate)

---

## 1. 총평 먼저 (솔직한 숫자)

| 컷 | 내용 | 집중 작업일 |
|---|---|---:|
| **A컷 — 포트폴리오 코어** | 후처리 체인 + PBR/IBL + CSM/라이트 + 발탄 연출 + 영상·문서 | **45~60일** |
| **B컷 — 차별화** | World Partition(진짜 async) + 툴 승격 + 베이크 GI | **+25~35일** |
| **C컷 — 여유 시** | TAA, 볼류메트릭 포그, HLOD, SSR, Clustered | **+20일 이상** |
| 합계 ("전부 다") | | **약 90~115일 ≈ 풀타임 4.5~6개월** |

전제: 혼자, 집중 작업일 기준(회의·빌드·팀 지원 제외), Winters를 만든 숙련도. **서버/온라인(P2~P5) 작업과 별개의 숫자**다 — 나는 그쪽도 담당이므로 실제 달력 기간은 병행 비율에 따라 1.5~2배로 늘어난다.

결론: **"전부 다"는 컷라인 없이는 달성 불가능한 규모다.** A컷을 완주 목표로, B컷을 조건부, C컷을 명시적 제외 후보로 관리한다.

---

## 2. 재고 조사 — Winters에서 무엇을 가져오고 무엇을 새로 만드나

### 2.1 이식(포팅) 가능 — Winters에 동작 코드 존재

| 자산 | Winters 위치 | LostArk 이식 난점 |
|---|---|---|
| GGX Cook-Torrance BRDF 라이브러리 | `Shaders/BRDF/BRDF_GGX.hlsli` (D_GGX/G_Smith/F_Schlick/에너지보존) | 낮음 — 헤더 이식 후 디퍼드 라이트 패스에 결선 |
| PBR 머티리얼 CB 패턴 (b3: metallic/roughness/AO/emissive) | `CMaterialPBR` + `CBPerMaterial.h` | 낮음 — LostArk `.wmodel`이 ORM/metallic/roughness/AO를 **이미 로드 중**(소비자만 없음) |
| GTAO 컴퓨트 + depth-aware blur | `Shaders/SSAO/GTAO_CS.hlsl` + `SSAOPass.cpp` | 중간 — LostArk에 컴퓨트 셰이더 인프라 신설 필요 |
| Bloom(soft-knee)+감마/채도/틴트/비네트 | `Shaders/PostFx/PostFx.hlsl` + `PostFxPass.cpp` | 낮음 — HDR 오프스크린 타깃(W1) 선행 |
| Contact shadow 데칼 | `Shaders/ContactShadowPlane.hlsl` | 낮음 — 텔레그래프 장판과 셰이더 구조 공유 |
| EditorTransaction (undo/redo) | `EldenRingEditor/World/EditorTransaction.cpp` | 낮음 — MapTool 공용 코어로 승격 |
| Sequencer 런타임 (Camera/Anim/Fx/Audio/Event 트랙) | `Engine/Private/Cinematic/CSequencePlayer.cpp` | 중간 — 발탄 등장/에스더 컷신용 |
| World Partition 골격 (grid/cellId/핸들 레지스트리/소스 반경/가시 수집) | `Engine/Private/World/WorldPartitionSystem.cpp` (1,211줄) | 중간 — 단 스트리밍이 가짜(§2.3) |
| FX 그래프→실행계획 구조 | `Engine/Private/FX/` | 참고만 — LostArk엔 자체 CEffect_Runtime이 이미 있음(승격이 우선) |

### 2.2 신규 제작 — Winters에 계획만 있고 코드 0건 (실측 확인)

| 항목 | Winters 실측 | LostArk에서의 규모 |
|---|---|---|
| **IBL** (prefiltered cubemap + BRDF LUT + ambient) | `TextureCube/Cubemap/Skybox/brdf_lut` 심볼 전 코드 0건 | 대 — 오프라인 프리필터 베이커 툴 포함 |
| **CSM 그림자 맵** | `ShadowMap/CSM/CascadeShadow` 심볼 0건 (Shadows 패스 실체는 접지 데칼) | 대 — 단 LostArk엔 단일 섀도맵이 이미 있어 절반 출발 |
| **ACES 톤맵 + 3D LUT** | Reinhard 한 줄뿐, ColorLUT 0건 | 소 |
| **TAA + 모션 벡터** | 심볼 0건 | 대 (C컷) |
| **Forward+/Clustered 다광원** | Tile Cull CS 0건, 포인트라이트 상한 4개 | 대 (C컷 — LostArk는 디퍼드라 라이트 볼륨 최적화로 대체 가능) |
| **SSR / VXGI / DDGI / Path Tracing** | 전부 0건 (Stage 3~5, 8 계획만) | GI는 §4 옵션 결정 |
| **진짜 비동기 스트리밍** | `bReadyImmediately = true` — 디스크 I/O 자체가 없는 가짜 | 중 — 클라 멀티스레드 설계(로딩 워커 계약)와 정합 |
| **HLOD** | `strHlodWmesh` 필드만 존재, reader 0곳 | 대 (C컷) |
| **에디터 뷰포트/기즈모/ray-pick** | "deferred for next gate" 로그 출력하는 셸 | **LostArk MapTool이 이미 보유 — 재제작 불필요, 확장만** |

### 2.3 Winters의 뼈아픈 교훈 2개 → LostArk 완료 정의로 승격

1. **PBR dead path**: Winters는 PBR 셰이더를 부팅 시 컴파일까지 하지만 **호출부 전원이 non-PBR 경로를 지정** — 만들고 아무도 켜지 않았다.
2. **기본 OFF**: GTAO도 PostFx도 구현됐지만 기본 비활성 + 디버그 토글 한정 — 영상에 안 나오는 기능이 됐다.

→ **LostArk 완료 정의(전 항목 공통): "발탄/베른 기본 실행 경로에서 켜져 있고, 영상에 나오고, 기술소개서에 실측 ms가 적혀 있다."** 만들었지만 꺼져 있는 기능은 없는 기능으로 계수한다.

---

## 3. 작업 패키지(WBS)와 공수

난이도: ●낮음 ●●중간 ●●●높음. 일수는 집중 작업일. (선행) 표기는 하드 의존.

### W1. 후처리 기반 체인 — 7~10일 ●●

| 작업 | 일수 | 비고 |
|---|---:|---|
| sRGB/감마 파이프라인 정리 | 1~2 | 로더 플래그 + RT/백버퍼 `_SRGB` |
| HDR 씬 타깃(R16G16B16A16F) + 톤맵 패스 | 2~3 | Reinhard 이식 → **ACES 신규** |
| Bloom 이식 | 1~2 | Winters PostFx.hlsl (선행: HDR 타깃) |
| FXAA | 1 | 톤맵 패스에 동거 |
| 3D LUT 컬러그레이딩 + 페이즈 보간 | 2 | 신규 — 발탄↔고스트 톤 전환 |

### W2. PBR + IBL — 11~17일 ●●●

| 작업 | 일수 | 비고 |
|---|---:|---|
| BRDF_GGX 이식 + G-Buffer 개편(metallic/roughness/AO 기록) + 디퍼드 라이트 패스 PBR화 | 4~6 | 에셋 ORM은 이미 로드됨 — 소비자 결선이 본체 |
| IBL: 오프라인 큐브맵 프리필터 베이커 + BRDF LUT + 앰비언트 결합 | 5~8 | **신규.** Winters 계획서(GGX+A Stage 7)의 미상환 부채 |
| 발탄/캐릭터/맵 머티리얼 튜닝 | 2~3 | 스킨드 5채널 확장 포함 |

### W3. 그림자·라이팅 — 10~15일 ●●●

| 작업 | 일수 | 비고 |
|---|---:|---|
| CSM (3~4 cascade, 직교 fit, PCF, 안정화) | 4~6 | 신규 — 기존 단일 섀도맵을 교체 |
| GTAO 이식 (+컴퓨트 인프라 신설) | 2~3 | Winters CS 이식 |
| 라이트 데이터화 + MapTool 라이트 편집 + 포인트 라이트 볼륨 최적화 | 3~4 | Light_Manager API 정리 포함 |
| 거리/높이 포그 | 0.5 | 볼류메트릭은 C컷 |

### W4. GI — 옵션 결정 필요 (5~15일) ●●●

| 옵션 | 일수 | 판단 |
|---|---:|---|
| (a) GTAO만으로 마감 | 0 | W3에 포함 — 최소선 |
| **(b) Irradiance 프로브 베이크 (권장)** | 5~8 | UV2 불필요, 발탄 정적 아레나에 적합. 오프라인 베이커 = Winters Stage 3(몬테카를로) 학습 목표를 실물로 소화 |
| (c) 라이트맵 베이커 | 10~15 | `.wmodel`에 UV2 채널 추가 필요 — 포맷 변경 팀 협의 필수 |
| (d) SSR/VXGI/DDGI/DXR | 15+ | **명시적 제외 권고** — 면접 가치 대비 일정 파괴 |

### W5. World Partition + 진짜 스트리밍 — 8~12일 (+HLOD 5~8일) ●●●

| 작업 | 일수 | 비고 |
|---|---:|---|
| 골격 이식 (grid/cellId/핸들·상태/소스 반경) | 2~3 | Winters 1,211줄 재사용 |
| 오프라인 cell 빌더 (mapplacements → cells) | 2~3 | 발탄 13,103 placement가 실데이터 |
| **진짜 async 디스크 로더** (워커+우선순위 큐+프레임 예산+main commit) | 4~6 | Winters의 가짜(`bReadyImmediately`)를 실물로 — 클라 MT 설계와 동일 계약 |
| HLOD 간이판 (cell 머지 메시) | 5~8 | C컷 |

### W6. 툴·연출 승격 — 9~14일 ●●

| 작업 | 일수 | 비고 |
|---|---:|---|
| EditorTransaction 이식 → MapTool undo/redo | 2~3 | Winters 교훈: 에디터는 sln/CI에 편입 |
| 이펙트 씬 승격 + EffectCue 브리지 | 3~4 | 북극성 §17.1 계약의 실물 |
| Sequencer 이식 (발탄 등장/에스더 컷신) | 3~5 | CameraCue 소비자 |
| MapTool: cell 시각화·포스트 프로파일 편집 | 1~2 | |

### W7. 발탄 전투 연출 (원작 격차 Tier 3~4) — 12~16일 ●●

데칼/텔레그래프 3~4, 소프트파티클+디스토션 2~3, 림/히트플래시 2~3, 카메라 셰이크·줌·히트스톱 2, 애니 크로스페이드+이벤트 배선 3~4.

### W8. 영상 + 기술소개서 — 14~19일 (분산 집행) ●●

| 작업 | 일수 | 비고 |
|---|---:|---|
| 기능별 before/after 클립 + 자막 (~15주제 × 0.5일) | 7~8 | **각 기능 완료 직후 즉시 캡처** — 마지막에 몰면 리그레션에 발목 |
| 기술소개서 (기능별 문제→선택→수식/구조→트레이드오프→실측 ms) | 4~6 | 이력서 문서 스타일(`2026-08-02_엔진지도_오브젝트수명_설계재료.md`)과 통일 |
| 마스터 포트폴리오 영상 편집 | 3~5 | |

---

## 4. 순서와 게이트 (북극성 정합)

```text
[선행 필수 — 이 견적 밖]  P2 베른 온라인 슬라이스 → P3 파티 barrier
[진입 게이트]            P4~P5 컨텐츠가 팀원 주도로 돌기 시작하는 시점부터 본 트랙 병행
[권장 순서]              W1 → W2 → W3 → W7 → (W4b) → W6 → W5 → C컷
[각 항목 게이트]          profiler 실측(적용 전/후 ms) 없이 다음 항목 착수 금지
```

- W1이 최우선인 이유: 이후 모든 항목(PBR·IBL·이펙트)의 시각 결과가 HDR/톤맵 위에서만 올바르게 보인다. 순서를 바꾸면 두 번 튜닝한다.
- W5(World Partition)를 뒤로 둔 이유: 발탄 단일 아레나는 스트리밍 없이도 돌아간다. 가치는 "진짜 async를 만들었다"는 스토리이므로, 클라 네트워크 워커(P2)로 스레드 계약을 먼저 실전 검증한 뒤가 안전하다.
- 몬테카를로/패스트레이싱/DXR(Winters Stage 3/4/8)은 **런타임 목표에서 제외**하고, W4(b) 오프라인 프로브 베이커의 수학으로 소화한다 — "이해해서 녹여낸다"는 목표를 일정 안에서 지키는 방법.

## 5. 위험 대장

| 위험 | 조기 신호 | 대응 |
|---|---|---|
| 서버 트랙과 일정 경쟁 (동일인) | P2 지연 중 렌더링 커밋 증가 | 북극성 게이트 준수 — 온라인 슬라이스 미완이면 본 트랙 동결 |
| dead path 재발 (Winters D-1/D-2) | 기능 머지 후 기본 OFF 방치 | §2.3 완료 정의 — 기본 ON + 영상 + 실측 ms 3종 세트 |
| `.wmodel` 포맷 변경 파급 (UV2/탄젠트) | W4(c) 선택 시 | 팀 협의 전 착수 금지, W4(b)로 우회 가능 |
| G-Buffer 개편이 팀원 셰이더 작업과 충돌 | Client/Bin/ShaderFiles 동시 수정 | slice PR 분리 + 셰이더 계약 공지 후 병합 |
| 툴 rot (Winters 부채 #11) | CMake 전용/CI 밖 툴 | 모든 툴 코드는 Framework.sln 안에서 빌드 |
| 캡처 부채 | "나중에 몰아서 찍자" | 기능 완료 = 클립 캡처 완료로 정의 |

## 6. 산출물 계약

- 항목별: 별도 slice PLAN(계획서작성규칙 준수, vcxproj/filters 등록 포함) → 구현 → RESULT 문서(실측 ms) → before/after 클립.
- 기술소개서 목차(항목당): 문제 → 후보와 선택 이유 → 핵심 수식/자료구조 → 트레이드오프 → 실측(해상도·프레임 ms·메모리) → 한계와 하지 않은 것.
- 최종: 마스터 영상 1편 + 기술소개서 1부 + 이력서 문서 갱신(`이력서/` 폴더의 면접 재료와 상호 링크).

---

## 7. 2026-08-03 통합 개정의 결론

이 절부터는 2026-08-03 현재 코드와 데이터, `C:\Users\user\Desktop\툴`의 레퍼런스 9장,
발표에서 언급된 PhysX·렌더링·최적화 내용을 다시 실측해 확장한 마스터 계획이다. 범위가
렌더링과 WorldTool을 넘어 밸런스·AI·이펙트·애니메이션 물리·카메라까지 늘어났으므로,
§1의 90~115일은 더 이상 전체 범위의 합계가 아니다.

### 7.1 핵심 판단

레퍼런스 툴의 본질은 ImGui 창의 모양이 아니다. 제작자가 다음 폐루프를 빠르고 안전하게
반복할 수 있게 하는 것이 본질이다.

```text
찾기 -> 선택 -> 맥락을 보며 편집 -> 즉시 미리보기 -> 오류 설명 -> 저장
     -> publisher 검증 -> 실제 runtime과 같은 경로로 재생 -> 측정/비교 -> 재현 자료 남기기
```

따라서 완성도를 `패널 수`나 `버튼 수`로 판단하지 않는다. 각 툴은 아래 다섯 조건을 모두
충족해야 완료다.

1. **정본 일치**: 툴 전용 사본이 아니라 현재 `Data -> publisher -> runtime` 계약을 편집한다.
2. **권위 보존**: UI가 packet, socket, snapshot, 전투 판정을 직접 다루지 않는다.
3. **실패 원자성**: `parse -> validate -> stage -> commit`이며 실패하면 기존 문서와 runtime을 보존한다.
4. **재현성**: stable ID, seed, revision, scenario ID, profiler artifact로 같은 결과를 다시 만든다.
5. **증거 완결성**: 실제 제품 경로 ON, 자동 검증, 실패 사례, 영상·자막, 수치가 함께 있다.

`완벽하게`는 끝이 없는 미감 표현으로 두지 않고 이 다섯 완료 조건과 도메인별 예산을 통과한
상태로 정의한다.

### 7.2 전 범위 재견적

| 범위 | 현재 기반 | 추가 집중 작업일(추정) |
|---|---|---:|
| 공통 Creator Workbench, transaction, 문서 세션 | F1 도구 허브와 각 툴은 있으나 공통 편집 코어 없음 | 10~15 |
| Render Target/Pass Debug + capture | profiler와 RT manager 일부 있음 | 8~12 |
| Balance Tool + 검증/publish + revision 기반 hot reload 2단계 | JSON/publisher/server catalog 있음, UI 없음 | 12~20 |
| AI/Encounter Tool + server trace | Valtan FSM/데이터/30 Hz runtime 있음, 범용 graph runtime 없음 | 15~25 |
| Effect Tool 제작자 UX 승격 + cue/runtime 연결 | schema v5와 preview/runtime 있음 | 15~25 |
| MapTool 제작자 UX 승격 + spatial/visibility authoring | 배치·preview·gameplay/nav/camera mode 있음 | 20~30 |
| Camera/Sequence Tool | follow/free camera만 있음 | 10~15 |
| 가시성 최적화(공간 분할·distance·LOD·occlusion) | instancing+CPU frustum 있음 | 20~35 |
| PBR/IBL·SSAO·outline·fog·조명/그림자 | HDR/bloom/distortion 기반 있음 | 25~40 |
| PhysX 도입 + secondary motion authoring/runtime | SDK와 solver가 전혀 없음 | 20~35 |
| 통합 회귀·영상·자막·기술 문서 | profiler JSON 기반 일부 있음 | 15~25 |
| **전체** | 병렬 중복 제거 전 | **약 170~277일** |

공통 코어와 검증을 재사용하면 실제 합계는 일부 줄어들 수 있다. 그래도 1인이 제품 경로,
에디터, 데이터, 하네스, 영상까지 모두 닫는 범위는 최소 반년 이상이다. 첫 출시 컷은
`공통 코어 -> Render Debug -> Balance -> Map/Effect -> AI -> Visibility -> Rendering -> Physics`
순으로 두고, 범용 Behavior Tree, volumetric fog, HLOD, articulation 고도화는 측정 결과에 따라
후속 컷으로 관리한다.

---

## 8. 레퍼런스 9장 해석: 화면 복제가 아니라 작업 의도 복원

| 레퍼런스 | 화면에서 보이는 기능 | 제작 의도 | 현재 LostArk | 개선 구현 방향 |
|---|---|---|---|---|
| `01_EffectTool_Texture_EffectDetail.png` | Texture/Particle/Decal/Trail, texture palette, keyframe, UV/sequence/life/billboard/pass | 한 effect의 시간 변화와 material 입력을 같은 맥락에서 편집 | Sprite/Mesh/Beam/Ribbon/AnimTrail, curve, collision, LOD, SubUV, world preview까지 이미 보유 | graph/curve timeline, deterministic scrub, validation, pass/RT 링크, 예산 표시를 한 문서 세션으로 통합 |
| `02_EffectTool_Mesh_Emissive.png` | mesh/material/emissive 편집 | geometry와 material response를 실제 조명 아래 확인 | `CModel -> CMaterial` runtime과 mesh effect 보유 | PBR/stylized lighting preset, overdraw/distortion/bloom 기여도, socket preview 추가 |
| `03_RenderTarget_DebugView.png` | 모든 중간 RT 모자이크 | 최종 픽셀의 원인을 pass별로 분해 | HDR/G-buffer/shadow/distortion/bloom RT는 있으나 LightDepth 외 debug 노출이 사실상 닫힘 | RT registry, grid/fullscreen, channel/mip/slice, depth linearize, normal remap, pixel probe, NaN/Inf, pass GPU ms |
| `04_MapTool_MeshFile_TransformMenu.png` | asset palette, hierarchy, transform, save/load | 대량 배치에서 찾기·선택·수치 편집 | 3-column palette/hierarchy/inspector와 stable placement ID 보유 | gizmo, multi-select, snap, transaction, dirty diff, validation, focus/isolate |
| `05_MapTool_AnimationMenu.png` | map object animation play/stop/frame/index | 움직이는 환경 오브젝트의 배치 상태 확인 | MapTool에는 해당 제어 없음 | 배치 인스턴스가 animation presentation profile을 참조하고, preview만 제어; gameplay state는 Server 소유 |
| `06_MapTool_Convenience_Instancing_AlphaBlend.png` | clone/rectangle random placement, alpha 분리 | 반복 배치를 빠르게 만들고 draw path를 제어 | runtime hardware instancing은 있음, authoring scatter 없음 | placement brush/scatter recipe, seed, preview/commit, collision/nav exclusion, batch eligibility 진단 |
| `07_MapTool_ColliderTool.png` | parent/child collider, sphere/box transform | 시각 자산과 collision shape를 함께 맞춤 | 간단 overlap collider만 있고 authoring/publisher 없음 | visual physics collider profile과 server nav/gameplay 영역을 분리; 안정 ID와 shape validation 사용 |
| `08_MapTool_QuadTreeTool.png` | box center/scale/depth/index | 광역 맵을 공간 단위로 잘라 culling | 모든 instance를 CPU frustum test하는 O(N) 경로 | 제작자는 cell을 그리는 대신 offline builder 결과를 시각화·진단; grid/loose quadtree/BVH를 실측 선택 |
| `09_CameraTool_CutList_PosList.png` | cut/position list, look-at, capture, play | 재현 가능한 연출과 촬영 | follow/free만 존재 | stable sequence/shot/key ID, transform/look-at/FOV/easing/event track, save/load/preview/export |

레퍼런스에서 가져올 것은 정보 구조와 피드백 속도다. 오래된 툴의 전역 mutable 상태, vector index
저장, 매 프레임 파일 읽기, 즉시 runtime mutation, 수동 quadtree index 같은 구현 습관은 가져오지
않는다.

---

## 9. 현재 LostArk 실측 기준선

### 9.1 이미 있는 것

- F1 Debug Developer Tools와 scenario metadata 기반 lazy tool 생성.
- `CMapTool`의 Map Assets / World Gameplay / Navigation / Camera 모드, stable placement ID,
  quaternion, signed scale, staged runtime commit/rollback.
- `CEffect_Tool`과 effect schema v5의 Sprite/Mesh/Beam/Ribbon/AnimTrail, curve, collision,
  event, LOD, SubUV, world preview.
- `CModel -> CMaterial` 통합 runtime과 animation 선택/crossfade/track position.
- deferred G-buffer, shadow, `SceneHDR`, distortion, half-resolution bloom, tone map/gamma.
- static deferred map object hardware instancing, per-instance bounding sphere CPU frustum culling.
- CPU scope, D3D11 timestamp query ring, counters, JSON capture profiler.
- `Data/Balance`의 player/skill/damage/boss 정본, strict publisher, Server gameplay catalog.
- `Data/Encounters/Valtan/ValtanEncounter.json`, 30 Hz Server `CValtanBrain`, navigation 추적,
  windup/active/recovery와 Server damage 판정.

### 9.2 없는 것 또는 닫히지 않은 것

- 공통 undo/redo transaction, multi-document dirty state, atomic save UI, publish 결과 탐색.
- Balance Tool과 revision 기반 runtime hot reload.
- AI graph editor와 graph runtime/compiler/debug trace.
- MapTool gizmo, multi-select, scatter, collider authoring, spatial cell 진단, camera cut sequencer.
- render target/pass의 안정적 introspection registry와 전용 debug viewer.
- distance culling, LOD/HLOD, occlusion culling, runtime world partition.
- PBR/IBL, SSAO, outline/toon, 환경 fog, CSM, light authoring.
- PhysX/Jolt/Bullet 등의 dynamics SDK, rigid body solver, joint, cloth/secondary motion runtime.
- `CBone`/`CModel`의 animation 이후 외부 pose modifier와 safe bone override 단계.

이 기준선 때문에 “있는 툴을 예쁘게 다시 그린다”와 “새 runtime 계약을 만든다”를 반드시 다른
slice로 나눈다. 전자는 비교적 짧은 UI/transaction 작업이고, 후자는 데이터·publisher·Engine·
Server/Client·harness를 함께 바꾸는 수직 슬라이스다.

---

## 10. 공통 Creator Workbench 설계

### 10.1 화면 구조

F1 하나로 여는 dockspace 안에서 다음 공통 영역을 제공한다. 새 전역 기능키는 만들지 않는다.

```text
Top bar       : scenario / document / dirty / validate / publish / runtime mode / capture
Left          : catalog, filter, tags, favorites, validation badges
Center        : viewport, gizmo, overlays, playback, before/after
Right         : selected object inspector, references, derived values, error repair
Bottom        : timeline/curve/log/profiler/transaction history/publisher output
```

한 시점에 하나의 tool runtime을 활성화하는 현재 수명 계약은 유지하되, 같은 tool 안의 패널을
ImGui docking으로 조합한다. 여러 tool이 같은 singleton이나 document를 서로 몰래 수정하게 하지
않는다.

### 10.2 공통 문서 세션

공통 세션은 다음 상태를 명시적으로 가진다.

- `Closed / Loading / Clean / Dirty / Validating / Publishing / Conflict / Failed`.
- 마지막 성공 원본 hash와 disk revision.
- working copy와 마지막 committed snapshot.
- validation issue의 stable issue code, severity, JSON pointer 또는 stable object ID.
- undo/redo transaction stack. selection, hover, panel 크기는 transaction이 아니다.
- save 전에 temp 파일을 쓰고 parse/validate한 뒤 atomic replace한다.
- 외부 변경이 감지되면 자동 덮어쓰지 않고 reload/compare/cancel을 선택하게 한다.

### 10.3 공통 public 계약의 의미

| 계약 | 소유 | 소유하지 않는 것 |
|---|---|---|
| Editor document session | working copy, dirty/revision/error/transaction | 실제 Server authority, raw socket, scene transition |
| Editor transaction | before/after 또는 reversible command, stable target ID | pointer/vector index, per-frame hover state |
| Validation report | issue code, path/ID, message, repair hint | 자동 silent fallback |
| Preview bridge | staged preview object 수명과 rollback | product runtime의 별도 두 번째 asset decoder |
| Publish bridge | 기존 PowerShell publisher 실행과 결과 구조화 | schema를 우회한 직접 bootstrap 편집 |

### 10.4 후보 파일과 등록 계약

다음은 slice 착수 시 확정할 후보이며 이 마스터 문서만으로 생성하지 않는다.

| 후보 | 책임 | 연결 |
|---|---|---|
| `Engine/Public/EditorTransaction.h`, `Engine/Private/EditorTransaction.cpp` | 범용 reversible transaction과 bounded history | Map/Effect/Balance/AI 문서 세션이 사용 |
| `Client/Public/CreatorWorkbench.h`, `Client/Private/CreatorWorkbench.cpp` | F1 dockspace, tool lifecycle, 공통 status/capture panel | `CMainApp` scenario metadata와 연결 |
| `Client/Public/EditorDocumentSession.h`, `Client/Private/EditorDocumentSession.cpp` | Client JSON authoring session과 publisher 결과 | 각 툴의 schema adapter와 연결 |

새 파일은 `Engine/Default/Engine.vcxproj(.filters)` 또는
`Client/Default/Client.vcxproj(.filters)`의 실제 물리 폴더 필터에 등록한다. Server runtime 파일이
추가되는 slice는 `Server/Default/Server.vcxproj(.filters)`까지 같은 변경에서 등록하고 Debug/Release
빌드로 누락을 검증한다.

---

## 11. Balance Tool 상세 설계

### 11.1 본질

Balance Tool은 숫자 입력 폼이 아니다. **한 수치가 어떤 참조를 거쳐 어떤 Server 판정과 HUD 결과로
나가는지 보여 주고, 잘못된 조합을 publish 전에 막으며, 변경 전후의 플레이 결과를 재현하는
도구**다.

현재 정본은 다음 네 파일이다.

- `Data/Balance/PlayerProfiles.json`: class별 HP/resource/move speed.
- `Data/Balance/PlayerSkills.json`: stable skill ID, action/timing/cost/range/damage profile 참조.
- `Data/Balance/DamageProfiles.json`: stable damage profile과 amount.
- `Data/Balance/BossProfiles.json`: boss archetype/encounter 참조와 기본 수치.

Valtan pattern timeline은 `Data/Encounters/Valtan/ValtanEncounter.json`이 소유한다. Tool은 이 경계를
한 화면에서 cross-reference하되 파일 소유권을 합쳐 버리지 않는다.

### 11.2 제작자 화면

- Catalog: Players / Skills / Damage / Boss / Encounter 탭, stable ID 검색, 참조 오류 badge.
- Matrix: class별 skill slot, cooldown, hit time, cost, damage, range를 나란히 비교.
- Inspector: 원본 필드, 단위(ms/s/unit/%), 허용 범위, reference target, 파생값.
- Derived panel: DPS를 정답처럼 단순 표시하지 않고 cooldown 기준 이론값, action lock 기준값,
  resource 제한값을 각각 분리한다.
- Timeline: action duration, hit time, 이동 구간, Valtan telegraph/active/recovery를 같은 시간축에 표시.
- Scenario: training ground의 deterministic seed와 입력 script로 before/after 결과를 비교.
- Diff: 값, 참조, schema revision 변화와 예상 영향을 commit 전 확인.

### 11.3 데이터 흐름

```text
JSON working copy
 -> schema/type/range/duplicate/reference validation
 -> existing Publish-GameplayBalance.ps1 staged publish
 -> Server Gameplay.bootstrap parse
 -> contract-test isolated Server room
 -> result artifact(accepted revision, damage/hp/resource/action timeline)
 -> Tool comparison and capture
```

Tool이 damage를 계산해 실제 결과처럼 보여 주지 않는다. 빠른 예상치는 `estimate`로 표시하고,
완료 판단은 Server contract 결과로 한다.

### 11.4 Hot Reload는 2단계

첫 slice는 `편집 -> validate/publish -> Server 재시작 -> training smoke`를 안전하게 만든다. runtime
Hot Reload는 다음 계약이 한 번에 닫힐 때만 연다.

1. `balanceRevision`과 content hash.
2. Server가 새 catalog를 별도 stage하고 room tick 경계에서 atomic swap.
3. 진행 중 action이 시작 revision을 끝까지 pin할지 취소할지 명시.
4. HP/resource 최대값 변경 시 current value 정책.
5. snapshot에 revision 포함, Client가 같은 revision presentation을 commit.
6. invalid/duplicate/missing reference/중간 실패/network 지연 rollback harness.

Client만 reload하거나 Tool이 Server memory를 직접 찌르는 방식은 금지한다.

### 11.5 완료 게이트

- 정상 edit/save/publish/restart/contract-test.
- 잘못된 version/ID/range/reference/duplicate에서 원본 보존.
- Lance Master `34060/34100`과 Valtan swing의 action/hit/damage timeline 추적.
- Debug/Release publisher/harness, JSON parse, ProjectAudit.
- 2~3분 영상: 변경 전 -> validation error -> 수정 -> Server 결과 -> HUD 결과.

---

## 12. AI Tool / Encounter Tool 상세 설계

### 12.1 본질과 현재 권위

AI Tool의 본질은 node를 예쁘게 연결하는 것이 아니다. **서버가 고정 tick에서 실행할 결정 구조를
데이터로 만들고, 어떤 조건과 blackboard 값 때문에 어떤 action을 선택했는지 재현 가능하게
설명하는 것**이다.

제품 AI의 권위는 Server다. 현재 `CValtanBrain`은 30 Hz fixed update에서 target 획득, nav path,
chase, pattern windup/active/recovery, damage를 판정한다. Client `CValtan` 로컬 AI는 Development
preview 외에 제품 정답이 아니다.

### 12.2 1차 목표: Valtan Encounter 편집기

범용 Behavior Tree runtime부터 만들지 않는다. 먼저 이미 소비자가 있는 Valtan 계약을 완성한다.

- State graph: stable state ID, action ID, transition, terminal state.
- Pattern table/timeline: range, telegraph, active, recovery, damage profile.
- Cross reference: BossProfile, DamageProfile, animation presentation action, effect/audio cue.
- Server trace: tick, boss state, target entity ID, distance, path status, selected pattern,
  transition reason, applied damage revision.
- Isolated simulation: training/contract room에서 pause, step, breakpoint. 실제 multiplayer room의
  authority를 임의로 pause하지 않는다.

현재 JSON state 목록과 C++ `CValtanBrain` 로직 사이의 중복을 드러내고, 어느 값이 데이터이며 어느
규칙이 코드인지 표시한다. Tool만 데이터 기반인데 runtime은 hard-code인 상태를 완료로 세지 않는다.

### 12.3 2차 목표: 검증 가능한 behavior graph

실제 두 번째 AI archetype 또는 Valtan의 여러 pattern 요구가 생긴 뒤 다음을 추가한다.

- typed blackboard key와 기본값/owner.
- selector, sequence, condition/decorator, wait, move-to, face-target, choose-pattern,
  start-action 같은 최소 node set.
- stable node ID와 asset version. pointer/vector index 저장 금지.
- compile 단계에서 cycle 정책, unreachable node, missing port/key/action/profile,
  non-terminal path, 무한 즉시 반복을 검출.
- immutable compiled graph를 Server room이 소유하고 tick 도중 교체하지 않는다.
- 실행 trace는 bounded ring buffer이며 production snapshot bandwidth를 늘리지 않는다.

새 `CMonster` placeholder, 빈 monster catalog, Client mock AI는 만들지 않는다. 실제 archetype, world
placement, server state, client presentation, protocol snapshot, rollback harness가 있는 수직 슬라이스가
승인될 때만 generic monster 범위를 연다.

### 12.4 실패와 완료

- 없는 state/action/damage/nav area, 중복 node ID, 끊긴 edge를 publish 거부.
- Server graph load 중간 실패 시 기존 graph 유지.
- 같은 seed/input/tick에서 같은 trace를 생성하는 replay harness.
- chase 중 target death/disconnect, no path, phase transition, action 중 reload 정책 검증.
- 영상에는 graph highlight만이 아니라 Server tick trace와 Client presentation의 대응을 함께 표시.

---

## 13. Effect Tool 상세 설계

### 13.1 현재 강점과 남은 문제

현재 Effect Tool/runtime은 레퍼런스보다 타입과 schema가 빈약하지 않다. 문제는 제작자가 많은
필드를 이해하고 결과의 원인을 찾고 gameplay action과 연결하는 흐름이 아직 분산돼 있다는 점이다.

### 13.2 개선 정보 구조

- **Emitter stack**: Sprite/Mesh/Beam/Ribbon/AnimTrail을 하나의 effect asset 안에서 계층적으로 조합.
- **Module groups**: Spawn, Lifetime, Location, Velocity, Size, Color, Rotation, Material,
  SubUV, Collision, Event, LOD를 일정한 순서로 표시.
- **Timeline/curve**: lifetime 정규화 축과 실제 초 축 전환, key tangent, multi-curve overlay,
  value range/units, selected emitter solo/mute.
- **Material preview**: base/noise/mask/emissive/dissolve input, UV channel, blend/sort,
  soft-particle, distortion, bloom contribution을 실제 render pass와 연결.
- **Attachment preview**: character/socket/action ID를 선택해 origin, local/world simulation,
  follow/linger 정책을 미리 본다.
- **Deterministic scrub**: random seed, warm-up time, fixed step, restart marker로 같은 frame 재현.
- **Budget panel**: live/peak particle, vertex/index, draw/instanced draw, texture bytes,
  overdraw heatmap, distortion/bloom RT cost, LOD 선택 이유.

### 13.3 runtime 연결

```text
Server-approved actionId + actionStartTick
 -> Client presentation callback
 -> stable effectCueId
 -> preloaded effect asset
 -> pooled runtime instance
 -> Renderer pass / profiler / RT debug
```

Tool이 skill damage나 hit를 발생시키지 않는다. gameplay event는 Server snapshot에서 오며 Effect는
presentation만 담당한다. runtime이 매 프레임 `.weffect`를 읽지 않고 level/scenario stage에서 parse와
resource preload를 끝낸다.

### 13.4 transaction과 실패

- curve key 이동, module add/remove, material change를 하나의 의미 단위 transaction으로 기록.
- missing texture/model/material/socket, invalid lifetime/key order, recursive event spawn,
  LOD gap, excessive spawn을 validation issue로 표시.
- preview object와 GPU resource 생성 중 실패하면 해당 stage를 전부 해제하고 이전 preview 유지.
- product runtime과 Tool preview가 같은 decoder와 `CModel -> CMaterial` 경로를 사용한다.

### 13.5 완료 영상

한 skill cue를 빈 상태에서 제작하고 character socket에 붙인 뒤, Server-approved action과 runtime에서
같은 결과가 나오는 과정, RT/overdraw/budget 최적화 전후를 자막으로 설명한다.

---

## 14. MapTool 상세 설계

### 14.1 네 정본을 섞지 않는다

| 책임 | 정본 |
|---|---|
| 시각 배치와 asset catalog | `Data/Maps/Authoring/<AreaId>/` |
| playerSpawn/NPC/boss gameplay placement | `Data/Worlds/<AreaId>/Gameplay.world.json` |
| Server navigation/walkability/destruction | `Data/Navigation/<AreaId>.navgrid.json` |
| sequence/camera shot | 별도 `Data/Cinematics` 계약(해당 slice에서 schema 확정) |

MapTool은 한 viewport에서 네 layer를 함께 볼 수 있지만 저장 파일과 publisher를 합치지 않는다.
`mapLoadBounds`와 `MAP_LOAD_SCOPE`는 loader와 placement runtime이 계속 동일하게 소비한다.

### 14.2 제작자 핵심 UX

- asset search: path만이 아니라 thumbnail, type, tags, material/shading, bounds, batch eligibility.
- outliner: stable placement ID, group/layer, filter, visibility, lock, validation badge.
- viewport: translate/rotate/scale gizmo, local/world, grid/angle/surface snap, focus/isolate.
- selection: click/box/multi-select, parent group은 editor organization만 담당하고 저장 ID를 바꾸지 않음.
- inspector: quaternion과 Euler 표현을 분리하고 저장은 quaternion, signed scale/mirror pass 경고.
- transaction: create/delete/transform/multi-edit/scatter를 의미 단위로 undo/redo.
- dirty diff: 추가/삭제/변경 placement 수와 ID를 save 전 표시.

### 14.3 Instancing/scatter의 올바른 경계

레퍼런스의 Clone/Rect 기능은 다음 recipe로 승격한다.

- stable recipe ID, source asset ID, seed, count/density, region, rotation/scale distribution.
- preview는 transient이며 Commit 시 각 placement에 stable placement ID를 부여.
- surface/nav/collision exclusion과 minimum separation.
- alpha/additive asset은 deferred static batch 대상이 아님을 inspector에서 설명.
- 대량 변경 중 실패하면 일부 placement를 남기지 않고 rollback.

runtime instancing은 이미 있는 `CMapStaticBatchObject`를 확장한다. editor의 clone list를 별도의
두 번째 renderer로 그리지 않는다.

### 14.4 Collider

collider 화면은 목적을 두 가지로 분리한다.

1. **Client visual physics/secondary motion collision**: PhysX slice의 shape profile.
2. **Server gameplay/navigation**: navgrid, destruction region, runtime blocker, encounter range.

Client PhysX collider를 Server damage/nav 정답으로 사용하지 않는다. shape는 stable shape ID,
parent bone/placement ID, type, local transform, collision layer를 저장하고 negative/zero size,
missing parent, unsupported dynamic triangle mesh를 거부한다.

### 14.5 Animation object

맵의 움직이는 장식물은 stable presentation profile로 animation clip, loop, initial time, playback
policy를 참조한다. Tool의 play/stop/frame은 preview 제어일 뿐이다. 문, 파괴물, boss gimmick처럼
gameplay 의미가 있으면 Server state가 snapshot action을 소유하고 Client가 presentation한다.

### 14.6 공간 분할 UI

제작자가 수만 placement에 quadtree index를 손으로 넣지 않는다.

- offline builder가 placement bounds로 spatial cell을 생성.
- Tool은 cell bounds/depth/occupancy/estimated draw/bytes를 색으로 표시.
- selected cell 강제 load/unload, camera path에서 churn 측정, outlier placement 탐색.
- 수동 override가 필요하면 stable region ID와 이유를 저장하고 자동 builder 결과 위에 적용.

Bern처럼 높이와 실내가 있는 맵에서는 2D quadtree가 최선이라고 가정하지 않는다. uniform grid,
loose quadtree, BVH/3D cell을 동일 데이터셋으로 benchmark해 build time, traversal CPU, cell churn,
memory, false positive를 비교한 뒤 선택한다.

---

## 15. Render Target / Render Pass Debug Tool

### 15.1 본질

최종 장면이 어두운 이유가 material인지 light인지 AO인지 fog인지 tone map인지 알아낼 수 없다면
고급 렌더링을 늘릴수록 디버깅 비용만 커진다. 그러므로 Render Debug는 PBR/SSAO보다 먼저다.

### 15.2 Engine registry

Renderer와 각 pass는 생성한 target을 다음 metadata와 함께 registry에 등록한다.

- stable target/pass ID와 표시명.
- producer pass와 읽는 pass.
- size, format, mip/slice/sample count.
- semantic: color/albedo/normal/depth/object ID/velocity/AO/shadow/emissive/distortion.
- color space와 visualization rule(linear/sRGB, normal remap, depth linearization, integer ID palette).
- transient/persistent lifetime과 frame revision.
- GPU timer ID와 주요 resource hazard 상태.

Engine registry는 ImGui를 알지 않는다. Client `RenderDebugTool`이 read-only snapshot을 소비한다.

### 15.3 화면 기능

- grid, stage filter, dependency order, fullscreen, pin, A/B freeze.
- RGB/A channel mask, exposure, range clamp, false color, mip/slice 선택.
- depth linearize와 near/far 표시, normal 방향 remap, motion vector scale.
- pixel probe: screen coordinate의 각 target 값과 최종 color 변환 단계를 나란히 표시.
- histogram, luminance percentile, NaN/Inf/negative/out-of-gamut heatmap.
- selected object ID highlight, bounding box, draw/material/placement stable ID 역추적.
- pass별 GPU ms, dimensions, bandwidth 추정, draw/dispatch 수.
- 명시적 capture 때만 staging readback/export. 매 프레임 GPU->CPU 복사를 하지 않는다.

현재 `Target_Diffuse/Normal/Shade/Depth/Specular/PickPos/Emissive/LightDepth/SceneHDR/
Distortion/Bloom*`를 첫 registry 대상으로 삼는다. raw depth나 `R32_UINT` object ID를 ImGui shader에
그대로 넘기지 않고 visualization texture를 거친다.

### 15.4 후보 파일

| 후보 | 책임 |
|---|---|
| `Engine/Public/RenderDebugRegistry.h`, `Engine/Private/RenderDebugRegistry.cpp` | target/pass metadata와 frame snapshot |
| `Client/Public/RenderDebugTool.h`, `Client/Private/RenderDebugTool.cpp` | ImGui viewer, probe, capture, profiler 연계 |

Engine/Client project와 filters 등록, Debug에서 tool 노출, Release에서는 runtime overhead가 0 또는
명시적 profiling flag일 때만 제한적으로 켜지는지를 검증한다.

---

## 16. 가시성 최적화: Frustum, Distance, LOD, Occlusion

### 16.1 각 기술의 역할

| 기술 | 질문 | 현재 | 주의점 |
|---|---|---|---|
| Frustum culling | 카메라 절두체 밖인가? | instance마다 bounding sphere 검사 | O(N) traversal 자체가 커질 수 있음 |
| Spatial coarse culling | 어떤 cell/node만 검사할까? | 없음 | tree를 만드는 것보다 데이터 분포와 churn이 중요 |
| Distance culling | 너무 멀거나 화면 기여도가 작은가? | 없음 | 전역 고정 거리보다 asset/profile+projected size+hysteresis |
| LOD | 보여야 한다면 어느 geometry/material 품질인가? | 없음 | pop, shadow LOD, animation LOD, memory와 연동 |
| Occlusion culling | 다른 opaque geometry에 가려졌는가? | 없음 | 결과 지연, query stall, camera teleport, false occlusion |
| HLOD/streaming | 먼 cell을 proxy로 합치거나 아예 안 올릴까? | 없음 | offline build, asset packaging, load budget까지 필요 |

### 16.2 적용 순서

1. Bern/Valtan/Training 고정 camera path로 baseline JSON capture.
2. spatial cell에서 coarse frustum, cell 내부 instance frustum.
3. distance/profile과 projected screen size, enter/exit hysteresis.
4. mesh/material LOD와 dither/crossfade 정책.
5. occlusion은 coarse cell/cluster부터 실험.
6. 여전히 CPU/GPU/memory 병목이 증명될 때 HLOD와 streaming.

Instancing은 draw call을 줄이지만 보이지 않는 instance의 CPU traversal과 vertex cost까지 자동으로
없애지는 않는다. 반대로 culling을 지나치게 잘게 하면 upload range와 batch가 깨진다. visible count만
보지 않고 draw count, instance upload bytes, traversal CPU, vertex/pixel GPU를 함께 측정한다.

### 16.3 Distance와 LOD 계약

- catalog의 asset ID가 visibility profile/LOD set을 참조한다.
- persisted ID는 asset/placement/LOD profile stable ID이며 runtime vector index가 아니다.
- 선택 기준은 거리만이 아니라 projected radius와 authoring bias를 포함한다.
- LOD 전환에는 hysteresis, 선택적으로 dither fade, shadow 전용 bias를 둔다.
- animated character는 bone update/animation sampling LOD를 별도 gate로 둔다.
- selected editor object, gameplay-critical telegraph, boss는 강제 visibility 정책을 명시한다.
- missing LOD는 마지막 유효 LOD로 조용히 fallback하지 않고 validation warning과 counter를 남긴다.

### 16.4 Occlusion 후보

per-object D3D11 hardware occlusion query를 수만 개 발행하고 같은 frame에 기다리는 방식은 CPU/GPU
동기화 위험이 크므로 기본안이 아니다. 두 후보를 실측한다.

1. **Coarse hardware query**: 큰 static cell/cluster만 query하고 이전 frame 결과를 사용.
2. **Hi-Z**: opaque depth pyramid를 만들고 projected bounds를 compute/CPU-assisted test.

둘 다 한 frame 지연과 보수적 확장, camera teleport 시 일시 disable, 최소 visible frame, hysteresis를
가진다. transparent/additive object는 occluder로 사용하지 않고, 작은 occluder/동적 occluder 정책을
분리한다. query stall, false negative, culled reason별 counter를 Render Debug에 노출한다.

### 16.5 완료 예산

고정 camera path마다 다음을 기록한다.

- total / cell rejected / frustum rejected / distance rejected / occlusion rejected / rendered.
- traversal CPU ms, culling dispatch/query ms, instance upload bytes.
- draw/instanced draw, vertex/index/instance, opaque/alpha pass GPU ms.
- camera 이동 시 visible set churn, LOD transition count, pop regression capture.

최적화 기능은 숫자가 좋아도 물체가 사라지거나 pop이 눈에 띄면 실패다. 성능 artifact와 visual
regression 영상을 같은 RESULT에 둔다.

---

## 17. 렌더링: PBR·SSAO·외곽선·환경 안개·렌더 타깃

### 17.1 먼저 바로잡을 현재 상태

2026-08-01 격차 문서의 일부는 이미 지나갔다. 현재 Renderer에는 FP16 Scene HDR, distortion,
half-resolution bloom, tone map/gamma가 있다. 그러나 PBR/IBL, SSAO, outline, fog는 아직 없고,
RT debug는 LightDepth 외에 제작자가 쓰기 어렵다.

### 17.2 PBR의 본질

PBR은 셰이더 함수 하나가 아니다. 다음 전체 계약이 맞아야 한다.

- linear/sRGB texture semantics.
- normal/tangent basis와 handedness.
- base color, metallic/specular, roughness, AO, emissive channel 계약.
- energy-conserving BRDF, direct light, IBL irradiance/prefiltered specular/BRDF LUT.
- material fallback과 legacy/stylized asset migration.
- HDR exposure/tone map 아래의 결과 검증.

Lost Ark 원작 감성은 사실적 PBR 하나로 끝나지 않는다. material에 명시적 `shadingModel`을 두어
PBR과 stylized/toon/environment response를 같은 renderer 안에서 선택하게 하고, 두 번째 model
runtime을 만들지 않는다.

### 17.3 권장 pass 순서

```text
Shadow
 -> G-buffer(material/shading/object data)
 -> SSAO(raw + bilateral denoise)
 -> Deferred direct light + IBL
 -> opaque scene HDR
 -> outline / selection composite
 -> transparent/effect + distortion
 -> environment distance/height fog
 -> bloom
 -> tone map / gamma / optional color grade
 -> UI / Render Debug overlay
```

실제 outline/fog 위치는 effect와 transparency 요구를 sample scene에서 비교한 뒤 확정한다.

### 17.4 SSAO

- depth와 normal을 사용하며 half resolution을 1차 목표로 한다.
- projection-space radius, thickness/bias, intensity, distance fade.
- edge-preserving bilateral blur/upsample.
- raw AO, blurred AO, final light contribution을 각각 RT registry에 공개.
- halo, thin geometry, sky/background, temporal flicker를 Bern/Valtan에서 검증.
- SSAO는 ambient/IBL 항만 감쇠하고 직접 조명을 무조건 검게 만들지 않는다.

### 17.5 외곽선과 선택

용도를 분리한다.

- gameplay/stylized outline: depth/normal/object mask edge, 거리별 두께와 색 profile.
- editor selection: stable object/placement ID mask, hover/selected colors, occluded outline 정책.

`PickPos`만으로 객체 identity를 추측하지 않는다. 필요한 경우 integer object ID target과 frame-local
ID -> stable ID lookup을 추가하되 저장 ID에는 frame index를 쓰지 않는다.

### 17.6 환경 안개

첫 단계는 저비용 distance + height fog다.

- area environment profile이 color, density, start/end, height falloff, sun scattering bias를 소유.
- depth reconstruction을 사용하고 sky/background와 transparent/effect 적용 순서를 명시.
- Bern/Valtan의 분위기를 살리되 telegraph와 player readability를 가리지 않는 upper bound.
- volumetric fog는 froxel memory, temporal reprojection, light injection, shadow cost가 필요하므로 C컷.

### 17.7 Light/Shadow

- 현재 Add/Render 위주의 Light Manager를 stable light ID, edit/remove, area profile, validation으로 확장.
- directional/point 범위와 shadow casting budget을 구분.
- single shadow map 개선 후 필요성이 확인되면 3~4 cascade CSM, stable split, texel snap, PCF.
- light count가 실제 병목일 때만 tiled/clustered를 연다.

### 17.8 완료 기준

- Debug/Release product scenario에서 기본 경로 ON.
- PBR material sphere만이 아니라 Bern/Valtan 실제 asset과 character/effect 회귀.
- pass별 RT, pixel probe, GPU ms, before/after, invalid material/channel diagnostic.
- 색공간·AO·outline·fog를 설명하는 게임 내 자막과 기술 문서.

---

## 18. Physics, solver, rigid body, 뼈 branch secondary motion

### 18.1 질문에 대한 정확한 답

PhysX를 기준으로 보면 **rigid body와 constraint solver는 라이브러리 안에 존재한다.**
`PxRigidDynamic`은 동적 rigid actor이고, 질량/관성/속도/힘과 충돌 shape를 가진다. joint는 두 actor의
constraint frame 관계를 dynamics solver가 만족시키며, `PxSphericalJoint`는 두 joint origin을
붙이고 회전은 허용하며 cone limit을 줄 수 있다.

그러나 현재 LostArk 저장소에는 PhysX SDK, `PxScene`, `PxRigidDynamic`, joint, solver가 없다.
현재 Engine collider는 Sphere/AABB/OBB overlap과 debug draw 수준이다. 따라서 기존 Physics Manager
안의 기능을 연결하는 작업이 아니라 dependency, scene, lifecycle, animation pose bridge까지 새로
도입하는 작업이다.

또한 “각 뼈를 rigid dynamic으로 만들고 spherical joint로 연결”한 것은 엄밀히 말해 full cloth
solver가 아니라 **jointed rigid-body skeletal secondary motion**이다. 치마, 코트 자락, 머리카락용
보조 뼈가 메시 skinning을 움직이게 하는 방식이다. 삼각형 천의 주름과 self-collision을 푸는 cloth
simulation과는 다른 문제다.

### 18.2 rigid chain과 articulation 선택

발표 방식은 다음 maximal-coordinate chain이다.

```text
animation-driven kinematic root
 -> PxRigidDynamic child bone
 -> PxSphericalJoint / PxD6Joint
 -> PxRigidDynamic child bone ...
```

이는 bone별 pose를 읽고 쓰기 쉬워 character animation과 섞기 직관적이다. 다만 긴 chain은 joint
drift, mass ratio, solver iteration, jitter를 튜닝해야 한다.

PhysX reduced-coordinate articulation은 tree 구조와 ragdoll에 적합하고 joint separation이 적지만,
child link pose를 직접 kinematic target으로 밀 수 없고 root/joint coordinate로 제어해야 한다.
따라서 첫 slice에서 3~6 bone chain으로 두 방식을 benchmark하고 다음 기준으로 선택한다.

- animation root follow 안정성.
- 30/60/120 fps와 fixed substep 결과.
- teleport/reset와 blend-in/out.
- CPU cost per character/branch/DOF.
- joint drift/jitter, mass ratio, collision response.
- authoring 난이도와 debug 가능성.

### 18.3 올바른 animation-physics 순서

```text
1. Animation sample / crossfade / root motion
2. Local bone pose 계산
3. Secondary-motion branch root의 animated world target 계산
4. kinematic/root target push
5. fixed-step PhysX simulate (bounded substeps)
6. fetch results
7. dynamic child actor world pose read
8. parent 역행렬로 local bone override 변환
9. animation pose와 weight blend + joint limit correction
10. final combined bone matrices 재계산
11. skinning / render
```

현재 `CBone`은 transform과 parent index만 소유하므로, `CModel`에 base animation pose와 external pose
modifier를 구분하는 단계를 먼저 만들어야 한다. actor world matrix를 bone local matrix에 그대로
넣으면 double transform, 축 뒤집힘, 부모 갱신 순서 오류가 생긴다.

### 18.4 Authoring profile

후보 정본은 `Data/Physics/SecondaryMotion/<ProfileId>.secondary-motion.json`이며 정확한 schema는
slice에서 확정한다.

- stable profile ID, skeleton asset ID, stable bone name/hash.
- branch root와 포함/제외 bone 규칙. runtime vector index 저장 금지.
- bone별 shape type/radius/length/local pose, mass/density/inertia scale.
- joint type/frame, swing/twist limit, stiffness/damping, max force.
- gravity/wind/drag, collision layer, self-collision policy.
- solver position/velocity iterations, fixed step, max substeps.
- teleport threshold, sleep/wake, reset pose, blend weight/duration.
- distance/visibility/character LOD에서 simulation disable 또는 저주기 정책.

Animation Tool의 Skeleton 패널에서 branch를 선택·미리보기하고, 별도 Physics 패널에서 shape/joint
gizmo, limit cone, center of mass, velocity, contacts, solver error를 표시한다. MapTool collider와 같은
shape renderer를 재사용하되 data owner는 분리한다.

### 18.5 Physics Manager 수명

Engine의 범용 manager가 PhysX foundation/physics/dispatcher/scene/material, fixed-step accumulator,
actor/joint 등록, simulate/fetch, level teardown을 소유한다. `CCharacter`는 SDK singleton을 직접
호출하지 않고 secondary-motion component/service에 profile과 model instance를 등록한다.

- create는 profile parse/validate 후 모든 actor/joint를 stage하고 마지막에 scene commit.
- 중간 실패 시 actor/joint/shape를 역순 release.
- level transition, disconnect, character despawn, model swap에서 bounded teardown.
- teleport/animation discontinuity에서 velocity zero와 animated pose reset.
- PhysX simulation은 처음에는 Client presentation-only. player 이동/damage/boss authority가 아니다.

### 18.6 후보 파일과 dependency gate

| 후보 | 책임 |
|---|---|
| `Engine/Public/PhysicsManager.h`, `Engine/Private/PhysicsManager.cpp` | SDK/scene/fixed-step/lifecycle |
| `Engine/Public/SecondaryMotionRig.h`, `Engine/Private/SecondaryMotionRig.cpp` | bone branch와 actor/joint/pose bridge |
| `Client/Public/SecondaryMotionToolPanel.h`, `Client/Private/SecondaryMotionToolPanel.cpp` | skeleton selection, shape/joint gizmo, preview/debug |
| `Tools/PhysicsPipeline/Publish-SecondaryMotion.ps1` | schema/reference 검증과 runtime publish |

SDK version, license/redistribution, x64 Debug/Release library, CRT, DLL packaging, PVD 사용 여부를 먼저
결정한다. Engine public header가 바뀌므로 Engine Debug/Release -> UpdateLib -> Client Debug/Release를
반드시 검증한다.

### 18.7 Physics 완료 게이트

- 1 branch 생성/삭제/저장/reload와 실패 rollback.
- missing bone, duplicate branch, invalid limit/mass/shape, actor 또는 joint 생성 중간 실패.
- idle/run/attack/crossfade, camera teleport, level reload, character despawn.
- 30/60/120 fps fixed-step 비교와 max-substep fail-safe.
- simulation OFF일 때 animated pose와 bit-for-bit에 가까운 회귀.
- character 1/4/N개에서 CPU ms, active/sleeping actor, contact/joint 수 측정.
- 영상에서 joint cone, rigid bodies, final skinned cloth bone, 성능 수치를 동시에 표시.

참고한 공식 계약:

- NVIDIA PhysX Rigid Body Dynamics: <https://nvidia-omniverse.github.io/PhysX/physx/5.4.0/docs/RigidBodyDynamics.html>
- NVIDIA PhysX Joints: <https://nvidia-omniverse.github.io/PhysX/physx/5.4.0/docs/Joints.html>
- NVIDIA PhysX Articulations: <https://nvidia-omniverse.github.io/PhysX/physx/5.4.1/docs/Articulations.html>

---

## 19. Camera Tool과 Sequencer

### 19.1 데이터와 역할

- stable sequence ID, shot ID, track ID, key ID.
- Camera track: transform, look-at target, FOV, near/far, easing, cut/blend.
- Animation/Effect/Audio track: stable presentation cue 참조.
- Event track: presentation event만 직접 실행. gameplay 변경은 Server-approved action을 기다린다.
- camera dummy/collider는 preview helper이며 저장 identity가 아니다.

### 19.2 제작자 흐름

record current camera -> key 생성 -> timeline에서 duration/easing 수정 -> scrub -> shot별 play ->
sequence play -> deterministic capture preset. MapTool camera mode는 placement context를 제공하고,
sequence document와 player는 별도 owner로 둔다.

### 19.3 영상 제작 연결

각 feature RESULT가 capture preset을 남긴다.

- scenario ID, level/area, camera sequence, resolution, warm-up, duration.
- profiler capture start/end marker.
- subtitle cue: 문제, 입력, 알고리즘, 결과, ms/개수, 실패 경계.
- before/after A/B가 같은 camera path와 seed를 사용.

마스터 영상은 마지막에 새로 연출하는 것이 아니라 각 완료 slice의 검증 clip을 묶어 만든다.

---

## 20. 통합 데이터·권위·스레드 계약

### 20.1 데이터 흐름

```text
Git Data authoring JSON / asset catalog
 -> Tool working copy
 -> schema + semantic + cross-reference validation
 -> temp/stage
 -> existing/new publisher
 -> runtime DataFiles / asset manifest
 -> level or server staged load
 -> atomic commit
 -> profiler/trace/capture artifact
```

`Client/Bin/Resources` payload, build/intermediate, generated bootstrap은 소스 커밋에 섞지 않는다.
Git 관리 `Data` 원본은 Client project의 `96.DataFiles` 아래 `None`으로만 노출한다.

### 20.2 authority 표

| 도메인 | authoring owner | runtime authority | Client tool/preview의 역할 |
|---|---|---|---|
| Balance | `Data/Balance` | Server catalog/GameRoom | 편집, publish, isolated 결과 비교 |
| AI/Encounter | `Data/Encounters` + Server behavior | Server 30 Hz | graph/trace 표시, isolated preview |
| Map visual | `Data/Maps/Authoring` | Client level/runtime placement | 실제 CModel/CMaterial staged preview |
| Spawn/Boss | `Data/Worlds` | Server bootstrap/world | placement 편집과 validation |
| Navigation | `Data/Navigation` | Server navigation | bake/paint/diagnostic, publish |
| Effect | effect asset/cue data | Client presentation, trigger는 Server state | deterministic preview와 budget |
| Camera | cinematic data | Client presentation | record/scrub/capture |
| Secondary motion | physics profile | Client presentation PhysX scene | branch authoring/debug |
| Rendering/visibility | material/area/profile data | Client Renderer | pass/RT read-only debug와 profile edit |

### 20.3 스레드와 수명

- ImGui는 command/selection만 만들고 긴 parse, bake, publish를 매 frame 수행하지 않는다.
- worker 결과는 stage object로 main thread에 전달하고 main에서 scene/GPU/PhysX commit.
- cancellation token, bounded join, progress/phase, timeout error를 공통 job panel에 노출.
- DeviceContext, ImGui, Scene mutation, PhysX actor add/remove의 thread-affinity를 문서화.
- tool close/level change/process shutdown에서 preview/job/readback을 먼저 취소하고 역순 teardown.

---

## 21. 구현 순서와 slice 게이트

각 항목은 착수 전에 `.md/GB/<MM-DD>/YYYY-MM-DD_<TOPIC>_PLAN.md`를 따로 작성하고, H 계약,
CPP 흐름, 파일별 역할, 실패/rollback, vcxproj/filters, harness를 확정한다.

| 순서 | Slice | 선행 조건 | 종료 증거 |
|---:|---|---|---|
| 0 | 현재 camera path/profiler/RT 기준선 고정 | 없음 | Bern/Valtan/Training capture artifact |
| 1 | Creator Workbench + EditorTransaction + document session | 0 | MapTool 한 편집 동작의 undo/redo/save conflict harness |
| 2 | RenderDebugRegistry + RenderDebugTool | 1 | 현재 모든 RT/pass grid, pixel probe, GPU ms |
| 3 | Balance Tool v1(manual publish/restart) | 1 | invalid/valid publish와 Server contract 결과 |
| 4 | Camera Sequence + deterministic capture | 1~2 | 동일 path/seed A/B capture |
| 5 | MapTool UX(gizmo/multi/scatter/diff) | 1 | Bern 대량 edit rollback와 runtime reload |
| 6 | Effect Tool UX/curve/cue/budget | 1~2 | skill cue 실제 Server action 연계 |
| 7 | Valtan Encounter Tool + Server trace | 1, 3 | fixed-tick deterministic trace/replay |
| 8 | spatial cells + distance + culling counters | 0, 2, 5 | Bern/Valtan CPU/GPU 비교와 visual regression |
| 9 | LOD, 그 후 occlusion 실험 | 8 | reason counters, pop/stall/false-cull 증거 |
| 10 | PBR/IBL foundation | 2 | material/color-space validation과 실제 scene |
| 11 | SSAO -> outline/selection -> fog -> shadow/light | 2, 10 | pass별 RT, GPU 예산, readability 회귀 |
| 12 | CModel pose modifier foundation | AnimationTool 회귀 고정 | physics OFF animation 회귀 |
| 13 | PhysX dependency/scene + 1 secondary branch | 12 | joint/debug/skinning/failure harness |
| 14 | Balance Hot Reload 또는 generic behavior graph | 실제 요구와 소비자 확인 | revision/tick commit/rollback 또는 graph compile trace |
| 15 | 통합 마스터 영상/자막/기술소개서 | 완료 slice만 | 재현 가능한 capture manifest와 최종 자료 |

### 21.1 Slice 공통 완료 체크리스트

- [ ] 실제 호출자, owner, 데이터 정본, 실패 소비자가 연결됨.
- [ ] Debug/Release product path에서 기능이 의도대로 ON/OFF 됨.
- [ ] 정상, version/ID/path 오류, duplicate, 중간 실패 rollback 검증.
- [ ] runtime pointer/vector index가 저장 ID로 들어가지 않음.
- [ ] profiler CPU/GPU/counter와 fixed scenario before/after artifact.
- [ ] 관련 publisher/protocol/server/client harness와 ProjectAudit.
- [ ] `git diff --check`, JSON/XML parse, project/filter 등록 확인.
- [ ] 영상, 자막, 기술 문서가 RESULT의 실제 구현 상태와 일치.

---

## 22. 명시적 금지선과 후속 결정

### 22.1 하지 않을 것

- 레퍼런스 ImGui 배치를 그대로 복사하고 save/runtime 계약을 비워 두기.
- Tool 전용 model/effect/map decoder 또는 두 번째 runtime 만들기.
- UI가 socket/packet/snapshot/Server memory를 직접 조작하기.
- hot reload를 Client만 켜거나 tick/revision/rollback 없이 켜기.
- `CMonster` placeholder, 빈 catalog, Client AI로 Server 구현을 대신하기.
- quadtree cell/vector index/prototype tag/pointer를 저장 identity로 사용하기.
- per-object occlusion query를 발행하고 즉시 기다리기.
- PBR/SSAO/fog를 default OFF demo toggle로만 남기기.
- PhysX secondary motion을 combat collision/damage authority로 사용하기.
- 매 프레임 파일 읽기, GPU readback, asset decode, physics rig rebuild 수행하기.

### 22.2 각 착수 전에 확정할 선택

- PBR material channel의 실제 `.wmat/.wmodel` 호환과 migration 범위.
- SSAO algorithm과 GPU 예산, fog의 first-cut 범위.
- uniform grid/loose quadtree/BVH의 실측 선택.
- D3D11 coarse query와 Hi-Z의 실측 결과.
- PhysX exact version, binary/build 방식, redistribution, PVD, rigid chain 대 articulation.
- generic behavior graph를 열 실제 두 번째 소비자.
- Balance hot reload의 in-flight action/max HP/resource 정책.

---

## 23. 최종 산출물 정의

1. 제작자가 실제 정본을 편집하는 Balance/Encounter/Effect/Map/Camera/Physics 패널.
2. renderer pass와 visibility/physics를 원인 단위로 보여 주는 Render Debug/Profiler.
3. publisher와 Server/Client runtime을 우회하지 않는 preview/publish/reload 흐름.
4. Bern/Valtan/Training의 deterministic capture manifest와 JSON profiler artifact.
5. 기능별 짧은 검증 영상: 문제 -> 선택한 본질 -> 데이터/알고리즘 -> 실패 처리 -> 결과/수치.
6. 게임 자막: 현재 mode, selected stable ID, pass/AI/physics 상태, 성능 before/after.
7. 최종 마스터 영상과 기술소개서. 구현하지 않은 항목과 수동 검증 미완료를 별도로 표시.

이 계획의 첫 실제 구현 slice는 **Creator Workbench 공통 transaction과 Render Target Debug**를
권장한다. 이후 모든 툴·렌더링·최적화·물리 작업의 디버깅과 영상 증거 비용을 가장 크게 낮추기
때문이다.

---

## 24. 2026-08-03 실행 확정: Character Select VFX·W7·고급 렌더링 G0~G10

사용자가 실제 구현 착수를 지시했다. 이제 §21의 전체 툴 스위트 장기 순서와 별도로, 사용자가
직접 구현하고 Codex가 조사·계획·코드 검토·빌드 검증을 보조할 `Character Select 제작 테스트베드
-> Effect Tool -> 발탄 전투 연출 -> 고급 렌더링`을
다음 열한 게이트로 실행한다. 각 게이트는 별도 검증 단위이며 앞 게이트의 RESULT와 빌드 증거
없이 다음 게이트를 완료 처리하지 않는다.

이 실행 트랙에서 `LEVEL::CHARACTER_SELECT`는 socket이 없는 제품 3D 선택 Level인 동시에 Debug
빌드의 캐릭터 표현 제작 테스트베드다. 실제 `CCharacter -> CModel -> CMaterial` 경로를 사용하고,
툴 전용 Character/Model/Effect runtime을 만들지 않는다. Character Select에서 직접 누르는 재생은
제작자 preview command일 뿐 gameplay 승인으로 취급하지 않는다. 발탄 제품 경로의 스킬·피격·보스
action 이펙트는 G9에서 Server snapshot의 의미 이벤트를 받은 뒤에만 재생한다.

### 24.1 전체 변경 지도

유지할 파일과 경로:

- `Client/Private/Effect_Runtime.cpp`: 현재 Sprite/Mesh/Beam/Ribbon/AnimTrail 단일 runtime을
  계속 확장한다.
- `Engine/Private/Renderer.cpp`: 현재 G-buffer -> SceneHDR -> Bloom -> Final 경로를 두 번째
  renderer 없이 확장한다.
- `Client/Private/Level_CharacterSelect.cpp`: 실제 preview Character와 camera owner를 유지한다.
- `Data/Effects/Authored`, 후속 `Data/Effects/Cues`, immutable resource pack: authored effect,
  Client presentation cue, payload 정본 역할을 분리한 채 유지한다. Server `Data/Balance`에는
  Client effect asset 경로를 넣지 않는다.

수정할 핵심 파일:

- `Engine/Public/Engine_Struct.h`, `RenderTarget.h`, `Target_Manager.*`, `GameInstance.*`: G1의
  읽기 전용 Render Target snapshot 계약.
- `Client/Public|Private/MainApp.*`: F1 diagnostics와 툴 수명 조율만 담당한다.
- `Client/Public|Private/Effect_Tool.*`: Character Select context, material/curve/budget authoring.
- `Client/Public|Private/Effect_Types.*`, `Effect_AssetIO.*`, `Effect_Runtime.*`: G4~G6의 저장·실행 계약.
- `Client/Bin/ShaderFiles/Shader_Deferred.hlsl`, `Shader_Effect*.hlsl[i]`: HDR/bloom/distortion,
  decal, PBR/SSAO/outline/fog 패스.
- `Client/Public|Private/Level_CharacterSelect.*`: 툴이 선택한 preview target을 읽는 typed 경계만
  제공하고 Effect Tool 구현을 Level 안에 넣지 않는다.

G1에서 새로 추가할 파일:

- 없음. 현재 F1 Diagnostics/Profiler의 표시 상태와 수명은 이미 `CMainApp`이 소유하므로 첫 RT
  grid도 같은 owner의 private 함수와 멤버로 닫는다. G2에서 channel shader, readback staging,
  capture 상태처럼 독립 수명이 실제로 생길 때 `RenderDebugTool` 분리를 다시 판정한다.

후속 신규 파일은 각 게이트 PLAN에서 실제 owner와 소비자를 다시 판정한 뒤에만 추가한다.
`EffectPlaybackService`, decal projector, character preview context는 이름만 먼저 만들지 않는다.

삭제할 파일:

- 이번 G0~G10 트랙에서 선제 삭제 없음. 다른 세션의 미커밋 파일을 이 트랙이 정리하지 않는다.

### 24.2 병렬 작업선과 같은 파일 충돌 금지

```text
에셋 추출선
  -> 외부 pack/payload + immutable manifest + stable Resources-relative asset ID

툴·진단선
  -> G1 Character Select Tool Host + RT Snapshot Grid -> G2 진단 고도화

이펙트 표현선
  -> G3 document/material parity -> G4 attachment/deterministic preview -> G5 W7 전반

애니·전투 연출선
  -> G6 animation presentation event -> G8 hit response -> G9 Valtan snapshot/camera cue

월드 렌더링선
  -> G2에서 기준선 확보 -> G5와 병행해 color/HDR 개선 -> G7 PBR/IBL -> G8 shadow/SSAO/outline/fog
```

병렬은 같은 파일을 동시에 편집한다는 뜻이 아니다. 에셋 추출선은 소스 파일을 수정하지 않고
resource pack과 manifest를 만든다. `MainApp`, `Client.vcxproj`, `.filters`, `Effect_Tool`,
`Effect_Runtime`, `Renderer`, 공용 shader는 각 게이트의 한 owner만 수정한다. 다른 세션이 해당
파일을 수정 중이면 그 게이트는 계획·read-only 조사까지만 진행하고, 상대 변경이 빌드 가능한
체크포인트가 된 뒤 현재 diff 위에 합친다.

### 24.3 G0~G10 게이트

| Gate | 한 문장 목표 | 실제 소비자와 핵심 범위 | 완료 증거 |
|---:|---|---|---|
| **G0** | 동시 작업과 기준선을 고정한다 | Character Select/Level/Loader 현재 변경, 추출 asset ID, 13 RT/7 MRT, Effect HDR readback의 owner를 확정 | dirty 파일 소유 구분, Debug/Release 기준 빌드, Character Select 실제 진입, baseline capture/profiler JSON |
| **G1** | Character Select에서 제작 툴과 모든 렌더 타깃을 여는 첫 진단 경계를 닫는다 | DEBUG `RenderTargetDebugSnapshot` registry + F1 `Render Targets` grid/fullscreen, Character Select의 Effect/Animation Tool host 허용 | Character Select에서 Effect/Animation Tool open, 13개 tag/format/size/SRV, 확대/복귀, Engine -> UpdateLib -> Client Debug/Release |
| **G2** | 픽셀 원인을 진단 가능한 형태로 만든다 | snapshot에 semantic/color space/producer/frame revision, R/G/B/A, normal remap, linear depth, HDR false color, NaN/Inf, pixel probe, capture, pass GPU ms | 고정 Character Select camera에서 G-buffer/HDR/distortion/bloom 수치와 이미지 artifact, LightDepth producer 없음 표시 |
| **G3** | Effect 문서와 material 입력의 정본을 닫는다 | schema 하위 호환, embedded ID/path 일치, unique emitter/module ID, emissive texture, Base/Mask/Dissolve/Distortion, radial UV | 구버전 load, 신규 save/reload, invalid path/domain/duplicate/key order/중간 실패 시 기존 preview 유지 |
| **G4** | Character Select를 deterministic Effect 제작 Lab으로 닫는다 | 선택된 preview Character context, root/bone attachment, asset revision과 transform revision 분리, play/pause/scrub/reset | 네 class 교체·Level 이탈 weak target 정리, missing bone 시 기존 preview 유지, follow 중 rebuild/file I/O 0회 |
| **G5** | HDR 이펙트 품질·성능과 W7 전반을 고정한다 | exposure/tonemap/bloom profile, soft particle, distortion, GroundTelegraph와 ProjectedDecal 분리, Sprite GPU instancing | 1/1k/10k particle draw·GPU ms, HDR/Bloom/Distortion readback, depth intersection·z-fighting·readability |
| **G6** | 애니메이션과 이펙트 시간을 하나로 묶는다 | 기존 crossfade 소비, stable presentation event/cue, source bone/socket, deterministic event cursor | idle/attack/crossfade에서 cue 1회, seek/replay 동일, HIT/CANCEL류는 gameplay 판정에 사용하지 않음 |
| **G7** | 실제 material을 PBR/IBL 조명 아래 둔다 | linear/sRGB, G-buffer metallic/roughness/AO, GGX direct light, irradiance/prefilter/BRDF LUT | Character Select 네 class와 Bern/Valtan material sweep, legacy fallback, RT pixel·GPU ms |
| **G8** | 환경 렌더링과 피격 표현을 가독성 기준으로 닫는다 | shadow producer 실점등 후 CSM 판단, SSAO, outline, fog, rim, hit flash | LightDepth producer 증거, AO/fog/outline이 player/telegraph를 가리지 않는 A/B, hit state 종료 rollback |
| **G9** | W7을 발탄 제품 전투 경로에 연결한다 | Server-approved action/HP edge/start tick -> Client semantic cue catalog -> decal/effect/rim/hit flash/camera shake·zoom/presentation hit stop/animation | 실제 Server+Client에서 action 1건 종단 추적, duplicate snapshot 억제, network·Server tick·입력 clock 비정지, disconnect/level teardown |
| **G10** | 전체 결과를 재현 가능한 캡처와 회귀 계약으로 닫는다 | 동일 camera/seed/warm-up capture manifest, gate별 RESULT, profiler artifact, asset pack receipt | 전체 Debug/Release, 실제 Server+Client, ProjectAudit+DeepAssetHash, 최종 영상/자막·기술 문서 |

### 24.4 G0와 G1의 즉시 실행 계약

G0에서는 다른 세션이 만든 Character Select·Level·MainApp·프로젝트 변경을 먼저 빌드 가능한 한
단위로 닫는다. 이 변경을 되돌리거나 재작성하지 않는다. 에셋 추출 결과는 바탕화면 절대 경로를
코드나 JSON에 기록하지 않고 immutable resource pack manifest와 `Character/...` 또는 `Effect/...`
asset ID로만 인계한다.

G1의 상세 CODE_EXPLICIT 계획과 사용자가 직접 반영할 코드는
`.md/GB/08-03/2026-08-03_RENDERTARGET_DEBUGGER_PLAN.md`가 소유한다. G1은 새 Level, scenario,
launch option, Client 내부 smoke/report writer를 만들지 않는다. Render Target Inspector는 Debug
diagnostic이며 Character Select, Development, Bern, Valtan 어디서나 같은 `CGameInstance` 읽기
경계를 사용한다. Engine은 raw map이나 `CRenderTarget` owner를 Client에 공개하지 않고 tag, SRV,
width, height, format만 담은 Debug snapshot을 반환한다. Character Select에서는 Map/UI authoring은
계속 막고 Effect/Animation Tool만 열어, G4에서 실제 preview Character 본·socket 경계를 연결할
준비까지만 한다. 실제 attachment 결선은 G4가 소유한다.

현재 Character Select 추출 맵 `LV_LOBBY_CLASSSELECT_SL00`은 resource manifest/admission과 수동
검증이 끝나지 않았다. G1~G4는 이 803-placement 맵을 제품 Level에 연결하지 않는다. G5의 지면
연출 검증은 승인된 Character Select 최소 floor 또는 기존 Development/Valtan 지면에서 먼저 하고,
추출 맵은 asset lock·Hydrate→Verify·MapCatalog/Loader/runtime placement 계약이 모두 닫힌 뒤 별도
검증으로 편입한다.

Effect identity는 source trace, authored asset ID, Client presentation cue ID, Resources-relative
payload ID 네 종류를 합치지 않는다. G4의 본 추적은 asset revision과 transform revision을 분리해
매 프레임 file I/O 또는 runtime rebuild를 금지한다. G6의 animation event 중 EFFECT/SHAKE/SOUND는
presentation으로 실행할 수 있지만 HIT/CANCEL/SUPERARMOR/INVULN/MOVE는 제작 화면의 표시·검증만
허용하고 Server gameplay authority로 사용하지 않는다.

### 24.5 W7 일정의 해석

기존 W7의 12~16일은 G6~G9의 발탄 연출 제작량이며 G1~G5의 진단·Effect runtime·성능 기반과
G10의 PBR/SSAO/fog 전체 비용을 포함하지 않는다. W7만 빠르게 닫을 때도 G1, G3, G5의 최소 완료는
선행한다. 전체 고급 렌더까지 포함한 일정은 기존 W1~W3 견적과 별도로 관리하고, 기능별 profiler
gate를 생략해 일정 숫자만 줄이지 않는다.

### 24.6 게이트 공통 빌드·실행 순서

```text
Engine 공개 변경이 있는 Gate
-> Engine x64 Debug/Release
-> UpdateLib.bat Debug/Release
-> Client x64 Debug/Release

Client/Data만 바뀐 Gate
-> 필요한 publisher Validate/Publish + rollback fixture
-> Client x64 Debug/Release

Server 의미 이벤트가 연결되는 G9
-> Shared/NetworkProtocolHarness Debug/Release
-> Server Debug/Release + --contract-test
-> Client Debug/Release
-> Framework.slnLaunch 실제 Server+Client 종단 확인

모든 Gate 종료
-> 관련 수동 scene 검증
-> ProjectAudit (+ resource 변경 시 DeepAssetHash)
-> git diff --check
-> RESULT에 실제 실행한 증거만 기록
```

첫 구현은 G1이다. G1 완료 후에는 카메라 툴로 이동하지 않고 사용자 요청대로 G2 진단 고도화와
G3 Character Effect Lab을 먼저 이어 간다.
