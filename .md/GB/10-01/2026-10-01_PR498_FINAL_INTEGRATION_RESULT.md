# PR498과 현재 반영 전체 통합·최종 빌드·ZIP 결과

## G01. 통합한 변경과 보존

사용자가 승인한 현재 폴더의 전체 변경84파일을 9403f6d49에 기록하고, PR498의
5e6bb64ea7c71419be88bff52135ef020e8519c1을 93d0aea6784676dee1f94e1e591279fc48bda43d로
로컬 병합했다. 공통 base는 PR497의 b83d646bebc8e6cffdd3a6cb84355b7c7f47f734다.
유일한 공통 수정 파일인 팀 사용서는 양쪽 변경이 자동 병합됐고, C++/data 파일 충돌은 없었다.
PR498의13개 비공통 파일과 기존 변경의83개 비공통 파일은 각 병합 전 commit과 동일하다.
이 상태의 서버 검증 후 GitHub PR498을 한 번 병합했고 main commit은
60b814d63418cdd4bb3c2717a2999876492a3b0d이다(2026-10-01 10:09 KST).
검증 전에 이미 합쳐 놓은 동일 PR 코드를 원격 main에도 반영한 것이며 중복 적용하지 않았다.

콜로세움 넉백10%(ALT_V1.6m/150ms, V0.51m/217ms), 용병 ALT_V 승인 간격30초,
콜로세움 HP 절반(92,660,680/20줄), 컷씬 HUD·이름 숨김, 결과 버튼/폰트, 캐릭터/MVP
TGA 연결, 쿠크1관문 입장 soundTrack 제거를 포함한다. 기존 가이드·CPU 이동·재질/배칭·
프로파일러·Client 종료 진단 변경과 현재 저장된 가이드 데이터도 함께 포함한다.
각 기능의 기존 PLAN/RESULT가 상세 구현·수치·이전 검사 정본이며 이 문서는 최종 통합 증거다.

기존 tracked diff와 untracked 원본은 out/FinalIntegrationPR49820261001에 보존했다.
backup/retired 및 비활성 생성본13개는 지우지 않았고 Git에 추가하지 않았다. 활성 Valtan descriptor
e22201a2122a4cecaf36c9a79145f318c8f1697ac1006041df5ed9403722dcc5는 bootstrap과 함께 포함했다.
오래된0byte index.lock은2시간 동안 갱신되지 않았고 git 프로세스·부모·파일 점유가 없는 것을
확인한 뒤 같은 out에 백업하고 제거했다. 사용자 Client/Server 종료 회신 후 Product를 빌드했다.

## G02. 명예의 속삭임 강화창

MainApp의 BuildItemUpgradeSlots가 보유 snapshot의 양수 수량, combat category,
weapon/helmet/shoulder/top/pants/gloves equipSlot 및 EQUIP_*_HONORWHISPER_<SLOT>
stable ID를 대조한다. 실제 보유·장착 항목만 남기며 다른 세트·장신구·소비품·소유하지 않은
장비를 추가하지 않는다. 현재 catalog의 해당 세트는5종 class×6부위=30개다.

창 열기와 매 프레임 후보 갱신이 같은 함수를 사용해 여섯 행 아이콘·배경·중앙 아이콘과
선택 표시를 동기화한다. 빈 행의 예시 아이콘을 숨기고 빈 목록에서는 성장/재련 입력을 막는다.
현재의 강화 결과/성공률·인벤토리·Server 계약은 변경하지 않았다.

실제 후보 함수/표시 동기화 함수와 ItemCatalog를 사용한 native focused probe는
19 PASS / 0 FAIL이다. 수정 전 함수는 같은 필터 조건에서8 PASS / 6 FAIL로 회귀가 재현됐다.
증거: out/ItemUpgradeHonor20261001/{baseline,candidate}/{receipt.json,test.log}.
강화창 수정 직후 MainApp.cpp SHA256은
fcc462aa4a0775d3faf6ec47b3aacce96050779f557f59ad37d0773015390e26이다. 이후 G06은
강화창 함수에 손대지 않고 Release 도구/FPS 경계만 바꿨다.

## G03. 통합 데이터·UI·패키징 검사

