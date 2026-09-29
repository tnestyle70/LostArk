# 마리오 색별 머리 표식 원본 추출 결과

## G00. 완료 상태

설치 원작 data3.lpk의 Offering buff와 UPK를 연결해 파란·빨간 광대 얼굴의 색상
처리까지 확인했다. 세 색 모두 공통 얼굴 `fx_l_symbol_63_cl`을 쓰며 buff LOA의
Color 파라미터로 색을 바꾼다. 별도의 색별 도형은 얼굴 위에 함께 재생한다.
도형만 추출한 앞선 결과를 전체 머리 표식처럼 설명한 것은 잘못이며 이 문서에서 교정한다.
원본 얼굴 PNG, 도형 PNG/DDS, HUD 저주 아이콘 및 원본 LOA 6개를 보존했다.
후속 구현 요청으로 marker authored3개와 Shared/Server/Client 진행 표시를 반영했다.
진행 문구는 맞는 공을 부쉈을 때1초 유지 후0.4초 투명도 fade로 제거한다.
현재 실행 중인 Client/Server를 유지하라는 사용자 지시에 따라 정식 EXE 교체는 보류했다.
격리 컴파일·검증과 실제 화면/4인 회차 재현을 구분한다. 구현 내역은 G05 이후를 따른다.

## G01. 머리 위 표식 정본

원본 `ParticleSoundNew/9_EF_PARTICLE_SOUND_DATA_BUFF_FX_Monster_BaseBuff_`
`KoukuSaton_Offering_<Color>_B.loa`의 같은 CEFParticleBuffInfo 안에는 공통
`Par_X_RPCT_Mark_04`와 색별 도형 ParticleSystem이 함께 존재한다.
`_B`가 없는 Offering 파일은 색상 override가 있는 공통 얼굴만 호출한다.
MarkOff에도 같은 색상 override가 있다.

| 색 | 얼굴 Color RGB | 외곽 ColorStr RGB | 함께 재생하는 위쪽 도형 |
|---|---|---|---|
| 빨강 | 4.5, 0.4, 0.4 | 1, 1, 1 | 별 |
| 파랑 | 1, 1, 16 | 0.5, 0.5, 0.5 | 마름모 |
| 노랑 | 2, 2, 0.1 | 1, 1, 1 | 삼각형 |

공통 Mark_04의 얼굴 emitter 0/29는 Color를 소비하며 material은
`FX_M_Mi_L_00.FX_Mi.fx_l_pa_rpctmark_02_1_tr_dt`, texture는
`FX_TEX_NOMIPMAP_00.fx_l_symbol_63_cl`이다. ColorStr는 외곽 emitter 7/8의
추가 배율이므로 얼굴 RGB에 곱한 값으로 기록하지 않는다.
프로젝트의 기존 복원 HLSL native3013은 texture RGB × particle RGB를 계산한다.
이는 이번에 원본 shader bytecode를 새로 검증한 결과와 구분한다.

수정 전 authored red/blue/yellow의 face는 같은 얼굴을 쓰면서 Color를 흰색으로 고정했다.
원본 색상 override 누락이 얼굴의 노란 원색이 남는 원인이다. 공통 텍스처 재사용 자체가
문제인 것은 아니다. 사용자가 제시한 파란 얼굴+마름모와 원본 buff의 구성은 일치한다.

원본 `FX_MN_RPCT_05_X` package의 4990 object를 해석했으며 property error는 0개다.
아래는 각 색별 variant의 위쪽 도형 부분만의 추출 결과다. 공통 얼굴을 대체하지 않는다.

| 색 | ParticleSystem suffix | 도형 MIC | Texture2D | 문양 | 도형 StartColor RGB |
|---|---|---|---|---|---|
| 빨강 | Par_X_RPCT_Mark_04_Red_LOC_INT | FX_M_Mi_X_00.FX_Mi.fx_x_pa_sy_14_02_tr | FX_TEX_06.fx_x_symbol_018_1_cl | 별 | 8, 0.3, 0.1 |
| 파랑 | Par_X_RPCT_Mark_04_Blue | FX_M_Mi_X_00.FX_Mi.fx_x_pa_sy_15_02_tr | FX_TEX_05.fx_l_symbol_41 | 마름모 | 1, 1, 10 |
| 노랑 | Par_X_RPCT_Mark_04_Yellow | FX_M_Mi_X_00.FX_Mi.fx_x_pa_sy_13_02_tr | FX_TEX_06.fx_x_symbol_017_1_cl | 삼각형 | 6, 6, 0 |

세 도형 텍스처는 흰색 마스크이며 색은 particle/HDR 재질이 적용한다. 도형
MIC는 emissive_power=1, emissive_desaturation=0, uv_scale=1이고 StartAlpha는 약1.2다.
선행 fade emitter와 외곽 noise MIC는 별도로 존재한다. 마스크 PNG를 원작 최종 색상
합성 화면이라고 표현하지 않는다. 원본 실수의 정확한 값은 source-chain JSON에 보존했다.

