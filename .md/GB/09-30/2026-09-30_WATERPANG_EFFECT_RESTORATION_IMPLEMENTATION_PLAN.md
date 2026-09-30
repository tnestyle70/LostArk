# 워터팡 원작 이펙트 복원과 World·All Effects 연결

## G00. 현재 소비자와 원본 범위

목표는 마하라카 워터팡의 중앙 기둥 분사, 중앙 바닥 예고·폭발, 물총 Q/W/E/R의 실제 원본 이펙트를 용도별 한국어 이름으로 정리하고 같은 문서를 경기와 도구에서 재생하는 것이다.

시작 기준은 `codex/valtan-authoring-and-entry-20260930`, HEAD `4fce79521e7a48f57c6d3351ab272dcbd0e67246`이다. 다수의 다른 작업 미커밋 변경을 보존하며 자동 stage/commit하지 않는다. LAN sync는 Team/server-host, 192.168.0.22:7777 reachable, 방화벽 준비 완료다.

기존 catalog는 워터팡 hazard 11개와 물총 총구 4개를 등록한다. World 분류가 `effect.world.`만 허용하여 `effect.maharaka.waterpang.*`와 `effect.maharaka.watergun.*`가 All Effects의 World 목록에서 제외된다. 또한 모든 `.restore`를 플레이어 스킬로 다루는 복원 preview 분기를 구분해야 한다. 단순 표시 이름 변경만으로 재생 연결이 완성되지 않는다.

원본 자료는 기존 `out/MaharakaWaterpangFX20260928`, `C:/LostArkExtract/MaharakaWaterGunFX20260928`와 설치된 UE3 추출 자료를 재사용한다. 물총은 원작 GADGET 56902(Q 연발), 56912(W 물폭탄), 56920(E 이동속도 증가), 56932(R 기본 사격) 및 projectile 569020/569120/569320을 대조한다. E의 action 부재를 임의 입자로 채우지 않고 buff 자료까지 확인한다. 원본 입자 복원과 기존 프로젝트의 중앙 반경 5m·경기 일정·서버 판정 조정은 구분한다.

## G01. 원작 구성과 material·resource 복원

`Tools/EffectPipeline/build_maharaka_waterpang_source_effects.py`와 `build_maharaka_watergun_source_effects.py`가 기존 Inanna/vehicle/native material 경로를 재사용한다. 중앙 분사·모코모코·바닥·시작/종료·분할 회전 문서의 source emitter coverage와 보류 재질을 확인한다. Q/R의 비행·피격, W의 비행·폭발 및 확인된 버프 표현을 별도 stable ID로 복원한다.

입자별 시간, 공간, notify TRS, 모델·socket basis, local/world 구분과 원본 material/texture를 보존한다. program ID는 설치된 registry와 충돌 없이 기존 adapter를 재사용한다. 후보는 별도 out/evidence 경로에 만들고 Data/Effects·Resources·공유 shader에는 검증 뒤 적용한다. 새 C++ 파일이 필요해지면 같은 변경에서 프로젝트와 filter 등록을 포함한다.

## G02. 네이밍과 저작 라이브러리

표시 이름은 `워터팡 | 중앙 기둥 물 뿜기`, `워터팡 | 중앙 바닥 장판`, `워터팡 | 중앙 바닥 터지기`, `워터팡 | Q 연발 물총 …`, `워터팡 | W 물폭탄 …`, `워터팡 | R 기본 물총 …`처럼 기능과 단계를 구분한다. 기존 effect ID와 WorldSequence instance ID는 유지한다.

`Data/Effects/Authored`의 root displayName, `EffectResourceTree.json`의 World/마하라카/워터팡 분류와 표시 이름을 같은 기준으로 연결한다. 신규 문서는 `EffectCatalog.json`의 DIRECT_AUTHORED_DOCUMENT 및 Client `96.DataFiles` None 목록에 추가한다. All Effects와 실제 재생이 다른 복사본을 참조하지 않게 한다. WorldSequence의 공격 template 이름도 동일한 용어로 변경하며 기존 motion/placement/material/camera 편집을 보존한다.

## G03. 실제 경기와 도구 재생

