# Effect 시스템 전수 검토 (2026-07-31)

대상: `C:\Users\user\Desktop\LostArk\Client`의 Effect 계열 6모듈, 총 ~4,400줄 실코드 전량 독해 (4갈래 병렬 리뷰).
핵심 판정: **강의 영상 속 "Effect Tool"과 이 폴더의 실코드는 서로 다른 두 개의 툴이다.** 영상 툴(5채널 텍스처 탭·Effect Detail·AttGravity 프리셋)의 위젯·문자열은 이 트리 어디에도 없다(grep 0건). 이 폴더에는 그보다 **한 세대 진화한 UE Cascade식 시스템**이 이미 들어 있다.

---

## 0. 모듈 지도

| 파일 | 줄수 | 역할 |
|---|---|---|
| `Effect.h/.cpp` | 48+139 | **레거시 샘플** — 쿼드 1장 + 90프레임 플립북 하드코딩. 수업 기본 제공 수준. 실시스템과 무관 |
| `Effect_Types.h` | — | **데이터 모델 정본**: `EFFECT_ASSET_DESC` → 이미터(≤128) → 모듈(≤64/이미터). 스키마 버전 3 |
| `Effect_Tool.h/.cpp` | 89+1,307 | ImGui 저작 툴 "LostArk Effect Tool" + "Source Catalog" 창 |
| `Effect_Runtime.h/.cpp` | 147+1,485 | 게임 측 렌더러(CGameObject) — 에디터↔게임 브리지, 리소스 해석, 4셰이더 렌더 |
| `Effect_ParticleSimulator.h/.cpp` | 57+571 | 순수 CPU 결정론 시뮬 (D3D/ImGui/파일 무의존) |
| `Effect_AssetIO.h/.cpp` | 27+566 | `.weffect` 바이너리 + JSON 봉투 직렬화, 라운드트립 검증 |
| `Effect_ResourceCatalog.h/.cpp` | 58+172 | 추출된 로아 ParticleSystem CSV 인덱스 (Bard/Glaivier/Common) |

출처 추정: 수업 기본 제공이 아니라 **최근 작업(병행 세션)의 산출물**로 보인다. 근거 — `LOSTARK_EFFECT_EXPORT_2026-07-29` 하드코딩 경로, Bard/Glaivier 카탈로그, Winters 쿡 포맷(.wmesh/.wmat/.wskel/.wanim) 소비, provenance(AUTHORED_VARIANT/RECONSTRUCTED) 개념, 생성자 내 Phase2 골든 검증. → 사용자 확인 필요.

## 1. 데이터 모델 (Effect_Types.h) — UE Cascade 미러

- **Asset**: id, 이름, provenance, duration, warmup(0..10s), 이미터 목록.
- **Emitter**: 타입 = `SPRITE / MESH / BEAM / RIBBON / ANIM_TRAIL`, 시뮬 공간(LOCAL/WORLD), **블렌드 모드**(ADDITIVE→pass1, 그 외→pass0 알파), 정렬 모드(거리/나이), delay, duration, loopCount, 모듈 목록.
- **Module ~20종**: REQUIRED(텍스처/메시/머티리얼 id 3개), SPAWN(rate/burst/max), LIFETIME, INITIAL_LOCATION/VELOCITY/SIZE/COLOR/ROTATION, ROTATION_RATE, SIZE/ALPHA/VELOCITY/COLOR_OVER_LIFE(구간선형 커브), SUB_UV(cols×rows·fps·start/end·loop), COLLISION(AABB 반사+반발), EVENT(정규화 시각 1회), LOD(near/far·spawn scale), MESH_ANIMATION(클립·배속±16·loop), BEAM(오프셋·폭·세그먼트·노이즈), TRAIL(폭·포인트 수명·본 2개).
- **분포**: CONSTANT / UNIFORM_RANGE (축별 독립 랜덤). **커브**: 키 목록, 빈 커브 = 1.0 항등.

영상 툴의 개념 대응: IsSequence/TileCount↔SUB_UV, LifeTime/StartDelay↔LIFETIME+fDelay, Particle Direction/Random↔INITIAL_VELOCITY 분포, Select Pass↔eBlendMode, 잔상(AfterImage)↔TRAIL(전용 시스템 없음), Billboard↔SPRITE 타입에 내장(토글 없음).

## 2. 툴 (Effect_Tool) — 저작 흐름

- 창 구성: 툴바(에셋명·Play/Pause·Restart·Loop·World Preview+위치·배속·LOD 거리·Save/Load JSON·Cook/Load .weffect·상태) + 3열(이미터 목록 | 모듈 목록+Apply/Restart | 속성 편집기+프리뷰 캔버스) + 별창 Source Catalog.
- 프리뷰 2계층: ①툴 내 **CPU 도식 캔버스**(ImDrawList, 18px/유닛, SubUV 아틀라스 UV로 실제 텍스처 표시, 디코드 실패 시 원 폴백) ②**월드 프리뷰** — `CEffect_Runtime::Publish_Preview(asset, enabled, pos)` 정적 브리지로 게임과 **동일한 렌더 경로**(RENDERGROUP::BLEND)에서 재생.
- 수치 편집은 즉시 asset에 반영되지만 **시뮬에는 Apply/Restart 후 반영**(시뮬레이터가 desc로 초기화되는 구조). 구조 변경은 전체 시뮬 리빌드+텍스처 캐시 클리어.
- Source Catalog: `particle_systems.csv`(9열 엄격, 3개 후보 경로) — 추출된 로아 ParticleSystem **이름 인덱스일 뿐**, Cascade 모듈·머티리얼 그래프는 복원 불가 명시. "Use Name" = 이름+provenance=RECONSTRUCTED만 설정, 리소스는 수동 배정.

