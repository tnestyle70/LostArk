# 2026-08-03 Effect 툴 발전 계획 (참조 영상 01~02 패리티 마감 + 게임플레이 적용)

참조 이미지: `C:\Users\user\Desktop\툴\01_EffectTool_Texture_EffectDetail.png`,
`02_EffectTool_Mesh_Emissive.png`
— Effect Type 5종(Mesh/Texture/Particle/Decal/Trail), 채널 5종(Base/Noise/Mask/
Emissive/Dissolve), UV 시퀀스/타일, 빌보드, 블룸/왜곡 강도, 잔상, 이름 있는
Data Files 저장/로드.

작성 모드: STRUCTURE_FIRST. 내부 슬라이스 A~C로 나누며 한 번에 하나만 구현한다.

## 0. 현재 체크포인트 — 패리티 실측표

우리 `CEffect_Tool`/`CEffect_Runtime`은 참조 영상의 어소링 기능 대부분을 이미
갖고 있다. 참조 영상 UI 항목 대비:

| 참조 영상 기능 | 우리 상태 |
|---|---|
| Effect Type: Texture(빌보드 스프라이트)/Particle/Trail | 있음 — SPRITE/BEAM/RIBBON/ANIM_TRAIL 이미터 |
| Effect Type: Mesh | 있음 — MESH 이미터(.wmesh/.wskel/.wanim, CModel 단일 경로) |
| Effect Type: **Decal** | **없음** — 이미터 타입에 DECAL 부재 (슬라이스 A) |
| 채널 텍스처(Base/Noise/Mask/Emissive/Dissolve) | 부분 — 실측 매핑: Base→strTextureAssetId(g_Texture), Mask→strOpacityTextureAssetId(opacityMask+임계값), Dissolve→strDissolveTextureAssetId, Noise→strDistortionTextureAssetId. **Emissive는 텍스처 슬롯 없음**(`fEmissiveStrength` 스칼라만, Effect_Types.h:145-160) → 슬라이스 A |
| UV 시퀀스(IsSequence/TileCount/TileIndex/Term) | 있음 — SubUV 모듈(CPU Calculate_UVRect) |
| UV Speed/Wave/Start | 있음 — g_vUVTiling/Offset/Panner |
| Billboard | 있음 — 스프라이트 경로 |
| Bloom/Distortion 강도 | 있음 — MRT_SceneHDR(SceneHDR+Distortion) → 디퍼드 블룸/Final |
| Radial Time/Intensity(방사형 UV 왜곡) | **없음** — Client/Engine 셰이더 전체에 radial/polar UV 항 부재(grep 0건) → 슬라이스 A |
| Life/Delay/AfterImage(잔상) | Life/Delay 있음. 잔상 모듈은 **부재 확정**(Client 전역 grep 0건) — 본 행렬 스냅샷이 필요한 별도 시스템이라 후속 단계로 유예 |
| 이름 있는 Data Files 저장/로드 | 있음 — Data/Effects/Authored/<id>.effect (LOSTARK_EFFECT 5) + Save As/Open/Refresh |
| All Effects(Time Reset All/Delete/Clear All + 인스턴스 목록) | 부분 — 전역 1슬롯 프리뷰의 Play/Pause/Restart만. 다중 스폰 인스턴스 목록·일괄 제어는 없음 → 후속 단계(슬라이스 C의 pArg 다중 인스턴스화가 기반) |

즉 "완벽하게 동일하게"의 잔여분은 Decal·Emissive 텍스처 슬롯·Radial UV(이상
슬라이스 A)와 멀티 스폰 프리뷰·잔상(후속 유예)으로 확정됐고, 구조적 갭은
**성능과 적용**이다:

- **성능**: `Render_SpriteEmitter`가 파티클 1개당 1드로우(상수 바인드 포함).
  `CVIBuffer_Instance`는 추상 기반만 있고 구체 서브클래스가 저장소에 0개.
