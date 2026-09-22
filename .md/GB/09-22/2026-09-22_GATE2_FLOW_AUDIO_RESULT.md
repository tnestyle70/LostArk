# 쿠크 2관문 반복·사운드·등장 자막 결과

## G01. 코드 반영 완료

`patternFlows.loopStartEntryId`를 같은 Flow의 stable entry ID로 저장한다. Codec, Boss Tool 저장·수동 Flow 재생, Python projection, PowerShell `RAIDGATE` 게시, Server catalog와 RaidFlow 완료 소비자를 모두 연결했다. 최초 앞 구간은 한 번 실행하고 마지막 wait 뒤 지정한 entry부터 반복한다. 미지정·빈 문자열은 기존 동작이며 저장 시 필드를 생략한다. 기존 12/13열 RAIDGATE도 읽으며 지정값만 optional 14번째 열로 게시한다.

Client는 기존 GATE1 처음 반복을 유지하고 Server는 GATE1/BINGO를 유지한다. 잘못된 참조는 저장·게시·catalog admission에서 거절한다. Tool에서 기점 entry 삭제 시 설정도 지우고 reorder에서는 stable ID를 유지한다. Server의 반복 기점이 첫 entry여도 최초 입장과 구분하여 epoch·게시 revision·잔여 row를 유지한다. Stop·중단·관문 완료 경로는 바꾸지 않았다.

변경한 기존 C++는 CompositionDocument, PatternAuditionService, BossTool, ActionWorkbench, GameplayCatalog, GameRoom_KoukuRaidFlow다. 새 C++ 파일은 없으므로 project/filter 등록은 없다. public 계약은 TEAM_GAMEPLAY_INTERFACE_HANDBOOK과 gotchas의 기존 Pattern Flow 문단에 갱신했다. 기존 다른 세션의 변경은 보존했다.

## G02. 사운드와 자막 연결

최종 통합은 authored revision2198에 설치됐다. 사용자의 정정에 따라 P11/P24/P25/P85의 기존
사운드6/7/20/11행은 추가·수정하지 않았다. 원본 대조에서 자동 보충하려던44개는 후보에만
있었으며 실제 파일에 설치한 적이 없다. `prepare_all_gate2_audio.py`는 보충 소유자를 P99로
제한하고 사용자 소유 Pattern 전체를 보존하는 검사를 수행한다. 최종 연결은 P105/P106의
Action28개, 저글링 공12개, 바람3개, 등장공 fixed template2개로 총45 SOUND 행이다.
실제 설치 음원은 기존232개와 추가16개, 총248개를 검사했다. 게시·전체 관문 검증의 최종
상태는 [2관문 통합 결과](2026-09-22_KOUKU_GATE2_PATTERN_RESTORATION_RESULT.md)를 따른다.

`out/Gate2FlowAudio20260922/prepare_audio_title.py`는 최신 문서를 입력받아 별도 복사본만 반환한다. 원본 MN_RPCZ_00 action 4219713/4219716의 AKEvent를 실제 clip·stage·source-in·playRate와 대조한다. 현재 두 저글링 stage 기준 나팔 12개, 저글링 16개 source action cue를 연결하며 기존 catalog와 설치 WAV 39개를 재사용한다. SOUND occurrence 길이는 resource 길이와 남은 pattern lifetime 중 작은 값으로 제한하고 animation/stage 시간을 늘리지 않는다.

P9에 사용자 문구를 기존 SUBTITLE 계약으로 연결하는 후보를 만든다. stable resource ID는 `subtitle.kouku.gate2.saydon.appear`, 이름은 `세이튼_등장`, 시작 0ms, 길이 7400ms다. 문구는 정확히 두 줄이다.

> 여러분들, 뿅망치 살인마가 등장합니다!
> 과연 누가 살아남을까요?

