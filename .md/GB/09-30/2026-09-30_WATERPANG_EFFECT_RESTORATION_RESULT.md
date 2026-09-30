# 워터팡 효과·AI 리소스 정리와 통합 인계

> G00~G05는 9월30일 당시의 기록이다. 현재 Q 보완·codec 검증은 G06~G07,
> 최신 GBResources2 전달과 최종 제품 빌드 대기 상태는 G08을 따른다.

## G00. 최종 상태와 작업 중단 범위

사용자가 다른 통합 세션에서 merge/build를 진행한다고 지시하여 이 세션의 추가 빌드와
게임 실행 검증을 중단했다. 타 세션의 MSBuild는 종료하지 않았으며 별도 채팅도 보내지 않았다.
핵심 코드는 현재 작업 트리에 있고 이펙트·저작 이름·native shader 소스·NPC 리소스는 설치했다.
통합 Product Build, 실제 경기 실행, Client 화면 검증은 완료로 기록하지 않는다.
이 작업은 commit/push하지 않았다. 기존 다수 미커밋 변경을 보존했다.

9월30일 당시 기능 브랜치는 `codex/valtan-authoring-and-entry-20260930`이었다.
통합 세션은 tracked diff뿐 아니라 아래 신규 파일도 포함해야 한다.

- `Client/Public/MaharakaAITool.h`, `Client/Private/MaharakaAITool.cpp`
- `Client/Private/ClientReplication_WaterGun.cpp`
- `Server/Private/GameRoom_MaharakaAI.cpp`
- `Server/Private/ServerGameplayContractTests_MaharakaAI.cpp`
- `Data/AI/MaharakaWaterpangAI.json`
- `Data/Effects/Authored/effect.maharaka.watergun.{q.flight,q.hit,w.flight,w.hit,r.flight,r.hit,shot.start,e.speed}.effect.json`
- `Tools/EffectPipeline/build_maharaka_watergun_projectile_effects.py`
- `Tools/EffectPipeline/build_maharaka_waterpang_smoke_candidates.py`
- `Tools/EffectPipeline/organize_maharaka_waterpang_library.py`

새 CPP/H/JSON의 Client·Server 프로젝트와 filter 등록은 반영했다.
그 외 Shared wire, GameRoom, CombatObjectRuntime, ClientReplication/registry,
Character preview setter, All Effects/World preview, MainApp, network sink/manager,
NPC clip 추가 도구의 변경도 같은 기능에 포함된다. `out`의 추출 원본·하네스·중간 산출물,
EXE/CSO/PDB와 `Client/Bin/Resources` 바이너리는 소스 커밋에 포함하지 않는다.

## G01. GBResources 전달 완료

전달 위치는 `C:/Users/user/Desktop/GBResources`다. `Client/Bin/Resources` 상대 경로를
그대로 유지했다. 이 기능의 전달 파일은 이펙트149개와 AI 의존477개, 합계626개
828,013,664 bytes다. 이번 복사 시 기존 충돌·교체는 없었고 모두 설치본과 SHA256이 일치했다.
폴더 전체의 파일 수가 아니라 이번 기능에서 추가한 전달분이다.

| 범위 | 파일 수 | bytes | 내용 |
|---|---:|---:|---|
| 이펙트 |149|5,910,744|23개 효과가 참조하는 메시·텍스처 등|
| AI 외형·동작 |477|822,102,920|NPC8종, 아바타12종, Guardian/Lance body·parts·기본 무기·물총·AnimSet 의존|

이미 Client에 존재하던 재사용 파일도 GBResources에 없으면 포함했다. 효과 native program의
새 DDS 생성이 필요하지 않았다는 사실과 전달 파일 추가는 별개다.
JSON과 shader 소스/CSO는 GBResources에 넣지 않았다.

증거:

- `out/WaterpangEffects20260930/effect-resource-delivery.json`
- `out/WaterpangEffects20260930/ai-resources.json`
- `out/WaterpangEffects20260930/ai-resources-delivery.json`

## G02. 효과 정본·게시 상태

기존 중앙 장치11개·총구4개와 신규8개를 총23개로 정리했다.
All Effects의 `World → 마하라카 → 워터팡` 아래 중앙 기둥·바닥, Q·W·E·R 물총,
원본 MK2 변형으로 분류한다. `워터팡 | 중앙 기둥 물 뿜기`, `워터팡 | 중앙 바닥 장판`,
`워터팡 | 중앙 바닥 터지기`, Q/W/E/R의 총구·비행·피격 이름을 실제 root displayName,
EffectResourceTree, DIRECT_AUTHORED_DOCUMENT catalog에 반영했다.

