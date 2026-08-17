# 워로드 BA3 transform이 실제 BA에 반영되지 않는 원인과 조치 결과

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`
신고: 워로드 `effect.warlord.skill.17000.ba3.unified`에 transform / position / rotation을
저장했는데 실제 BA로는 그 값이 빠진 이펙트가 나온다.

## 1. 결론

**저장은 정상이고, 재생이 다른 문서를 본다.** Effect Tool의 `Save Changes`는
`Data/Effects/Authored`의 저작 원본만 쓴다. 제품 BA 재생과 Effect Tool의 Product Play는
둘 다 `Client/Bin/DataFiles/Effect`의 **publish된 런타임 문서**를 읽는다. 이 저장소에서
마지막 publish는 손튜닝보다 앞서므로, 재생은 계속 튜닝 이전 revision을 그린다.

런타임 코드에는 transform을 버리는 결함이 없다. 아래 §2.3에서 확인했다.

## 2. 실측

### 2.1 두 문서가 서로 다르다

```text
저작 원본  Data/Effects/Authored/effect.warlord.skill.17000.ba3.unified.effect.json
           68,719 B   2026-08-17 10:11:10
           element 3개
             36c9c05798078f5da5e3bc1c   rotationDegrees [0, 90, 0]
             ca9c36bec404174e4cfb23d9   position [0.18, 0, 2.54]  rotationDegrees [0, -90, 0]
             4373c8203e0a1fe1a2a8b220   identity
           sha256 332eb02d94b65ad3d521b6b9601189e41e65c43ed2929c3600c3d22a58b96296

런타임 문서 Client/Bin/DataFiles/Effect/Authored/
           effect.warlord.skill.17000.ba3.unified.8b465e03...bdd.effect.json
           130,941 B  2026-08-17 02:48:23
           element 6개, transform 전부 identity
```

`EffectCatalog.runtime.json`의 해당 행 `contentSha256`은 `8b465e03...bdd`로, 위 런타임
파일을 정확히 가리킨다. 저작 원본의 sha와는 무관하다.

### 2.2 커밋 이력이 같은 말을 한다

```text
4f8a3951  perf(effect): seal compacted runtime Effect documents   <- 마지막 publish
   ...
eb57f9fa  Keep the hand tuning done on the two Warlord Effects    <- 손튜닝 (더 나중)
```

`Client/Bin/DataFiles/Effect/`를 건드린 마지막 커밋이 `4f8a3951`이고, 손튜닝 커밋
`eb57f9fa`는 그 뒤다. 그 사이 publish가 없었다. 런타임 파일은 Git 추적 대상이므로
publish 없이는 갱신될 방법이 없다.

### 2.3 런타임에는 transform을 버리는 경로가 없다

의심 경로를 하나씩 확인했다.

```text
Effect_Playback.cpp:4596-4631  Evaluate_ElementWorld
    Detail.Transform.vPosition / vRotationDegrees / vScale 를 직접 읽어
    Local = S * R * T 로 합성. sourceRecipe.enabled 게이트 없음.

Effect_Playback.cpp:4587       m_TransformMasterIndices
    transformInheritance.enabled 인 element만 master의 transform으로 대체된다.
    ba3 3개 element 전부 transformInheritance.enabled=false 이므로 해당 없음.

Publish-Effects.ps1 + compact_effect_document.py
    compaction은 문자열 리터럴 밖의 공백만 버리는 어휘 변환이다.
    숫자 토큰과 키 순서를 바이트로 보존하므로 transform을 잃지 않는다.
```

즉 publish만 되면 값은 그대로 화면까지 간다.

### 2.4 도구가 이 어긋남을 말해주지 않는다 — 이것이 고칠 결함

```text
Effect_Tool.cpp:13200-13205   Try_SelectProductCue
    pRuntimeDocument = CEffectCatalog::Find(assetId)      <- 런타임 카탈로그
Effect_Tool.cpp:13283
    Stage_WorldPreview(*m_SourcePreviewDocument, true)    <- 그것을 미리보기로 올린다
Effect_Tool.cpp:13301
    m_strElementStatus = "Playing full Effect | ..."      <- 성공 문구만 남는다