원본 projectile 421971601/421971901의 AKEvent 네 개는 설치 Wwise bank 166743361에서 실제 Play target과 random playlist를 추적했다. MOB_GLOBAL3_NONSTREAM에서 미디어 16개를 sparse 추출하고 vgmstream으로 PCM16 WAV를 변환했다. 각 이벤트는 서로 다른 네 variant이며 원본 가중치는 모두 1/4다. 임의 합성·복제 음원을 만들지 않았다.

| Event | 각 variant 수 | 최대 길이 |
|---|---:|---:|
| g_kouku1_attack04_proj1 | 4 | 2767ms |
| g_kouku1_attack04_projexp1 | 4 | 1367ms |
| g_kouku1_attack07_proj1 | 4 | 3667ms |
| g_kouku1_attack07_projexp1 | 4 | 1734ms |

`prepare_projectile_audio.py`의 `apply_projectile_audio_resources(composition, catalog, events=None)`는 선택 이벤트의 resource/catalog 후보만 반환한다. 최종 통합은 저글링 Attack04와 등장공 Attack07의 네 이벤트·16 variant다. 등장공 Effect와0/3033ms SOUND는 동일 SHOWTIME fixed group에 들어가 기존 targeted MAP SoundCue 소비자를 재사용한다. Codec/projector는 소유자가 있는 fixed MAP 그룹에만 SOUND를 허용한다. 원본 bank/WEM/WAV SHA256, package offset, 확률 근거는 `projectile-audio/candidate-events.json`과 `weights.json`에 보관했다.

하위 오디오 작업은 후보만 작성했고 통합 담당이 승인된 최신 저장본 기준 stable field 병합과 atomic 설치를 수행했다. 최종 게시 상태는 전체 Gate2 결과를 따른다.

## G03. 실행한 검증

- `test_raid_flow_projection.py`: 11개 PASS. 실제 PowerShell RAIDGATE 14열 게시, legacy 형식, optional boundary, 다른 gate/없는 ID/잘못된 type 거절, reorder와 입력 불변성을 포함한다.
- 현재 production Composition codec native probe: 9개 PASS. parse/serialize, stable ID reorder, invalid parse의 기존 문서 보존, legacy 생략을 검사했다. 통합 담당의 최종 circle·223-element candidate도 갱신 후 같은 실제 codec probe 9개에 다시 통과했다.
- 현재 Server 완료 전이 블록 native probe: 14개 PASS. prefix 1회, 반복 꼬리 여러 주기, reorder, GATE1/BINGO legacy 반복, GATE2/GATE3 legacy 종료, 누락 기점 종료를 검사했다.
- 실제 현재 CGameplayCatalog native probe: 5개 PASS. 설치 bootstrap, scratch optional loop 열, 잘못된 loop admission 거부 및 기존 generation 보존을 검사했다.
- 기존 production TU 직접 Debug 컴파일 PASS: CompositionDocument, PatternAuditionService, BossTool, ActionWorkbench, GameplayCatalog, GameRoom_KoukuRaidFlow. 후속 singleton 변경과 첫-entry epoch 조건을 포함한 CompositionDocument/RaidFlow도 다시 컴파일했다.
- audio/title 후보와 resource/catalog 후보 멱등성, 기존 assets 존재, 이벤트/variant 연결, WAV 헤더·길이·SHA256을 확인했다.
- scoped `git diff --check` PASS.

초기 catalog probe에서 이전 날짜의 Brain/Logic object를 링크해 root-motion 충돌처럼 보였으나 현재 source의 Brain/Logic/Navigation/PlayerSkillSystem을 다시 컴파일해 실제 catalog 5개 검사가 통과했다. 라이브 데이터의 motion 결함으로 보지 않으며 관련 baseline을 수정하지 않았다.

전체 249개 projector suite는 기존 데이터 기대값 실패 후 중단했으므로 PASS로 기록하지 않는다. 대표 Albion takeoff 기대값 실패는 이번 변경 전 projector를 현재 데이터에 실행해도 동일했다. 이번 기능에 필요한 위 11개 검사는 통과했다.