수정 전 제품은 세 색 모두 별도 고정 오각별을 사용했다. 따라서 얼굴 Color 복원뿐 아니라
색별 도형 연결도 원본과 대조했다. 이전의 도형만으로 전체 표식을 판단한 설명을
복원 문서와 gotchas에서도 교정했다.

## G02. 산출물

기본 폴더: `out/MarioMarkerExtraction20260929/`

- `textures/fx_x_symbol_018_1_cl.png`, `.dds`: 빨강용 별 원본 마스크
- `textures/fx_l_symbol_41.png`, `.dds`: 파랑용 마름모 원본 마스크
- `textures/fx_x_symbol_017_1_cl.png`, `.dds`: 노랑용 삼각형 원본 마스크
- `source-investigation/marker-materials.json`: 원본 MIC 6개
- `source-investigation/marker-source-chain.json`: 원본 emitter·material·color 연결
- `source-investigation/graph/FX_MN_RPCT_05_X.particle-graph.json`: 원본 graph
- `texture-extraction.receipt.json`, `verify_extraction.py`: source/output hash와 독립 decode 검증
- `face-investigation/fx_l_symbol_63_cl.png`: 원본 공통 얼굴 atlas
- `source-investigation/base-face-emitter-chain.json`: 공통 얼굴의 원본 emitter 연결
- `source-investigation/offering/*.loa`: 원본 Offering 색별 buff 6개
- `source-investigation/offering/offering-parameter-overrides.json`: Color/ColorStr 타입·값·offset
- `offering-verification.receipt.json`, `verify_offering.py`: 원본 LPK 재독해와 parameter 검증