All Effects의 World 분류와 선택/미리보기 경계를 함께 확장한다. 중앙 이펙트는 기존 WorldSequence의 실제 object/model/bone을, 물총 총구는 실제 player prop bone을 사용한다. 단독 root projectile/hit은 부착 효과와 구분한다. source model이 필요한 이펙트를 임의 player root로 재생하지 않는다.

원작 비행·피격의 경기 연결은 기존 서버 권위 projectile 경로의 확정 위치/시각을 Client presentation이 소비하도록 한다. Client가 피격·피해를 독자 판정하지 않는다. 불필요한 gameplay 수치/일정 변경을 넣지 않는다. 기존 lifecycle과 실패 시 이전 상태 보존, 레벨 퇴장/중단 시 handle 해제를 유지한다.

## G04. 최신 저장본 반영과 검증

현재 Client/Server/VS가 실행 중이다. 후보·코드·구조 검증을 먼저 마친 뒤 저장 여부와 최종 반영을 한 번 확인한다. 승인 후 최신 저장본을 다시 읽어 stable ID/필드 기준으로 병합하고 hash 재확인·백업·원자 교체·자기 변경 rollback을 유지한다. 렌더링 옵션은 수정하지 않는다. WorldSequences는 해당 scope의 정본 publisher로 검증·게시한다.

검증은 변경 JSON/XML parse, 원본 occurrence/resource 연결, source validator의 해당 문서 검사, 실제 codec/playback의 시간 sweep·유한 transform·0이 아닌 draw 제출 가능 범위, 필요한 정상 증분 Product Build와 `git diff --check`다. 다른 빌드와 동시에 같은 출력을 쓰지 않으며 실행 중 EXE의 링크 잠금은 사용자 종료 뒤 해결한다. Client/UI 실행·캡처와 최종 화면 판정은 사용자가 한다. RESULT에는 소스 적용, 게시, 빌드, 수치 검증, 사용자 화면 확인을 각각 기록한다.

## G05. AI 20명과 3분 경기

후속 요청에 따라 인간 최대 4명 외에 `WATERPANG_AI` 20명을 별도 Server-owned player로 생성한다. 가짜 socket/session이나 인간 party member를 만들지 않는다. 가디언 나이트·창술사와 장착 아바타 12명, 워터팡 주민·베른 NPC 외형 8명을 사용한다. NPC 외형은 immutable spawn의 stable NPC archetype ID로 전달하고 기존 NPC catalog/model 경로에서 Client presentation만 생성한다. 이동·피격·물총·낙사·부활은 동일 Server player 경로를 따른다.

도입 예약의 실제 경기 시작부터 180초를 진행한다. Server tick에서 판단·대상 선택·목적지 갱신·Q/W/E/R 사용을 수행하며 게임 종료 시 AI와 잔여 투사체를 정리한다. 인간은 navigation/collision 검증된 경기장 외곽 섬 좌표로 이동하고 같은 마하라카에서 자유 탐험을 이어간다. Client 남은시간·종료 연출은 같은 Server 시계와 STOP 메시지를 소비한다. AI를 인간 파티 인원, 가이드, 레이드 투표와 혼동하지 않는다.

## G06. AI Tool의 서버 튜닝

AI Tool은 봇 수, 판단 tick, 이동 목표 갱신 tick, 스킬 간격 tick, 대상 탐색 거리, 이동 확률, 공격 성향, 넉백 거리/시간의 현재 값과 편집 draft를 분리한다. typed command sink를 통해 GET/APPLY/SAVE를 보내며 Server가 범위와 expected revision을 검사한다. 성공 응답 전에는 적용으로 표시하지 않는다.

정본 `Data/AI/MaharakaWaterpangAI.json`은 서버가 읽고 Save+Apply로 원자 저장한다. 저장 실패나 revision 충돌은 이전 실행 값·파일·UI draft를 보존한다. Client가 파일 저장이나 socket 호출을 직접 하지 않는다. 변경한 wire 계약은 protocol 127로 함께 전달하며 신규 CPP/H/JSON은 기존 프로젝트/filter에 필요한 항목만 등록한다. 사용자 화면 실행은 여전히 수동이다.