신규 물총8문서에는 원본57 emitter가 있다. Q/R 비행·피격, W 비행·폭발,
Q/R launch splash, E의 buff569200 원본 SpdFast를 연결했다. 기존4문서에 빠져 있던
중앙 장치 연기5개도 추가했다. MK2 총구2개는 원본 변형으로 보존하고 W에 오연결하지 않는다.

native 신규26개(program5273~5298)와 연기3633을 기존 material table·shader group에
추가했고 기존4개는 재사용한다. 다른 shader program과 렌더링 옵션은 보존했다.
후보와 최신 입력을 비교하고 백업·원자 교체로 설치했다.

WorldSequence의 공격 template5개 이름만 바꾸고 나머지 tracks·배치·재질·카메라를 보존했다.
`Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Scope WorldSequences -Mode Publish`
완료, Client 런타임1파일 게시 SHA256은
`cffe044f35e5b72d343fbeca99ff4906d624ac17ffe47ba93930e96f5092bd2f`다.
Data/Effects는 기존 direct authored 소비 경로다.

중앙 효과 preview는 실제 WorldSequence object/model/bone을 사용한다.
물총 투사체는 기존 Server CombatObject의 spawn/current pose/impact/despawn으로 재생한다.
Q/R 발사 offset은 원본 UE(70,11,75)cm를 forward/right/up으로 한 번 적용하고
source+X→gameplay+Z는 effect particleSystem yawOffset -90도로 한 번 적용한다.
W의 비행 높이는 원본60~90cm·apex ratio0.4를 참조한 bounded parabola로 재구성한 값이며
원본 엔진의 전체 grenade 이동 구현까지 동일하다고 기록하지 않는다.

## G03. AI·3분 경기·AI Tool 구현 상태

protocol127에 `WATERPANG_AI(2)`, immutable NPC 외형 ID 및 AI tuning request/result를
함께 연결했다. 인간4인 roster와 별도인 기본20명은 Guardian/Lance 아바타12명과
Maharaka/Bern NPC 외형8명을 사용한다. fake session을 만들지 않고 기존 Server player의
이동·Q/W/E/R·피격·낙사·복귀를 사용한다. 인원 증가 stage 실패는 기존 AI를 보존한다.

도입 시작20초 뒤부터 실제 경기180초를 계산한다. 종료 시 STOP을 보내고 AI·물총을 정리하며
참가 인간은 같은 섬의 `island.spawn.party02` 주변 navigation/collision 검증 위치로 이동한다.
이미 탐험 중인 인간은 보존한다. Client 남은시간·STOP 소비도 구현했다.
넉백 기본6m/242ms는 사용자가 요청한 쿠크 레이저 기준이며 원작 워터팡의 작은 밀림값과 다르다.

Debug F1 `Waterpang AI Tool`에는 봇 수, 판단 tick, 이동 목표 갱신 tick, 스킬 간격 tick,
대상 거리, 이동·공격 확률, 넉백 거리·시간9개 값이 있다. Refresh/APPLY/Save+Apply는
typed command sink를 사용한다. Server가 revision CAS 및 범위를 검사하고 정본
`Data/AI/MaharakaWaterpangAI.json`을 원자 저장한다. 실패·충돌·timeout 시 draft를 보존한다.
이 동작은 코드 연결 상태이며 실제 UI/왕복 실행 검증 완료는 아니다.

## G04. 주민 애니메이션 리소스

주민4종의 WModel에 원본 `run_battle_1`, `walk_normal_1`을 추가해 설치했다.
기존 mesh/material/skeleton/idle section은 모두 byte 동일하다.
설치 WModel의 UModel-glTF basis를 기존 idle의124bone 전체 키와 비교해 확인한 뒤
`Tools/ModelAssetConverter/append_psa_clip_to_wmodel.py`의 명시 profile로 변환했다.
기존 position186,744개 오차0, quaternion186,744개 최대오차2.98e-7이며,
추가 position34,968개·quaternion34,968개는 원본 변환값과 오차0이다.

`out/WaterpangEffects20260930/npc-locomotion/installation.json`과
`run-candidate-manifest.json`에 변경 전후 hash·clip 근거를 보존했다.
이4개 수정 모델도 GBResources 전달477개에 포함했다.

## G05. 실행한 검증과 남은 경계

완료한 검증:

- source 문서8개/57 emitter의 구조 감사, 연기5개 원본 coverage와 native deferred0 확인.
- 신규 shader가 소비되는 Mesh5248/Particle5248/Particle3584/Decal/Trail 후보5개 fxc 컴파일 성공.
- Client Debug10CPP·Release11CPP 문법 검사 오류0. 이후 소규모 NPC guard/preview setter는 추가 컴파일하지 않음.
- Server 첫8TU와 투사체2TU 문법 검사 성공. 별도68TU 격리 컴파일·링크 성공.
- 신규 JSON 및 Client/Server 프로젝트4개 XML parse 성공, 변경 파일 diff whitespace 검사.
- WorldSequences scope publish 성공, GBResources626개 전달 및 해시 확인.

미완료·통합 세션 확인 항목:

1. protocol127 Client/Server의 최종 Product Build. 이 세션은 사용자 지시에 따라 추가 빌드를 중단했다.
2. 실제 AI20명 이동·스킬·넉백·낙사·3분 종료·재입장, AI Tool 왕복/저장/충돌 처리.
   Server contract 및 AI Tool headless 실행은 중지했으므로 통과로 기록하지 않는다.
3. All Effects와 실제 경기 화면 확인. Client/UI를 에이전트가 실행하거나 캡처하지 않았다.
4. program5275의 Q/W/R flight bubble3곳에는 원본 VS의 sin-wave vertex 변형이 아직 없다.
   native PS/material과 source geometry 복원, deferred0을 완전한 VS 복원으로 대신하지 않는다.
5. Beda 원본 AnimSet은 idle3개뿐이다. 다른 NPC도 전용 물총 발사 자세가 없는 경우
   현재 존재하는 동작으로 이동하며 발사는 Server 투사체로 표현한다. 임의 player rig/socket을 이식하지 않았다.

백업은 `out/WaterpangEffects20260930/restoration-install-backup`, `library/backup`,
`npc-locomotion/install-backup`에 있다. 이전 문서 전체로 롤백하지 말고 최신 저장본과
변경 필드를 비교해야 다른 세션의 편집을 보존할 수 있다.


## G06. 10월1일 Q 세 갈래·provider·bubble 보완

Q는 원본 MK2 action57002/Att4의 notify703.593ms에서 projectile570020 세 발을 생성한다.
기존 gameplay skill56900과3초 cooldown을 유지하고 총구 offset은 owner basis의
forward70/right20/up75cm 한 번만 적용한다. 각 ray의 yaw는0/+30/-30도이고 속도10m/s,
최대거리3.3m다. R56930은 기존 단발10m/s·3m, W56910은 기존 수류탄식 비행과 착탄1회 폭발이다.
MK2의 정지형 mine W를 사용자가 요청한 수류탄 W로 바꾸지 않았다.

실제 로그의 Q flight codec 거부는 original ERM_None provider가 drawing source material을
가지고 있던 원인이다. non-drawing provider의 shader를 effect.standard로 보존하고 source material을
비활성화했다. Q.flight10/Q.hit9/Q.start4 emitter는 원본 source ID를 사용하며 기존39 particle
resource를 hash 동일하게 재사용한다. 새 Q.start를 catalog/tree·Client prewarm·cast에 연결했고,
Att4 shot 음원3 WAV를 Client Resources와 GBResources에 동일 hash로 설치했다.

program5275는 실제 material의 ShaderObject byte268과 VS16~23을 추적한 전용 carrier 분기에서
UV sin-wave normal 변형을 복원했다.16,632개 수치 비교의 최대오차0이며 cm→m를 한 번 적용한다.
PS/material 성공을 VS 성공으로 대신했던 이전 미복원 항목은 소스·수치 단계까지 해소됐다.
이 섹션은 이전 G05의 해당 미완료 소스 상태를 갱신하며 실제 GPU 화면 판정은 여전히 사용자 확인이다.

증거는 out/WaterpangQFan20261001의 installation/product-validation/server-apply-receipt,
audio/delivery-receipt 및 out/Waterpang5275Vertex20261001에 있다.
현 단계 JSON/XML/resource/hash/재생성 검사는 통과했다. 최종 제품 빌드와 실제 codec·room 계약
결과는 후속 검증 섹션에 기록한다. Client/UI는 에이전트가 실행하지 않았다.

## G07. 실제 codec 및 Debug 빌드 추적 복구

2026-10-01 정상 Debug Product가 완료된 현재 OBJ로 실제 CEffectDocumentCodec probe를 다시
링크했다. Resource 경계12개와 Q flight/hit/start 및 provider fixture를 포함한5문서 검사는
failures0, expected rejection1로 통과했다. 이전 Q의 non-drawing provider를 metadata 누락이
아닌 원래 비표시 render mode 불일치로 거절하는 것도 확인했다.
증거: out/WaterpangQFan20261001/native-debug/codec-recovered.log.

