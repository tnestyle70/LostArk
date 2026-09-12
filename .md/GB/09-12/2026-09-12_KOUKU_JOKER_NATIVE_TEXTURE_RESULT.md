# 쿠크 조커 카드 native texture 연결 결과

## G00. 실제 반영 상태

`Data/Actors/BossCatalog.json`에서
`Character/KoukuSaton/MN_RHOC_00-1/MN_RHOC_00-1.wmodel` 모델 행의 네 문자열을 교정했다.
sourceMaterial은 `mn_rhoc_00-1.mat.mn_rhoc_00-1_mi`를 사용하고 native texture 0/1/2는
설치된 `Character/KoukuSaton/MN_RHOC_00-1/textures/mn_rhoc_00-1_{n,d,s}.dds`를 사용한다.
실제 변경은 sourceMaterial 1개와 texture asset ID 3개다. 일반 카드 행, cooked materialName,
family, 36개 parameter, texture index·색 공간과 공통 texture 3..6은 유지했다.

조커의 native material에 일반 카드 texture가 연결된 소스 결함은 확인했다. 사용자 화면에서
앞면 이미지가 최종적으로 정상인지, 별도 색·alpha 문제가 남는지는 아직 확인하지 않았다.
Client/UI를 실행하거나 조작·캡처하지 않았다.

## G01. 기존 기록과 현재 원인

[09-07 리소스 대장](../09-07/2026-09-07_KOUKU_WORLD_OBJECT_RESOURCE_LEDGER.md)은
조커의 원본 `MN_RHOC_00-1_MI`, 별도 WModel과 variant D/N/S를 기록한다.
[09-11 보스·소품 재질 결과](../09-11/2026-09-11_KOUKU_BOSS_PROP_NATIVE_MATERIAL_IMPLEMENTATION_RESULT.md)는
native 입력을 ActorCatalog/CModel로 연결하고 당시 33모델·50slot load/bind 검증을 기록한다.
그 결과는 조커 앞면의 원본 그림 판정이 아니다. 같은 결과의 `MN_RPCZ_00-1`에는 cooked
slot 이름과 source MIC가 다른 variant를 교정한 기록이 있지만, 조커 행에는 일반 카드 입력이
남아 있었다.

잘못된 조커 행은 `359412c46d12fd5a0df3155045d6468a01acfd12`에서 추가됐고, 같은 변경에서
WorldSequencePlayer가 ActorCatalog의 native descriptor를 소비하기 시작했다. 현재
worldsequences의 조커 모델 ID와 애니메이션 대상은 variant를 가리키며 명시 materialProfile이나
diffuse override가 없다. 따라서 이 경로에서 BossCatalog의 일반 카드 입력이 실제 우선한다.

현재 native effect 복원 데이터의 조커 MIC는
`Data/Effects/Authored/effect.kouku.source.fx_mn_rpct_06_x.par_x_rpct_cast_03_01.effect.json`의
첫 element에 있다. 그 source profile의 24 scalar·12 vector는 현재 BossCatalog와 정확히 같다.
조커와 일반 카드의 원본 pixel shader도 `e5ff54c5c354204e951b91b524341cb3`으로 같다.
Effect texture는 variant D/N/S를 참조하며 기존 Character variant 파일과 각각 byte hash가 같다.
이 Effect profile은 비교 근거이고 World Object를 Effect runtime으로 옮기는 변경은 없다.

## G02. 실제 소비 경로와 데이터 소유자

| 실제 코드 | 이번 데이터의 소비 |
|---|---|
| `Client/Private/ActorCatalog.cpp:39` | `CProjectDataRoot::Resolve`로 Data 정본을 읽는다 |
| `Client/Private/ActorCatalog.cpp:187` | 모델·slot identity, family parameter, 필요한 texture index 및 색 공간을 검사한다 |
| `Client/Private/ActorCatalog.cpp:917` | 정확한 모델 asset ID로 shared boss material override를 descriptor에 담는다 |
| `Client/Private/WorldSequencePlayer_Objects.cpp:155` | World Object 모델 생성 전에 ActorCatalog descriptor를 받는다 |
| `Engine/Private/Model.cpp:1708` | cooked materialName이 같은 slot에 native surface와 texture 입력을 적용한다 |
| `Engine/Private/Material.cpp:514` | sourceCharacterTextures에 지정된 실제 DDS를 준비한다 |
| `Engine/Private/Material.cpp:905` | 준비된 texture를 `g_SourceCharacterTexture<index>`에 바인딩한다 |
| `Client/Bin/ShaderFiles/Shader_SourceCharacterPrograms.hlsli:34667` | program 26 Base가 expression 1의 RGBA를 읽고 alpha 약 1/3 미만을 discard한다 |