- **적용**: 게임플레이 트리거가 전무. `CEffect_Runtime`은 프로토타입/레이어에
  등록된 적 없고, 유일 인스턴스를 툴이 수동 구동하며, 에디터→런타임 다리는
  전역 1슬롯(`Publish_Preview`). `PlayerSkills.json`에 이펙트 필드 없음,
  `Play_Skill`은 애니메이션 CLIP_CHAIN만 재생. `.weffect` 런타임 소비자/퍼블리셔 없음.
- WORLD/LOCAL 시뮬레이션 공간이 직렬화만 되고 미적용, 본 어태치(strSourceBone)
  저장만 되고 미소비.

---

## 슬라이스 A — DECAL 이미터 + 패리티 잔여 마감

### A-1. 완료 조건

1. 이미터 타입 DECAL 추가: 표면에 투영되는 사각 데칼(위치+법선 기준 정렬,
   페이드 인/아웃, 채널 텍스처 재사용).
2. 확정된 매핑 공백을 보강한다: `EFFECT_MATERIAL_DESC`에 emissive 텍스처
   asset id 슬롯 추가(+`Shader_EffectMaterial.hlsli`에 g_EmissiveTexture 항과
   `Bind_EffectMaterialResources` 바인드), Radial Time/Intensity(방사형 UV
   왜곡) 파라미터 추가. 저장 포맷 왕복(Validate_RoundTrip)까지 통과.
   Noise→distortion·Mask→opacity는 기존 슬롯으로 매핑 종결이며, 잔상은
   후속 단계로 유예한다(0절 표 참조).
3. 기존 .effect 파일 하위 호환: 스키마 버전 5→6 승격 시 5 로드 허용
   (AssetIO가 이미 4+5를 수용하는 전례를 따른다).

### A-2. 설계 요점 (파일 책임)

```text
수정 Client/Public/Effect_Types.h: EFFECT_EMITTER_TYPE에 DECAL 추가.
  데칼 전용 desc(투영 크기, 표면 오프셋, 페이드 곡선 파라미터).
  스키마 버전 상수 승격. 새 필드 기본값은 "없던 시절과 동일 동작".
수정 Client/Private/Effect_AssetIO.cpp: 새 키 직렬화 + 구버전 로드 기본값.
수정 Client/Private/Effect_Runtime.cpp: Render_DecalEmitter 추가.
  1차 구현은 표면 위 오프셋 쿼드(월드 정렬)로 시작 — 깊이 재투영식 풀 데칼은
  G-Buffer 접근이 필요한 별도 렌더 작업이라 후속으로 분리. Target_Depth를
  이미 소프트 파티클로 샘플하므로 접촉 페이드는 1차에서도 가능.
수정 Client/Bin/ShaderFiles/Shader_EffectPrimitive.hlsl 또는 데칼 전용 패스:
  기존 DefaultTechnique Alpha/Additive 2패스 구조 유지, 패스 추가는 끝에만.
수정 Client/Private/Effect_Tool.cpp: 타입 콤보에 DECAL 노출 + 전용 패널 절.
```

### A-3. 검증

Validate_RoundTrip(신규 필드 포함), 구버전 .effect 로드 후 저장 시 데이터 보존,
HDR readback 스모크(`render.hdr-readback`)가 기존 수치 유지(데칼 미사용 씬),
데칼 사용 씬 눈 확인.

검토 Breakpoint와 관찰값: 스키마 5 파일 로드 분기(신규 필드가 기본값으로
채워지는가) → 저장 직전 desc의 DECAL/emissive/radial 필드 값 → 재로드 후 동일
값 왕복 → 기록된 헤더 버전이 6인지.

---

## 슬라이스 B — GPU 스프라이트 인스턴싱

### B-1. 완료 조건

1. 스프라이트 이미터가 파티클 N개를 1드로우(DrawIndexedInstanced)로 렌더.
2. 동일 이펙트의 화면 결과가 기존과 시각적으로 동일(HDR readback 수치로 회귀).
3. 프로파일러 카운터로 드로우 콜 감소 확인(기존 Profiler 오버레이 활용).

### B-2. 설계 요점

