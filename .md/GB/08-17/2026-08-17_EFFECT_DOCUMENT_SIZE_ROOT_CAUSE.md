# 2026-08-17 Effect 문서 크기 병목 근본 원인

`.md/GB/08-17/2026-08-17_EFFECT_PREWARM_STALL_DIAGNOSIS_AND_OPTIONS.md`의 B안 조사 결과다.
전부 정적 실측이며 코드는 아직 바꾸지 않았다.

## 1. 근본 원인

**저작 증거 포맷이 곧 런타임 포맷이고, 그 사이에 컴파일 단계가 없다.**

원본 Unreal Cascade 모듈 덤프(`stableId` / `className` / `objectPath` / `literals` /
`distributions`)가 그대로 런타임 문서에 실려 있고, publisher는 이를 **바이트 단위로 그대로 복사**한다.
런타임은 그 원문을 문자열 비교로 매번 해석한다.

```text
Effect_Playback.cpp:286   Module.strClassName == pClassName
Effect_Playback.cpp:302   Distribution.strPropertyPath == PropertyPath
Effect_Playback.cpp:737   Literal.strPropertyPath == PropertyPath
Effect_Playback.cpp:352   Module.strClassName != "particlemodulelocationdirect"
Effect_Playback.cpp:567   Modules 선형 탐색으로 typeData 모듈 식별
```

즉 런타임이 매번 모듈 배열을 선형 탐색하며 `"particlemodulelocationdirect"` 같은 문자열을 비교한다.
Track A 복원이 원본 값 보존을 우선하면서 생긴 구조이고, 그 값이 그대로 제품 경로에 남았다.

## 2. 실측 — 259개 저작 문서 전수

```text
files                    259
on disk                  210.9 MB
compact serialized        97.6 MB   (들여쓰기 공백 113.4 MB = 54%)

sourceRecipe              60.5 MB   (compact의 62%)
  distributions           35.4 MB   (83,320 행)
     of which lut/keys     9.0 MB
     전부 0인 no-op         5.7 MB   (18,267 / 83,320 행 = 22%)
  literals                13.4 MB   (193,615 행)
  stableId + objectPath    5.3 MB   (55,477 모듈)
```

런타임 산출물도 같은 규모다.

```text
Client/Bin/DataFiles/Effect/   101개   122.2 MB
최대 단일 파일                  6,363 KB
```

**저작본과 런타임 파일의 바이트가 동일하다.** 6,363 KB 파일이 양쪽에 그대로 있다.

## 3. publisher가 그대로 복사한다는 근거

```text
Tools/EffectPipeline/Publish-Effects.ps1:791
    $payload = [IO.File]::ReadAllBytes($Record.SourcePath)
Tools/EffectPipeline/Publish-Effects.ps1:793
    $actualSha256 = Get-Sha256Hex $payload      # 저작 원본 SHA
Tools/EffectPipeline/Publish-Effects.ps1:833
    [IO.File]::WriteAllBytes($destination, (Read-PinnedDirectAuthoredBytes $record))
```

publish는 변환이 아니라 **무결성 핀이 걸린 복사**다. 런타임 파일명에 들어가는 SHA도 저작 원본의
SHA이며, 런타임의 `DIRECT_AUTHORED_RUNTIME_SOURCE::strContentSha256`이 이를 검증한다.

이것이 이번 수정의 유일한 실질 제약이다. publish 산출물을 변형하면 "런타임 파일 == 저작 파일"이라는
현재 무결성 계약이 깨지므로, 저작 SHA(출처 핀)와 런타임 payload SHA(무결성)를 분리해야 한다.

## 4. 수정 사다리

가치/위험 순이다. 1번만 해도 절반이 사라진다.

### 4.1 publish 시 compact 직렬화 — 113.4 MB (54%)

의미 변화 0. 저작본은 git diff와 툴 편집을 위해 계속 pretty로 두고, publish 산출물만 compact로 쓴다.

