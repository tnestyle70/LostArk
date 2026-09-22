# 주사위 속박·무력화·백스텝 복원 결과

## G00. 현재 상태

코드 변경과 원본 기반 이펙트 후보 9개를 작성했다. 후보는 `out/KoukuPatternRestore20260922/manifest.json`에 SHA 및 stable-ID/field guard와 함께 기록했다. 이 담당 작업에서 live Composition/Catalog/Tree/published DataFiles를 교체하지 않았다. 통합 담당이 최신 저장본에 병합·설치·publish하고 제품 빌드를 확인한다. Client/UI는 실행하지 않았다. 사용자 화면 확인은 미완료다.

## G01. 실제 원인과 변경

| 범위 | 확인한 원인 | 변경 |
|---|---|---|
| 주사위 속박 바닥 | Server의 `isPatternBound`와 deadline은 도착하지만 바닥 이펙트 소비자가 없었다. 원본 Spotlight01의 지속 문양은 35초 particle lifetime으로 기존 30초 Detail 검사에도 걸렸다. | 서버 상태를 기존 `Sample` 세션으로 소비한다. 바닥11요소, 정상 해제9요소. 사망·퇴장·Reset은 즉시 정리한다. Source recipe만 bounded 120초를 허용하고 수동 particle은 30초를 유지한다. UI 편집 상한도 동일하게 연결한다. |
| 카드 준비/출력 | 원본 stage1의 14요소가 없고 stage2는 Sk08의 6요소만 남아 있었다. 추가 Sk08_2의 Beam2는 과거 action projection이 particle/sprite로 분류했다. | 원본 준비14, 출력19 요소를 만든다. Beam2 두 요소는 원본 leaf의 trail/beam/Detail Trail을 보존한다. 기존 출력4회의 사용자 시각·위치·크기는 유지하고 resource만 교체한다. |
| 주사위 본체/폭발 | 이미 53개 원본 요소와 실제 리그 기반 비행 보정·사용자 1.5배 크기가 있었다. | 기존 저장 요소를 보존한 full.restore 문서를 등록한다. 기존 충돌 카드폭발7요소도 native 검사했다. |
| 무력화 | 현재 P1의 마지막 폭발은 Showtime bomb blue였고 별 그룹 smoke5개는 임시 native2992였다. 방패 지속3요소는 생략된 Required EmitterLoops 기본값0을1로 투영해 6초에 종료됐다. | 원본 Shield01 10요소, 별28요소, Sk12_5 무지개폭발38요소. 별 smoke는 원본 native3008로 복원하고 통합 담당이 정확한 sprite admission만 추가했다. 방패 원본 loop0을 복원하고 occurrence의 10.684초가 수명을 소유한다. 임시 V2 중복18행만 제거하고 원본 외곽 경계/레이저/Collider/음원은 유지한다. |
| Backstep 링 | P102의 7개 MAP 링이 G3의 Z930~950m에 남아 있었다. | canonical Gameplay의 G1−G3 중심 차이 `[2.999999970665357e-10, -4.9999999918171056e-08, -204.80001700000003]`m를 xyz에 더한다. 간격·회전·크기·타이밍은 그대로다. |
| Backstep 잔상 | 공통 NPC 잔상은 돌진22번 clip만 처리했다. source5ms float를 Python decoder가 `.005` 경계에서 거절하기도 했다. | 34_04 clip의 원본0~50ms notify에 현재 CNpc/CModel 자세를 저장한다. source5ms/500ms/sourceIntensity1과 요청한 흰 반투명 exposure를 적용한다. style은 방출이 끝나도 tail 동안 유지한다. decoder는 float 오차 1e-8만 허용한다. 별도 원본 모델 잔상 문서도 라이브러리 후보로 제공한다. |

속박/방패 버프 CEFParticleData의 원본 TRS는 position0/rotation0/scale1, parameter override 없음이다. `FX_Buff_01` 이름을 각 플레이어의 실제 본으로 실측한 것은 아니다. 현재 바닥의 actor-origin translation 적용을 명시적 ground projection으로 기록한다. Backstep의 흰색/rim/fade appearance는 **PROJECT_AUTHORED**이며 원본 shader ABI 복원 완료라는 뜻이 아니다. NPC 잔상은 Product의 실제 소유자 경로에 연결했고, 별도 모델 cue 후보는 Effect 라이브러리에서 확인하는 경로다.