```text
신설 Engine 파생: CVIBuffer_Instance 최초의 구체 서브클래스
  (예: CVIBuffer_Rect_Instance). 기반 클래스의 이중 스트림 바인드 +
  DrawIndexedInstanced + 프로파일러 카운터는 이미 동작 — 서브클래스는
  슬롯1 인스턴스 정점 레이아웃과 동적 버퍼 갱신만 소유한다.
  근거: "두 번째 런타임 경로 금지"는 동일 역할의 중복 금지다. 추상 기반의
  최초 구현은 기존 경로의 완성이며, 맵 배치 인스턴싱(CModel::Render_Instanced +
  VTXMESHINSTANCE)과 같은 사상이다.
인스턴스 정점: 위치/크기/회전/틴트/UVRect — 현재 파티클당 상수로 바인드하던
  값들을 인스턴스 스트림으로 이동.
수정 Shader_EffectSprite.hlsl: 인스턴스 입력 시맨틱 추가 버전의 VS.
  (Shader_VtxMeshMapInstance.hlsl의 슬롯1 레이아웃 선례를 따른다.)
수정 Effect_Runtime.cpp Render_SpriteEmitter: 파티클 루프 -> 인스턴스 버퍼
  채우기(Map/Discard, 파티클 상한은 기존 캡 유지) -> 1회 드로우.
  동적 버퍼는 이미터 rebuild 시 상한 크기로 생성, 렌더 루프 할당 금지.
```

### B-3. 검증

동일 .effect의 before/after 스크린 비교 + HDR readback 평균 오차 허용치 내,
파티클 수 스윕(1k/10k/65k)에서 프레임 시간 로그, Begin/End_MRT 균형 유지.

검토 Breakpoint와 관찰값: Map/Discard 직후 채운 인스턴스 count == 살아있는
파티클 count, 프로파일러 드로우 콜 카운터 N→1 감소, 인스턴스 스트라이드와
슬롯1 입력 레이아웃 일치.

---

## 슬라이스 C — 게임플레이 적용 (스킬 → 이펙트 수직 슬라이스)

가장 크고, Data → (Shared/Server 판단) → Client 프레젠테이션 → 하네스를 관통한다.
핵심 원칙: **서버 승인 경로에 이펙트를 싣되, 이펙트는 클라 연출 전용이다.**
서버는 새 정보를 만들 필요가 없다 — 이미 스냅샷으로 오는 action/skillId가
트리거의 전부다. 따라서 Shared/Server 계약 변경은 **불필요**하다는 것이 이 설계의
결론이다(스냅샷의 PLAYER_ACTION_STATE + SKILL_ID를 클라가 이펙트로 매핑).

### C-1. 완료 조건

1. `Data/Balance/PlayerSkills.json`의 스킬 항목에 선택적 `effectId` 필드(클라
   연출 참조) — publisher(`Publish-GameplayBalance.ps1`) 검증 목록에 같은 변경으로
   추가(Assert-ExactProperties가 미지 필드를 거부하므로 동시 수정 필수).
2. Q(34060) 사용 시: 서버 승인 → 스냅샷 → `Apply_NetworkAction` → 스킬 매핑된
   이펙트가 시전자 위치에서 재생되고 수명 종료 시 정리된다.
3. Local Preview는 폐기한다. 실제 Server 승인 → snapshot presentation 경로만 지원한다.
4. 이펙트 정의는 `.weffect`를 퍼블리셔가 `Client/Bin/DataFiles/Effects/`로 승격,
   로더가 레벨 진입 시 선로드(매 프레임 파일 IO 금지 계약 준수).
5. `.weffect` 산출물의 Git 배포 갈래를 **구현 착수 전 팀 합의로 확정**한다.
   현재 `*.weffect`는 `.gitattributes`(LFS)에도 `.gitignore`에도 없어, 이대로
   승격하면 비-LFS raw 바이너리가 커밋되거나 미커밋 누락으로 팀원 런타임 입력이
   빠진다. 권장 (a): Server bootstrap처럼 로컬 생성물 취급 — Git 정본은
   `Data/Effects/Authored/*.effect` 텍스트뿐이고 `.weffect`는 퍼블리셔가
   재생성하며, ignore 규칙은 `!/Client/Bin/DataFiles/**` 부정 규칙(.gitignore:75)
   **뒤에** 경로 한정(`/Client/Bin/DataFiles/Effects/`)으로 넣어야 실제로 먹힌다.
   대안 (b): `.gitattributes`에 `*.weffect` LFS 패턴을 추가해 승인된 LFS 입력으로
   승격(.mapset 계열 선례). 대안 (c): 리소스 팩 payload 이관.

