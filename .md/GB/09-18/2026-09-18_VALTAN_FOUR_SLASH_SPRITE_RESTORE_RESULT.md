# 발탄 4연속 공격 사전 생성 Sprite 검격 복원 결과

## G00. 실제 반영 범위

실제 `VALTAN_FOUR_SLASH` SLASHES/SPIN cue가 소비하는 제품 carrier-v1 clip01/02를 수정했다.
Full Restore와 제품 문서는 별개이며, 기존 Full Restore stage008/009에는 이미 원본5요소가 있었다.
사용자가 전달한 타임라인은 추출 원본
`out/ValtanPriorityRestore20260915/MN_RPBF_00.all.action-effects.json`의 action420609과 일치했다.
시간만으로 carrier를 단정하지 않고 해당 시스템의 실제 SpriteParticle5종을 대조했다.

| 제품 문서 | 변경 | 원본 notify 창 |
| --- | --- | --- |
| `effect.valtan.carrier-v1.attack.four-slash.active.clip-01` | 기존7요소 유지, Sprite5추가 →12요소 | stage008 notify003,0.494731992초/3.005268097초 |
| `effect.valtan.carrier-v1.attack.four-slash.active.clip-02` | emitter27의 기존 stable ID 유지 복원, 나머지4추가 →6요소 | stage009 notify003,0초/2.896986008초 |

두 문서의 `valtan.clip01/02.source.atk-02-08` 그룹은 emitter21/14/27/25/26을 포함한다.
native2426/2458/3005/2326/2995, sourceRecipe, sourceParticle 크기·색·속도·수명,
StartControl/b_wp_r_01 attachment, socket TRS와 notify TRS를 해당 Full Restore에서 재사용했다.
clip02 emitter27은 `effect.standard`, sourceProfile.disabled, emissive0.05와 root attachment에서
원본 source material, emissive1, 무기 follow로 복원했다. 기존 ID는 `source.04edcf16319413095ac9`다.

기존 weapon-slash3개, hit-spark, fragments, FilmNoise, ZoomBlur와 clip02 mesh trail은
JSON 값과 원래 byte span을 보존했다. native Trails4개 추가 및 Atk_01_01의19개 전체 폭발은
이번 수정에 포함하지 않았다. 사용자가 지목한 사전 생성 검격의 두 occurrence만 복원했다.

## G01. 제품 본 연결과 단위

`Client/Private/Effect_PresentationService.cpp`에서 두 정확한 제품 ID의 StartControl을
기존 `Requires_SourceBoneImportScaleNormalization`에 포함했다. 기존 Full Restore와 같은
0.01 basis 정규화를 재사용하며, 요청 수집에서는 측정된 유령 발탄의 unit basis 허용도 연결했다.
다른 asset/slot에는 적용하지 않았다. Tool도 같은 helper를 사용한다.

설치된 `Character/Valtan/MN_RPBF_01.wmodel`과
`Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel`에서
mesh_att_battle_10_01/02를60Hz로 각각361회 샘플했다.
bodyPreScale0.0001, weaponPreScale100, 실제 combined bone basis0.01과 source socket을
함께 계산했다. 무기와 source notify 중심의 계산 차이는 최대1.89e-15m였다.
이는 파일 기반 CPU 수치 검증이며 실행 중 모델이나 GPU 화면 판정은 아니다.
제품 cue의 기존 GAMEPLAY_FOOTPRINT/worldScale1.5는 보존했다.
stage009의 원본 notify scale1.3은 별도로 유지한다.

## G02. 저장과 검증 증거

후보·원본 백업·설치 hash는 `out/ValtanFourSlashSprite20260918/`에 보존했다.
`prepare.py`가 최신 입력 hash를 확인하고 요소 단위 byte splice와 원자 교체로 두 문서를 반영했다.
무관한 쿠크 dirty 변경과 catalog/tree 등록은 수정하지 않았다. 데이터는 기존 catalog와
`CProjectDataRoot`를 통해 읽으며 새로운 asset 등록이나 project/filter 변경은 없다.