## 3. 런타임 (Effect_Runtime + Simulator) — 잘 설계된 결정 5개

1. **에디터/런타임 절단**: "에디터는 D3D 리소스를 소유하지 않고, 런타임은 ImGui 상태를 소유하지 않는다" — 뮤텍스+세대(generation) 브리지, 세대 변화 시에만 리빌드. 끄기도 세대를 올려 즉시 소멸.
2. **결정론**: LCG(1664525/1013904223), 이미터별 시드 = 황금비 스텝 XOR emitterId → 리빌드/리플레이가 동일 파티클 스트림 재현. **컴파일 내장 골든 테스트**(4/s·1s수명·2s → t=0.5/1.0/1.5에 alive 2/4/4) 결과가 프리뷰 상태줄에 상시 표시.
3. **용량 통치**: 이미터 65,536 / 이펙트 262,144 예산을 리빌드 시 SPAWN 모듈에 선착순 클램프, 드랍 수 카운트. isfinite 가드.
4. **관대한 리소스 해석**: id당 7개 후보 루트 시도, 실패 시 1×1 흰 텍스처 폴백 — 프레임을 죽이지 않는다. 80GiB 추출 트리는 절대 스캔 안 함(주석 명시). `.wmesh`는 스킨+애님까지(쿡 모델은 가변 상태 때문에 의도적 비캐시).
5. **워밍업**: 루프 이펙트를 0..10s 고정 1/60 스텝 사전 시뮬 — 스폰 직후 중간 상태로 등장.

**성능 한계 (의도된 Phase 2 CPU 레퍼런스)**: 스프라이트/메시 = **파티클당 드로우콜 1개**(인스턴싱 없음, 헤더에 "future GPU renderer will consume Get_Particles()" 명시), 매 프레임 정렬(std::sort), 모듈 탐색 O(파티클×모듈). 리본/빔은 CPU 빌드 동적 VB 1드로우. 프로파일러 드로우콜은 프리미티브 경로만 계수(과소 계상).

## 4. 직렬화 (Effect_AssetIO) — 위험 3개 (팀 규칙으로 박을 것)

1. **스키마 정확 일치만 허용**: v3 불일치 = 로드 거부, 마이그레이션 없음. JSON도 실데이터는 hex 바이너리 봉투라 전방 호환 0. → **규칙: `EFFECT_*` 구조체 멤버 추가/재배열 = 반드시 `EFFECT_ASSET_SCHEMA_VERSION` 범프 + 기존 에셋 재저장.** POD memcpy 직렬화라 패딩 변화만으로도 와이어가 조용히 깨진다.
2. **저장 슬롯이 working 1개뿐**: `working.effect.json`/`working.weffect` 고정 경로, save-as 없음 — 영상 툴의 AttGravity1~4식 네임드 프리셋이 없다. A가 이펙트 N개를 만들려면 **네임드 저장 확장이 선행 과제**.
3. **CWD 의존 상대 경로**: 카탈로그 CSV 3후보·기본 텍스처 등이 실행 디렉토리에 따라 갈림. VS 디버그 vs 배치 실행에서 다르게 동작 가능.

방어는 좋다: magic `EFCT`+FNV-1a 해시+64MiB 캡+트레일링 바이트 거부+저장/로드 양측 한도 검증+시작 시 라운드트립 스모크(Client.cpp).

## 5. 영상 툴 vs 폴더 시스템 — 공백 지도

| 축 | 영상 툴 (강의) | 폴더 시스템 (실코드) |
|---|---|---|
| 구조 | 요소 합성형(타입 5종 라디오) | **이미터×모듈 조합형(Cascade)** — 우세 |
| 머티리얼 | **5채널(Base/Noise/Mask/Emissive/Dissolve)+UV Wave/Distortion** — 우세 | 이미터당 텍스처 1장, 채널 없음 |
| 시퀀스 | 8×8 타일 플립북 | SUB_UV 동등+ |
| 파티클 | Direction/Random/Spread | 분포+커브+충돌+이벤트+LOD — 우세 |
| 저장 | 네임드 프리셋 다수 | working 1슬롯 — 열세 |
| 프리뷰 | 게임 씬 직접 | 도식 캔버스+월드 브리지 — 동급+ |

→ **A 레인 로드맵이 이 표에서 바로 나온다**: ①네임드 저장(AttGravity식) ②멀티채널 머티리얼(Noise/Mask/Emissive/Dissolve — 셰이더+REQUIRED 확장, 스키마 v4) ③GPU 인스턴싱(성능 필요 시점에).

## 6. 다음

- [x] 출처 확정(2026-08-01): 병행 세션 산출물 — `.md/GB/07-29/2026-07-29_EFFECT_TOOL_PHASE_3_12_IMPLEMENTATION_RESULT.md`, `2026-07-29_BARD_GLAIVIER_EFFECT_*` PLAN/RESULT 실재
- [ ] 리드 ⓪ 세션: Publish_Preview 브리지→Rebuild→Simulator.Update→Render 경로 손 추적 (티칭 선행)
- [ ] A 첫 카드에 "working 1슬롯" 한계 반영 (네임드 저장 확장을 프레임워크 레인 과제로)
- [ ] 스키마 버전 규칙을 팀 규약에 등재
