# 쿠크 갈고리 마지막 상승 시작점 해제 계획

## G00. 실제 grip과 해제 소비자

기존 `Drag_HookedPlayer`는 전체 XYZ의 마지막 위치 도착을 해제로 간주한다. 게시된
P18/P19 각18개 track은 staggered bake 위상에 따라 local6542/6550/6558/6567ms,
Y 약2.240m에서 마지막 상승을 시작하고 마지막Y가22.94m다. P33의15개 후반
track은local1701ms/Y2.276m에서 같은 상승을 시작한다.
`Release_PlayerAttachment`는 현재 좌표를 그대로 유지하므로 공중에 남을 수 있다.

## G01. KoukuSaydonLogicRuntime.cpp

WORLD anchor의 baked grip에서 높이가 엄격히 증가하는 마지막 연속 상승 구간을
역방향으로 찾는다. 같은 높이의 수평 접근/대기 구간으로 되감지 않고 몸체 높이보다 큰 마지막
상승만 해제 경계로 사용한다. 마지막 위치 hold와 legacy/no-grip track의 기존 해제는
유지한다. 해당 시간이 되면 현재 공중 sample을 적용하기 전에 경계 key의XZ를
Server navigation에 검증하고 floor XYZ를 commit한 뒤 기존 attachment deadline을
현재 tick으로 당긴다. 표현용 훅 자체의 모션은 바꾸지 않는다.

바닥셀자체가없는grip XZ는SameLevel의기준높이가유효하지않으므로기존player-spawn
`Project_Point`로근처바닥을찾는다. 양쪽projection 모두동일navigation grid,실제walkable,
수평거리와높이차각각player 전체높이1.8m이내여야한다. 임의의Y를만들지않는다.

검증할 바닥이 없으면 상승 모션에 따라가지 않고 경계 위치에서 기존 attachment를
유지해 다음 tick에 다시 admission한다. 잘못된 바닥을 임의 좌표나0m로 대체하지 않는다.
Shared/Client snapshot 계약,1/3관문 일반 펜스와 부활 목적지는 바꾸지 않는다.

## G02. 기존 계약 검사와 완료 경계

`ServerGameplayContractTests_KoukuOverlap.cpp`의 기존 runner에 상승 직전/시작/이후,
실제 게시 P18/P19/P33 grip의 바닥 투영과 정상 해제, 일반 hook hold 보존을 추가한다.
수평 접근 후 상승하는 synthetic track과 이동·회전한 boss가 WORLD grip의 좌표에
재적용되지 않는 경계도 검증한다.
새 C++/schema가 없어 project/filter 등록이나 이번 기능만의 publish는 없다.
통합 담당자가 Debug/Release Product 및 focused Server 검사를 실행한다.
Client 실행과 실제 잡힘→끌림→내려놓기 화면 검증은 사용자가 수행한다.
