# Guardian 재생·화신화·ALT V 연결 결과

> G00~G05는 앞선 배포 기록이다. 이후 사용자 화면 피드백에 따른 최신 수정·검증은 G06 이후를 따른다.

## G00. 실제 반영 범위

Z 화신화의 최대 identity 진입 조건을 제거했다. 빈 게이지에서도 기존 Server command로 요청하며, 실제 stance commit에서 identity와 Ember orb를 채운다. 기존 3초 공통 cooldown, action/stance 검사와 15초 변신 지속·해제는 유지한다. Client에서 게이지나 stance를 임의 변경하지 않는다.

Full Restore Play All의 COMBO stage는 선택한 실제 모델의 skillbindings와 animevents를 직접 대조한다. 전체 Effect tree의 다른 class enrichment 실패 때문에 ProductCues가 비어 있어도 선택한 LMB의 exact clip/stage를 조회한다. 모델 선택 중 catalog reload가 발생하므로 이전 skill 포인터를 재사용하지 않는다. saved Effect load 실패, disabled 버튼 이유, 재생 시작 이후의 실패는 현재 Effect 목록에 표시한다. 모델 시간·root·factory가 실패하면서 이유를 남기지 않던 경로도 상태 문구를 남긴다.

ALT V의 opaque background mesh 7개에 기존 `compositionLayer=sceneBackdrop`를 연결했다. 실제 해당 carrier가 활성인 구간에 기존 공통 배경 숨김을 사용한다. 최신 JSON bytes를 백업하고 해당 7필드만 바꾸어 원자 교체했다. camera source basis와 Close-up/Zoom out row는 [별도 결과](2026-09-22_GUARDIAN_ALTV_CAMERA_RESULT.md)를 따른다. 카메라의 90도 basis 오류는 교정했다. gameplay에서만 용 자체가 90도 달라 보이는 별도 원인은 재현·확정하지 못했으며 정상 Play All까지 일괄 회전하지 않았다.

일반 S49220 스피닝 플레임은 GROUND_POINT로 연결했다. S로 파란 범위·청백색 목표 원을 열고 LMB로 확정한다. 승인된 목표에 원본49290의 body 두 클립, 실제 head/neck CModel 두 개와 85 elements를 재생한다. source native110/111, 실제14 bones, 원본 재질·본 부착을 사용한다. body 재생률2.888889/1.333333은 기존 Server 첫 hit300ms·총2초와 맞춘 presentation adapter다. 변신 S49230과 기존 gameplay ID·피해·Ember 수치는 유지한다. [파츠 결과](2026-09-22_GUARDIAN_PREVIEW_PARTS_RESULT.md)에 원본 조사와 shader 검증을 기록했다.

## G01. 연결한 다른 작업

- [Monster 결과](2026-09-22_MONSTER_COMPOSITION_DAMAGE_ONLY_RESULT.md): Character/Clown/Monster 접기, 13종 실제 모델·462 clips·공격 collider, 공격 피해 1회 및 push/down 제거.
- [Gate3 결과](2026-09-22_GATE3_WORLD_AURA_IMPLEMENTATION_RESULT.md): 원본 진입/활성/리스폰 오라와 replicated pose·Server tick 기반10초, 기존 typed 입장·파티 동의 경로.
- [Guardian shader/carrier 결과](2026-09-22_GUARDIAN_CARRIER_RENDER_REPAIR_IMPLEMENTATION_RESULT.md): native Trail/Decal 처리 범위·UV/접선·masked mesh 원본 입력 및 ALT V 후속 화염 복구.

## G02. 검증 상태

World publisher를 live `Publish`로 실행했고 종료코드 0이다. runtime diff는 Valtan/Kouku/Character Select의 spawngroupsbootstrap 3개이며 대상 몬스터의 공격 push/down 4필드가 반영됐다. boss profile과 몬스터 자신의 피격 반응은 보존했다. Server 재시작 이전의 실행 메모리는 갱신되지 않는다.

