# 쿠크 레이드 낙사·연출·빙고 복구 구현 계획

## 목표와 현재 실측

2026-09-21 사용자 요청 범위는 레이저 재피격, 대형 세이튼 2·3연타의 장판 중심 기준 포물선 넉백과 낙사, 2관문 전체 클리어 연출, 쇼타임 회전, 3관문 가짜 클리어 후 8초 앵콜과 빙고 최종 엔딩이다. 추가 요청의 원본 음향·유리·빙고 화살표·위로 솟는 타일 오라·폭탄 확대·뒤집힘 멈춤·3관문 갈고리 포획도 같은 작업에서 연결한다.

현재 레이저 Trigger는 repeatAfterKnockback이 이미 켜져 있어 Once 표시만으로 원인을 단정할 수 없다. 쇼타임의 실제 Server 판정은 현재 보스 회전을 소비하지만 노란 warning 네 occurrence의 followBoss가 false다. GATE3 intro는 2관문 클리어 원본 SCENE02A의 뒤쪽만 잘라 놓은 Sequence7이며 전체 Sequence5는 별도로 존재한다. 기존 Character 11클립 베이킹에 SCENE01B 최종 엔딩 배우는 포함되지 않았다.

## G01. Server 권위 포물선 넉백

`KoukuSaydonLogicRuntime`, `PlayerSkillSystem`, `ServerCombatHitRuntime`, `GameRoom_PlayerSimulation`에서 실제 접촉 영역 중심을 전달하고 비행 중 navigation/collision 투영을 생략한다. 수평 거리와 비행 시간으로 속도를 정하고 9.8m/s² 중력으로 Y를 적분한다. 바닥 없는 공간의 하강은 기존 FALLING과 사망을 사용한다. 막힌 바닥과 허공은 별도 surface 정보로 구분한다.

Composition Result의 optional `pushBallistic`과 `AWAY_FROM_CONTACT`를 Client codec/editor → Python projector → Gameplay publisher → Server parser에 연결한다. 기존 push 기본값·행 형식은 유지한다. ballistic은 거리 0 초과 100m 이하, 시간 100~5000ms, pushCanLeaveArena=true를 요구한다. 접촉 방향에는 yaw offset을 허용하지 않는다. ColliderDamageSettings 편집도 force/canLeave/ballistic/yaw 정책을 보존한다.

## G02. 클리어·앵콜·엔딩 상태 연결

Sequence5 전체 35.368초를 GATE3 handoff의 intro로 선택하며 Sequence7의 네 player arrival을 원본 offset 16.710초만큼 뒤로 옮겨 사용한다. GATE3 보스 사망은 Dungeon Clear 표시 후 8초를 Server tick으로 대기하고 Sequence10 원본 앵콜을 재생한다. MVP는 이 가짜 클리어에서 생략한다. 앵콜이 끝나면 빙고 전투, 빙고 보스 사망은 Sequence9 최종 엔딩으로 이어진다.

기존 Character 모델에 원본 배우의 실제 SCENE01B 클립을 추가하고 원본 카메라·음향·유리 타임라인과 비교한다. Resources 상대 ID와 기존 CModel/WorldSequence 재생 경로를 사용한다.

## G03. 빙고·갈고리 표현과 판정

첨부 원본과 설치 effect/source material을 대조해 화살표 모양과 타일 경계 오라의 방향을 복원한다. 원본 크기와 사용자 요청 배율을 구분해 빙고 폭탄을 최소 5배로 적용한다. 뒤집기에서의 동기 준비·반복 모델/애니메이션 로딩 여부를 실제 호출자로 확인하고 필요한 준비를 기존 preload 경로로 옮긴다. 갈고리는 authoring collider/grip → publisher → Server contact/grab → Client attachment를 끝까지 대조한다.

## G04. 설치와 검증