Light26도 같은 native diffuse·alpha 입력을 사용한다. `diffusecolor=[1,1,1,1]`과 상태·색
parameter는 원본과 일치하므로 보라색 곱셈을 추정해 shared shader나 parameter를 바꾸지 않았다.
`Shader_VtxAnimMeshBinary`는 native program이 활성화되면 해당 평가 결과를 사용하므로
일반 `g_DiffuseTexture` 입력을 바꾸는 것으로 이 결함을 고칠 수 없다.

BossCatalog는 authored Data 정본이고 이 모델 행의 추적된 전용 재생성 writer는 없다.
Effect native 생성기는 별도 profile의 소유자여서 변경하지 않았다. CProjectDataRoot는
`LOSTARK_PROJECT_DATA_ROOT`가 있으면 해당 Data root를, 없으면 실행 파일의 상위 Data 폴더를
찾는다. 별도 publisher·DataFiles·Resources 설치 변경이 필요 없는 기존 경로다. 사용자 Client의
카탈로그·모델 prototype을 새 데이터로 다시 만들려면 Client를 재시작한다.

## G03. 실행한 검증

기준 HEAD는 `2f289461461434bcc0442bae5a71b795e7b34c62`다. 일회성 Python 데이터·바이너리 읽기로
다음 검사를 실행했고 모두 통과했다. 새 validator나 harness 파일은 추가하지 않았다.

| 검사 | 결과 |
|---|---|
| 수정 JSON parse | 성공 |
| HEAD 대비 JSON semantic diff | 조커 override의 sourceMaterial·texture asset ID 3개, 정확히 4 leaf |
| 나머지 BossCatalog root·모델·parameter·texture 필드 | semantic 동일 |
| 필요한 native texture | 7/7 unique index 0..6, Resources 상대 경로·DDS magic·물리 파일 존재 확인 |
| 색 공간 | N linear, D/S srgb, 공통 3..6 기존 값 유지 |
| 설치 WINT/WMA2 material | 1 slot, `mn_rhoc_00_mi` 일치 |
| WModel 내장 D/N/S와 override | 내장 D→expression 1, N→0, S→2 경로 일치 |
| 조커 source profile parameter | 36/36 동일 |
| 조커·일반 카드 원본 native shader | source VF/VS/PS identity 동일 |
| Character variant와 Effect variant D/N/S | 세 파일 byte hash 동일 |

수치 기록은 `out/KoukuJokerNativeTexture20260912/input_verification.json`에 있다.
이 파일은 이번 읽기 검증 증거이며 runtime manifest나 Resources 배포 조건이 아니다.
처음 일회성 binary 검사에서 중첩 WINT의 16-byte header를 WMA2 magic 위치로 잘못 읽어
assertion이 실패했다. 실제 중첩 WINT/WMA2 header 위치를 대조한 뒤 검사 offset을 바로잡았고,
위 검사를 다시 실행해 통과했다. 제품 파일이나 WModel은 이 과정에서 수정하지 않았다.

전체 작업 트리의 `git diff --check`는 exit 0으로 통과했다. 새 C++·shader·schema 파일이 없어
project/filter 등록은 필요 없다. JSON만 수정했으며 별도 Client/Engine 빌드·EngineSDK 복사·DLL
배포는 하지 않았다. 통합 담당의 Sequencer 수정 빌드와 이 검증을 구분한다.

## G04. 미실행 검증과 사용자 확인

ActorCatalog→CModel→CMaterial 소비는 실제 소스로 확인했다. 09-11 결과가 참조한
`out/KoukuBossNative20260911/catalog_probe.exe`가 이 PC에는 없어 이번 변경 후 실제 C++
catalog load·CModel 생성·GPU native bind를 실행한 것으로 기록하지 않는다. Data parse와
WModel input 비교는 실제 C++ decoder나 렌더링 성공을 대신하지 않는다.

사용자는 Client 재시작 후 KoukuSaydon 아레나의 F1 → Object Tool에서
`world.object.kouku.joker_card`(조커카드)의 `Preview Default`를 누르고, 조커 찾기 패턴에서 앞면 그림을 확인한다.
판정·HP·스케일·애니메이션과 카드 생성 시점은 이번 데이터 수정 범위가 아니다. 카드 사전 생성,
Sequencer 전투 handoff, 인형 변형, 외곽불의 색·개수·배치는 각 후속 작업에서 다룬다.