```

저작자가 "실제 BA 기준"을 확인하려고 누르는 바로 그 버튼이, 방금 저장한 문서가 아니라
publish된 문서를 재생하면서 그 사실을 한 마디도 하지 않는다. 증상이 "저장이 안 먹는다"로
읽히는 이유가 이것이다.

도구가 이미 `m_bActiveDocumentMatchesRuntime`(`Effect_Tool.cpp:20224`)로 등가성을 계산하고
있었지만, 그 결과는 Effect Detail 하단의 회색 한 줄로만 나오고 Product Play 경로에서는
전혀 조회되지 않았다.

### 2.5 어긋난 문서는 BA3 하나가 아니다 — corpus 전수 실측

catalog 99행 중 `DIRECT_AUTHORED_DOCUMENT_V13` 98행에 대해, 저작 원본과 publish된
런타임 문서의 **값 트리**를 비교했다. publisher가 봉인하는 것은 공백만 제거한 사본이므로
값 트리는 같아야 한다.

```text
catalog rows            99   (LEGACY_ASSEMBLY_V1 1건 제외)
source == runtime       94
DRIFTED                  4
authored source missing  0
runtime file missing     0

assetId                                              srcEl  rtEl  shared  trDiff
effect.warlord.skill.17000.ba3.unified                   3     6       3       2
effect.warlord.skill.17030.unified                       7    39       7       5
effect.warlord.skill.17040.unified                      11    46      11       5
effect.dimensionmaster.skill.2050120.clip3.unified      51    51      51       0
```

`trDiff`는 양쪽에 공통으로 존재하는 element 중 `detail.transform`이 다른 개수다.

- 워로드 3건이 이번 신고와 같은 원인이다. 손튜닝으로 element를 줄이고 transform을
  옮겼는데 publish가 없었다. `17030`은 39개가 7개로, `17040`은 46개가 11개로 줄어 있다.
- `2050120.clip3`은 element 수와 transform이 같고 material 실행 경로만 다르다.
  다른 세션의 진행 중 변경이며 성격이 다르다(§5.1).

나머지 94건은 이미 일치한다. 즉 **전면적인 파이프라인 붕괴가 아니라, 마지막 publish
이후 손댄 문서만 어긋나 있다.**

## 3. 구현한 것

`Client/Public/Effect_Tool.h` +4 / `Client/Private/Effect_Tool.cpp` +45 -3.
런타임 계약, 저장 형식, publisher, 어떤 문서든 바꾸지 않았다.

### 3.1 Product Play가 어긋남을 말한다

`Describe_ProductPlaybackAuthoredDivergence()` 신규. 활성 문서가 재생 대상과 같은
asset ID인 AUTHORED 문서일 때만 판정하고, 아니면 빈 문자열을 반환해 아무 말도 하지 않는다.

```text
미저장 편집 있음  -> "STALE PRODUCT: ... unsaved edits. Save, then Publish + Reload."
저장했고 != 런타임 -> "STALE PRODUCT: ... Saving does not republish; use
                       Publish + Reload Product Test to apply the saved edits."
저장했고 == 런타임 -> 아무 말도 하지 않는다
```

`Try_SelectProductCue`의 성공 status에 이어 붙는다. visual program projection 경로
(도화가 F adapter packet)는 비교 대상이 저작 문서가 아니므로 `nullptr == pProductVisualProjection`
일 때만 호출한다.

재생을 막지 않는다. Product Play는 계속 제품이 실제로 그리는 것을 그대로 보여줘야 하며,
`.md/GB/gotchas.md` 12.3이 경고한 "저작 문서와 제품 문서가 다르다"는 사실 자체를 숨기면 안 된다.

### 3.2 Editing Session 바에 Runtime 상태를 상설 표시

`Runtime_SyncLabel()` 신규. 기존 `Source | Draft | Document | Preview` 줄에 한 칸을 더한다.

```text
Runtime: none | unsaved | unpublished | synced | STALE
```

`Document: saved`는 파일 상태일 뿐 제품 상태가 아니다. 두 사실을 같은 줄에서 분리해
보여주는 것이 이번 변경의 핵심이다.

## 4. 자동 검증 — 실행함

```text
Client x64 Debug 빌드 + 링크    PASS
    Client/Bin/Debug/Client.exe  22,147,584 B  2026-08-17 11:07:07
    새 경고 없음. 기존 CP949 C4819과 LNK4099 PDB 경고만 남음

git diff --check                PASS
    (다른 세션 파일의 기존 CRLF 경고만 출력)