| 검사 | 실제 결과 |
|---|---|
| 변경 JSON12개 parse 및 통합 diff check | PASS |
| Colosseum cutscene roster | 381 checks PASS |
| Colosseum debug preview | 11 tests PASS |
| Colosseum combat HUD | 9 tests PASS |
| Guide 저장 병합·충돌·검증실패·동시 추가 | 4 tests PASS |
| Maharaka 기존 entry test2개 | 2 tests PASS |
| ReleasePackaging | 22 tests PASS |

Maharaka source/viewer/bootstrap은 revision372, 배치42개와 점프 목적지3개가 일치한다.
양쪽 nav SHA256은 f7b2c08359b60425987e0285c9147b0167419f86bb7917eb904a5ada68a507ad로 같다.
낙하·점프·넉백 착지 변경은 MAHARAKA+live intro 조건이며, 콜로세움의 동일 활성match·서로
다른 유효team 공격 조건과 PvP10% 넉백·HP·용병30초 변경은 병합 전 그대로 보존됐다.

패키저는 Gameplay.bootstrap의 단일 유효 PATTERNPRESENTATIONGENERATION을 해석해
활성 descriptor1개만 넣는다. 누락·중복·잘못된 ID·hash 불일치는 실패한다. 실제 디스크의
60개 중 비활성59개를 패키지에서 제외했으며 원본60개 hash는 전후 동일하다.
증거: out/ReleasePackaging/active-generation-fix/{collect-receipt.json,test-package-tools.log}.

## G04. 정상 Product 빌드 및 실행 계약

Debug/Release 정상 Product(Engine→Shared→Server→Client)는 모두 PASS, SkipBuild=false이며
missingRuntimeInputs/invalidRuntimeInputs는 모두0개다. Debug receipt는
out/BuildPipeline/runs/20261001T010915300Z-debug-product.json이다.
Release receipt는 out/BuildPipeline/runs/20261001T011148751Z-release-product.json이다.
두 receipt 모두 G06 촬영 표시 숨김까지 포함한 마지막 정상 Product 빌드다.
기존 문자 인코딩 경고와 DirectXTK PDB 부재 링크 경고는 남았으며 오류 없이 링크·배포됐다.

| 실행 계약 | Debug | Release |
|---|---:|---:|
| Maharaka AI / World playback | 286 PASS | 246 PASS |
| Colosseum Combat | 95 PASS | 95 PASS |
| Colosseum Match | 318 PASS | 318 PASS |
| Colosseum base | 25 PASS | 25 PASS |
| Navigation | 48 PASS | 48 PASS |
| Guide AI | 76 PASS | 76 PASS |
| Server 합계 | 848 PASS / 0 FAIL | 808 PASS / 0 FAIL |
| CPU 이동 dispatch | 27 PASS | 27 PASS |
| 로컬 이동 following | 189 PASS | 189 PASS |

Maharaka 차이40개는 WorldPlayback의 _DEBUG 전용 viewer/arrival 검사다.
Debug scratch runner는 제품6개 검사 완료 뒤 OrderedDictionary 합산에서만 오류를 냈다.
원래 summary를 보존하고 실제 개별 exit0·PASS 로그와 hash를 다시 합산한 verified receipt를
남겼다. Release runner는 전체 exit0이다. 일반 서버 loop/Client를 실행한 검사가 아니다.
증거: out/FinalIntegrationPR49820261001/focused-contracts-final.json,
focused-contracts-Debug-verified.json, focused-contracts-Release.json 및 movement-*/result.json.

마지막 촬영 UI 재빌드 뒤 두 Server.exe와 Gameplay.bootstrap이 실제 검사 때의 SHA256과
동일함을 확인했다. Client/Engine/Server의 최종 hash와 Product receipt 연결은
out/FinalIntegrationPR49820261001/final-built-inputs.json에 기록했다. 서버를 중복 검사한
것으로 표시하지 않았으며 G06은 Client MainApp.cpp 한 파일만 다시 컴파일됐다.
Client/UI는 실행하지 않았고 화면·실제 오디오·실플레이 판정은 사용자 확인 범위다.

## G05. 리소스 전달과 최종 배포

사용자 지정 C:/Users/user/Desktop/GBResources에 아래 파일을 실제 설치본과 byte/hash가
동일하게 전달했다. 기존 GBResources2에도 이전 요청의 TGA 전달본을 보존한다.

