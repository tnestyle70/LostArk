# 베른 국소 조명과 세 클래스 full restore 통합 결과

대응 문서는 [구현 계획](2026-09-09_BERN_LOCAL_LIGHT_AND_THREE_CLASS_FULL_RESTORE_IMPLEMENTATION_PLAN.md)이다. G00~G04는2026-09-09 중간 점검을 보존한 이력이고, 현재 세 클래스 전체 스킬·소환·dust·공포 조사 결과는 G08~G12를 따른다. 구현과 자동 검증을 사용자 시각 승인으로 취급하지 않는다.

## G00. 현재 반영 위치와 범위

차원술사 복원에서 사용한 원본 Action 활성 호출 → 첫 LOD → MIC·상속·static switch → 선택 VS/PS 계산 → 기존 Effect renderer 입력 경로를 재사용했다. 새 문서는 기존 unified의 `.full.restore.effect.json` 형제이며 기존 gameplay binding을 새 문서로 일괄 전환하지 않았다. 파일 추가량, 실제 로드·Solo 검사, 실제 화면 일치는 서로 다르다.

| 대상 | 현재 저작 파일 | 연결한 핵심 | 검증 또는 남은 연결 |
|---|---:|---|---|
| 워로드 A/S/F/V | 4문서·115 elements | A 사슬 COLOR/WPO, S 원본 3회·방향, F/V 번개, source light11, native program73 | 115개 Load/Save/실제 Solo 필터 Drawable 통과. F 전체와 해당 속도 크기 행의 실제 Playback sweep 통과 |
| 도화가 D/T/V/Alt V | 5문서·341 elements·modelCue2 | 호랑이·용, native program174, 카메라에 붙은 원본 시각 요소·light·Sky | D26/V69/Alt1 52/T23 총170 Load/Save/Solo 통과. Alt2는 복수 Orbit LINK 미지원으로 로드 차단 |
| 창술사 V/Alt V | 7문서·546 elements·modelCue12 | native program176, particle mesh30, 말 4 material section × 3 clip | V50/78/29와 Alt clip1 162 합319개 Load/Save/Solo 통과. Alt clip2/3/4는 SubUV Movie 미지원으로 로드 차단 |
| 베른 | 국소 Point10 | 기존 스폰·16초 카메라 경로에 영향이 있는 source SL00 light | authoring Publish/Check 통과. 원본 baked 기여를 현재 direct light로 대체한 범위이며 baked/UV1 복원 완료가 아님 |
| 세이튼 등장 공 | 기존 world object 모델1개 | 같은 원본 MIC의 D/N/S/E texture4개 | 실제 CModel 4슬롯 로드 통과. 타이밍·배치·world revision412 보존 |

카메라 이동·회전·FOV·shake와 Sequencer 확장은 다른 세션 담당이다. 이 작업의 카메라 관련 시각 요소는 기존 attachment를 사용하는 mesh/particle/post이며 카메라 연출 편집과 구분한다. 베른 전역·쿠크·발탄 신규 조명/안개 전수 복원은 마지막 사용자 범위 조정에 따라 확장하지 않았다.

## G01. 실제 런타임 연결

- 공용 source texture 입력을 9개로 확장하고 각 family의 원본 texture/parameter·sampler·분기를 기존 mesh/particle shader에 연결했다.
- ModelCue의 optional material을 Codec → resource stage → CModel clone → 기존 animated shader에 연결했다. material이 없는 기존 모델은 기존 CMaterial을 사용한다.
- Artist 호랑이·용과 Lance 말 네 section은 원본 skeleton/animation을 CModel로 cook했다. 실제 CModel6개·clip21개·bone palette189개 검사에서 누락·비유한 행렬0이었다.
- Native screen post는 기존 Engine `IPresentationScreenPostMaterial` 소비자에 immutable frame 입력을 전달한다. DM68/76/155/156, Warlord415/672, Artist876/894, Lance659를 연결했고 source CB0 의미가 닫히지 않은 Lance600은 지원 목록에 넣지 않았다.
- 워로드 F의 SizeMultiplyVelocity는 cm/sec 속력, 현재 Size 곱, 축별 mask, 양수 cap, source spawn/update 순서를 소비한다. source SizeMultiplyLife도 spawn과 update의 mask를 반영했다. BaseSize를 보존하고 spawn footer와 authoring trim이 계산된 live size를 지우지 않도록 교정했다. 별도 legacy receipt opcode schema의 지원 상태와 이 portable runtime 구현을 혼용하지 않는다.

장면 깊이 바인딩은 새 family의 depth sample뿐 아니라 scene-color UV 계산에 필요한 depth SRV 크기까지 포함한다. 음수 determinant인 one-sided native mesh는 기존 반대 cull pass로 전환한다. Artist894 후처리도 실제 소비자에 연결했다. 원본 shader 식만 추가하고 실제 입력이 null이던 경계를 교정했다.

## G02. 실행한 검증과 한계

| 검사 | 실제 결과 |
|---|---|
| Engine Debug x64 | Build 성공 |
| Client Debug x64 C++ | 실제 Client.vcxproj /t:ClCompile 종료0, 컴파일 오류0. 최신 Renderer/NativeScreenPost focused compile도 성공. 전체 Shader/최종 link는 미실행 |
| animated/mesh/native screen-post FXC | 각 focused `fx_5_0` compile 성공. 진행 중 후속 family 변경은 다시 최종 빌드 필요 |
| 실제 CModel 소환 모델 | 6개·21clips·189 bone palettes, errors0 |
| 공 material4개 | 기존 geometry와 실제 CModel load 유지, D/N/S/E4개 존재 |
| Bern map publisher | Publish 및 Check 성공, 13shards·50,017placements·29files, source/runtime light10 일치 |
| 워로드 문서 저장/Solo | 4문서115개 통과, 숨김0·잠김0 |
| 워로드 F 실제 Playback | 266 checks/0 failures. 전체260steps peak155, 속도 크기 대상 Solo224steps peak50, 수치·축 mask·cap·Spawn/BaseSize·45회 Update/Seek 일치, nonfinite0. 다른 bone attachment는 CPU 시험용 identity 입력이므로 실제 캐릭터 attachment 화면 증거가 아님 |
| 전체 문서 저장/Solo | 16문서 중12문서·604 visible elements 통과, hidden/locked/unexpected0. 나머지4문서는 Artist Alt2 복수 Orbit LINK와 Lance Alt2/3/4 SubUV Movie에 의해 로드 차단 |
| F/SubUV/Orbit 실제 Playback | F/SubUV976검사 실패0. Orbit 추가6개는 liveOffset=base+sourceCurve 전표본 오차0·Update/Seek 일치. 복수 LINK1행 및 그 행을 포함한 group은 거부되어 합1058검사 중2실패이며, 전체 PASS 아님 |
| Client/UI 실행·캡처·시각 판정 | 수행하지 않음. 사용자 확인 전 |