Publish-Effects.ps1 -Mode Validate   FAIL  (§5.1, 이번 변경과 무관)
```

`Runtime_SyncLabel()`은 매 프레임 호출되므로 등가성을 재계산하지 않고 기존 플래그만
읽는다. 초안은 여기서 `Refresh_RuntimeEquivalence()`를 불렀으나, 그 함수는 최대 수 MB의
canonical 문자열 두 개를 비교하므로 렌더 루프에서 제거했다. 플래그는 save / load /
create / edit 경로가 이미 전부 갱신한다.

## 5. 미실행과 그 이유

### 5.1 publish를 실행하지 않았다 — 지금은 실행하면 안 된다

사용자의 튜닝을 실제 BA에 반영하려면 publish가 필요하지만, 현재 worktree에는 다른 세션의
진행 중 변경이 들어와 있다.

```text
 M Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl
 M Data/Effects/Authored/effect.dimensionmaster.skill.2050120.clip3.unified.effect.json
?? Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli
?? Tools/EffectPipeline/materialize_dimensionmaster_2050120_spritewave.py
```

그리고 그 변경 때문에 corpus 검증이 통과하지 못한다.

```text
Source Material Template requires an enabled profile in
effect.dimensionmaster.skill.2050120.clip3.unified: authored.source-particle.167f5000cbd1d56c042022dd
   Publish-Effects.ps1:2032
```

HEAD의 같은 문서는 `sourceProfile.enabled=true`이고, worktree 사본만 `false`다. Cascade
material graph 계획의 G02가 의도한 소유권 전환(`sourceProfile.enabled=false`,
`execution.enabled=true`)이며, publisher 쪽 계약이 아직 따라오지 않은 중간 상태다.

이 사실 자체가 이번 신고와 이어진다. **`Publish-Effects.ps1 -Mode Publish`는 corpus 전체를
한 트랜잭션으로 검증하므로, 문서 하나가 깨져 있으면 워로드 ba3를 포함한 나머지 전부가
publish되지 않는다.** 사용자가 그때 `Publish + Reload Product Test`를 눌렀더라도 같은
이유로 실패했을 것이다.

다른 세션의 미커밋 변경을 되돌리지 않고, 그 중간 상태를 런타임 산출물에 봉인하지도 않는다.
publish는 그 슬라이스가 착지한 뒤에 실행한다.

### 5.2 실행 검증

빌드는 통과했지만 실제 Effect Tool을 띄워 새 `Runtime:` 표시와 Product Play 경고 문구를
확인하지는 않았다. 이는 사용자 실행 영역이다.

첫 시도에서는 사용자가 실행 중이던 `Client.exe`(PID 40840, 10:35 시작)가 출력물을 점유해
링크가 `LNK1104`로 멈췄다. 프로세스를 종료시키지 않고 대기했고, 이후 해제되어 링크가 끝났다.

### 5.3 화면 판정

`AGENTS.md`의 사용자 전용 화면 검증 경계에 따라 에이전트는 Client·Effect Tool을 실행·조작하지
않았고 화면을 판정하지 않았다.

## 6. 사용자가 할 순서

1. Client를 닫는다. (링크 점유 해제)
2. Cascade material graph 슬라이스가 착지해 `-Mode Validate`가 통과하는 상태가 되면 publish한다.

```bash
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
```

3. 통과하면 `-Mode Publish`로 바꿔 실행하고 `Client/Bin/DataFiles/Effect`의 diff를 커밋한다.
4. Client를 다시 빌드·실행하고 워로드 BA3를 확인한다.

`Publish + Reload Product Test` 버튼을 도구 안에서 쓰려면 순서가 있다. 저작 문서를 먼저
열고(활성 문서로 만들고), 그 다음 All Effects에서 같은 cue를 Product Play해야 버튼이 켜진다.
반대로 하면 문서 열기가 Product preview를 지워서 버튼이 계속 비활성이다.

## 7. Cascade material graph 슬라이스 교차 검증 (2026-08-17)

같은 worktree에 들어온 `2026-08-17_EFFECT_CASCADE_MATERIAL_GRAPH_RECONSTRUCTION_RESULT.md`의
주장을 실제 트리로 확인했다.

### 7.1 사실로 확인된 것

```text
변경 element    8개 정확     추가·삭제 0, 나머지 43개 무변경
   4개 fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_85_tr
   4개 fx_m_mi_s_00.fx_s_me_spritewave_01_01_tr
8개 모두 sourceProfile.enabled true->false, execution.enabled false->true,
   opcode 15, backend runtimeMaterialV2
detail / sourceRecipe / resources / transform / visible   전부 무변경
texture lane 32개 전부 Client/Bin/Resources 실파일로 해석됨   missing 0
HLSL dispatch 경로 존재   Shader_VtxEffectParticle.hlsl -> g_RuntimeMaterialV2Opcode
   -> RUNTIME_MATERIAL_V2_UE3_SPRITEWAVE_TR = 15u
