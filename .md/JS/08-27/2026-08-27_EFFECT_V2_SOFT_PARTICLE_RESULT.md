# Effect V2 소프트 파티클 결과

대응 계획서는 `2026-08-27_EFFECT_V2_SOFT_PARTICLE_PLAN.md`다.
Effect Tool V2로 저작한 이펙트가 지형·메쉬와 교차할 때 depth test가 남기는 절단선을
씬 깊이 기반 알파 페이드로 없앴다. 적용 범위는 V2 경로 하나이며 제품 V1 경로
(`CEffectDocumentRenderer` + `Shader_VtxEffect*.hlsl`)는 건드리지 않았다.

branch `feature/effect-v2-soft-particle` (origin/main `298aa47b` 기준).

## 1. 구현 완료

계획서의 G01~G06을 전부 반영했다. 12개 파일, +54/-3.

```text
Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli   +11
  g_SoftFadeDistance uniform, g_DepthTexture 선언
  PS_EFFECT_IN 에 float4 vProjPos : TEXCOORD2
  PS_EFFECT_V2 의 알파 확정 직후, 기존 discard 직전에 페이드 블록

Client/Bin/ShaderFiles/Shader_EffectMeshV2.hlsl        +2   VS_MAIN, VS_OUTLINE
Client/Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl    +2   VS_MAIN, VS_OUTLINE
Client/Bin/ShaderFiles/Shader_EffectRectV2.hlsl        +1
Client/Bin/ShaderFiles/Shader_EffectParticleV2.hlsl    +1
Client/Bin/ShaderFiles/Shader_EffectTrailV2.hlsl       +1
  전부 output.vProjPos = output.vPosition; 한 줄

Client/Bin/ShaderFiles/Shader_EffectDecalV2.hlsl      +1-1
  중복이 된 Texture2D g_DepthTexture; 삭제
  effectInput.vProjPos = float4(0,0,0,0) 으로 페이드 opt-out

Client/Public/EffectV2_Object.h                        +1   PARAMS::fSoftFadeDistance
Client/Private/EffectV2_Object.cpp                    +6-1
  Bind_Common 에 g_SoftFadeDistance, 값이 0 보다 클 때만 Target_Depth SRV 바인딩
Client/Private/EffectV2_Document.cpp                   +7   read / 음수 거부 / write
Client/Private/Effect_Tool_V2.cpp                     +10   Soft Fade 슬라이더 + 안내 문구
```

### 1.1 확정된 계약

```text
params.softFadeDistance   월드 단위 거리. 0 이 "끔". 기본값 0.
formatVersion             1 유지. Read_Number 가 키 부재를 허용해 기존 문서 61 개가 그대로 통과.
데칼                       vProjPos.w == 0 으로 셰이더 레벨에서 opt-out. Tool 슬라이더도 비활성.
깊이 SRV                   fSoftFadeDistance > 0 인 이펙트만 Target_Depth 를 묶는다.
아웃라인                   PS_OUTLINE_V2 는 페이드 대상이 아니다.
```

### 1.2 설계상 남긴 판단

`Bind_Common`의 `Target_Depth` 바인딩을 조건부로 둔 이유는 실패 반경 때문이다. 무조건
묶으면 깊이 타깃 조회가 실패하는 상황에서 소프트 페이드를 쓰지 않는 이펙트까지 전부
`E_FAIL`로 끌려간다. 현재 저작 문서 61개가 전부 0이므로 사실상 모든 이펙트가 사라지는
회귀가 된다. 셰이더 분기 조건(`g_SoftFadeDistance > 0.f`)과 같은 조건을 써서 둘이
어긋날 여지도 없앴다.

Tool 슬라이더를 Depth Test 체크박스 바로 아래가 아니라 Blend section 맨 끝에 둔 이유는
그 뒤의 decal 안내 문구와 Billboard 체크박스가 `ImGui::SameLine()`으로 Depth Test 줄에
붙기 때문이다. 사이에 위젯을 끼우면 그 `SameLine`들이 새 위젯 줄로 옮겨붙어 기존 배치가
깨진다. 계획서 초안의 위치가 틀렸고 반영 전에 고쳤다.

## 2. 실행한 자동 검증

```text
Client x64 Debug ClCompile + Build/Link
  -> Client\Bin\Debug\Client.exe 생성, MSBuild exit 0, error 0
  -> warning C4819 는 Shared/Public/Network/*.h 의 기존 인코딩 경고이며 이번 변경과 무관
git diff --check
  -> 공백 오류 없음
Valtan 데이터 원복 확인
  -> git status 에 Data/Valtan/ 없음
```