Gameplay 공식 Validate·Publish도 통과했다. 게시 bootstrap의 `SKILLTARGET 49220 GROUND_POINT 4.5 1`과 actual catalog 소비를 확인했다. 일반 S의 청백색 preview tint는 Client presentation 필드다.

actual Product Server object와 게시 bootstrap을 연결한 headless 진단은20/20 통과했다. 빈 identity Z 승인·commit 전 값 보존·commit 후100/10orbs·15초 해제, Monster 피해·event1회 및 push/down 없음, Gate3 entry/respawn/층/NaN, S의 intent·사거리 실패 시 비용 보존과 승인 목표의300ms hit를 검사했다. 로그는 `out/GuardianPlaybackRepair20260922/guardian-server-probe.log`다.

Product Debug의 Engine/Shared/Server/Client 전체 컴파일·링크·배포는 `20260922T052333767Z-debug-product.json`에서 통과했다. 중간 MSB3107 project XML 및 missing PLAYER_STANCE_ID include 오류는 수정 후 재빌드했다. 마지막 shadow VS 수정이 FXC 실행 중 들어가 기존 timestamp 검사만으로는 최종 shader 반영을 보장하지 못했다. 실제 CSO의 새 light 행렬 심볼 부재를 확인하여 해당7개 shader input을 다시 컴파일했고 `20260922T055940381Z-debug-product.json`에서7CSO 생성·Client 링크·배포를 통과했다.

완성 Product CSO7개를 headless WARP의 실제 CShader로 로드했다. base+6cohort의 input layout/pass 계약, scene/light 행렬 바인딩, native0/1/9/17/25/80/84/98/99/110/111의 pass15 Begin11건이 통과했다. 이 검사에서 HLSL 내부static인 g_EffectSceneReadMode를 외부 uniform으로 바인딩하던 호출을 찾아 C++에서 제거했다. 해당 변수는 shader invocation의 초기0을 그대로 사용한다. 셰이더 수식 변경은 없으며 마지막 C++ 파일의 Product 증분 빌드·링크·배포도 `out/BuildPipeline/runs/20260922T060146218Z-debug-product.json`에서 통과했다(Engine/Shared/Server/Client PASS, Client OBJ1·binary1·CSO0).

최종 설치 Shader7개의 새 light 행렬·shadow pass 심볼과 SHA-256은 `out/GuardianPlaybackRepair20260922/final_probe/shader-receipt.json`, 실제 로더 검사는 `shadow_runtime.log`에 기록했다. 작업 중 별도 세션이 소스 변경을 `bcea95d88`로 commit했으며 이 작업은 그 commit을 되돌리거나 추가 commit/push하지 않았다. 이후 최종 바인딩 수정2줄과 검증 문서 갱신은 현재 working tree에 남는다.

최신 JSON55개·project XML2개 parse, source actor4개·camera/owner control9개 Python 검사와 `git diff --check`를 통과했다. 검사 대상 animevents는 JSON이 아니며 실제 Client parser/slot join15건으로 확인했다.

광역 Server 계약 검사는 기존 KoukuProduct fixture의7개 연쇄 실패를 확인한 뒤 중단했다. HEAD와 현재 Gameplay.world.json 및 게시 worldbootstrap은 모두 g1kouku/g1saydon을 disabled로 둔다. 이번 World publish에서 해당 worldbootstrap의 diff는 없다. 해당 기존 실패와 위 focused20/20 통과를 구분한다.

## G03. ALT V 화염과 그림자

원본 Action49420 notify044(3.79초)의 SkillEffect/Projectile494200을 추적했다. 후속 FireBreath01/Local,02/03/04, noise, DragonDecal의58 elements를 추가해 현재 문서는136 elements다. 새 native4553..4566의14 color programs와18 distortion companions를 원본 DXBC와 대조했다. 추가 리소스7개를 설치했고 기존 background7개·ModelCue5개를 보존했다.