중간 검증 산출물은 `out/ThreeClassFullRestore20260909`와 각 family의 `out/*Restore20260909`에 있다. 전체 변경 worktree에는 다른 작업의 수정이 함께 있으므로 자동 stage/commit하지 않았다.

## G03. 아직 남은 병목

1. 현재 확정된 문서 로드 병목은 Artist Alt2의 복수 Orbit LINK와 Lance Alt2/3/4의 SubUV Movie다. 단일 Orbit offset은 spawn 옵션과 base/live offset reset을 연결했다. random SubUV는 원본의 출생 선택·Changes0 유지·serialized 상대나이 interval을 연결했다. Sprite는 실제 원본 SubUV VF가 atlas UV를 소비하고 Artist mesh536/825/826은 실제 native VS대로 geometry UV를 보존한다.
2. sibling particle 이벤트·위치 의존, Ribbon history, 닫히지 않은 decal/VS/scene 입력은 source ID와 이유를 보존하고 최종 full elements에서 제거한다. 핵심 모델이나 복구 가능한 계산을 지워 숫자만 통과시키지 않는다.
3. 일부 원본 masked 모델은 현재 forward alpha/depth-read 경로에 연결됐으며 원본 GBuffer depth-write와 동등하지 않다. 원본 environment cube/SH/sky·primitive aggregate bounds·light size→radius tick 중 미연결 입력도 남아 있다. 전체 원본 장면 계산을 복원했다고 주장하지 않는다.
4. 현재 빌드 판단 checkpoint에서 제품 소스/Data 확장을 멈췄다. C++ compile은 통과했지만, 남은4문서·전체 Playback/renderer stage와 최종 Shader/link 확인은 필요하다. 전체 재생의 실제 크기·색·겹침은 사용자가 판단한다.

## G04. Resources와 실행 준비

새/수정 runtime 입력은 Git에 추가하지 않는다. 주요 위치는 `Client/Bin/Resources/Effect/Artist/Models/SK_SDM_TIG_00`, `SK_SDM_DRA_00`, `Effect/LanceMaster/Models/SK_FLM_HOR_00`, `Effect/Warlord/FullRestore`, `Effect/KoukuSaydon/Meshes/wp_mn_rhcn_00/mesh/fm_d_rhcn_00.wmodel`이다. Artist/Lance/Warlord 각 source texture 폴더도 함께 필요하며 Drive 전달은 아직 수행하지 않았다.

LAN 설정은 server-host, 방화벽 준비, endpoint `192.168.0.14:7777`이다. 마지막 점검에서 Client/Server는 실행 중이지 않았고 endpoint는 not-listening이었다. 최종 빌드 후 사용자가 Visual Studio `Server + Client` profile에서 Ctrl+F5 → Lobby → Character Select → 해당 class → F1 Effect Tool V1의 대응 full restore를 선택해 Solo/Play All을 확인한다. 이 경로 안내는 현재 모든 문서가 runtime-ready라는 뜻이 아니다.

## G08. 전체 full restore 구현과 실제 스킬 연결

2026-09-10 요청의 LMB와 현재 스킬 전체를 기존 Effect 경로에 연결했다. 도화가19문서540요소, 워로드27문서718요소, 창술사46문서1,144요소로 총92문서2,402요소다. 이 수에는 워로드 V 통합 검토본과 clip별 실행본의 중복 발생이 포함되며 한 번의 스킬에서 동시에 생성하는 수가 아니다. CModel cue는 도화가3개와 창술사12개다. native program은 각각267/269/331개이며 공용 ShaderFamily58개 carrier 항목과 프로젝트/filters에 연결했다.

EffectCatalog에92개 full asset을 등록했고 Artist/Warlord/LanceMaster `.animevents`의 실제19/26/46개 clip이 full 문서를 참조한다. 원본 `src=orig` 이벤트와 비이펙트 이벤트는 보존했다. 워로드 평타 세 단계가 같은 문서를 호출하던 문제, E 첫 clip이 두 번째 문서를 호출하던 문제, T의 stage 번호 오접속을 교정하고 T의 _03/_04를 추가했다. 워로드 V는 start/loop/attack별 시계를 사용하고 통합본은 Tool 검토용으로 남겼다. 창술사 이동·기상 세 clip의 누락된 asset 이벤트도 추가했다. Artist A의 linear-reveal 이름만 source full ID로 명시 변환했고 LMB/R의 기존 source stage 대응은 유지했다.

사용자가 승인한 워로드 F full39요소와 창술사 V clip2 full78요소는 보존했다. 각 클래스에서 닫히지 않은 sibling particle/LocationEmitter, collision, source material·VF·socket과 typed parameter 발생은 source ID 및 이유를 각 EXCLUSIONS에 남기고 full 요소에서 제외했다. 정상 delayed occurrence와 말의 model-anchor 의존62행은 유지했다. Source reference와 Resources를 삭제하지 않았다. 세부 원본 범위와 제외는 각 클래스 RESULT가 소유한다.

## G09. 용·호랑이·말과 방패, dust

도화가 T 미르새김의 용 모델 정본은 `Effect/Artist/Models/SK_SDM_DRA_00/sk_sdm_dra_00_sk.wmodel`이며 `sk_dragonrising_01`을 사용한다. 원본 ground skeleton과 FIXAREA/speed0을 유지하며 cue delay0.6초, duration5초, yaw-90도다. 원본 애니메이션 자체가 바닥 XZ 평면에서 움직이고 key 재생시간은2초다. 사용자 첨부 이미지에서는 검은 붓자국 형태가 보이고 용의 몸을 식별하기 어려웠다. 첨부 이미지를 열어 진단했지만 새 결과의 시각 일치로 사용하지 않았다.

호랑이는 `Effect/Artist/Models/SK_SDM_TIG_00/sk_sdm_tig_00_sk.wmodel`의 `sk_cloudtiger`를 사용하며 실제 길이는0.733333초다. 용·호랑이와 `Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section0~3.wmodel`의 말 세 clip은 WANM이1000tick으로 저장되어 Engine 고정30tick에서33.33배 느리게 움직이던 원인을 확인했다. duration과 모든 key time을 함께30/1000로 정규화했고 geometry/pose 값과 Engine의 playable character 시계는 보존했다. 실제 CModel6개,21clip 전부에서 시간별 bone 변화와 finite 행렬을 확인했다.

창술사 Alt V를 막던 SubUV Movie는 기존 source particle executor에 FrameRate, StartingFrame, bUseEmitterTime을 연결했다. 출생 random은 한 번만 선택하며 update는 fixed delta를 사용한다. realtime 요청은 명시 거부한다. 말 네 material section과 세 animation cue를 유지했다. 도화가 Alt V는 꽃밭222요소를 유지하고 문서 로드를 막던 복수 Orbit LINK 꽃잎1개만 제외했다.