빌드에는 아래 3.1의 우회가 필요했다. 우회 없이는 컴파일이 시작조차 되지 않는다.

## 3. 검증 중에 발견한 별건 — main Client 빌드 차단

**이번 변경과 무관하며, origin/main 자체가 깨져 있다.**

`Client/Default/Client.vcxproj:3031`의 `ValidateValtanSplitProducts` 타깃이
`BeforeTargets="ClCompile"`로 걸려 있고, 이 검증이 실패한다.

```text
phase-2/3 animation intake must exact-join manualAuditions
  manual (Data/Valtan/Valtan.gameplay.json)             20 개
  debug  (Data/Valtan/Valtan.presentation.debug.json)   22 개
  차이: 'dead', 'respawn'
```

원인은 2026-08-26 머지된 PR #233의 `341cc85a data(valtan): author the phase 3 respawn and
dead chains`다. 해당 PR 본문이 "Validate still fails, and the remaining step is a data
decision rather than a format one"이라고 미해결로 남긴 항목인데, 이 validator가 Client
빌드의 사전 조건이라 **현재 origin/main에서 Client x64 빌드가 전면 차단된다.**

`manualAuditions`에 두 chain을 넣을지, 아니면 debug 문서에서 뺄지는 애니메이션 의도
결정이라 이번 변경에서 손대지 않았다. 소유자 판단이 필요하다.

### 3.1 이번 빌드에서 사용한 우회

`Data/Valtan/`을 PR 직전 커밋 `382237ae` 상태로 되돌려 빌드하고 즉시 `origin/main`
상태로 복원했다. 저장소에 남은 변경은 없다. 이 우회는 검증용 일회성이며 커밋 대상이
아니다.

### 3.2 빌드 환경 메모

`python`이 PATH에서 Microsoft Store 스텁
(`%LOCALAPPDATA%\Microsoft\WindowsApps\python.exe`)으로 먼저 잡혀 Valtan 파이프라인이
exit 9009로 죽는다. 실제 설치는 `C:\Program Files\Python312`이며 이 경로를 PATH 앞에
붙이면 정상 동작한다. Visual Studio에서의 빌드에는 재현되지 않을 수 있고, 다른 셸이나
자동화에서 9009가 나오면 이 원인이다.

## 4. 사용자 화면 확인

Server(7777 LISTEN) 위에서 Client를 띄우고 `esther.thirain.lumen_1`을 기준 케이스로
사용했다. 이 문서는 Mesh shape에 40도 기울어진 원기둥, Additive, `depthTest: true`,
`bloomIntensity 10.0`, 수명 1초 동안 알파 1 → 0이라 절단선이 가장 심하게 드러난다.

```text
사용자 관찰 (2026-08-27): "잘 나오는듯"
```

`AGENTS.md`의 사용자 전용 화면 검증 경계에 따라, 이 관찰은 개선이 보인다는 사용자의
서면 확인이다. 아래 항목은 아직 서면 판정을 받지 않았으므로 `visual PASS`나 occurrence
승인으로 기록하지 않는다.

```text
[미확인] 확정 softFadeDistance 값
[미확인] 데칼 회귀 (슬라이더 비활성, 이전과 동일한 렌더)
[미확인] Blend section 배치 회귀 (Depth Test / Billboard 같은 줄 유지)
[미확인] 저장 왕복 후 softFadeDistance 키 생성과 값 보존
[미확인] 저작 문서 61 개 전부 로드
```

## 5. 남은 경계

```text
제품 V1 경로는 범위 밖이다. 같은 증상이면 Shader_VtxEffect*.hlsl 계열에 별도로 필요하다.
Target_Depth 는 불투명 지오메트리만 담으므로 이펙트끼리 겹치는 경계는 부드러워지지 않는다.
bDepthTest == false 인 이펙트에 Soft Fade 를 켜면 지형 뒤에 숨는 동작이 새로 생긴다.
  막지 않고 Tool 에서 경고 문구만 띄운다.
해상도 변경 시 Target_Depth 재생성 경로는 이번 변경이 건드리지 않았고 확인하지도 않았다.
```

## 6. 다음 단계

```text
1. lumen_1 의 확정 softFadeDistance 를 문서에 반영
2. 4절의 미확인 항목 확인 후 이 RESULT 갱신
3. main Client 빌드 차단(3절) 처리 방향 결정 - 팀 공유 또는 별도 브랜치
4. 커밋: 셰이더 6 + C++ 4 + PLAN/RESULT 를 한 단위로.
   Save 왕복으로 "softFadeDistance": 0 한 줄만 늘어난 저작 문서는 커밋하지 않는다.
```