opcode 전달 경로 C++ 변경 불필요   Effect_DocumentRenderer.cpp:3785, 4167
Claude 측 Effect_Tool.cpp/.h 변경 보존됨   워로드 데이터 무변경
```

"원본 DDS·MIC 수치·Dynamic Parameter 그대로", "Cascade spawn/lifetime 유지",
"8개", "opcode 15", "C++ 무변경"은 전부 맞다.

### 7.2 그러나 이 문서는 현재 로드되지 않는다

C++ codec에 publisher와 **동일한 불변식**이 있다.

```text
Effect_DocumentCodec.cpp:11017
    if (!bSourceContract && iSourceVersion >= 11u &&
        strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
        !Element.Material.SourceMaterial.bEnabled)
        -> "Effect source Material template requires a staged profile."  실패

문서 version                              13
EFFECT_SOURCE_CONTRACT_FORMAT_VERSION     14
  -> bSourceContract = false      면제되지 않는다
templateId                                "effect.source_material"   유지됨
sourceProfile                             {"enabled": false}         축소됨
```

그리고 renderer는 정확히 반대를 요구한다.

```text
Effect_DocumentRenderer.cpp:3487
    Execution.iOpcode == 0u || Element.Material.SourceMaterial.bEnabled
        -> "Authored material execution identity is invalid"
```

**codec은 `source_material` 템플릿에 profile이 켜져 있기를 요구하고, renderer는 execution을
쓰려면 꺼져 있기를 요구한다.** 두 계약이 정면으로 충돌하며, 이번 슬라이스는 renderer 쪽만
만족시켰다. `Publish-Effects.ps1:2030`의 같은 규칙이 실제로 실패하는 것으로 이미 실증됐다
(§5.1). codec 규칙도 같은 조건이므로 Effect Tool의 문서 열기 역시 통과하지 못한다.

결과적으로 RESULT 문서 §5의 확인 순서 4단계(`문서를 열고 clip3 재생`)는 현재 상태로는
수행할 수 없다. `materializer stable`, `JSON parse PASS`, `HLSL compile PASS`,
`C++ compile PASS`는 모두 맞지만, 그 중 어느 것도 **C++ codec의 문서 로드**를 통과시키지
않는다. Python 파서와 C++ codec은 다른 검증자다.

### 7.3 닫으려면 필요한 것

소유권 전환을 유지하려면 두 곳을 같은 변경 단위에서 함께 열어야 한다.

```text
Client/Private/Effect_DocumentCodec.cpp:11017   execution.bEnabled 일 때 예외 허용
Tools/EffectPipeline/Publish-Effects.ps1:2030    같은 예외를 publisher에 반영
```

또는 그 8개의 `templateId`를 `effect.standard`로 바꾸는 방법이 있으나, 그러면
`sourceMaterialPath` provenance의 의미가 달라진다.

부수적으로 `sourceProfile`이 `{"enabled": false}` 한 필드로 축소되면서
`parentMaterialPath`, `semanticStatus`, `dynamicParameterSemantics`, `subUVMode`가
문서에서 사라졌다. 8개 element가 어느 parent family의 식을 실행하는지가 문서에 더는
기록되지 않는다. 이 슬라이스의 주장이 `MATERIAL_FAMILY_RECONSTRUCTED`이므로 그 근거
필드는 남기는 편이 좋다. 소유자 판단 사항으로 남긴다.

## 8. 남은 경계

- 이번 변경은 진단 표시다. 저장이 자동으로 publish되게 만들지 않았고, 그럴 계획도 아니다.
  publish는 corpus 트랜잭션이므로 저장마다 돌 수 없다.
- 활성 문서가 아닌 asset의 어긋남은 판정하지 않는다. 모든 Product Play마다 디스크에서
  저작 문서를 파싱하면 비용이 생기고, 실제 저작 흐름에서는 대상 문서가 활성 상태다.
- 런타임 카탈로그에 저작 원본의 sha가 기록되지 않는다(`SourceSha256`은 publish 중
  메모리에만 존재한다). 이것이 있으면 파싱 없이 파일 해시 하나로 어긋남을 판정할 수 있다.
  publisher와 카탈로그 스키마를 함께 바꾸는 별도 슬라이스이며, 현재 다른 세션이
  `Publish-Effects.ps1`을 수정 중이라 이번 변경에 넣지 않았다.