워로드 방패는 실제 `Effect/Warlord/FullRestore/Meshes/sk_wgl_gdd_01.wmodel`을 사용한다. V는 반지름4.5m의5방향과 caster를 감싸는 sphere 보호막, Alt V는 반지름6m의6개와1.4m의6개를 배치했다. 수·반경은 사용자 요청을 구체화한 저작 배치다. 실제 원본 masked 방패 MIC와 native prefix를 연결했고 기존 F 번개 계산은 보존했다. 새 screen post profile도 generic 효과로 우회하지 않고 기존 native post 소비자로 연결했다.

차원술사 dust는 현재 검은 중심 보강을 사용하던 R tuning3행과 A full4행을 `effect.project-tuned.dimensionmaster-glasshole-purple-rim.v1`로 바꿨다. 원본 Q51 RGB mask를 이용해 검은 부분은 alpha0, 경계는(0.48,0.12,0.90), intensity0.5/opacity0.35로 계산한다. 기존 depth·particle fade와 원본 Q51 및 이전 alias는 유지했다. 사용자에게 대상 스킬을 추가로 확인했으나 답변 전 현재 R/A 소비 문서를 기준으로 진행했다. R full의 별도 원본 재질은 변경 대상이 아니다.

## G10. 공포 모션과 gameplay 상태 조사

여섯 클래스 모두 실제 WModel에 공포 clip이 있다. Artist `sdm_abn_fear_1`, LanceMaster `flm_abn_fear_1`, Warlord `wgl_abn_fear_1`, Gunslinger `gdh_abn_fear_1`, Slayer `wbk_abn_fear_1`, DimensionMaster `pc_sp_m_00_sk_abn_fear_1`이다. 고정30tick 기준 길이는 앞의 네2초 모션(Artist/Lance/Gunslinger/Slayer), Warlord1초, DimensionMaster2.933333초다. serialized24tick metadata와 실제 runtime30tick을 구분해 `out/ThreeClassFullRestore20260910/fear_clips.json`에 기록했다.

`Shared/Public/Network/PacketMessages.h`의 PLAYER_ACTION_STATE에는 FEAR가 없고 Server/Client에도 공포 상태를 적용·해제하거나 해당 모션으로 표현하는 소비자가 없다. 이번 요청에서는 모션 존재와 상태 정의 부재를 조사했다. 실제 공포를 발생시키는 gameplay 요구 없이 enum/placeholder를 추가하지 않았다.

## G11. 실제 자동 검증과 남은 실행 경계

| 검사 | 실행 결과와 증거 |
|---|---|
| Client Debug x64 C++와 전체 FX | 실제 Client.vcxproj ClCompile/FxCompile 종료0. `out/ThreeClassFullRestore20260910/client-compile-shaders.log`. FXC X4000 경고는 존재하며 Warlord sampler/dispatch 입력 초기화·전체 return을 읽기 검토해 실제 누락은 발견하지 못함 |
| 새 Client.exe 링크 | 제품 project의 PrepareForBuild/ResolveReferences/_Link로 별도 `out/ThreeClassFullRestore20260910/linked/Client.exe`와 PDB 생성 성공. `client-staged-link.log`. 기존 DirectXTK PDB 누락 경고는 있으나 링크 오류0 |
| 전체 full Load/Save/Solo | 원래88문서2,178요소 통과에 F33/T12+12/Alt167 수정 재검증을 합쳐92문서2,402요소 통과. `codec-final.log`, `codec-final-fixed.log`, `codec-final-alt.log`. hidden/locked/unexpected0, model-anchor 의존62행 보존. 검사는 실제 Codec/Save_Atomic와 Tool Solo 필터의 Drawable 검증이며 GPU pixel 판정이 아님 |
| 차원술사 R tuning/A full | 수정한2문서79요소 Load/Save/Solo 통과. `codec-final-fixed.log`. alias strict parameter7행 검사 및 별도 읽기 검토 통과 |
| 실제 소환 CModel |6모델·21clip·189 bone palette,21clip 모두 움직임, nonfinite/error0. `summon-models.log` |
| 실제 particle Playback | 기존 F/random SubUV976회 회귀와 새 Movie195표본·Update/Seek 대조 포함 총1,109검사 실패0. `playback-movie.log` |
| JSON/XML와 Git whitespace | 각 클래스 source JSON/리소스 참조, catalog/tree/event count와 target ID, project/filter XML 및 GUID를 확인. `git diff --check` 종료0. source/build/개인 파일을 자동 stage하지 않음 |
| Client/UI와 최종 화면 | 자율 실행·조작·캡처 미실행. 새 용·호랑이·말·방패·전체 스킬·보라 경계는 사용자 육안 확인 전 |

원래 검사에서 발생한 Artist F native-v14→ordinary-v13 메타데이터 잔존, T 빈 module class, Alt vectorfield 누락·원본 반복 모듈의 stable ID 중복은 실제 소비자를 따라 수정했다. 원본 반복 camera module은 지우지 않고 reference occurrence별 ID로 분리했다. 최종 성공은 수정 후 실행한 해당 문서 재검증을 합친 결과이며 이전 실패 로그를 PASS로 바꾸지 않았다.

정식 `Client/Bin/Debug/Client.exe` 교체는 현재 Client 프로세스가 실행 중이어서 아직 수행하지 않았다. 실행 중 Client/Server를 에이전트가 종료하지 않았다. 사용자에게 Client 직접 종료만 요청했고 Server는 그대로 두도록 안내했다. 종료 확인 뒤 정식 경로의 링크·배포를 진행해야 새 C++ 코드가 현재 실행 준비에 반영된다.

## G12. Resources와 사용자 확인 경로

Resources는 로컬 설치했으며 Git에 추적하거나 Drive에 업로드하지 않았다. 도화가는 `Effect/Artist/Models/SK_SDM_TIG_00`, `SK_SDM_DRA_00`의 변경 WModel, `Effect/Artist/Textures`의 새43DDS 및 관련13mesh가 필요하다. 창술사는 `Effect/LanceMaster/Models/SK_FLM_HOR_00`의 네 변경 section WModel과 `Effect/LanceMaster`의 새25WModel/58DDS가 필요하다. 워로드는 `Effect/Warlord/FullRestore/Meshes`의 새35geometry, `Effect/Warlord/Textures`의9DDS, `Effect/Warlord/VectorFields/fx_cm_05.fx_n_vector_field1.wvectorfield`가 필요하다. 정확한 Resources-relative ID는 각 클래스 RESULT/설치 기록을 따른다. binary payload를 force-add하지 않는다.