정상 Product Debug runner와 후속 증분 빌드 결과는 아래 G04에서 구분한다. Client/UI 실행·청취·전체 장면 화면 확인은 하지 않았다.

## G04. Product 빌드와 동시 shader 편집의 경계

첫 정상 Product Debug runner에서 Engine/Shared/Server 컴파일과 링크는 PASS했다. Server support contract는 별도 격리 DataFiles와 이번 Server.exe로 실행하여 271.537초, failures 0을 확인했다. 실행 근거는 `out/Gate2Restoration20260922/source-effects/server-support-contract-isolated.log`이며 fixed-only 등장 yaw/target 보존과 기존 P103 카드비 회귀가 포함된다.

사용자의 추가 ALT V 중앙 cube 수정이 첫 Client shader compile 중 반영됐다. 기본 Product runner는 Core의 source fingerprint 안정성 검사와 무효화를 호출하지 않으므로, PASS 또는 output mtime만으로 새 shader 적용을 판정할 수 없다. 실제 FXC PID 45844/50324/56560의 시작은 main HLSL 수정(17:52:18) 전이고 SourceGroup160/176/192 CSO 출력은 수정 후였다. 이 3개는 read/write tlog로 source→output을 정확히 연결했다. base animated CSO도 새 uniform을 반드시 포함하도록 함께 재생성한다.

위 4개 생성 CSO만 절대 경로·현재 hash·source hash·활성 FXC를 확인하는 백업 후 무효화 후보를 준비했다. 실행 직전 다른 정상 Product의 해당 FXC가 시작돼 검사에서 중단했으며 이 작업은 실제 CSO를 삭제하지 않았다. 이후 그 정상 Product가 base·160·192를 새로 생성했고 마지막 176도 다음 정상 Product 소유자가 재생성했다. 다른 생성 파일·tlog를 광역 삭제하거나 `Rebuild`·source timestamp 조작을 하지 않았다. 조사 근거와 실행 전 검사 코드는 `stale-shader-invalidation.json`, `Invalidate-CubeShaders.ps1`, `Invalidate-Shader176.ps1`에 남겼다.

첫 정상 Product runner의 최종 상태는 **Client FAIL**이다. Engine/Shared/Server는 PASS했고 Client에서는 shader 20개와 OBJ 46개를 작성했으나 링크에 도달하지 않았다. receipt는 `out/BuildPipeline/runs/20260922T091134682Z-debug-product.json`이다. 당시 compiler 출력은 경고로 잘려 실제 error 원문을 확보하지 못했으므로 특정 원인을 확정하지 않는다.

shader를 다시 실행하지 않는 `Client.vcxproj /t:ClCompile` 진단은 별도 오류 로그를 지정해 재실행했고 exit 0, 오류 로그 0바이트로 통과했다. 근거는 `out/Gate2FlowAudio20260922/client-compile-recovery.log`, `client-compile-errors.log`다. 이 CPP 성공을 Product 링크·배포 성공으로 대신하지 않는다.

추가 독립 검토에서 `CShader::Stage_ProgramVariants`가 전체 uniform 일치를 요구하므로, 새 `g_ALTVCaptureBoneIndex`가 base animated와 SourceGroup 10개 모두에 있어야 함을 확인했다. 정상 후속 생성물 전체 11개를 아래 G05의 실제 reflection과 CShader admission으로 검사했다. 같은 출력 경로의 다른 정상 Product가 진행 중일 때에는 생성 CSO 무효화와 추가 Product 실행을 보류하여 중복 작업을 피했다.