### C-2. 구조 변경 지도

```text
1) CEffect_Runtime 다중 인스턴스화:
   - Clone(pArg)의 pArg에 이펙트 초기화 desc(EFFECT_ASSET_DESC 참조 + 월드
     트랜스폼 + 수명 정책)를 도입. 전역 Publish_Preview 다리는 툴 프리뷰
     전용으로 유지하고, 게임플레이 인스턴스는 pArg 경로만 사용한다(두 경로의
     소유권이 겹치지 않게 — 다리는 에디터, pArg는 게임).
   - CLoader::Ready_For_Level_*에서 Prototype_GameObject_EffectRuntime 등록,
     Layer_Effect 클론. 표준 5단계 GameObject 레시피 그대로.
2) 신설 Client/Public·Private/EffectPlaybackService.{h,cpp}(가칭):
   존재 이유: "어느 스킬이 어느 이펙트를" 매핑과 스폰/수명 관리를 Character에서
     분리(Character는 이미 비대, 담당 경계상 이펙트는 별도 소유).
   한 문장 역할: 승인된 스킬 프레젠테이션 이벤트를 Layer_Effect 인스턴스로 바꾼다.
   소유하는 상태: effectId -> EFFECT_ASSET_DESC 캐시, 활성 인스턴스 목록.
   소유하지 않는 상태: 이펙트 렌더 상태(인스턴스가 소유), 스킬 판정, 네트워크.
   owner/생성 시점: CAnimationTargetService와 같은 Client 정적 서비스 계층.
     수명은 CMainApp이 소유(명시적 Initialize/Shutdown), desc 캐시는 레벨
     로더(Ready_For_Level_*)가 진입 시 구축. 제품 코드이므로 _DEBUG 아님.
   파괴 시점: 캐시/활성 목록은 레벨 전환 정리 경로에서 비움, 서비스 자체는
     CMainApp 종료 시.
   입력 데이터의 출처: 스냅샷 액션 시작 에지(skillId, caster transform) +
     publish된 밸런스 데이터의 effectId 매핑.
   출력 데이터의 소비자: Layer_Effect의 CEffect_Runtime 인스턴스.
   실패가 전달되는 경로: 레벨 진입 선로드 실패는 로더가 HRESULT/상태 문자열로
     보고하고 빈 캐시로 진행(연출 결손은 페일 소프트, 레벨 진입은 막지 않되
     로그로 반드시 드러낸다). 재생 시 매핑 부재/캐시 miss는 무동작 + 로그.
     클론 상한 도달 시 가장 오래된 인스턴스 재사용(정책 명시).
   금지: DirectInput/socket/판정 접근 없음. 프레젠테이션 전용.
3) CCharacter::Apply_NetworkAction: 스킬 시작 판정 지점에서
   EffectPlaybackService에 (skillId, casterTransform) 통지 — Play_Skill 내부에서
   호출하지 않는다(Logic_*/입력 경로에서 이펙트를 트리거하지 않는 경계 유지,
   네트워크 승인 경로에서만).
4) 신설 Tools/EffectPipeline/Publish-EffectAuthoring.ps1:
   Publish-GameplayBalance.ps1의 형태를 복제 — Validate|Publish 모드,
   .effect 텍스트 왕복 검증, 참조 텍스처가 Resources/Effect에 존재하는지 확인,
   .weffect 쿡 산출물을 staged promote + rollback, -FailureAfterPromote 주입.
5) ProjectAudit: effect.authoring-contract(Validate 실행) +
   effect.publish-rollback(픽스처) 체크 추가 — world.* 체크 구현을 본뜬다.
```

### C-3. CPP 흐름 (승인 → 재생)