**DragonDecal은 실제 원본 effect element다.** 시작은3.79+1.4=5.19초이며 용 실루엣 DDS를 원본 재질로 투영한다. 전체221 elements/7 ModelCues의 최신 Codec·roundtrip·Stage·Seek 검사에서 이 decal은5.225~7.675초의50개 표본을 출력했다(50ms 간격). 이전 진단이 particle/trail만 세어 decal의 `Frame.Elements`를0으로 보고한 문제도 진단에서 교정했다.

이 element와 별개로 움직이는 용의5개 skeletal ModelCue에 `castsShadow=true`를 연결했다. 원본 EF skeletal actor/component의 공통 CastShadow·dynamic/static shadow 기본값은true이며 notify 자체에는 override 필드가 없다. native actor 생성 구현까지 확인한 것은 아니므로 원작의 개별 actor 정책과 스크린샷이 실시간 그림자임을 확정한 것으로 기록하지 않는다. 이 구간은 원본 기본값에 근거한 복원 판단이다.

실제 구현은 기존 CModel pose/root/material track과 SHADOW render group을 사용한다. 추가 pass15는 opaque/masked surface의 원본 dissolve·opacity를 평가한다. source 재질의 실제 scene camera와 raster용 light View/Proj는 분리한다. 다른 ModelCue의 optional 필드 기본값은false이며 afterimage·native Effect material·translucent shadow는 거부한다. 최신 CPU 검사에서5개 opt-in 왕복, legacy 누락=false, 잘못된 타입 거부를 확인했다.

## G04. GBResources 전달

`C:/Users/user/Desktop/GBResources`의 기존 Resources-relative 구조에 추가 파일675개를 넣었다. 이미 동일했던234개를 포함해909개(457,672,433bytes)의 SHA-256이 설치 Resources와 일치한다. 기존 다른 파일은 보존했고 교체한 기존 파일은0개다. Guardian 본체·파츠·이펙트, 일반 S 용머리, ALT V 후속화염/DragonDecal, World 오라와 이들이 공유하는 texture/model을 함께 포함했다. Data·소스·컴파일 산출물은 이 resource 전달본에 넣지 않았다.

전달 목록과 해시는 `out/GuardianPlaybackRepair20260922/resource_delivery/receipt.json`이다.

## G05. 화면 확인 경계

Client 실행·UI 조작·화면 캡처는 하지 않았다. shader 수식, 설치 모델·본, 실제 codec/playback, Server 명령·피해, 컴파일 검증과 원작 화면 일치를 구분한다. E의 Tool/gameplay 표시 차이와 ALT V의 gameplay 전용 방향 차이는 독립 재현 증거가 없어 해결됐다고 단정하지 않는다. 사용자 화면에서 정상 S 목표 원→LMB 용머리/원형화염, 붉은 LMB/F, ALT V 줌아웃·배경 숨김·후속화염·그림자를 확인한다.

## G06. 후속 범위와 All Effects 문서 편집

사용자가 Character Select 맵 재질·캐릭터/의상 재질 복원을 별도 세션으로 이관했다. 이 후속 작업은 Guardian 이펙트, All Effects 편집, 공통 조명·그림자 합성 및 Guardian Kouku 입장 준비를 소유한다. 미완성 의상 consumer hunk는 다른 변경이 없는지 확인한 뒤 자기 변경만 역적용했고 인계 patch·조사 자료를 보존했다.

preview Character clone에는 Server snapshot이 없으므로 `Try_Get_NetworkStance`가 실패했다. preview 종료 시 복원할 presentation stance를 별도로 읽도록 고쳤다. 실제 scene Character는 이 편집용 stance 변경의 대상이 아니다. Saved Unified Effect에는 기존 Editable Elements tree를 연결하고 Open 실패 이유를 표시한다. 화면의 `Last document validation ... passed`는 실패가 아니었으며, 새 표시는 Saved document의 실제 검사 상태다.