각 담당은 독립 코드와 out 후보를 준비하며 root가 최신 디스크 문서의 stable ID·필드 단위로 병합한다. 동시 저장 검증과 백업·원자 교체·실패 rollback을 유지한다. 필요한 domain publish 후 실제 parser, 포물선·낙사·반복 피격·raid tick 흐름·타임라인·model clip native 검사를 수행한다. 변경 TU 컴파일과 git diff --check를 실행한다. 실행 중 제품 EXE/DLL을 덮어쓰거나 사용자 프로세스를 종료하지 않는다. Client/아레나 화면·오디오 최종 확인은 사용자가 수행한다. 추가 Resources는 GBResources에도 동일 hash로 전달한다.

기존 C++ 파일만 확장한다. 새 제품 C++ 파일이 필요해지면 project/filter 등록을 같은 변경에서 반영한다. 실행한 검사와 미실행 제품 화면 검증은 RESULT에서 구분한다.

## G05. 게시 뒤 Complete Play와 빙고 Sequence admission

현재 게시 WORLD revision2145는 ending 배우5개를 포함하지만 실행 Client는 게시 전 문서를 사용한다. 새 Complete Play 준비에서 idle인 기존 WorldSequencePlayer를 공식 published Area Load 경로로 갱신한다. 실행 중 sequence는 교체하지 않고 명시적으로 중단을 안내하며 parse/validate 실패는 이전 문서를 보존한다. 준비 중 revision 검사는 유지한다.

Sequence P10 앵콜은 Server raid와 publisher에서 BINGO combat handoff로 소비하지만 Client Validate_Shape의 GATE1~3 whitelist가 이를 격리한다. BINGO를 현재 계약에 맞게 허용하고, 연결된 Complete Play는 Server admission에서 실제 raid intro를 선택한다. standalone queue만 기존 단일 entry 조건을 유지한다. 빙고 버튼은 반복 전투 재생임을 표시하고 컷씬 단독은 P10/P9의 일반 Play를 사용한다.

새 C++ 파일과 project/filter 등록은 없다. 실제 Composition reader로 P10/P9가 격리되지 않고 원본 lane/시간을 보존하는지 확인하고 변경 TU를 컴파일한다. 현재 Client 출력 점유가 해제된 뒤 정본 증분 Product Build로 설치한다. 데이터 원본·게시본 재작성과 Client/UI 자동 실행은 하지 않는다.

P10의 MAP SOUND 3개는 source codec·Product occurrence reader·Python publisher가 같은 고정 음향 계약으로 허용한다. bone/follow/world occurrence/emission index는 거부한다. 같은 Area WORLD 갱신은 Loader가 소유한 AREA_LEAF snapshot을 보존하고 sequence 전용 GPU 준비는 갱신한다.

## G06. 최종 엔딩 얼굴 본 단위 수정

사용자가 재생한 최종 엔딩에서 세이튼 턱이 아래로 길게 늘어났다. 새 ending baker는 원본 SkelControl translation에 BASIS만 적용하지만 기존 Saydon bake는 본체 로컬 단위에 맞게 0.01을 적용한다. 설치 골격·root 및 원본 control 값을 직접 대조한 뒤 영향을 받는 ending clip만 재베이크한다. Character 본체 geometry/material/skeleton/다른 clip은 byte 단위로 보존한다. 새 후보는 out에 만들고 현재 파일 hash 재확인·백업·원자 교체로 설치하며 동일 Resources를 GBResources에도 반영한다. 저장된 Sequence/Pattern ID·시간·위치·카메라는 변경하지 않는다. 전체 timeline 본 위치와 원본 control delta의 단위 일치를 확인하고 화면 최종 판정은 사용자에게 남긴다.

## G07. 앵콜 이펙트 형식과 최종 설치본 검증

실제 실패한 spark0와 같은 source의 spark는 v13 내용에 version15만 기록돼 필수 runtimeExtensions가 없다. 정상 generator 후보와 현재 저장본을 byte 단위로 대조하고 version만 복구한다. P9/P10의 전체 이펙트·모델 clip·음향·카메라 참조를 함께 검사한다. 검증된 최종 후보 hash와 실제 설치 hash를 대조해 중간 staging의 오래된 문서가 설치되는 경우를 거부한다. source 또는 사용자 저장본이 바뀌면 자동 덮어쓰지 않는다.
