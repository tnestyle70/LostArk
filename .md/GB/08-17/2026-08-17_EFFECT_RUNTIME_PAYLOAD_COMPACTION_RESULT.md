# 2026-08-17 Effect 런타임 payload compaction 결과

branch: `feature/effect-tool-texture-kind-filter`
기준 조사: `.md/GB/08-17/2026-08-17_EFFECT_DOCUMENT_SIZE_ROOT_CAUSE.md` 4.1

## 1. 구현 상태

publish 시점에 sealed Effect 문서의 무의미한 공백을 제거한다. C++ 변경은 없다.

```text
Tools/EffectPipeline/compact_effect_document.py   신규
Tools/EffectPipeline/Publish-Effects.ps1          +48 -10
```

### 1.1 C++를 바꾸지 않아도 되는 근거

런타임은 저작 파일과 대조하지 않는다. catalog 행의 `contentSha256`, 파일명에 박힌 SHA,
실제 런타임 파일 바이트의 SHA 세 개가 서로 일치하기만 하면 된다.

```text
Effect_Catalog.cpp:3954   Compute_Sha256Hex(Text) != Source.strContentSha256 -> 거부
Effect_Catalog.cpp:117    파일명은 Authored\<assetId>.<contentSha256>.effect.json 이어야 함
```

따라서 publisher가 compact payload를 쓰고 그 payload의 SHA를 catalog와 파일명에 함께 쓰면
런타임 계약이 그대로 성립한다.

### 1.2 JSON 왕복을 쓰지 않은 이유

`ConvertTo-Json` 또는 `json.dumps` 왕복은 두 가지를 깬다.

```text
float 재포맷   0.20000000298023224 -> 0.2 로 정밀도 손실
키 순서        런타임 codec은 Has_ExactOrderedKeys 로 키 순서를 정확히 비교한다
```

그래서 `compact_effect_document.py`는 파싱이 아니라 **어휘 변환**이다. 문자열 리터럴 밖의
space/tab/CR/LF만 버리고 나머지 바이트는 그대로 옮긴다. 숫자 토큰과 키 순서가 바이트 단위로 보존된다.
도구는 변환 후 원본과 값 트리가 같은지 자체 검사한 뒤에만 출력한다.

### 1.3 SHA 계약 분리

`Read-PinnedDirectAuthoredBytes`가 두 가지를 각각 검증한다.

```text
SourceSha256 / SourceByteCount   저작 문서가 publish 도중 바뀌지 않았는지 (기존 핀 유지)
Sha256 / ByteCount               sealed compact payload의 런타임 무결성 (신규)
```

catalog의 `contentSha256`과 파일명 SHA는 이제 compact payload의 것이다.

## 2. 자동 검증 — 실행함

### 2.1 무손실 확인 (6.3 MB 문서 단건)

```text
원본  6,515,998 bytes
압축  3,060,828 bytes   -53.0%
값 트리 동일                        True   (독립 json.load 비교)
"0.20000000298023224" 원문 보존      True
```

### 2.2 publish 실행

```text
powershell -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish

PASS: ARTIFACT-CHECK ... programs=13 rows=135 productMutation=false
PASS: published 99 Effects, 1 Components, visual-program sidecar
```

publish에 포함된 round-trip 검증이 통과했다.
`Publish-Effects.ps1:3010`의 `Get-Sha256Hex(sealed) == contentSha256`은 이제 compact payload를
검증하며, Python `validate_direct_authored_effect_runtime.py`도 함께 통과했다.

### 2.3 크기 결과 — catalog 참조 기준

```text
런타임 sealed 문서 합계   122.2 MB -> 54.8 MB   (-55%)
최대 단일 문서            6,363 KB -> 2,989 KB  (-53%)
```

class별 prewarm target 합계다. 프레임당 1개를 파싱하므로 최대값이 최악 프레임 비용이다.

```text
class             target   합계        최대
LanceMaster          41    28.0 MB    2.9 MB   (이전 60 MB / 6.2 MB)
Warlord              24    11.8 MB    2.5 MB
DimensionMaster      19    10.2 MB    2.3 MB
Artist               15     4.8 MB    1.1 MB
```

## 3. 수동 검증 — 미실행

```text
Character Select 진입 시 정지 구간이 실제로 줄었는지
GunSlinger / Slayer 진입 시 정지가 없는지 (target 0)
Effect Tool의 Publish + Reload Product Test 경로가 계속 동작하는지
```

## 4. 이번 변경이 만든 후속 항목

### 4.1 이전 publish의 고아 파일 98개

파일명이 content-addressed이므로 SHA가 바뀌면 새 파일이 생기고 이전 파일이 남는다.

```text
Client/Bin/DataFiles/Effect/Authored/   199개 파일
  catalog 참조                            98개   54.8 MB
  고아(이전 publish 산출물)               101개   약 122 MB
```

런타임은 catalog가 가리키는 것만 로드하므로 **동작에는 영향이 없다.** 디스크만 차지한다.
publisher에 고아 정리를 넣는 것이 정석이지만 이번 변경 범위에 넣지 않았다.
`Client/Bin/DataFiles`는 생성물이므로 재publish 전에 `Authored` 폴더를 비우고 돌리면 정리된다.

### 4.2 Artist target 1건이 catalog에 없음

Artist는 animevents cue target 15개 중 14개만 catalog에 있다.
이전부터 있던 상태이며 이번 변경과 무관하다. 별도 조사 항목이다.

## 5. 하지 않은 것

```text
4.2 전부 0인 distribution 제거 (5.7 MB, 18,267행)
    "distribution 부재"와 "전부 0인 distribution"이 propertyPath마다 동치인지
    Effect_Playback.cpp:302 의 miss 기본값 경로를 먼저 확인해야 한다.
    확인 없이 제거하면 표현이 조용히 바뀔 수 있어 이번 변경에 넣지 않았다.

4.3 provenance 문자열 분리 (5.3 MB)
4.4 문자열 비교 -> 컴파일된 인덱스
    Effect_Playback 전면 변경이며 실제 프레임 프로파일을 본 뒤 판단할 항목이다.

A안 loader 이동
    이번 변경으로 총량이 절반이 됐으므로 이동 비용도 절반이다. 다음 단계.
```