GunSlinger animevents의 선언2327행/실제2326행 불일치가 전체 class enrichment를 중단하여 Guardian·Dimension의 Product 연결까지 비우던 경로도 수정했다. 실패 class만 이전 상태로 보존하고 다른 class를 계속 연결한다. 헤더 한 필드 수정본은 후보로 준비했다. 변경3TU compile, actual Client parser, lazy metadata9개 검사 통과. 기존 exact revision test의 MainApp 함수명 기대1건은 이번 변경과 무관한 기존 오류로 분리했다.

## G07. Kouku 준비 실패와 native trail 시간

사용자 실행 로그는 Server world5 승인 뒤 Client 준비52/57에서 실패했다. 원인은 LMB49000clip0/1, Q49100clip0, R49260clip2, D49150clip2의 source-owned AnimationTrail closure였다. 원본 baked sample은 notify 종료보다 먼저 끝날 수 있는데 runtime projection이 playback clamp와 notify duration의 일치를 강제했다. source native animationTrail의 clamp를 min(notify duration, source end)와 대조하고 일반 저작 baked trail의 엄격한 duration 계약과 sourceRecipe.enabled 금지는 유지했다.

기존 동일57 target의 실제 document-owned projection → resource preparation → prevalidated stage →0.05초 seek가57/57 통과했다. CPU 문서·준비 검사이며 Client/Server의 실제 입장 화면을 확인한 것은 아니다. [용 shader·입장 근거](2026-09-22_GUARDIAN_DRAGON_SHADER_RAID_GATE_RESULT.md)에 로그와 수치를 기록했다. 최종 S binding을 실제 Client parser로 읽은4개 cue join이 통과했으며, `Load_ForProductPrewarm`가 반환한 Guardian46 IDs는 기존 입장 target의46개와 정확히 같다.

## G08. 공통 렌더링 합성과 설명

PBR map marker3의 RNM/IBL과 실제 emissive를 분리했다. 기존 geometry MRT의 비어 있던 RGB로 간접광을 보낸 뒤 Deferred에서 SSAO·fog와 합성한다. 진짜 발광과 캐릭터 material path는 유지한다. optional `shadow.dynamicBakedStrength`는 기본0이며 static shadow cache와 최종 depth의 차이를 이용하는 프로젝트 동적 차폐 근사다. 원본 GI 복원으로 기록하지 않는다. 캐시 무효 시 추가 감쇠만 끈다.

Workbench는 SSAO의 metre 단위, shadow texel과 bias의 실제 거리, direct/indirect/emissive의 차이를 설명한다. Source Tone이 켜져 계산에 쓰이지 않는 Hable White Point는 비활성화한다. [빛·그림자·GI·Nanite 가이드](2026-09-22_RENDERING_LIGHT_SHADOW_GUIDE.md)에 현재 수식, 흐림의 원인과 조절 순서를 정리했다. 바닥 노멀/재질 연결의 정확성 및 노란 색감의 재질 부분은 별도 세션 범위다.

실제 Deferred WARP 검사는 새 간접광·AO·동적/정적 shadow·emission48경우와 기존 receiver6/조명216경우 모두 통과했다. profile optional field의 roundtrip 및 invalid type/range rollback 검사와 후보 publisher Validate가 통과했다. Engine C++ 컴파일·링크, Client C++ 컴파일과 shader 컴파일을 수행했다. 실제 CShader base/cohort probe는126program,504light passes,144native passes,6clones,9failure cases를 통과했다. 이는 shader 로더/바인딩 검사로 draws0·windows0이다.

현재 데이터 후보의 최종 저장본 병합·runtime publish·Product 최종 링크/배포 상태는 아래 최종 적용 기록에서 별도로 확정한다. 이 문서의 사전 검사만으로 후보 설치나 화면 일치를 주장하지 않는다.

## G09. 후속 데이터 반영과 수치 검증