```text
S2C_WORLD_SNAPSHOT 수신 (기존)
-> CClientReplication -> CCharacter::Apply_NetworkAction(state, skillId, tick)
-> 액션 시작 에지 판정 (기존 애니 재생과 같은 지점)
-> EffectPlaybackService::On_SkillPresentation(skillId, casterTransform)
   -> effectId 매핑 조회 (없으면 무동작 — 이펙트 없는 스킬은 정상)
   -> desc 캐시 조회 (레벨 로드 시 채워짐; miss는 로그 후 무동작, 파일 IO 금지)
   -> Layer_Effect에서 유휴 인스턴스 획득 또는 클론 상한 내 생성
   -> 인스턴스 Reset(desc, 시전자 위치/방향)
매 프레임: 인스턴스 Update가 수명 관리 -> 만료 시 서비스가 레이어 제거
레벨 전환: LEVEL::STATIC이 아니므로 Change_Level 정리에 자연 편승 + 서비스
  캐시는 레벨 로드 단위로 재구축
```

### C-4. 검증

```text
publisher: Validate 정상/오류 픽스처(-FailureAfterPromote 롤백 포함).
회귀: Visual Studio Server+Client 실행에서 Q 승인 후 이펙트 생성, 이펙트 없는 스킬(W)은
  무변화를 수동 관찰한다. Client 내부 smoke/report 분기는 추가하지 않는다.
  Release 빌드 컴파일(서비스는 제품 코드이므로 `_DEBUG`가 아님)을 함께 확인한다.

검토 Breakpoint와 관찰값 — 실제 값 skillId=34060 하나를 종단 추적:
(1) CClientReplication 스냅샷 파스: skillId / PLAYER_ACTION_STATE / tick 값
(2) Apply_NetworkAction 액션 시작 에지: 이전/신규 action state 분기 조건
(3) On_SkillPresentation: effectId 매핑 조회 결과, desc 캐시 hit 여부,
    활성 인스턴스 count 변경 전후
(4) 인스턴스 Reset: 적용된 caster transform 값, 수명 만료 시 count 감소
```

---

## 후속 단계 (범위 밖)

- WORLD/LOCAL 공간·본 어태치 소비(트레일 strSourceBone) — 캐릭터 이펙트 밀착에
  필요, C 완료 후.
- 보스/몬스터 액션 이펙트 매핑(BossProfiles 쪽 동일 패턴).
- 잔상(After Image) 전용 시스템(참조 영상 01의 After Image 필드) — 스킨드 메시
  고스트는 본 행렬 스냅샷 링버퍼가 필요한 별도 설계.
- 툴 멀티 스폰 프리뷰(참조 영상 All Effects 패리티) — 슬라이스 C의 pArg 다중
  인스턴스 경로를 툴 소비자로 확장해 N개 스폰 목록 + 일괄 Time Reset/Delete/
  Clear를 붙인다. 1슬롯 Publish_Preview 다리는 라이브 편집 프리뷰 전용으로 유지.

## 프로젝트 설정·등록

- 신설 파일 vcxproj/filters. `Data/Balance/PlayerSkills.json` 스키마 변경은
  publisher Assert 목록과 **동일 커밋**. 신규 Data/Effects/Authored/*.effect
  커밋 시 96.DataFiles 등록.
- `.weffect` 배포 갈래 결정(C-1 조건 5)에 따른 `.gitignore` 또는 `.gitattributes`
  변경을 **같은 커밋**에 포함(팀 합의 선행 — CLAUDE.md "새 바이너리 자산은
  LFS/Drive 팩 판단 먼저" 규칙).
- Engine 변경은 슬라이스 B의 VIBuffer 서브클래스뿐 — 해당 슬라이스만 UpdateLib.
- 스모크: 기존 effect.preview / render.hdr-readback 유지 + C의 online smoke 확장.

## 사용자가 먼저 작성할 범위

슬라이스 A의 Effect_Types desc 확장과 AssetIO 왕복부터. 슬라이스 C는 구조 확정
후 EffectPlaybackService 골격 → Apply_NetworkAction 훅 순서로.
