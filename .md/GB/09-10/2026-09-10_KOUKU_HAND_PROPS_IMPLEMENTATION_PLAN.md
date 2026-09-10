# 쿠크 손 소품 월드오브젝트와 패턴 연결 구현 계획

## G00. 현재 상태와 목표

현재 브랜치 `codex/kouku-publish-live-update`의 기존 미커밋 변경을 보존한다.
작업 시작 Composition revision239, Area WorldSequence revision423을
`out/KoukuHandProps20260910`에 보존했다. Client와 Server는 종료 상태다.

`쿠크_레이저` P21은 5833ms, `쿠크_팡파레` P23은 10667ms이며 두 패턴 모두
2관문 쿠크를 사용한다. 원본 WP_1 소켓의 cooked BODY bone은 `b_wp_1`이다.
기존 WorldSequence WORLD/PLAYER와 Composition BOSS_SPAWN은 손 추적을 지원하지 않는다.

목표는 두 소품을 World Object Tool에 등록하고 기존 패턴의 0ms부터 끝까지 WORLD 박스로
연결하여, 보스 몸동작의 손 위치와 회전을 따라가게 하는 것이다. 쇼타임은 원본 총 리소스를
확인해 재사용 가능한 Object로 준비하고 미구현 쇼타임 패턴은 임의 생성하지 않는다.

## G01. 기존 WorldSequence의 보스 본 추적

WorldSequence v3의 objectResources와 instances에 `anchorKind=BOSS`를 추가한다.
부모 resource의 `anchorBossArchetypeId`는 BossCatalog의 stable archetype ID,
`anchorBone`은 실제 BODY bone 이름이다. instance는 부모의 앵커 정보를 소비한다.
비보스 앵커는 두 필드를 비운다. 기존 WORLD/PLAYER와 Composition의 NONE/BOSS_SPAWN은 유지한다.

WorldSequencePlayer는 기존 target provider와 CModel 경로를 확장해 살아 있는 복제 보스의
본 행렬을 매 샘플에 조회한다. 본의 orientation/translation과 확인된 import 단위를 유지하고
소품의 local Transform을 합성한다. 보스 부재 시 숨기며 잘못된 본을 root로 대체하지 않는다.
패턴 종료·Reset·보스 소멸은 기존 occurrence 정리 경로로 처리한다.

World Object Tool의 생성·상세·저장, C++ document parser/validator, Map publisher와
Composition projector가 같은 필드를 소비한다. 새 C++ 파일과 project/filter 등록은 필요하지 않다.

## G02. 리소스와 시퀀스 저작

기존 `fm_g_reup_01.wmodel`과 `fm_g_rhkp_06.wmodel`은 실제 소품 geometry지만 재질 texture 경로가
비어 있다. 기존 Effect용 입력을 보존하고 원본 texture를 연결한 WorldObjects 전용 cooked 모델을
Resources에 설치한다. runtime binary는 Git에 추가하지 않는다.

`월드오브젝트_쿠크트럼펫`, `월드오브젝트_쿠크레이저대포` 부모와 기본 정지 상태를 등록한다.
앵커는 `BOSS_KAKULSAYDON_G2_KOUKU / b_wp_1`, Composition worlds는 `NONE`으로 저장한다.
기존 P21/P23 stage·animation·Logic과 다른 패턴을 보존하고 WORLD occurrence만 추가한다.
원본 socket/particle에서 확인한 local 회전·크기를 적용하며 사용자 화면 검토가 필요한 값은 결과에 명시한다.

원본 정본을 쓰기 직전 backup bytes와 비교하며 revision을 각각 한 번 올린다.
Area WorldSequences scope Publish/Check와 KoukuSaydon domain publish로 생성물을 갱신한다.

## G03. 검증과 사용자 실행

변경 JSON/PowerShell/XML parse, 기존 focused world/document/projector 검사, 최소 Debug 컴파일과
정규 EXE 링크·배포, `git diff --check`를 실행한다. 모델·texture 물리 존재와 실제 bone 단위 및
패턴 시작/종료 시간 참조를 확인한다. 신규 검증 프레임워크는 만들지 않는다.

Client/UI 실행·조작·캡처는 하지 않는다. Server + Client profile을 사용자가 Ctrl+F5로 실행하고,
Lobby → KoukuSaydon → F1 → KoukuSaydon Complete Play → Gate 2 →
쿠크_팡파레 / 쿠크_레이저를 재생한다. 손 정렬·크기·색의 최종 판단은 사용자 검토로 남긴다.