| Resources 상대 경로 | bytes | SHA256 |
|---|---:|---|
| Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga | 1048620 | 1135362e95f0ecca6c648d7a6d16e196ebe2ea7cf96e60c26646def8bbf3c8d2 |
| Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.tga | 194948 | 1513554e7e79aed15e1784fb4d07d5b791b8e3f42dfc0f87dfb1337ebb8f2c6c |
| Character/LanceMaster/textures/brdf_beckmann_spec.tga | 194948 | 1513554e7e79aed15e1784fb4d07d5b791b8e3f42dfc0f87dfb1337ebb8f2c6c |
| Sound/Maharaka/WaterpangSource/scene_maharakap_fallout_foley.wav | 660258 | 7a33497a2839d1f7053c34a3aa8a044a2155f3451e3c8fefdbc39a8819dc9e52 |

최종 ZIP 생성과 검사는 PASS다. 파일은
C:/Users/user/Desktop/LostArk-Release-20261001-FINAL.zip, 168,880,935bytes(약161.06MiB),
SHA256 e955f4f7891459aa3eb703189a10dbd859e4409640ec3fe76f640cde63ad3b21이다.
payload2766개, 직접 Data2178개, 컴파일 shader256개, numeric binding587개를 포함한다.
Resources는0개이며 기존 외부 Resources 연결을 사용한다. 위 GBResources 전달은 별도다.
ZIP CRC·모든 payload SHA256·중복 경로·Resources/PNG/PDB 제외·최종 원본 불변 검사가 PASS다.
launcher --check는 PASS이고 Client/Server 게임 프로세스를 시작하지 않았다.

이번 Release build receipt, launcher/CRT, 최종 EXE/DLL/CSO와 Data/DataFiles를 사용했다.
패키지의 gitHead는 실제 제품 소스가 확정된209be209709fcce9bf4e96a3c3c44791fec079ab이다.
이후 통합 RESULT 문서 commit과 원격 병합이 제품 소스·데이터를 바꾸지 않는지 마지막으로 대조한다.
증거: out/ReleasePackaging/portable-delivery.receipt.json,
preflight-20261001-final-pr498.json, 20261001-final-pr498/bundle-manifest.json.
원격 통합 PR의 실제 병합 commit은 GitHub와
out/FinalIntegrationPR49820261001/remote-merge-receipt.json에 기록한다.

## G06. 촬영용 Release와 Visual Studio 설정

사용자가 추가 요청한 F1 ImGui, F7 프로파일러, FPS 숫자 숨김은209be2097에 반영했다.
MainApp.cpp의 F1/F7 입력, DeveloperTools/Profiler 렌더, RenderFpsText 출력을 기존
_DEBUG 전용으로 제한한다. Level별 창 제목 FPS도 이미 _DEBUG 전용이다. Debug의
저작 도구, 일반 제품 UI·채팅·파티·레이드 UI와 ImGui backend 수명, F6, 프레임 제한,
비동기 profiler 저장 완료 회수는 유지한다. 새로운 설정 파일이나 runtime 분기는 추가하지 않았다.

최종 MainApp.cpp SHA256:
58e169396ff68ef31364d173d0747b14b551a012583e373a44343dd268f2a54b.
증거: out/ReleaseRecording20261001/source-receipt.json. 이 변경 뒤 양 구성 Product를
다시 빌드했고 G04에 최종 receipt를 기록했다.

Visual Studio의 Client/Server .vcxproj 및 개인 .user 실행 설정을 읽어 확인했다.
작업 폴더는 각 ProjectDir, Client는192.168.0.22:7777, Server는0.0.0.0 bind이며
현재 PC의 Wi-Fi 2가192.168.0.22를 소유한다. Framework.slnLaunch의 Server + Client
profile이 두 프로젝트를 실행한다. Release/x64에서 그 profile을 사용하며 F5/Ctrl+F5는
VS 설정에 따라 Build를 수행할 수 있다. 설정 확인과 실제 게임 화면 실행 성공은 구분한다.
만료된 LAN 자동 설정 스크립트는 실행하거나 AllowExpired로 우회하지 않았다.