현재 LAN은 server-host, TCP 방화벽 준비, `192.168.0.14:7777` 응답 상태다. 사용자 검토 중인 Server CMD와 Client는 유지했다. 정식 실행파일 반영 뒤 사용자가 기존 Server를 유지한 채 Client를 Ctrl+F5로 시작하거나, 둘 다 꺼져 있다면 `Server + Client` profile로 시작한다. Lobby → Character Select → 대상 class → 실제 LMB/skill 입력을 확인하고, F1 → Effect Tool V1 → All Effects/Refresh → 해당 full restore에서 Solo와 Play All을 확인한다. 차원술사는 R tuning/A full의 dust를 확인한다. 이 경로 안내와 자동 구조 검증은 manual first pixel 또는 visual PASS 기록이 아니다.

## G13. R/D/S 후속 변경을 포함한 최신 링크 상태 — 2026-09-10

위92문서·3개 class 복구를 보존한 채 사용자의 차원술사 후속 요청을 Round2 RESULT G37과
성능 RESULT G06에서 진행했다. 공용 비베이크 분포와 일반 저작 준비 개선의 최소 Client 컴파일,
현재 제품 객체의 링크가 성공했다.

최신 산출물은 `out/DimensionMasterRDSAltV20260910/linked/Client.exe`(50,576,384byte)이며
앞선 ThreeClassFullRestore20260910/linked 파일보다 새것이다. 여전히 Client PID55640이 실행 중이라
정식 EXE 교체와 사용자 실제 재생 승인은 남았다. 후속 수치 검증을 이전92문서 전체의 visual PASS로
확대하지 않는다.

## G14. full 광원 전수 검토와 도화가 꽃밭의 정체 — 2026-09-10

현재 full 문서에 연결된 것과 원본 동작 전체를 복원한 것은 다르다. 이번 추가 감사는
`Data/Effects/Authored/*full.restore.effect.json`106개를 실제로 읽고 선택된 source first-LOD,
제외 기록, cleanup 이전 자료와 대조했다. 광원119행이 있지만 원본 입자 광원 후보 중23행이
빠져 있고, 연결된 행도 초기 색·알파·입자 수명과 실제 조명 소비에 공통 미복원 경계가 있다.
앞선92문서 Codec/Save/Solo 성공은 이 광원 동작이나 전체 원본의 시각적 완료를 뜻하지 않는다.

### 연결된 입자 광원과 누락

| class | full 문서 | 저장된 광원 | 같은 full 범위의 source 후보 | 누락 |
|---|---:|---:|---:|---:|
| Artist |19|15|15|0|
| LanceMaster |46|59|59|0|
| Warlord |27|41|42|1|
| DimensionMaster |14|4|26|22|
| 합계 |106|119|142|23|

Warlord V 통합 검토 문서에는 clip별 광원8개가 중복 보존되어 있다. 이를 제외한 canonical
집계는 Warlord33/34, 전체111/134다. 현재 full 밖의 미선택 Warlord17820 stage002/003/007과
full 문서가 없는 다른 action은 위 누락에 더하지 않았다.142는 선택한 입자 광원 후보이며
directional-light control이나 모든 source action의 광원 총수가 아니다. 원본 LOA4개를 직접
다시 파싱한 결과, 현재 full의 선택 stage에는 `DominantDirectionalLight_Control`25건
(Artist3/Lance10/Warlord9/DimensionMaster3)이 별도로 있다. 미선택 variant까지 합친56건과
구분했다. 현재 typed decoder의 enabled=true는 하드코딩이고 semanticDecoded=false이므로
25건을 활성 광원 누락으로 확정하거나 위142의 분모에 더하지 않았다. 정확한 payload SHA,
cue/stage와 원문은 `Lights/action-light-controls.json`에 있다.

DimensionMaster 누락22행은 LMB7, W1, A4, D2, F2, Alt V6이다. 최근 R/S에 추가한 각각1개와
V의2개가 현재 연결된4개다. Warlord 누락1행은 T17240 stage001 notify007의
`FX_Weapon_01` source socket 미해결 건이다.119행은 모두 실제 kind=light이고 visible/enabled,
range/intensity가 양수다. sprite 이름에 Light가 있거나 emissive/bloom이 밝다는 이유로
실제 주변 표면을 비추는 광원에 포함하지 않았다.

### 연결 후에도 남아 있는 원본 소비 문제

현재 일반 LIGHT 경로는 `CEffectPlayback::Rebuild_Frame → Try_BuildEffectPointLightDesc →
CEffectObject::Submit_Presentation → CPresentation_Manager → CLight_Manager`다. Playback은
typed timing/range/intensity와 ColorOverLife/ColorScaleOverLife를 소비한다. source 입자의
StartColor/StartAlpha/Lifetime과 개별 burst/rate·이동·radius update를 같은 방식으로 시뮬레이션하지
않으며 활성 element당 광원 하나를 만든다. 따라서 원본 particle 수명과 위치에 따라 여러 광원이
생기는 emitter를 단일 고정 light로 대체한 행도 그대로 원본 동작이라고 볼 수 없다.

원본 constant particle Lifetime과 typed light clock의 차이를114행에서 확인했다.
초기 색이 흰 typed base에 투영되지 않은 행은105개, 초기 alpha가 typed intensity에 반영되지
않은 행은73개다. 세 수는 중복되는 결함 분류여서 합산하지 않는다. 예를 들어 Artist LMBba1은
typed 수명1.75초/흰색/intensity10이지만 source Lifetime은.3초, StartColor는(1,1,1.5),
StartAlpha는.2다. Lance LMBba1도1.75초와 source .3초, source color(2,2.5,2.8), alpha.5가
다르다. 이 값은 단순히 파일에 없는 것이 아니라 현재 일반 light 소비 경로에 연결되지 않았다.

light.color.w=0인 Artist/Lance 행을 무광원으로 판정하지 않았다. 변환기는 RGB×intensity를
LightDiffuse.xyz로 전달하고 Deferred/Combined는 그 RGB로 표면을 비춘다. alpha metadata와
광원의 RGB 에너지는 별개다.142개 후보의 source component/CDO/archetype 상속은 모두
닫혔으며 현재 intensity는 component brightness와 일치한다. 현재 고정반경이 source200cm와
다른 행은 S의 저작3m와 V2050520 두 행의10.24m다. component 반경과 particle size에 의한
반경 갱신은 별개이며 후자의 원본 EF tick은 미복원이다. S의 이전 Size 기반 저작 projection과
V의 component 차이를 구분하고, R/S의 부분 회수를 모든 광원의 native parity로 확대하지 않는다.