## G02. 검증 증거

- `native_result.log`: 현재 Codec 5개 TU를 `out`의 독립 obj로 컴파일하고 실제 제품 Playback을 링크했다. 9개 문서 Stage/Validate/Playback, 총 **66,299** particle/light/trail 검사, Beam **332** point, finite/capacity 실패0. Native3008 sprite 별 선의 CPU 입자 평가도 통과했다. 이 검사는 GPU carrier 허용을 검증하지 않았으며, 이후 G05 재감사에서 실제 shader 누락을 발견·수정했다. Source lifetime120초 허용, 120.001초 및 non-source35초 거절을 확인했다. 기존 max particle2048와 finite 검사는 유지했다. 원본 emitter duration600초 경계 및 source preview600초 경계 안에서 실제 속박 document71.1초가 통과한다.
- `anchor-evidence.json`: 설치 `MN_RPCT_05.wmodel` SHA `d4d10a7334e1965085a5559173968bafc110ca22e85373c9e03563fa3af3c0f0`, Catalog preScale0.017, 실제 `b_wp_1`/`bip001-l-hand` 총816개 bone sample을 기존 socket TRS에 연결했다. cooked root100배를 정규화하지 않았다.
- `model_result.log`: 창 없는 D3D WARP/CModel probe. 실제 Backstep 1.6초 clip의 0, 1/60, 2/60, .05, .5초에서 5mesh×840bone, 총4,200 palette 행렬 유한. 설치된 animated shader pass14, diffuse texture와 sourceIntensity1 바인딩 통과. 픽셀 색/화면 방향 판정은 하지 않았다.
- `card-impact-probe/native-summary.csv`: 기존 카드 충돌 폭발7요소, peak29, 1,120 sample, 실패0.
- `source/buff-receipts.json`: 원본 LOA SHA와 CEFParticleData 실제 suffix decode 및 원본 byte offset. decoder에 진입하기 위한 합성 외부 notify header는 원본 증거로 사용하지 않았다.
- `source/backstep-ghost-receipt.json`: Action4219951/stage004/notify003 원본 field decode. MN_RPCT_07 LOA SHA `815959aa8ef30d8500eeec74b87adf44682380f115526ddc48ca04ad323f91f7`.
- `stable-merge-validation.json`: revision2193 (`587576d685fe6fe85c2f4917a5ab7876326722cf3247ac8bdc6fd16934c50392`) 기반 semantic candidate2194. guard18, occurrence추가1, 임시행삭제18, resource추가9. P1/P78/P102만 변경하며 나머지94개 pattern 불변. 기존 projector `validate_document` 통과. live 파일 불변. 통합 설치 전이므로 projected output freshness/publish 완료를 주장하지 않는다.
- Python compile과 해당 변경 `git diff --check` 통과. 제품 C++ 빌드는 통합 담당 결과에 기록한다.

## G03. 통합 시 주의와 남은 확인

P78 새 준비 occurrence는 현재 next ordinal33을 사용하고 counter를34로 늘리는 guard를 포함한다. 오디오 담당의 추가4행도 같은 저장본에서 출발하므로 통합 담당은 새 행의 ordinal을 순차 배정하고 관련 counter만 조정해야 한다. 기존 행의 stable ID를 재배치하지 않는다.

무지개 폭발 원본의 document tail4.2초를 보존하려고 P1 끝은17,161ms가 된다. 발사 카드4회의 기존 occurrence duration은 사용자 저장값을 유지한다. 원본 연결/유한 geometry 검증과 최종 사용자 화면 판정은 별개다. 사용자 화면에서 속박 원형 문양·버블, 보라 방패의 방향/지속, 별 선과 무지개 폭발, G1 링 위치, 이전 위치의 흰 잔상을 확인해야 한다.

## G04. 공통 문서와 GBResources 전달

통합 담당이 Npc/PresentationPlayer/MaterialDetail을 포함한 Client6개 TU의 scratch compile PASS를 전달했다. 제품 링크·publish 결과와는 구분한다. 반복 결함 원칙을 `gotchas.md`와 `렌더링이펙트복원V2.md`의 끝에 반영했다.