| 검사 | 결과 | 증거 |
| --- | --- | --- |
| 변경2문서 JSON 및 기존 material/color-space/module/attachment/native 옵션 validator | 통과 | `resource-validation.json` |
| runtime resource closure | 기존19파일,1,611,644bytes 존재 | 같은 파일 |
| codec Load/Drawable/Roundtrip/Stage | 8검사,0실패 | `codec.log` |
| 설치 실제 모델 기반60Hz playback6초와 rewind | 두 클립의5요소씩 모두 생성, 변환 finite, rewind 개수 일치 | `sampled-anchors.json`, `playback.log` |
| 수정 Effect_PresentationService.cpp 격리 컴파일 | exit0, 기존 header C4828경고 | `compile-service.log` |
| 사용 native particle CSO | 기존2304/2368/2432/2944구간 파일 존재 | 설치 Debug 폴더 확인 |
| Valtan presentation generation Validate | 144 artifact closure 통과 | `valtan-generation-validation.json` |
| 설치 hash·무관 기존 요소 byte span·git diff --check | 통과 | `manifest.json`과 최종 확인 |

clip01 최초 입자는 emitter21/14가0.5초,27/26이0.55초,25가0.583333초다.
clip02는 emitter21/14가0.016667초,27/26이0.066667초,25가0.1초다.
원본 notify 시작과 실제 첫 spawn tick을 구분한다. 두 클립 모두 peak 입자 수는
21/14/25=3,26=9,27=15다. emit 종료 이후의 원본 입자 tail도 유지했다.
화면의 색·형상·밀도·크기는 아직 사용자 확인 전이다.

## G03. 제품 빌드·publish 상태

정본 Product Debug runner를 실행했으나 `ProductOutputGuard.psm1`이 실행 중인
Client41868/Server28980을 감지해 compile 전에 중단했다. 사용자 프로세스는 종료하지 않았다.
수정 C++의 격리 compile 성공을 제품 EXE 설치 성공으로 기록하지 않는다.
정본 `Publish-GameplayBalance.ps1 -Mode Publish`는 선행 KoukuSaydon validation에서
`projected Product is stale: Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`으로
중단했다(exit1). 시작부터 존재하던 쿠크 Composition dirty 변경의 generated Product 불일치다.
이 세션은 쿠크 저작본이나 생성물을 덮어쓰지 않았다. `publish-gameplay.log`에 실패를 보존했다.
발탄 단독 presentation generation Validate는144 artifact closure를 통과했으며 세대는
`3f05694a2b161613eb9684fa159c6b0659a21154ef79e2cc9f3f592a1ccc60da`다.
이 값은 검증 결과이며 Server bootstrap 게시나 실행 중 Server 적용 완료를 뜻하지 않는다.

위 기록은 최초 구현 종료 당시 상태다. 후속 사용자 빌드로 EXE 반영 상태가 바뀌었다(G05).
당시 실행 중 Client에는 source-bone 보정이 없어 Reload만으로는 올바른 크기를 확인할 수 없었다.
공용 Gameplay Publish 실패를 이번 Effect 파일의 로드 차단 조건으로 해석한 이전 안내는
G05의 실제 consumer 재검토로 정정한다. Client/UI 실행·GPU draw/readback은 수행하지 않았다.

새 Resources와 새 shader는 없다. 다른 PC에는 기존 Effect/KoukuSaydon 및 Effect/Artist
텍스처와 기존 native particle CSO가 필요하다. 이번 작업에서 Drive 파일을 추가·교체하지 않았다.

## G04. 사용자 확인 경로

최신 Product 빌드의 Client를 사용한다. 이번 Effect-only 변경의 로드는 G05를 따른다.
F1 → Effect Tool → Core Server Patterns → 4연속 공격에서 SLASHES/SPIN의 제품 문서를
Reload/Open하고 재생한다. `Atk_02_08 | Sprite 21/14/27/25/26` 그룹을 확인한다.
SLASHES에서 첫 휘두르기1.6665초 이전인 약0.5초부터 도끼에 사전 생성 에너지가 있어야 한다.
SPIN에서는 시작부터 같은5종이 생성된다. Full Restore stage008/009 화면과 제품 문서를
혼동하지 않는다. 최종 시각 PASS는 사용자 판정 이후에만 기록한다.