별도의 실제 렌더링 순서 결함도 확인했다. `Engine/Private/Renderer.cpp`의 일반 light pass는
source-character row를0으로 둔다. `Shader_Deferred.hlsl`은 이 pass에서 marker5 캐릭터 픽셀을
건너뛰고 뒤의 source-character material별 pass에서 비춘다. 그런데
`CLight_Manager::Render_Lights`가 첫 pass 종료 때 `Clear_TransientLights()`를 호출해서,
뒤의 캐릭터 pass에는 effect 광원이 남아 있지 않는다. 영구 `m_SceneLights`는 그대로여서
일반 맵과 캐릭터의 효과 광원 수신이 달라지는 직접 경로다. 독립 읽기 검토도 같은 결론이었다.

후속 수정의 소유자는 이미 존재한다. `CRenderer::Draw`의 정상 종료와 `FailFrame`은 모두
`Presentation.Clear_Frame()`을 수행한다. pass별 조기 clear를 없애 이 프레임 경계에 맡기면
새 queue나 두 번째 lighting runtime 없이 모든 material pass에서 같은 transient 광원을 소비할 수
있다. 이번 G14는 감사 결과이며 해당 제품 코드 수정이 적용됐다고 기록하지 않는다.

광원마다 full-screen rectangle을 그리는 현재 구조와 global transient64개 한도도 확인했다.
64초과는 자동 품질 조절이 아니라 GLOBAL_RUNTIME 실패와 frame submission rollback이다.
다만 한 full 문서1instance의 typed 시간 구간을 겹친 현재 상한은 최대7개이고 실제 clip 문서는
최대6개다. DimensionMaster Alt V는 현재 광원0개여서64개 광원 한도가 해당 문서의 중단 원인은
아니다. 여러 효과·플레이어가 동시에 있는 frame의 합산 비용은 별도다.

### 도화가 Alt V 꽃밭은 무엇으로 이루어져 있는가

`31930.clip1.full.restore`52행과 `clip2.full.restore`170행, 합222행이다. 구성은 sprite154,
mesh particle61, `Sky_Mirror` static mesh2, point light4, native ZoomBlur1이다. skeletal
꽃밭 모델이나 꽃222개가 아니다. `sdm_sk_super_pungnyudo_01/02` 두 clip에 바닥·꽃과 꽃잎·
나비·발걸음·빛·카메라 전면 입자를 시간순으로 배치한 연출이다.

Resources의 실제 texture를 해독해 열람하고 glTF/WModel topology 및 UV와 대조했다.
512² `fx_m_flowergarden_d_01` atlas에는 큰 꽃·네잎 꽃·작은 꽃·낱꽃잎·나비 날개와 몸통이
함께 있다. `flowergarden` 이름만으로 모든 mesh를 꽃으로 분류하면 틀린다.

| 메시 | 실제 UV가 읽는 그림 | geometry | 사용하는 emitter 행 |
|---|---|---:|---:|
| flowergarden_01 | 서로 다른 꽃3개 묶음 |12 triangles|Field02의2행|
| flowergarden_02 | atlas 아래쪽 낱꽃잎 |4 triangles|Field02 1+FootStep2=3행|
| flowergarden_04 | atlas 우하단 나비 날개와 몸통 |6 triangles|Butterfly/Cam02/Cam03의9행|

바닥의 `lv_common_grass_19_d`1024² texture에는 녹색 잎과 흰 꽃이 직접 그려져 있다.
현재 꽃 군락의 핵심은 그 풀꽃 바닥 sprite와 flowergarden_01의 두 emitter이며, 카메라 가까이
떠다니는 것 중 상당수는 나비와 꽃잎이다. triangle 수 자체는 작지만 발생량, 큰 투명 면의 중첩,
카메라 계열65개 emitter 및 현재 mesh particle별 draw가 비용을 만든다.65개는 저장 행 수이며
65개가 항상 동시에 보이거나 화면 전체를 덮는다는 실측은 아니다.

Alt V 원본 후보261행 중39행이 제외되어 있다. 이39행은 Artist 전체 제외86행의 일부다.
live sibling particle provider34, ribbon1, material map2, engine/vertex input1,
Orbit LINK1로 나뉜다. 특히 `Field_01`32행 전체가 빠져 있으며 이 중30행은 다른 emitter의
살아 있는 입자 위치를 따라 생성해야 하는 계열이다. `fx_m_petal_01`27vertices/36triangles를
쓰는24행이 Petal_01/02 재질을 각각12행씩 사용하고,
나머지는 glow4/shine2다. 나머지2행은 EngineDefaultParticle material이다.

따라서 Field02와 바닥이 연결되어도 Field01의 위치 연동 꽃잎 군집까지 복원된 상태는 아니다.
원본 `EFLocationEmitter`의 sibling emitter 선택·sequential 진행·위치와 회전/속도 상속을
기존 Playback에 연결해야 한다. 외형이 비슷한 static mesh를 추가하는 것으로 이 발생 동작을
대신할 수 없다. 또한 clip1 ExMove_02의 `artist-full.2332584ff9b7e1720ca23325`는 source burst1000을
보존하지만 현재 maxParticles64라 단일 요청부터 용량에 잘린다.222개의 행이 연결되어 있다는
사실은 원본 발생 수 전체가 유지된다는 뜻도 아니다.

action 원문에는 PostProcessChain4건과 DominantDirectionalLight_Control1건도 있다.
이들은 위222 particle/mesh 문서 행 및39제외와 다른 action-level 계약이다. source enabled
필드가 미해독인 제어까지 원본에서 활성이라고 단정하거나, 현재222행에 포함된 것으로 세지 않는다.
선택된 source action에 맞는 제어 입력과 실제 consumer를 별도로 확인해야 한다.

### 결과 파일과 검증 경계

전수 행·source ID·scope 제외·timing 동시 상한은
`out/FullRestoreLightAltVAudit20260910/Lights/`에, 꽃밭 전체 source/mesh/texture와 제외39행,
실제 DDS decode는 같은 작업의 `ArtistFlower/`에 있다. 이미지 열람은 Resources asset 진단이며
Client 화면 캡처나 사용자 visual 승인 자료가 아니다. Alt V318의 CPU·geometry·발생 상한 측정과
구조 개선 순서는 성능 RESULT G07을 따른다.

이번 추가 작업은 읽기 감사·out 수치 실행·PLAN/RESULT 정리다. 제품 광원·renderer·skill JSON과
Resources를 수정하지 않았고 새 제품 EXE 배포도 없다. 이전 G13의 EXE와 실행 확인 경계는
그때의 증거다. 사용자 실제 화면 확인과 원본 시각 일치는 이번 감사로 PASS 처리하지 않는다.

## G15. full 공통 성능 구현과 광원 수명 소유권 교정 — 2026-09-10

G14 이후 사용자가 Alt V 외에도 수십 element에서 발생하는 한 자리 FPS의 구조 수정을 요청했다.
네 클래스 full106문서가 공유하는 Playback/Renderer 경로를 수정했다. source update 분포·literal의
반복 검색은 Stage에서 준비한 index/옵션을 사용하고, SceneColor 재질 검증은5개 정상 Stage에서
준비한다. 두 diagnostic Stage는 캐시를 비운다. F1 값·Visible 편집의 재stage와 실제 Solo/alpha
조건은 유지한다. native static mesh는 같은 occurrence의 연속 pass를 순서대로 instance 제출한다.

