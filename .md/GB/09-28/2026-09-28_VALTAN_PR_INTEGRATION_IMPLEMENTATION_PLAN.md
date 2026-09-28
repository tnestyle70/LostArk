# 발탄 수정·PR 471~474 통합 구현 계획

## G00. 기준선과 보존

현재 `GB/Valtan-Patttern-Complete`의 HEAD는 `369987261a1e99c1de98a370c26b69acf3fc7023`이며 origin/main보다 3 commit 앞선다. 사용자는 이전 편집 저장·종료와 전체 적용을 승인했고, 이번 요청에서 4연속 공격과 추적 도끼의 판정, 복구 돌 이펙트 연결, Resources 전달, 네 PR의 검토·통합·Debug/Release 빌드·merge를 요청했다. 기존 dirty/untracked 199개 파일 150,301,842 bytes와 binary patch를 `out/ValtanIntegration20260928/baseline`에 보존했다. 무관한 사용자 변경을 되돌리지 않고 Resources·컴파일 산출물·로컬 백업은 Git에 넣지 않는다.

## G01. 판정과 표현 연결

FOUR_SLASH의 현재 단일 CONE와 HIGH_JUMP/LAND의 900 ms 공격력 비례 피해는 사용자의 저장된 표현과 맞지 않는다. 실제 effect cue의 source trim/offset/rate를 적용한 시각과 공간을 측정하고, 쿠크 노란 장판이 사용하는 Shared ATTACK_HIT_TEMPLATE 및 ServerCombatHit 판정 경로를 재사용한다. 추적 도끼의 마지막 착지는 최대 체력 50% 피해와 넉백을 같은 hit로 처리한다. 저작 Data, publisher, Server 권위 판정, Client preview 소비와 집중 검증을 같은 변경으로 연결한다. animation·Sound·camera의 사용자 편집은 유지한다. 구체적 source hit.contacts → Product attackContacts → STAGE bootstrap과 7개 판정 수치·구현 파일은 같은 날짜의 `2026-09-28_VALTAN_FOUR_SLASH_TRACKING_AXE_COLLIDER_IMPLEMENTATION_PLAN.md`에 기록한다.

피자·땅구르기 후 사자후의 돌 폭발은 현재 active에 연결된 복구 돌의 mesh/material과 대조하고 기존 발생 시각·파동·입자 수명을 보존한다. 버러지/발악의 데이터상 대상은 선택 질문으로 확인하며, 답변 전에는 실제 돌을 생성하는 STRUGGLING을 적용 대상으로 가정한다고 알리고 그 별도 explode도 같은 기준으로 연결한다. 현재 TRASH의 8개 연출 track에 돌이 없다는 실측을 보존하고 새 spawn은 추가하지 않는다. Source JSON 수정과 실제 재생 소비자를 함께 검사하고 새로운 별도 effect runtime을 만들지 않는다.

## G02. Resources 전달

### 복구 돌 폭발의 구체적 변경

현재 standing rock은 native2391과 복구 texture 5개, 사용자의 gap_offset 0.25를 사용하지만 debris는 native2452의 다른 회색 표면이다. `six-pizza.rock.explode`, 그 내용을 복사한 편집용 `six-pizza.rock.explosion.full`, `ground-roar.rock.explode`에서 debris material과 native2391 필수 dynamic 4채널만 바꾼다. X는 normalized age 0.7까지 1.1을 유지한 뒤 종료 때 0, Y/Z/W는 1이다. 원래 파편 수명 1.2초·16개 burst와 종료 fade 구간을 유지한다.

`fm_a_stone_001`은 32정점/12삼각형의 10.54×5.99×9.98 cm 파편이고 생성 돌 `fm_d_stoneparts_003`은 52정점/78삼각형의 175.48×433.60×118.45 cm 기둥이다. 기둥 geometry를 파편으로 그대로 복제하면 높이가 72.42배가 된다. 파편 geometry·UV·이동·색·size와 사용자 검은 파동·telegraph는 보존하고 생성 돌의 복구 표면만 공유한다. 파편의 UV0/N/T 채널과 native2391 wrap sampler 계약은 설치 WModel로 확인한다. 이는 요청한 동일 돌 표면의 저작 파편화이며 원본 폭발 전체를 복구했다는 주장이 아니다.

확인된 변경 물리는 유령 발탄의 원본 normal/tangent 복원 `Character/Valtan/Ghost/MN_RPBF_02.wmodel`과 에테르 복원의 `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_00/fx_a_line_002.dds`, `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_high_00/fx_c_atypical_003.dds`다. 설치된 현재 bytes를 `C:/Users/user/Desktop/GBResources2` 아래 같은 Resources-relative 경로에 복사한다. 이번 추가 돌 수정에서 실제 물리 파일이 필요해지면 그 참조도 포함한다. Data JSON은 Git, EXE/DLL/CSO는 구성별 제품 빌드로 전달한다. 복사 뒤 원본/대상 길이와 SHA-256을 비교하며 전달 증거는 out에만 둔다.

## G03. PR 검토와 통합

검토 기준은 #471 `369987261a1e99c1de98a370c26b69acf3fc7023`, #472 `451de3f42dd92558c70ed90c8781b7a92ea6107b`, #473 `ad3c821f0023fc946b5cf6e331a89fb8dccd5620`, #474 `bf9092793a726c9116a33b24f38ea005f95feb4e`다. #471은 현재 작업 브랜치다. 각 HEAD의 변경과 실제 소비자를 검토하고, #473의 파손 HUD 크기 누적 문제는 통합본에서 원본 크기를 기준으로 계산하도록 수정한다.

발탄/쿠크 저장 변경을 기능 commit으로 보존한 후 나머지 세 PR의 exact HEAD를 현재 브랜치에 병합한다. 충돌은 필드·함수별로 해결하며 파일 전체 ours/theirs로 기존 기능을 폐기하지 않는다. 다른 PR의 source material program 등록·shader mirror·새 project/filter 항목·게시 데이터와 현재 발탄 native84 복원을 함께 보존한다. 필요한 publisher만 실행해 source와 runtime output을 일치시킨다.

## G04. 검증과 main 반영

변경 기능의 focused 계약 검사, JSON/XML parse, diff check를 수행한다. 통합 exact HEAD를 push하고 독립 검토한 뒤 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug` 및 `-Configuration Release`의 Product Build를 순서대로 실행한다. Engine → Shared → Server → Client와 SDK/shader/runtime 배포, missing/invalid runtime input을 확인한다. 일반 Build를 사용하고 Clean/Rebuild나 추적 기록 조작은 하지 않는다. 통과한 통합본을 #471을 통해 main에 merge하고 포함된 다른 PR의 병합 상태와 최종 main ancestry를 확인한다. merge 후 소스가 달라지면 해당 변경을 다시 검증한다.

Client/UI를 자동 실행하지 않는다. 실제 화면과 게임 FPS는 사용자 확인이며 빌드·구조·Server 판정 검증과 구분해 RESULT에 기록한다.