## G05. 현재 EXE 반영 재검토

사용자 요청으로 실제 파일과 실행 프로세스를 다시 확인했다. Debug
`Effect_PresentationService.obj`는2026-09-18 12:36:35 KST,
`Client/Bin/Debug/Client.exe`는12:36:55 KST에 갱신됐다.
OBJ 안에 새 두 제품 asset ID가 존재하고, 제품 link.command tlog가 이 OBJ를 입력으로
Client/Bin/Debug/Client.exe를 출력했다. 현재 Client PID42028은 같은 경로에서12:36:56에 시작했다.
즉 최초 구현 당시의 빌드 차단 상태는 해소됐고 본 배율 보정은 현재 실행 EXE에 반영됐다.
이 검토에서는 에이전트가 Client를 실행하거나 UI를 조작하지 않았다.

설치된 두 제품 JSON의 SHA-256은 최초 검증·설치 후보와 동일하다. 원본 donor와10요소를
다시 대조하면 각 요소에서 id/groupId/displayName만 다르고 재질·sourceRecipe·시간·크기·
attachment·transform은 동일하다. donor는 정확히 다음 둘이다.

- `effect.valtan.action.420609.stage008.full.restore`: 표시명
  `Valtan 420609 / Main [8] full restore`, mesh_att_battle_10_01,3500ms.
- `effect.valtan.action.420609.stage009.full.restore`: 표시명
  `Valtan 420609 / Main [9] full restore`, mesh_att_battle_10_02,3167ms.

각 문서의 notify003/Par_O_RPBF_Atk_02_08 emitter21/14/27/25/26만 제품으로 옮겼다.
전체 Full Restore 교체가 아니며 추가 axe ribbon/worms, native Trails4개와 다른 notify는
이번 복원의 donor 범위에 포함하지 않았다.

현재 bootstrap의 게시 generation은40c44cd9…, 현재 디스크 generation은3f05694a…로 다르다.
그러나 `ValtanPresentationGenerationAdmission.cpp`의 현재 consumer는 packaged manifest의
hash/inventory를 local authoring admission gate로 사용하지 않고 현재 physical closure를 읽는다.
`Effect_Catalog.cpp`도 CProjectDataRoot에서 authored 파일을 연다. 따라서 이번 Effect-only 변경은
공용 Gameplay Publish 실패만으로 적용 불가라고 판단하지 않는다. 이전 답변에서 게시와 재시작을
무조건 필요한 단계로 안내한 부분을 정정한다. EXE·데이터 준비 완료와 실제 GPU 표시·사용자
시각 판정은 별개이며, 후자는 여전히 미확인이다. 증거는
`out/ValtanFourSlashSprite20260918/current-exe-audit.json`에 보존했다.

## G06. 사용자가 확인한 Full Restore의 실제 투명 Sprite 결함 수정 (2026-09-18)

사용자는 Effect Tool의420609 stage008/009 Full Restore에서 확인했다고 명시했다.
이전 G00~G05의 수정 대상은 carrier-v1 clip01/02다. 해당 코드가12:36 EXE에 들어간 사실은
Full Restore의 표시 개선을 의미하지 않는다. Full Restore는 원본5개 Sprite의 donor여서
그 작업에서 바뀌지 않았다. 이 차이를 분리하여 실제 문서와 Tool 소비 경로를 재조사했다.

### build_valtan_four_slash_axe_ribbon.py / 두 Full Restore 문서