별도 복원 결함 두 가지도 교정했다. Light_Manager가 일반 조명 패스 직후 transient light를
삭제하던 한 지점을 제거했고, 기존 Renderer::Draw의 성공·실패 Clear_Frame이 정리를 소유한다.
실제 Render_Lights 호출 두 곳에서 source-character pass까지 동일 frame의 효과 광원을 유지한다.
추가로 Artist/Warlord/Lance의 새 native profile971저장행이 기존 shader frontend 범위 밖에서
clip되던 조건을 고쳤다. 이 숫자는 중복 검토 문서를 포함한 저장행 수이며 동시 입자나 화면 승인 수가 아니다.

광원119/142와 누락23행, source 초기 색·alpha·particle 수명, directional action의 활성 의미는
G14의 미완료 경계 그대로다. 이번 queue 교정을 모든 광원 원본 복원의 완료로 기록하지 않는다.
Artist 꽃밭의 정체는 풀꽃 바닥과 꽃·낱꽃잎·나비·빛의 조합이며 Field01의 sibling-provider32행과
현재 Alt V 제외39행도 이번 성능 수정에서 추가 복원하지 않았다. 발생 cap과 원본 시간을 임의
변경하지 않았으므로 기존 생성량 손실 역시 남는다.

full106문서29,974step/30,822 frame·state checkpoint 및 별도 가변 delta·rollback2,060점은
수정 전과 byte-exact다. Debug headless 재생 계산 합계38.4초→18.0초는 화면 FPS가 아니다.
Engine/Client와 FX를 빌드했고 Client/Bin/Debug에 실제 Client.exe/Engine.dll을 배포했다.
모델 기하와 GPU 비교의 상세 결과·최종 shader closure는 성능 RESULT G08 및
`out/FullRestorePerfImplementation20260910/`에 기록한다. Client/Server 자율 실행·화면 캡처·사용자
visual PASS는 하지 않았으며 사용자가 같은 full 문서의 Play로 실제 화면과 FPS를 확인해야 한다.


## G16. 사용자 저장 이후 도화가·워로드 시각 결함 후속 — 2026-09-10

### 사용자 입력과 보존 기준

사용자가 첨부한7장을 직접 열람했다.1장은 도화가 원작의 낮은 카메라·하늘·풀꽃·나비,
2/3장은 동일시점 황금 원환의 다른 시점,4장은 Saved Element FOLLOW 거절,
5장은 현재 세 가닥 먹선,6장은 원작의 넓은 용 몸통,7장은 워로드 원작 금빛 방패와 낙뢰다.
이 자료는 결함 진단 입력이며 새 Client 화면의 승인 자료가 아니다.

사용자의 `최신 튜닝 저장 완료` 이후 Artist19/Warlord27 full 문서와 두 animevents,
총48파일을 `out/ArtistWarlordVisualFollowup20260910/before/Data/`에 보존했다.
`user-saved-baseline.json`이 그 시점의 파일·해시 기록이다. 정확한 대상은 Alt V의
`31930.clip2.full.restore`와 V 몽유도원의 `31910.full.restore`다. 이미지4의 New Effect
입력칸은 현재 문서 이름과 다르므로 해당 텍스트를 현재 저장 identity로 사용하지 않았다.

### Saved Element의 카메라 FOLLOW와 V 원환

`Build_PortableAuthoredElementStartingCopy`가 모든 FOLLOW를 소유자 애니메이션 이력으로
거절하던 것이 Cone45/46 Add 실패의 직접 원인이다. 두 source 행은 meshModel이 존재하며
`CAMERA_VIEW`·bone 없음·model cue 없음이다. 기존 Tool/Product resolver는 이 부착을
현재 view와 SocketLocalTransform으로 계산하므로 원본 animation history가 필요 없다.
검증된 camera-view FOLLOW만 부착을 유지해 복사하도록 좁게 허용했다. bone FOLLOW,
Trail, transform master, SourcePresentation 및 native Renderer 소유권 거절은 유지한다.

실제 제품 codec으로 두 cone을 복사하고 startDelay/emitterDelay0, 독립 stable copy ID로
`31910.full.restore`에 추가했다. source 위치 `[0,-.4,0]`/`[0,0,0]`, Z45° 및 socket은
보존했다. source와 target의 Effect-level ParticleSystem 네 값도 동일하다.58개 요소가 되며
기존55행은 모든 field가 동일하고, 나머지 기존1행은 아래 Sprite29의 두 field만 변경했다.
복사물은 Save/Load canonical 왕복을 통과했고, bone FOLLOW 대조 입력은 계속 거절하면서
호출자의 기존 출력 문서도 보존했다.

Sprite29 stable suffix `42875d1a88aaef7067025288`의 native520은
`fx_e_noise_005`를 포함한 복합 원환 재질이다. 원본 `EPAL_Z`는 Client positive Y로
매핑되어 기본 quad의 법선을 수평면 방향으로 고정한다.2/3장만으로 실제 quad가 기울었다고
확정할 수는 없다. 다만 Billboard renderer가 Transform 회전을 버리고 고정 방향을
재구성하므로 사용자의 Rotation 변경이 면 방향에 반영되지 않는 경로는 코드로 확인했다.

기존 `Particle Billboard=false`와 Transform X90°를 사용해 바닥 방향을 저작자가 조절할 수
있게 했다. 저장된 Z−45°, 위치, 크기와 재질·분포는 그대로다. 실제 Playback의61개 입자
샘플에서 quad normal의 수평 성분 합 최대4.37114e−8로 평면을 확인했다. 이는 CPU quad
계산 검증이며 GPU의 최종 색·WPO나 사용자의 육안 승인과 구분한다.

검증 기록은 `copy-plane-result.log`, `artist31910-patch.json`에 있다. 기존 Saved Element
테스트19개도 통과했다. 처음 out 검증용 빌드의 std::max 매크로/링크 의존성 및 실행 리소스
root 오류는 제품 결함이 아니며, 현재 실제 codec/playback으로 다시 빌드·실행한 기록을
사용한다. 해당 검증 도구는 Client 화면을 실행하지 않았다.

### 미르새김의 본체와 별도 먹선

현재 세 가닥 선은 native482의 `fm_j_helixline_1`3개이며 native461의
`artist_31950_dra_source_projectile` 용 본체와 별개다. 본체 Scale이 세 먹선 폭을 바꾸지 않는
것은 정상이다. 원본 TGA와 runtime DDS의 픽셀·alpha는 동일했다.