두 번째 정상 Product receipt `out/BuildPipeline/runs/20260922T094535521Z-debug-product.json`도 Client FAIL이다. 이번에는 diagnostic log로 EffectBounds/EffectAmbient/EffectMarker profiler enum 미선언 및 Profiler 배열 길이 불일치를 확인했다. 로그는 `out/CharacterMaterials20260922/product-build/20260922T091955958Z-Client-Debug.log`다. 이 Client 시작은 18:19:55였고 Engine/Public/Profiler.h는 18:37:53에 다른 작업에서 갱신됐다. 18:46:42 시작한 다음 정상 Client 빌드에서는 제품 `PrepareEngineSDK` 복사 경로를 거친 source/SDK Profiler.h SHA256이 일치한다. 표준 라이브러리 전용 PCH에는 해당 header가 들어가지 않는다. 이번 범위의 enum·Profiler 코드는 추가하거나 되돌리지 않았다.

세 번째 정상 Product는 `out/CharacterSizeSave20260922/product-final-build` 로그를 남기며 Engine 재링크(18:46:39), Shared/Server와 Client 빌드·링크·배포를 모두 완료했다. 마지막 animated SourceGroup176은 18:54:16에 정상 FXC `/O1`로 생성되어 대상 11개 CSO 모두 새 세대가 됐다. 이 작업은 다른 세션의 compiler나 공유 출력 파일을 중단·삭제하지 않았다. 최종 receipt는 `out/BuildPipeline/runs/20260922T100454659Z-debug-product.json`이며 19:04:54에 Product Debug **PASS**로 완료됐다. Engine/Shared/Server/Client 전부 PASS, Client compiler error 0, missing/invalid runtime input 모두 0이다. 추가 중복 Product는 실행하지 않았다.

## G05. 새 Product shader의 실제 CShader 검증

새 `Engine/Bin/Debug/Engine.dll`(18:46:39, SHA256 `022d9c8f8b9ab5cc53d178db75e082e94179ddfc3ca714d2449928fa01ddfce7`)과 정상 FXC가 생성한 animated base·SourceGroup 10개의 실제 CSO를 `out/AltVCentralCube20260922/independent-review/admission`으로 복사하여 1회 검사했다. source 3개, Engine Shader source 2개, 원본/복사 DLL, 원본/복사 CSO 11개를 실행 전후 SHA256으로 고정했다.

- FX11 reflection 11/11: `g_ALTVCaptureBoneIndex` valid, uint scalar 4 bytes, SetRawValue/GetRawValue S_OK.
- 실제 `CShader::Create -> Stage_ProgramVariants`: base와 10개 program의 input layout·pass·uniform 계약 PASS.
- native178 uniform Bind/Begin, sentinel mask와 중앙 bone mask 각각 모든 variant로 copy/Begin하는 20개 호출 PASS.
- exit 0, generationUnchangedAfterAdmission=true. Client/UI를 열지 않았고 이 검사는 scene draw를 수행하지 않았다.

명령은 `python out/AltVCentralCube20260922/independent-review/admission/run_after_product_build.py --engine-dll Engine/Bin/Debug/Engine.dll`이다. 근거는 같은 폴더의 `admission.receipt.json`, `reflection-fresh.log`, `admission.log`다. Product 배포 완료 뒤 `Client/Bin/Debug/Engine.dll`과 CSO 11개 모두 이 검사의 입력 hash와 정확히 일치함을 확인했다. 최종 설치 대조 근거는 `out/Gate2FlowAudio20260922/product-deployed-admission-hash.json`의 `allMatch=true`다. 동일 입력이므로 CShader 검사는 반복하지 않았다. Client.exe SHA256은 `0ec9339e77bcb102115ea664a1aa8cf27c03a7f51d1890ef0169bbb33610815c`다. 이 Product 성공은 아직 미설치인 Gate2 authoring/Resources 후보 게시나 사용자 화면 판정을 대신하지 않는다.

최종 scoped `git diff --check`는 변경 18개 경로에서 PASS했고, 기존 C++ 10개의 BOM·CRLF 보존을 확인했다. 이 하위 작업은 코드·원본 음원 후보·검증을 완료했다. 사용자 최종 저장본 병합·데이터 게시 여부와 실제 화면 확인은 통합 RESULT에 별도로 기록한다.