확정한 별도 결함은 후속 저작된 axe worms Sprite36개다. orbit_track은 위치 곡선을 만들며
불필요한 alphaScaleKeys=[]도 기록했다. Effect_DocumentCodec_SourceRecipeIo의
Read_SourceTransformTrack은 해당 필드가 있으면 optional AlphaScale 분포를 생성한다.
빈 키의 기본값은0이고 Effect_Playback::Evaluate_Color가 이를 원본 alpha에 곱했다.
그 결과 stage008의27요소와 stage009의9요소는 생성·quad 계산이 정상이어도 전부 투명했다.

builder에서 alphaScaleKeys 출력을 제거했다. 최신 두 Authored 저장본에서는 stable ID가
valtan.worms.*인36요소의 빈 필드만 제거했다. 실제 diff는36개 필드와 그 앞 쉼표의 삭제이며,
원본 Atk_02_08 Sprite5개씩, 리본, 본 부착, positionKeys, clock, 재질, 사용자 편집값은 보존했다.
C++/shader/EXE와 전역 curve 기본값은 변경하지 않았다. carrier clip01/02에는 이 빈 알파
필드가 없으므로 추가 수정하지 않았다.

### 실제 설치 모델·Codec·Playback·최종 quad 검증

out/ValtanFullRestoreRuntime20260918은 현재 설치 WModel의 실제 본361샘플×2와 현재
Build_SourceBoneAnchorWorld 구현을 사용한다. Full Restore의 source owner scale1.4와
ARENA_ABSOLUTE effect root1을 구분했다. 현 제품 Codec/Playback/Geometry OBJ로
최종 CPU quad까지 검사했으며 Client/UI/GPU는 실행하지 않았다.

| 항목 | stage008 | stage009 |
|---|---:|---:|
| 수정 대상 axe worms 요소 | 27 | 9 |
| 대상 입자 프레임 수 | 15,915 | 5,521 |
| 유색·비투명 입자 프레임, 수정 전 | 0 | 0 |
| 유색·비투명 입자 프레임, 수정 후 | 15,693 | 5,440 |
| 전체 최종 quad 계산 실패 | 0 | 0 |
| 실제 본 중심 최대오차 | 9.8483e-7m | 9.57392e-7m |

나머지 일부 프레임의0 alpha는 원본 수명 곡선의 fade이며 추가로 제거하지 않았다.
메모리 A/B에서 변경된 항목은 alpha와 그에 따른 비투명 프레임 수뿐이다. 위치·크기·clock·
생성 수·원본5 Sprite 결과는 동일하다. 원본5 Sprite는 조사 전부터 유효한 색/alpha/quad를
생성했고 이36개 결함과 혼동하지 않았다. shader dispatch·DDS 채널·재질 활성 상태도 검사했지만
이것을 실제 GPU 시각 성공으로 기록하지 않는다.

준비한 데이터의 이전 hash/36 stable ID가 위 A/B 입력과 같음을 확인하고,
out/ValtanFullRestoreOpacity20260918/before에 백업했다. 교체 직전 raw bytes를 다시 비교하여
동시 저장을 보호하고 commit_library의 원자 교체·자기 변경 rollback으로 설치했다.
설치 후 프로브를 임시 알파 보정 없이 다시 실행했으며 검증된 A/B 수정 결과와 출력이 완전히
같았다. 생성자 위치 키 수·Python syntax, 두 JSON parse, 설치 hash, git diff --check도 통과했다.
격리 CPU 프로브 종료 시 라이브러리 정적 할당의 CRT leak dump가 남으므로 메모리 누수 검증을
완료했다고 주장하지 않는다. 커밋/푸시는 하지 않았다.

### 현재 실행 적용 경계

현재12:36 EXE에 필요한 Codec/Playback 동작은 이미 있다. 이번 데이터 수정은 재빌드나
Gameplay publish가 필요하지 않다. Effect Tool에서 각 Full Restore를 열고 Editing Session의
Load Saved를 눌러 새 저장본을 읽은 뒤 Play All로 확인한다. Restart Preview만 누르면 이미
메모리에 열린 이전 문서를 다시 재생할 수 있다. 미저장 draft를 버리는 Reload/종료는 자동으로
하지 않았다. 사용자의 최종 화면 판정은 대기 상태다.