사용자의 계속 진행 지시에 따라 최신 저장본을 stable ID·변경 field로 병합했다. Guardian11파일117필드, RenderingProfiles18필드와 GunSlinger animevents header1필드의 총13개 source 파일을 반영했다. 교체 직전 batch/각파일 SHA를 확인하고 백업·같은 디렉터리 임시파일·원자 교체·자기 변경 rollback을 사용했다. Guardian11파일은 조사 이후 충돌/재기반0이며 최종 후보와 의미상 동일했다. 실행 중 Tool draft를 Reload하거나 Client/Server를 종료하지 않았다.

| 대상 | 실제 변경 | 근거·경계 |
|---|---|---|
| 일반 S49220 | 원본 FlameScale2클립, GROUND_POINT row 제거, 원본 Projectile492200의16emitters 추가 | 기존 즉시 AIM_POINT 입력. 원본 속도11m/s·최대4.5m·수명2.5s를 공통 source transform track으로 전달. 피해/쿨타임은 유지 |
| 화신화 S49230 | source49290 용2클립 재연결 | 기존 GROUND_POINT4m·skill_target/snapshot을 유지. 원본49230 스킬과 동일하다는 주장은 하지 않음 |
| 일반 LMB3 | wind/meshtrail7요소를 수직 평면으로 보정 | plane normal(0,-1,0)→(-1,0,0). 원본 반경·UV·curve 보존. 사용자 방향 보정 |
| R clip2 | 원본4개 baked 검격 strip·native lane·타일링 및 clamp 연결 복구 | 원본17표본은 XZ 회전 궤적. 별도 수직화는 하지 않았으며 LMB3 방향 보정과 구분 |
| Z LMB BA2/3 | 해당 검격36요소만 Y+1.2m | 실제 설치 모델/본의 BA2 mesh Y[-.526,-.252]→[.674,.948], BA3[-.921,-.658]→[.280,.542]. 바닥 decal 유지 |
| 일반 A 방패 | 모델 preScale0.01→0.03, 약3배/폭1.02m | 본/socket/particle 원본 크기 보존. 실제3clip 최저Y .314/.315/.064m로 지면 위. 과대한10배 후보는 폐기 |
| Z A | 창·나선4mesh만 시계90도 | TypeData/MeshRotation·높이·크기·다른6mesh 및 decal/sprite 보존 |
| ALT V | 독립 DragonDecal 방향90도, 용5part Y+3m | 사용자 장면 보정. 자식11개는 변경하지 않아 부모 높이가1회만 전달 |
| ALT V 화면 불꽃 | camera29 socket 및 fixed-axis sprite13개 보정 | 활성 원본124/124 이미 연결됨. camera 아래/near plane 뒤로 가던 좌표 교정. native camera 코드 복구와 구분 |

최종 후보가 들어간57문서의 actual projection·resource preparation·prevalidated stage·첫 frame은57/57 통과했다(2250elements/25models/41ownerControls). 모든 문서의 전체 수명을 GPU로 검증한 것은 아니다. 앞선 전체 CPU 수명 순회57개와 후보8개, 추가 camera/bone/수명/negative clamp focused probe를 각각 구분한다. 원본 clamp6문서의 허용/조기/초과18조건이 통과했다.

신규 원본 effect native4567~4571의 DXBC/HLSL20조건 최대오차0, 현재 Product4544코호트2개의 FX ABI, 실제 CShader Create/Begin38pass와 선택5개 PS 상수·texture binding 검사가 통과했다. S 용 native110/111의 zero BRDF lookup NaN 수정과 양수 lookup 원본동치 검증은 [별도 결과](2026-09-22_GUARDIAN_DRAGON_SHADER_RAID_GATE_RESULT.md)를 따른다. 원본 BRDF LUT/SH는 미복구이며 shader의 수치 안정성을 원작 조명 완전 일치로 부르지 않는다.

