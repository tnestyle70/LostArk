# 2026-08-02 LostArk 고급 렌더링·월드 툴 통합 견적 (SCOPE PLAN)

문서 유형: 범위·공수 견적서 (개별 slice 구현 계획서 아님 — 각 항목 착수 시 별도 PLAN 작성)
목표 선언: **Winters에서 구현했던 것과 계획만 하고 못 한 것(PBR, GI, 고급 셰이딩, World Partition, 툴)을 전부 이해한 상태로 LostArk에 녹여내고, 기능마다 영상+자막+기술소개서를 성의 있게 만든다.**

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