```text
효과   런타임 122.2 MB -> 약 56 MB
       LanceMaster 41 target 60 MB -> 약 28 MB
       프레임당 평균 1.5 MB -> 0.7 MB, 최악 6.2 MB -> 2.9 MB

필요   Publish-Effects.ps1이 payload를 재직렬화해 쓰고 그 결과를 해싱
       catalog record에 sourceSha256(출처 핀)과 payloadSha256(런타임 무결성) 분리
       Effect_Catalog의 DIRECT_AUTHORED_RUNTIME_SOURCE가 payloadSha256을 검증

위험   낮다. JSON 의미가 동일하므로 파서 변경이 없다.
       계약 변경이 3개 파일에 걸친다는 점이 유일한 비용이다.
```

### 4.2 전부 0인 distribution 제거 — 5.7 MB, 18,267행

`lookupTable`도 `keys`도 없고 `defaultMinimum`/`defaultMaximum`이 전부 0인 distribution은
값을 하나도 운반하지 않으면서 행당 약 330 byte를 쓴다. 전체 distribution의 22%다.

```text
필요   런타임이 "distribution 부재"와 "전부 0인 distribution"을 같게 취급하는지 확인
       Effect_Playback.cpp:302의 propertyPath 탐색이 miss일 때의 기본값 경로 검증
위험   중간. 위 동치가 성립하지 않는 propertyPath가 하나라도 있으면 표현이 바뀐다.
       제거 전에 propertyPath별로 miss 기본값을 확인해야 한다.
```

### 4.3 provenance 문자열 분리 — 5.3 MB

`stableId`("FX_PC_FLM_06:export:2771@ref:0")와 `objectPath`
("FX_PC_FLM_06.par_v_flm_chehuexp_ss_68.particlemodulerequired_91")는 출처 추적용이다.
다만 `Effect_Playback.cpp:567-574`가 adapter packet identity 검증에 실제로 사용한다.

```text
필요   해당 identity 검증이 필요한 모듈만 남기고 나머지는 저작본에만 두는 분리
위험   중간. 검증 경로를 잘못 끊으면 adapter packet 승인이 조용히 통과할 수 있다.
```

### 4.4 문자열 비교 -> 컴파일된 인덱스

런타임이 매번 `className` / `propertyPath` 문자열을 선형 비교하는 구조 자체를 없앤다.
publish 시점에 모듈 종류와 property를 enum/인덱스로 확정한다.

```text
효과   parse 비용과 별개로 playback의 프레임 비용까지 줄어든다.
위험   높다. 가장 큰 설계 변경이며 Effect_Playback 전반을 건드린다.
       1~3번을 끝낸 뒤 실제 프레임 프로파일을 보고 판단할 일이다.
```

## 5. 권고 순서

```text
1) 4.1 compact publish        절반 감소, 의미 변화 없음, 지금 바로 가능
2) 예측 검증                  GunSlinger 진입 시 정지 없음 확인 (target 0)
3) 4.2 no-op distribution     propertyPath별 기본값 확인 후
4) A안 loader 이동            여기까지 하면 남은 총량이 절반 이하라 로딩 증가폭도 절반
5) 4.3 / 4.4                  실제 프로파일 수치를 보고 결정
```

4.1을 먼저 하면 A안(로딩 화면 이동)의 비용도 같이 절반이 된다.
반대로 A안을 먼저 하면 120 MB를 로딩 화면 뒤로 옮기기만 한다.

## 6. 이번 회차에 하지 않은 것

```text
코드 변경 없음. 빌드 없음.
4.1의 SHA 계약 분리는 publisher / catalog schema / catalog parser 3곳을 함께 바꾸는
변경이라 사용자 확정 후 진행한다.
런타임 프레임 프로파일(parse vs GPU vs playback 문자열 비교 비중)은 측정하지 않았다.
4.4 판단에는 필요하지만 4.1~4.3에는 필요하지 않다.
```
