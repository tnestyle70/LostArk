# 쿠크 카드비·회전 카드·DJ 안내와 카드병정 연결 결과

## 현재 반영 범위

2026-09-22 Gate 1 추가 요청 중 카드비 낙하·폭발, 회전 카드의 준비 연출 분리, DJ 안내 이미지, 카드병정의 Server 소환 계약을 구현했다. 기존 사용자 Composition 편집과 원본 라이브러리 효과를 보존했다. 후보 manifest는 `out/CardRain20260922/candidate-manifest.json`이며, 통합 담당이 승인된 최신 저장본에 stable ID와 변경 필드만 병합했다. 실제 파일 적용 receipt는 `out/Gate1Restored20260922/applied-manifest.json`이다. Product 빌드·publish·Server 실행 검증의 최종 상태는 통합 RESULT가 기록한다.

Client를 실행하거나 UI를 조작하지 않았다. 아래 Effect 검증은 native codec와 CPU playback 결과이며 GPU 픽셀 결과나 사용자의 화면 확인을 대신하지 않는다.

| Asset ID | 문서/구성 | CPU duration | 연결 계약 |
|---|---|---:|---|
| `effect.kouku.gate1.cardrain.drop` | V1 27 elements | 7.20초 | WORLD, follow false, 노란 경고+원본 낙하 카드+원본 폭발 한 group |
| `effect.kouku.common.spinning.card.overhead` | V1 4 elements | 3.25초 | BOSS follow true, 내부 원본 notify 0.25초 |
| `effect.kouku.common.spinning.card.flying` | V1 208 elements | 19.99초 | 기존 throw의 준비 16개만 분리한 파생 |
| `boss.kouku.dj.cardrain` | V2 ScreenPost LEAF | 5초 | 원본 DJ 얼굴/프레임, `쏟아져라, 카드 비!` |
| `boss.kouku.dj.delivery` | V2 ScreenPost LEAF | 5초 | 원본 DJ 얼굴/프레임, `특급 배송 출발!` |

V2는 GROUP가 아닌 LEAF로 등록한다. 신규 Resources는 다음 PNG 두 개뿐이며, 후보 외 원본 추출 파일·폰트·도구·중간 산출물을 배포하지 않는다.

- `UI/KoukuSaydon/GameNote/dj_kouku_cardrain.png` — SHA-256 `60367bd0a0ee2fd74c1274fb8dfcb4da200f7e31f64edd136294294468884a74`
- `UI/KoukuSaydon/GameNote/dj_kouku_delivery.png` — SHA-256 `a052a9a9dfc49561d39f190768d12b52c3dcb44e6514d16343c96f4156c3ef6e`

후보 파일은 `out/CardRain20260922/Resources/` 아래에 있으며 제품 위치는 `Client/Bin/Resources/`에 같은 상대 경로다. V1 mesh, texture, native material은 이미 설치된 Kouku FullRestore 리소스를 재사용한다. 새 native shader program이나 shader wrapper는 추가하지 않았다.

## 카드비 원본과 재생 수정

Action `4219803`은 `MN_RPCT_05`의 `rpct00_att_battle_16_01/02/03`을 사용한다. 원본 Projectile `421980301.loa`의 `CEFSequenceSummonsActionTimer`를 읽으면 카드 생성은 1.5초, 폭발 생성은 약 1.65초다. 두 ParticleSystem의 notify position/rotation은 0, scale은 1이다. 근거는 `out/KoukuAllEffects20260912/source/Projectile/421980301.loa`, `out/CardRain20260922/source/MN_RPCT_05.action-effects.json`, `source/cardrain-runtime-contract.json`이다.

`par_g_rpct_05_card_prj_01_loc_int`의 15 emitters와 `par_g_rpct_05_card_exp_01_loc_int`의 11 emitters를 복원했다. 카드 mesh `Effect/KoukuSaydon/FullRestore/Meshes/mn_rpct_05_01.wmodel`과 native material 2864/2865를 포함한 원본 material/resource/rotation 경로를 유지한다. 카드 테두리와 문양은 이 원본 mesh/material 경로가 소비하며 Light만 추가한 구현이 아니다.

7m 낙하가 보이지 않던 원인은 두 단계였다.