원작 설치 입력은 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/`의
FX_TEX_05=`YGI3SORGM3I10GHA5BMJ815CZ.upk`,
FX_TEX_06=`YGI3SORGM3I17GHA5BMJ8E5CZ.upk`다.
기존 `Tools/LpkPipeline/export_upk_texture.py`와 lostark_v7 UModel을 사용했다.
이번 out의 Python dependency 디렉터리만 추가했으며 global dependency는 설치하지 않았다.

## G03. HUD 저주 아이콘

이 파일들은 머리 위 문양과 다른 HUD buff 아이콘이다. data2.lpk의 SkillBuff 행과
data3.lpk의 IconInfo.loa에서 atlas/rect를 확인했다.

| 색 | SkillBuff | 논리 PNG | EFUI_ICONATLAS_B texture/rect | 출력 |
|---|---:|---|---|---|
| 빨강 | 4219924 | Buff_411.png | buff_3 / 576,128,64,64 | ui-icons/red_Buff_411.png |
| 파랑 | 4219926 | Buff_444.png | buff_1 / 448,512,64,64 | ui-icons/blue_Buff_444.png |
| 노랑 | 4219925 | Buff_443.png | buff_1 / 384,512,64,64 | ui-icons/yellow_Buff_443.png |

원본 atlas PNG는 `ui-atlas`, 연결과 파일 hash는 `ui-icons/source.json`에 보존했다.
원본은 광대 인형과 칼을 그린 서로 다른 색의 64×64 이미지다.

## G04. 검증과 남은 경계

- 위쪽 도형 3개 모두 256×256 RGBA, alpha 범위0~255다.
- 직접 UPK decoder의 PNG와 독립 UModel DDS를 decode한 모든 RGBA pixel이 일치했다.
- source package의 추출 전후 SHA256이 일치했다. resizing/recolor/recompression은 하지 않았다.
- 추출 PNG 6개를 파일 자체로 열람했다. Client 화면 실행·캡처는 하지 않았다.
- 원본 LPK에서 다시 추출한 LOA 6개 모두 보존 파일·SHA256과 일치하며 vector 36개를
  FString 이름, 타입3, float3 값, None sentinel까지 독립적으로 검증했다.
- 생성 JSON parse와 이번 문서 변경의 git diff --check를 통과했다.
- 새 C++와 project/filter 변경이 없어 Product build는 실행하지 않았다.
- 최초 추출에서는 제품 marker를 수정하지 않았다. 후속 반영은 아래 G05에 구분한다.

## G05. 원본 색·도형 제품 반영

`Data/Effects/Authored/effect.kouku.mario.marker.red/blue/yellow.effect.json`의 얼굴과
외곽 Color를 원본 Offering 값으로 교체하고 외곽에만 ColorStr를 적용했다.
위쪽 `.star` stable ID는 유지하되 빨강별/파랑마름모/노랑삼각형의 실제 원본 DDS,
particle RGB/alpha,1×1 UV를 사용한다. 얼굴 atlas의2열 index1은 유지한다.
도형은 existing `effect.standard` mask renderer로 연결했고 미등록 원본 MIC를 다른
native shader ID로 위장하지 않았다. 원본 전체 MIC shader 신규 복원은 아니다.
위치·크기·SESSION/Sample timing을 보존했으며 DDS3개는 설치본과 원본 SHA256이
동일해서 바이너리를 교체하지 않았다. Effect는 authored 직접 소비이므로 publish는 없다.

정확한 변경과 백업 hash: `out/MarioMarkerExtraction20260929/implementation/applied.receipt.json`.
대상3개 구조·원본 LUT·resource closure5개/721536bytes 검증을 통과했다.
전체 Effect validator는 기존 별도 `effect.kouku.gate1.blade-dance.circle.impact.effect.json`의
`v15 Product document has no runtime carriers` 오류로 실패했다. 해당 무관한 파일은 수정하지 않았다.

## G06. Server 진행 snapshot과 일시 표시

protocol121의 SNAPSHOT_PLAYER에 `iMarioRequiredColor`와 `iMarioMatchingBallCount`를
추가했다. Server는 마리오 입장자의 실제 목표색과 기존 탈출 판정 함수
`Mario_MatchingBallCount` 결과를 최대3으로 제한해 복제한다. Mario 밖에서는 둘 다0이다.
외부 참가자의 머리 표식 색인 `iMarioMarkerColor`는 별도 유지한다.
writer/reader는 범위·목표 없는 count·Mario 밖 진행값을 거부하며 실패 시 기존 output을 보존한다.
기존 랜덤·공 파괴·탈출 권한 판정은 바꾸지 않았다.

HUD view model은 두 필드를 복사한다. Level의 기존 Mario update는 count 증가에만 표시 타이머를
시작하고 화면 하단에 `[1 / 3] 노란색 공` 형식으로 표시한다. 같은 count snapshot과 다른 색 공
파괴는 타이머를 재시작하지 않는다. 진입·배치/목표색 변경·복귀는 조용히 기준값을 초기화한다.
1초 유지 뒤0.4초 동안 RGB/alpha를 함께 줄여 SpriteBatch의 premultiplied-alpha fade를
수행한다. 노이즈 마스크로 글자가 깎이는 별도 dissolve shader를 추가한 것은 아니다.
기존 중앙 저주 해제3초 안내는 유지한다. 필요한 한글·숫자 glyph가 실제 폰트에 있음을 확인했다.

## G07. 빌드·실행 경계

표준 Release Product 시도는 `Tools/Build/ProductOutputGuard.psm1:85`에서 실행 중인
Debug Client34084/Server30756 때문에 컴파일 전 차단됐다. 사용자 답변은 실행 유지다.
기존 EXE/DLL을 교체하거나 프로세스를 종료하지 않고 `out/.../isolated`의 별도 OutDir/IntDir를
사용해 Shared·Server·NetworkProtocolHarness Debug를 컴파일했다. Client는 새 PCH와
CombatHUDViewModel/Level_KakulSaydonArena/Level_KakulSaydonArena_WorldObjects3개를
개별 ClCompile하여 OBJ 생성까지 확인했다. Client 링크와 정상 Product 완료는 주장하지 않는다.
기존 코드페이지 경고 C4819와 프로젝트 참조의 격리 IntDir 추적 경고 MSB8028은 있었으며
이 경고를 없애기 위한 인코딩 변환이나 무관한 project 설정 변경은 하지 않았다.

격리 NetworkProtocolHarness는 신규3색×0..3, malformed rollback, wire길이 검증을 포함해
`failures : 0`을 확인했다. 로그는 `out/MarioMarkerExtraction20260929/protocol-test.log`다.
Server contract test의 최초 격리 실행은 EXE 상대 DataFiles가 없어 실패했으므로 유효한 제품
검증으로 취급하지 않는다. 기존 지원 환경변수 `LOSTARK_SERVER_DATA_ROOT`를 실제
`Server/Bin/DataFiles`로 지정한 재실행은 선행 통합 시나리오 진행 중 중단했으며 전체 통과로
기록하지 않는다. 중단 대상은 이 작업이 만든 격리 Server뿐이고 사용자 실행 프로세스는 유지했다.
추가한 snapshot 테스트 fixture의 tick0을 유효한 tick1로 수정하고 해당 테스트 구간의 격리
컴파일까지 확인했다. 최종 Mario1~4 진행 snapshot 실행 검증은 완료하지 않았다.

사용자의 최종 요청인 “빌드 하지 않아도 돼 마무리 해줘 그냥”에 따라 추가 빌드와 실행 검증은
진행하지 않고 코드·데이터 반영 상태로 종료한다. 신규 제품 이미지·DDS는 없으며 설치된 원본
리소스를 재사용하므로 `C:/Users/user/Desktop/GBResources`에 추가 전달할 제품 리소스도 없다.

정식 Client/Server 빌드·재실행 및4인 화면 검증은 남아 있다. protocol121은 서버와 접속 Client
모두 같은 새 빌드가 필요하다. 사용자에게 Client/UI 실행·Reload·편집 폐기를 대신 수행하지 않았다.
