# 쿠크 관문 BGM 이동·시퀀스 재개 구현 계획

## G00. 현재 실측과 경계

`jump.2`는 플레이어를 Y=6.11에서 Y=3.56으로 이동한다. 기존 BGM 선택기는 `COMBAT`이 아니면 시작 구역(Y>=5.64) 또는 G3 대기 발판 안에서만 시작 음악을 반환하므로 두 번째 G 이동에서 음악이 중지된다. `Sound_Manager::Play_Music`의 교체는 이미 transactional이며 Sound cue와 Music 채널은 독립적이다.

## G01. Level BGM 선택과 억제

`Client/Private/Level_KakulSaydonArena.cpp`의 `Resolve_KoukuRaidBgmAsset`에서 준비·이동 구간에 시작 곡을 유지한다. Server COMBAT/WAIT_MINIGAME/WAIT_GATE의 현재 관문을 사용하며 G1/G2/BINGO는 설치된 `Sound/KoukuSaton/S_BGM_COMMANDERRAID/midnightc_ed__398225682.wav`, G3는 `midnightc_ed__1053752270.wav`를 사용한다. G3 진입 전 발판은 시작 음악이 우선한다. Mario/Maze는 자신의 replicated mechanic을 유지한다.

`Update_RaidBgm`는 기존 local sequence, admission pending, raid CINEMATIC, 본인 sequence camera 억제에 실제 소유한 Composition cinematic camera를 추가한다. 같은 곡/상태에서 재시작하지 않고 시퀀스가 끝나면 기존 edge 처리로 재개한다. COMPLETE/ABORTED, local player 부재는 중지한다. raid phase가 INACTIVE인 직접 관문 입장은 이미 승인된 gate presentation의 stable placement ID를 사용한다. H public 계약·Engine·Shared·데이터 형식은 변경하지 않는다.

## G02. 검증과 반영

기존 C++ 파일을 그대로 유지하므로 project/filter 등록은 없다. 현재 함수 본문을 추출한 native probe로 jump.1/2/3, 관문/기믹 전환, G3 대기, 시퀀스/Composition camera 억제와 재개, 반복 snapshot/실패 재시도 억제를 확인한다. 설치 WAV의 형식·길이를 읽는다. Client TU는 통합 담당이 Debug/Release 컴파일한다. Client를 실행하거나 실제 소리를 재생하지 않는다. live authoring 변경은 필요 없으며 검증 산출물은 `out/KoukuCollider20260923/bugfix-bgm`에 둔다.