이전 실패는 Debug Client의240개 OBJ가 PCH만 헤더 의존성으로 기록하여 오래된 재질 registry와
class ABI를 재사용한 것이었다. manifest SHA를 확인한240개 OBJ만 out으로 백업·격리했다.
소스·PCH·tlog·Release·shader timestamp를 변경하지 않고 정상 Product Build가240OBJ를 다시
생성했다. 재검사에서 PCH-only 누락은 Debug0/Release0이며 데이터/validator 완화는 없었다.
quarantine-receipt.json과 tracking-after-recovered-build.json에 전후 증거가 있다.
Debug Product receipt는 20260930T184933055Z-debug-product.json, PASS다.
이 검사는 GPU 물줄기·소리 출력 또는 사용자 화면 판정을 대신하지 않는다.

## G08. Q 리소스 전달 재검증과 최신 제품 빌드 경계

현재 효과 구성은 Q.start 추가 후 24개다. 최초 GBResources 정리에서 Q의 기존 source resource 39개를 다시 확인한 결과,
`GBResources`에 없던 `fm_k_tornado_01.wmodel`과 텍스처 3개를 같은 Resources 상대 경로로
추가했다. 이 4개는 Client 설치본을 그대로 전달했으며 원본 asset 생성·변경은 없다.
기존 Att4 shot WAV 3개는 이미 동일하므로 다시 복사하지 않았다.

| Q 후속 전달 범위 | 파일 수 | bytes | GBResources 처리 |
|---|---:|---:|---|
| 기존 source에서 재사용한 누락 mesh/texture | 4 | 603,164 | 신규 복사 |
| Att4 shot WAV | 3 | 901,240 | 동일 hash 확인 후 건너뜀 |

Q 후속 전달 7개와 재사용 의존성 39개 모두 Client 설치본·GBResources SHA-256이 일치한다.
전체 통합 전달은 콜로세움 1,109개를 포함해 1,116개이며 신규 1,113개, 동일 3개,
기존 파일 교체·삭제 0개다. 최초 전달 증거는
`out/GuidePersonal20261001/resource-delivery/{preflight,delivery-manifest,delivery-summary}.json`이다.
기존 G01의 626개는 당시 전달분이며 이 증분 수치로 과거 기록을 덮어쓰지 않는다.

사용자가 최종 목적지를 `C:/Users/user/Desktop/GBResources2`로 지정하여 위 Q 후속 7개를
그곳에 신규 추가했다. mesh/texture 4개 603,164 bytes와 WAV 3개 901,240 bytes,
합계 1,504,404 bytes다. 기존 콜로세움 1,109개 / 411,358,322 bytes는 동일하여 건너뛰었다.
선택한 총 1,116개 / 412,862,726 bytes는 모두 Client 설치본과 SHA-256이 일치하며,
상이한 기존 파일 교체·백업과 삭제는 0개다. 기존 GBResources는 그대로 보존했다.
Q 재사용 의존성 39개 중 이 증분의 4개 외 나머지 35개는 이미 업로드한 GBResources의
동일 hash 기준분을 사용한다. 최종 전달 증거는
`out/GuidePersonal20261001/resource-delivery/delivery-manifest-gbresources2.json` 및
`delivery-summary-gbresources2.json`이다.

G07의 실제 codec PASS와 04:43 KST의 이전 Debug·Release Product PASS는 확인된 검증이다.
이후 최종 이동 수정·실패 알림 연결을 포함하는 최신 Product 빌드는 아직 대기 중이다.
최신 전체 빌드 성공이나 Q 세 갈래 물줄기·W 폭발·음원의 GPU/사용자 화면 판정이 완료됐다고
기록하지 않는다. 코드·Data/DataFiles는 저장소 변경을 함께 사용하고, 전달 폴더의
`Effect`와 `Sound`는 팀 PC의 `Client/Bin/Resources` 아래 동일 경로에 반영한다.

## G09. 최종 Q/W/R 제품 빌드

이동 보완과 모든 최신 Client/Server 소스를 포함한 정상 Product Debug/Release가 모두 성공했다.
앞선 EXE 잠금·최종 링크/제품 빌드 대기는 해소됐으며 실행 파일과 실제 로그는
`../09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G09에 기록했다.
이 결과는 기존 기능별 검증을 대체하거나 실제 Client 화면·다인 플레이·성능 확인으로 확대하지 않는다.