최종 source JSON과 게시 Rendering runtime의 strict JSON parse13개 및 `git diff --check`가 통과했다. Rendering profile 공식 Publish는 성공했다. 일반 S targeting 공식 후보 Validate/격리 Publish가 통과하여 SKILL49220/49230은 보존되고 `SKILLTARGET49220`만 제거됨을 확인했다. live Gameplay Publish·최종 Product Debug 상태는 G10에서 확정한다.

이번9개 Effect 문서가 쓰는 설치 Resources318개를 점검했다. GBResources에 없던 공용 texture5개만 추가했고318/318 SHA가 일치한다. 기존 파일 교체0, 신규 native 프로그램은 shader 소스/CSO이며 Resources 파일을 새로 만들지 않았다. 전달 receipt는 `out/GuardianVisualFollowup20260922/gbresources-receipt.json`이다.

Guardian 최종 백업·교체 증거는 `out/GuardianFollowup20260922/apply/20260922-165750-036990/applied.json`, rendering/header 증거는 `out/GuardianVisualFollowup20260922/root-data-installed.json`에 있다. 수치 비교와 테스트 명령은 각 `out/GuardianFollowup20260922/RESULT.md`, `out/GuardianPlaybackRepair20260922/native4567_probe/RESULT.md`에 기록했다.

## G10. 최종 게시와 사용자 빌드 경계

사용자가 최종 Debug 빌드는 직접 실행하겠다고 지정했다. 따라서 사전 Engine/Client 컴파일·shader/loader 검사와 최종 Product 빌드·링크·배포를 구분한다. 이번 후속 변경을 포함한 최종 Product 빌드는 실행하지 않았으며, 앞선 G02의 EXE 완료 기록으로 이를 대신하지 않는다. Debug 전체 빌드 후 새 Server/Client로 실행해야 한다.

첫 live Gameplay Publish는 사용자가 새로 저장한 Kouku 원본 revision2197에 비해 생성 문서가 오래돼 차단됐다. 새 패턴105 `쿠크_나팔액션`,106 `쿠크_저글링액션`을 보존하고 공식 projector로 Encounter/patternbindings를 재생성했다. 기존94패턴·raidGates·9bundles는 보존하며 새5 animation stage가 추가되는 정식 게시 경로다. 잘못된 상태를 무시하는 Validate 우회는 하지 않았다.

공식 Kouku Publish와 후속 Gameplay Publish는 모두 exit0으로 완료했다. Kouku sourceRevision2197, productPatternCount96, productStageCount505이며 새2패턴은 AUDITION_ONLY다. Gameplay hit-shape92/92, Valtan source/parity/alignment 검사가 통과했다. 최종 bootstrap에 `SKILLTARGET 49220`은 없고 `SKILLTARGET 49230 GROUND_POINT 4 1`은 보존된다. 로그는 `out/GuardianVisualFollowup20260922/kouku-publish.log`, `gameplay-publish-final.log`, 최종 hash/확인은 `final-publish-receipt.json`에 있다.

사용자가 저장 후 Client/Server 종료를 확인했고 실행 프로세스 부재를 확인했다. 그 이후13개 source 파일 hash는 적용 receipt와 모두 일치했다. Client/UI는 자율 실행·Reload·종료하지 않았다. 재질/비행은 별도 세션 작업이며 해당 변경을 이 Guardian 후속 검증 결과로 인증하지 않는다.

Guardian/공통 렌더링 변경 범위의 최종 `git diff --check`는 통과했다. 전체 workspace 검사에서 별도 차량 작업의 `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1:92` trailing whitespace1건이 관찰됐으며 이 세션에서 무관한 hunk를 수정하지 않았다. 해당 내용과 profile source/runtime 의미 일치는 `out/GuardianVisualFollowup20260922/final-verification.json`에 구분했다. 최종 사용자 화면 확인 항목은 편집 열기·Kouku 입장, 두 stance의 S/LMB/A/R, ALT V 불꽃/그림자/용 높이, 공통 그림자 경계다. 원작 화면 일치나 원본 BRDF LUT/GI 전체 복구는 미확정이다.