1. LocationDirect의 중첩 `ScaleFactor`에서 instance delta `Distribution=None`만 남아 runtime이 0배율로 계산했다. 원본 `engine.default__particlemodulelocationdirect`의 `LookupTable=[1]*8`과 일치함을 확인하고 후보에 상속값을 넣었다. 기존 라이브러리 전체에 임의 상수를 덮어쓰지 않았다.
2. EventReceiverSpawn이 부모의 7m 위치를 기본 origin으로 넣은 뒤 자식 LocationDirect의 7m emitter-local 위치를 다시 더해 본체가 14m에서 시작했다. `Client/Private/Effect_Playback.cpp`의 `Spawn_Particles`는 prepared recipe에 활성 `LOCATION_DIRECT`가 있을 때만 event origin을 0으로 시작한다. 일반 event receiver의 위치와 속도 상속은 유지한다.

실제 60Hz playback에서 발광 카드와 본체가 모두 7m에서 시작하고 약 0.35m에 정착한다. 원본 life와 normalized Direct curve가 달라 각각 약 0.15초/0.27초에 내려온다. 두 위치를 같은 곡선으로 강제하지 않았다. source integration의 소규모 overshoot를 포함한 최소 높이는 각각 0.340738/0.333332m이며 마지막 높이는 0.349944/0.349899m다.

노란 경고는 기존 native ground circle을 재사용한 `PROJECT_TUNED` 조합이다. 기본 반경 1m, 지름 2m, 0~1.65초이며 내부 채움은 1.5초에 완료한다. 실제 양의 alpha 샘플은 0.016667~1.633333초다. 원본 카드 1.5초/폭발 1.65초 지연은 유지했다. Server의 random nav target와 occurrence uniform scale 1~2는 통합 담당의 typed random emission이 소유한다. 해당 scale은 warning/card/impact group에 함께 적용되므로 경고 반경도 1~2m가 된다.

## 머리 위 카드와 기존 던지기 저작 보존

Action `4219819`의 stage 0, `par_l_rpct_05_sk_13_loc_int`가 실제 카드 4 emitters다. `sk_13_2`는 카드 mesh가 없는 링 연출이므로 머리 카드 대신 사용하지 않았다. 원본 notify는 0.25초, `Bip001-L-Hand → bip001-l-hand`, local position `[0,0.5,0]m`, scale 2다. 이 값은 effect 안에 있으므로 Composition occurrence에는 추가 bone/offset/scale을 중복 입력하지 않는다.

P48에 추가하는 overhead occurrence는 0~3250ms, BOSS follow true, position/rotation 0, scale 1이다. 설치된 MN_RPCT_05 WModel의 실제 `rpct00_att_battle_4_04` bone transform에 CModel preScale 0.017을 적용했다. bone index 27, 181 samples를 사용했고 0.25초 손 위치는 `(0.814862,2.221483,0.687902)m`다. 원본 cooked root 100배와 preScale을 함께 보존해 basis를 임의 정규화하지 않았다.

원본 `effect.kouku.common.spinning.card.throw` 224 elements는 보존했다. 파생 flying은 stage 0/3 준비 연출 16개만 제외하고 stage 1/4 링·먼지 16개와 24 projectile notify의 192 emitters를 그대로 유지한다. 각 element의 timing, attachment, material이 원본과 같은지 비교했다. 기존 P48 throw occurrence 6개의 시작/길이/transform은 통합 병합에서 보존하고 resourceId만 파생으로 바꾼다. 단일 발로 재해석하거나 기존 3.43~14.56초 내부 발사 지연을 재배치하지 않았다.

flying의 `FX_Buff_01`은 모델 bone 이름이 아니라 source slot이다. 실제 binding은 `b_root`, local position `[0,2,-3]`, rotation `[0,-16.5,90]`이다. 기존 sourceModelPreview의 여섯 clip window를 그대로 샘플링해 `b_root` index 2의 실제 transform 1,213개를 공급했다. root와 socket offset을 중복 적용하지 않았다. 근거는 `source/actual-anchor.json`, `source/flying-actual-anchor.json`, `sample-anchor.py`다.

## DJ 안내 원본 근거

원본 `EFTable_GameNote.db`의 110149/110157은 `common_gameNote_npc_img` index 231, AutoShutdown 5000ms를 참조한다. `EFTable_GameMsg.db`의 `tip.desc.guide_gamenote_raid_kouku_02/10`에서 두 문구와 `DJ쿠크` 이름을 확인했다. audio 담당이 사용하는 동일 GameMsg/SOUND ID와 연결한다.

portrait는 `EFUI_ICONATLAS_C`의 `common_gamenote_npc_img_4.dds`에서 index 231을 추출했다. 996×996 atlas의 7×7 grid, 142px cell과 1px border를 따라 `(853,569)..(995,711)`을 crop했다. 원본 `EFUI_COMMONOBJECT`의 GameNoteFrame symbol 853에서 frame/background/portrait/text 위치를 읽고, 원본 `HANYoonGothic760.ttf`로 문구를 합성했다. 배포 이미지는 671×145px다. 원본 프레임과 얼굴을 다시 그리거나 ImGui에서 캡처하지 않았다.