PSA→glTF 변환의 non-root quaternion W 부호 누락으로 본체 삼각형이 뒤집혔다.
0초의2,432면 중906면이 뒤집히던 것이 교정 뒤 모든 면의 정상 방향으로 바뀌었다.
원본 UModel의 PSA/GLTF export 좌표 계약으로 root와 child를 각각 대조했으며, 수정 전
child 회전 대조 실패와 수정 후 기존8+추가1개 테스트 통과를 확인했다.

원본 폭의 DRA만 다시 cook했다. 두 clip의122프레임에서 모든2,432면이 finite/정상 방향이며
UV·topology·weights·inverseBind·position/scale keys,2초/30fps는 유지했다. 첫 시점의
alpha 가중 앞면 면적은.1984→.6799m²다. 폭5배 실험물은 out에만 남겼고 runtime에는
설치하지 않았다. 사용자31950 JSON과 cue 위치·회전·스케일은 바이트 단위로 보존했다.
이 교정은 면 복구이며 첨부 원작과 같은 두께로 보인다는 판정은 사용자가 한다.

변경 runtime 입력은 `Client/Bin/Resources/Effect/Artist/Models/SK_SDM_DRA_00/`
`sk_sdm_dra_00_sk.wmodel`이다. Git에 binary를 추가하지 않는다. 새 Engine/Product 모델
경로를 만들지 않고 기존 CModel을 사용한다.

### 도화가 Alt V 카메라와 워로드의 현재 진행 경계

원본 STANDARD_SKILLCAM_YINYANGSHI의 선택 경로221→204→212→48은 플레이어에
camera root를 부착하고 Matinee를 재생한다. 이 경로에 맵 이동은 없다. 현재 Sky_Mirror_SM은
안쪽을 보는 약71.68m 구체다. 낮은 전용 카메라·하늘 구체·꽃밭의 조합이며 새 공간이나
캐릭터 중복 소환을 만들 필요가 없다. 사용자 sky 위치·yaw는 보존했다.

clip2 full Preview가 skill 전체01/02를0초부터 시작하던 문제를 해당 clip과 원본 camera 구간으로
맞췄다. 공용 `CEffectRecoveryCamera`가 검증·샘플링을 소유하고 F1과 제품 presentation이
같은 sidecar를 소비한다. 제품은 사전 준비 시 읽어 캐시하며, 현재 local owner와 action에만
카메라를 적용하고 중단·종료·카메라 전환 시 해제한다. 원본 local-only 플래그를 가진 하늘·
카메라 요소는 clip1 24개, clip2 43개이며 remote owner의 렌더 제출에서 제외한다.

실제 C++ parser/sampler를 연결한 검증은 3,883개 정수 ms 시각 × 2개 owner transform ×
3개 aspect에서23,298회 샘플링했다. 최대 eye 오차 .000417381m, look 오차 .000334182m,
FOV 오차 .000437959°이며 종료·overlap·invalid axis·malformed local-only와 실패 시 기존
출력 보존을 포함한70,012개 확인에서 실패0이다. 이는 카메라 수치 계약이며 화면 승인이 아니다.

워로드 V에는 승인된 F17140의 native446 낙뢰를 황색으로 지정한4 wave를 추가했다.
source burst4와 rate/WPO/alpha를 보존하고, 전체 문서 시점1.1667/1.5/1.8334/2.5초로
안쪽과 바깥쪽에 교차 배치했다. 원본 낙뢰 개수 복제가 아닌 사용자 요청의 저작 튜닝이며 F는
바이트 동일하다. 방패39개는 기존 transform/timing을 보존하면서 원본 PBR02(1122)와
장식 PBR01(1123)을 연결했다. 원본 장식 모델7,691정점/10,256삼각형의 추가39개를 포함해
최종 V 전체52행·clip2 29행·clip3 24행·Alt V clip1 179행·clip2 155행이다.

맵16개와 transient64개까지의 directional/point/spot 입력을 기존 순서대로 공급하고 거리·
cone 감쇠는 현재 deferred 계산을 재사용한다. 원본 PS 계산을 가져온 것이며 원작의 모든
engine 전역값 복구를 뜻하지 않는다. engine 소유 cubemap/BRDF 및 light prefix 일부의
정확한 정체가 미확정이므로 환경 반사 완전 복구로 기록하지 않는다.

### 실행 경계

Engine 격리 출력 Build와 Client ClCompile/FxCompile은 성공했다. Client 통합 컴파일은
3분32.54초, 오류0이며 shader 기존 경고를 포함한7,868개 경고가 있다. 최종 Client 링크·
배포 전이며 이전 G29 개별 Element Sequencer도 같은 최종 EXE에 포함될 대상이다.
이후 꽃밭 validator 수정과 창술사 T 추가는 해당 변경 뒤 필요한 컴파일을 다시 확인한다.
최근 process 조회에서는 Client/Server가 없었다. 에이전트가 종료하거나 실행하지 않았다.
사용자 화면 확인은 미실행이다.

## G17. 워로드 본체·장비 원본 재질 연결

기존 Warlord의9개 모델14개 material slot에는 원본 source program 연결이 없었다.
원본 MIC13종의 유효 상속 값·texture flag·shader map을 대조하고57개 texture 입력을
`Character/SourceMaterials/` 아래 준비했다. 기존3개는 byte 동일,54개는 신규였다.
기존 프로그램1/2/5/7은 해당 uniform expression 계약이 같은 경우만 재사용하고,
lower/upper/head/armor-emissive/weapon-emissive의5개 원본 PS 쌍을10~14로 추가했다.
기존1~9의 계산은 유지했다. Engine의 원본 재질 admission 상한도14로 함께 변경했다.

눈은 본체 material5의185정점 UV1/UV2, 머리는880정점 UV1을 원본 triangle-corner 및
native half 자료로 복구했다. 기존 position/normal/tangent/UV0/index/weights와 animation
section을 유지했다. 정본 입력은 `Character/Warlord/Warlord.wmodel`과
`Character/Warlord/Warlord_Hair.wmodel`이며 binary는 Git에 추가하지 않는다.

격리 Engine.dll과 새 animated/static CSO를 사용하는 실제 CActorCatalog/CModel/CMaterial
검사44개에서 실패0이다.9개 모델14개 재질 모두 생성·clone·FX binding이 성공했다.
복구 전 UV 모델, 없는 material/texture, NaN 입력은 거절하고 기존 clone을 보존했다.
이 검사 뒤 CharacterCatalog의 Warlord 항목만 설치했으며 다른 character 항목은 보존했다.
`out/ArtistWarlordVisualFollowup20260910/WarlordBody/activation-result.json`과
`catalog-install.json`에 실제 결과와 변경 전후 identity를 기록했다.