사용자 승인에 따라 `C:/Users/user/Desktop/GBResources`에 이번 리소스24개, **351,616,235바이트**를 추가했다. Clown 적용 manifest의 Resources9개, RaidAudio의 notice01 제외11개 WAV, CardRain의 UI PNG2개, 기존 설치본과 사용자 원본 SHA가 일치하는 Bern/Valtan WAV2개다. 모든 원본 manifest SHA와 전달본 SHA/크기가 일치한다. 기존373개 payload와 기존 폴더를 보존했고 README의 잘못된 Resources 래퍼 설명만 교정하고 이번 전달 설명을 추가했다.

최상위는 기존 Character/Effect/Sound/UI를 유지한다. `Resources/` 래퍼를 생성하지 않았다. 새 `manifest-2026-09-22-kouku.json`은 파일별 상대경로·크기·SHA256·출처를 기록하며 SHA는 `0c245f9d831d1113dca9b0b9ba3edd58dc1fc49802c7ef2142286cbbcc8cf218`이다. 전달 기록은 `out/KoukuPatternRestore20260922/gbresources-package-result.json`에 두었다. 이 복사는 코드·DataFiles 설치나 실행 중 세션 갱신을 뜻하지 않는다.

## G05. 현재 설치본 재감사와 GPU carrier / Tool 잔상 보완

초기 CPU·모델 팔레트 검증만으로 GPU 표시 완료를 판단하지 않고 현재 Composition2195·patternbindings2195·Catalog·Tree·Resources를 다시 읽었다. 주요13개 asset은 Catalog/Tree 각각1개, stable element ID 중복0, 참조 Resources156개 누락0이다. 현재 native function/dispatch/HasProfile carrier guard 검사에서 제외0이다. 자산별 정확한 displayName·ID·현재 timing·attachment·PASS/UNVERIFIED는 `out/Gate1EffectReaudit20260922/flame-dice-stagger-backstep-review.md`와 `installed-audit-inventory.json`에 기록했다.

무력화 별선5sprite의 native3008은 C++가 허용했지만 실제 particle shader 세 guard에서 제외되어 clip(-1)했다. 정본 installer의 exact material/PS 및 ribbon/sprite VF·VS 검증을 추가하고 세 생성 guard만 교정했다. 원본 PS 본문 SHA와 기존 ribbon 허용은 동일하다. 실제 ParticleKouku3008 shader/instanced layout/pass를 D3D11 WARP로 그려 수정 전0pixel, 수정 후 white4096pixel(원본 RGB 식 최대오차9.54e-7), 설치6DDS2212pixel, opacity0 control0pixel을 확인했다. D3D11 error0. 이는 전체 무력화 화면 확인과 구분한다.

Backstep Product는 실제 CNpc/CModel의 CSkeletalAfterimage를 사용하나 Tool bundle preview 활성 연결이 빠져 있었다. 기존 Charge_AfterimageActive 판정을 preview에 연결하고 CNpc에 optional preview clock을 추가했다. Product 호출의 기본값은 기존 wallclock을 보존한다. preview pause에서는 delta0, rewind에서 history clear, stop/release에서 enabledfalse+Reset을 수행한다. 실제 현재 setter/Reset/Late_Update clock 부분을 verbatim 추출한 console 검사에서 Product/첫프레임/60frame pause/advance/tail/rewind/stop/mode복귀 PASS다. 실제 Tool 조작과 잔상 픽셀 결과는 사용자 확인 전이다.

P78 emit4회 duration2635/2635/2635/1255ms 및 P1 별선10694ms는 사용자가 저장한 값이므로 보존한다. 원본 요소의 시작은 window 안이지만 P78 마지막 emit의 일부 Sk08_2 tail1.5~2.8초와 P1 긴 tail을 모두 보존하는지는 UNVERIFIED다. conservative document 상한20.15s/16s로 자동 확대하지 않았다. 카드 출력의 원본19element와 기존 카드 충돌/만료 폭발7element의 현재 targeted registration은 연결돼 있다. source 전체 요소 복원과 사용자window의 잔여 전체 표현은 별도 판정이다.

root가 Product receipt `out/BuildPipeline/runs/20260922T045116788Z-debug-product.json` PASS를 보고했다(105OBJ,5CSO,1binary). 본인 generator 최종 변경04:51:14 UTC는 해당 build 시작04:51:16.788 이전이다. 이후 통합 Tool 회전 보완의 최종 증분 빌드·publish는 통합 담당 기록을 따른다. Client/UI는 실행하지 않았다.