표시 방식은 기존 `boss.kouku.fear.face_1`과 같은 V2 ScreenPost `TexturedOverlay`를 재사용한다. normalized center `[0.5,0.78]`, size `[671/1920,145/1080]`, displaySpace true, bloom 0, lifetime 5초다. 화면 위치·fade·이름의 mint 색·한글 텍스트를 PNG로 고정한 부분은 `PROJECT_TUNED`다. 원본 이름 TextField는 orange 계열이지만 사용자 참고 이미지의 mint 색을 반영했다. 원본 font·프레임·초상화·문구를 사용했다는 것과 원작 HUD 전체를 동일 구현했다는 것은 구분한다.

근거는 `source/gamenote-kouku.json`, `source/dj-gamenote-provenance.json`, `source/commonobject.xml`이다. 재생성 도구는 `Tools/EffectPipeline/build_kouku_dj_gamenote_effects.py`, 카드 효과 도구는 `build_kouku_cardrain_groups.py`다. 원본 게임 파일은 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame`과 앞선 추출 cache에서 읽었다.

## Server 카드병정 계약

| 원본 Effect / NPC | source model | 기존 archetype |
|---|---|---|
| 421980307 / 480726 | `MN_PPCH_00-2` | `MONSTER_KOUKU_CARD_CLUB` |
| 421980308 / 480727 | `MN_PPCH_00` | `MONSTER_KOUKU_CARD_HEART` |
| 421980309 / 480728 | `MN_PPCH_00-1` | `MONSTER_KOUKU_CARD_DIAMOND` |

세 NPC의 ActionGroup은 `MN_PPCH_00`, ModelSize 120, SummonSpawnAction 2, SummonLifetime 0이다. 카드미로 원본의 동일 세 model family와 대응하며 spade는 포함하지 않는다. 원본 stage 1 summon notify는 각각 1.0/2.6/3.9초다. source row의 SummonCount/Amount 10을 이번 구현의 1마리 값으로 위장하지 않는다. 근거는 `source/card-soldier-provenance.json`이다.

새 `CARD_RAIN_SOLDIERS`는 추가 parameter가 없는 TRIGGER다. Composition logic 129에서 사용하며 `Composer parse/validate/serialize → Python projector → Publish-GameplayBalance → GameplayCatalog → LogicRuntime scheduled output → GameRoom Commit_KoukuMechanicTriggers`로 소비한다. Workbench trigger 선택 항목과 설명도 추가했다. Client가 local spawn하거나 새 네트워크 메시지를 만들지 않는다.

`Spawn_KoukuCardRainSoldiers`는 같은 방의 살아 있는 실제 boss·active pattern·catalog revision·navigation을 확인한다. 각 profile과 서로 1.5m 이상 떨어진 nav 위치 3개를 먼저 준비하고 기존 `Spawn_Monster`를 사용한다. 같은 owner/sequence의 live batch는 중복 생성하지 않는다. group은 `kouku.cardrain.<owner>.<sequence>`이며 maze target ledger에 등록하지 않는다. entity growth는 기존 world iteration이 끝난 commit 시점에 처리한다. detached occurrence owner가 live boss와 다른 경우 소환을 건너뛴다.

1종당 1마리, boss 반경 3~6m, 30초 cap, owner pattern 종료/sequence 변경/죽음/삭제 시 정리는 `PROJECT_TUNED`다. 원본 SummonLifetime 0을 무한 누적 소환으로 사용하지 않았다. world 초기화도 추적 map을 비운다. 설치된 세 maze MonsterProfiles를 그대로 재사용하므로 HP 300, 낮은 movement/aggro 0.01인 일반 타격 대상이며 원본 공격 AI를 복원한 작업은 아니다. 기존 maze 모델 scale 0.015와 source Npc ModelSize 120 차이도 재사용 범위다.

## 실행한 검증과 남은 확인

`out/CardRain20260922/final-probe.log`:

- V1 3개 `Stage_ProductLoadTarget → Stage_Document → 60Hz Playback` PASS, V2 2개 native Parse/Serialize 왕복 PASS.
- drop 27 elements / peak 64 / 마지막 visible particle 3.65초; overhead 4 / peak 11 / 2.25초; flying 208 / peak 606 / 17.0333초.
- 118,333 samples의 finite/capacity 검사 실패 0. WORLD root yaw 0/90/180도와 translation `(9,3,-11)` 공변 변환 오차 최대 `9.53674e-7`.
- 이전 playback과 동일 cardrain 문서를 비교하면 EventReceiver Direct 자식 109 samples에서 중복 7m만 제거되고 나머지 4,281 samples는 같다. LocationDirect를 제거한 controlled CPU event-receiver fixture는 이전/현재 4,390 samples가 byte-identical이다. 이 fixture를 실제 모델 부착 성공 근거로 사용하지 않았다.
- 새 parameterless trigger 허용과 unrelated teleport/HUD/count field 거부, flying detail/attachment/material 보존 검사 PASS. `focused-validation.json`에 결과를 기록했다.
- 통합 담당의 scratch C++ 18 TU 컴파일에 이 작업의 GameRoom/GameplayCatalog/CompositionDocument/CardMazeTests가 포함되어 PASS했다. 이후 추가한 Workbench의 작은 dropdown 변경은 최종 통합 빌드에서 확인한다.
- `ServerGameplayContractTests_CardMaze.cpp`에 실제 Saydon placement/nav로 typed commit 소환·세 suit·maze 비등록·중복방지·30초 만료·패턴 종료 정리 검증을 추가했다. 실제 실행 결과는 통합 담당의 `--card-maze-contract-test` 결과로 판정한다.
- 관련 변경의 `git diff --check` PASS. GPU의 발광 정도·화면 카드 크기·원본 HUD와의 최종 시각 차이는 사용자 확인 대상이다.

## DJ PNG의 실제 V2 Prewarm 복구

후속 Complete Play의 `player 3 preparation failed → raid effect prewarm failed → boss.kouku.dj.cardrain`은 파일 누락이 아니라 decoder 불일치였다. 두 설치 PNG는 기존 SHA-256과 같은 정상 RGBA8 671×145 이미지다. `CEffectV2Object::Acquire_Texture`가 확장자와 관계없이 DDS 전용 함수를 호출해 PNG를 거부했다. 이 DJ LEAF는 V1이 아닌 V2 ScreenPost 경로를 사용하며 delivery도 같은 결함의 대상이다.

`Client/Private/EffectV2_Object.cpp`에 WIC 헤더와 DDS/WIC 확장자 분기를 추가했다. DDS flags, authored colorTexturesSRGB의 FORCE/IGNORE 정책, Resources 상대 경로 검증, 성공 후 cache commit을 유지한다. WIC에는 device만 전달하므로 Loader worker에서 immediate context를 사용하지 않는다. 기존 MainApp/Loader COM 초기화를 소비한다. Data·PNG·asset ID·프로젝트 등록은 변경하지 않았고 DDS 변환도 하지 않았다.

수정 TU의 MSVC Debug 컴파일 PASS(`out/KoukuDjPrewarm20260922/compile.log`). 기존 EngineSDK의 CP949 주석을 scratch /utf-8로 읽는 C4828 경고는 있으며 컴파일 오류는 없다. 실제 설치 파일과 DirectXTK/WARP 검사에서 이전 DDS loader는 두 PNG 모두 `0x80004005`, WIC는 모두 S_OK였다. 생성 SRV는 linear BGRA8 형식87 또는 sRGB 형식91이며 Engine Presentation_Manager의 alpha coverage 허용 형식이다. 기존 fear.face_1 DDS는 BC3 linear77/sRGB78을 유지한다. 근거는 `asset-probe/probe.log`다.

수정한 전체 EffectV2_Object.cpp OBJ와 실제 RuntimeAssetRoot를 링크한 소형 native probe에서 두 PNG×linear/sRGB의 첫 Prewarm 및 cache 재사용 8회 성공, missing PNG와 root 탈출 경로 2회 거부, failures=0을 확인했다. 관련 없는 Part_Body/Valtan target-anchor 기호3개는 호출 시 abort하는 링크 전용 stub이며 이 검사는 SCREEN_POST Prewarm만 호출한다. Client/UI 실행·스폰·시각 판정을 대신하지 않는다. 근거는 `out/KoukuDjPrewarm20260922/prewarm-probe.log`다.

공식 Debug Product Build는 `20260922T142932411Z-debug-product.json`에서 실행 중인 Client PID31844와 Server PID45680의 표준 출력 점유로 시작 전에 차단됐다. 실행 파일은 아직 교체하지 않았다. 사용자에게 저장 후 직접 종료를 요청했으며 프로그램을 자동 종료하지 않았다. 최종 Product 반영과 사용자 Complete Play 확인은 이 검증과 분리한다.