본체의 material 소유2D IBL/BRDF 텍스처는 연결했다. summon shield의 engine cubemap
미확정과는 다른 범위다. weapon emissive의 미확정 instance phase를 추정하지 않고 원본에
노출된 fixed-speed 옵션을1로 설정했다. 이 값은 프로젝트 튜닝이며 원작 기본0의 복제가
아니다. original engine prefix/ambient 전부의 동일성이나 최종 색·광택은 승인하지 않았다.

## G18. 꽃밭 provider와 창술사 검격·T 연결 확인

도화가 Field01은 기존 170행의 저작 값을 보존하고 simulationOnly 위치 provider 2행과
꽃 30행을 연결해 202행이다. 실제 제품 codec/playback에서 Save/Reload, 30개 Solo,
5개 Seek 및 13개 잘못된 참조의 거절을 확인했다. Seek 최대 위치 차이와 숨은 provider의
draw 제출은 모두 0이다. 속도·회전 상속과 provider chain은 지원으로 기록하지 않는다.
증거는 `out/ArtistWarlordVisualFollowup20260910/Field01/provider-final-result.log`다.

창술사 BA/Q/W/E/R/S는 사용자가 지정한 A 첫 검격의 4개 mesh 구성을 재사용했다.
S의 네 타격과 V 마지막 돌진에 기존 v1/애니메이션 시각을 맞췄다. 변경 대상 16문서의
232개 element와 23개 presentation을 실제 codec으로 읽고 저장하는 검사는 통과했다.
기존 유지 행 141개는 모든 field가 동일하다. 기존 Server root motion은 계속 이동 권위를 가진다.

짧은창 T 34650 clip2는 원본 PlaySkeletalMesh notify의 용 모델을 기존 CModel 경로에
연결했다. 기존 74개 element를 보존하고 bone child 6개 및 model cue 2개를 추가했다.
0.5초 시작, 1.2초 노출과 `sk_dragoncleave_03` clip을 사용한다. 실제 제품 codec의
80개 element 왕복과 CModel 2개·clip 2개·finite palette 18개 검사가 통과했다.
모델 내부 상대 경로가 요구하는 `Models/SK_FLM_GDR_01/textures/fx_c_noise_009.dds`를
추가해 실제 CModel 로드 누락을 수정했다. 증거는
`out/ArtistWarlordVisualFollowup20260910/LanceT/t_models_codec_probe-final.log`다.
이 항목 역시 사용자의 실제 화면 확인은 별도다.

## G19. main 통합과 최소 리소스 전달 — 2026-09-10

사용자 요청에 따라 추가 복원 작업을 중단하고 전체 변경을 PR #348
`effect restore, kouku pattern 2` 기준으로 통합했다. 사용자 commit `c67a47b2`를 보존하고,
PR #346을 먼저 병합한 main `a83f2a31`을 작업 브랜치에 병합했다.
충돌은 stable ID 기준으로 양쪽 변경을 보존했다. World Sequences는 revision 423,
resource 19개·template 149개·instance 185개이며 Composition은 revision 230,
저장 pattern 26개다. main의 갈고리 pattern 18/19와 충돌한 기존 공포/Mario pattern은
25/26으로 참조와 함께 옮겼다. Shared protocol은 FEAR와 Bingo/Mario/hook을 함께 담는
78이며 73/77 EXE와 혼용하지 않는다.

Map WorldSequences publish와 KoukuSaydon domain publish는 통과했다. 갈고리 outcome을
거절하던 Gameplay publisher를 기존 Server 계약에 맞췄다. 통합 데이터의 bootstrap이
4,122행이 되어 기존 4,096행 상한을 넘는 실제 로드 오류도 확인했다. 문서 전체 행 상한을
Server와 publisher 모두 8,192로 맞추고 개별 curve의 4,096 제한은 보존했다.
publisher 경계 검사는 0/8,193행 거절과 4,122/8,192행 허용을 확인했다.
Bingo 테스트의 폭탄 carrier가 snapshot Players에 없던 fixture도 해당 player로 보완했다.

Engine, Server, Client의 최종 Debug x64 Build가 모두 오류 0으로 끝났다.
Client는 3분 08.05초이며 기존 shader 및 DirectXTK PDB 경고를 포함해 경고 7,610개다.
Client/Bin/Debug에 Client.exe와 Engine.dll을 배포했고 G29 개별 Element Sequencer도 포함한다.
NetworkProtocolHarness 전체 실행은 failures 0이다. 변경 JSON 127개와 XML 2개 parse,
충돌 표식 및 미해결 index 검사를 통과했다. ActorX의 현재 코드·테스트 해시는 기존 11개
테스트 통과 당시와 동일하다. PR 변경 범위의 `git diff --check origin/main`도 통과했다.
검증 로그는 `out/FullRestoreMergeReview20260910/`에 있다.

최소 리소스는 최초 조사 시 2026-09-09 13:00 KST 이후 생성·수정 후보 576개,
Downloads/다운리소스에만 있던 추가 필요 경로 57개, T 모델 2개와 필수 DDS 1개를 합친
636개·390,187,854 bytes다. 다운로드의 동일 파일 3,954개는 제외했다.
서로 다른 두 파일은 현재 runtime 정본을 보존했고, 시간 범위 밖 기존 파일은 다시 넣지 않았다.
파일 시각은 복사 과정에서 달라질 수 있으므로 과거 변경 이력의 확정 증거로 주장하지 않는다.

전달 폴더는 `out/ResourceDelivery20260910/staged-minimal/GB_Resources/`이고
단일 ZIP은 `out/ResourceDelivery20260910/GB_Resources.zip`이다. ZIP은 142,356,663 bytes,
README 포함 637개 entry로 내부 ZIP은 없다. 전체 CRC·경로·크기와 현재 runtime 대비
내용 비교를 통과했다. Character/Effect/Map 폴더를 기존 Client/Bin/Resources에 병합한다.
Resources와 EXE·중간 산출물은 Git에 추가하지 않는다.

최종 Server 계약 검사는 Bundle 38개, Bingo 45개, ObjectOverlap 55개,
WorldPlayback 12개로 합계 150개 통과·실패 0이다. 마지막 Bundle 검사에서 관문 전체 재생이
보스 archetype만 확인해 GATE1 목록에 GATE3 갈고리 두 패턴을 넣고 후단 검증에서 전체를
거절하는 실제 통합 오류를 확인했다. 기존 selector에 호출자가 가진 gate/target placement
범위를 전달해 최초 선택에서 필터하고 순서·transition·후단 guard는 보존했다.
이 수정 뒤 Server를 다시 빌드하고 실패했던 Bundle 38개만 재실행해 통과했다.
최종 로그는 `server-contracts/kouku-bundle-final-scope.log`다.

Client를 자율 실행하거나 화면을 캡처하지 않았으며 사용자 visual PASS로 기록하지 않는다.
이 검증 상태의 전체 변경을 기존 PR #348로 push하고 merge한다.
