# 베른 계단 통행 수정 결과

## G00. 변경

50×347/0.5m navigation의row47,x23..29는지상대신아치상단높이를선택했다. 실제installed STAIR02D·preScale0.01·배치5671 transform으로7셀의y=50.576641m를확인해navpaint를수정했다. 기존3ridge·북쪽진입확장·1m step정책·벽/절벽은유지한다. 과거기능확인용collision.bern.editor-proof만disabled로바꿨다.

## G01. 실행한 확인

Navigation/World publisher PASS. installed runtime의walkable7,671→7,674,spawnreachable5,767→6,138,이전막혔던남쪽NPC3명의접근이가능해졌다. source측성분계산과runtime확장은분모가다르므로runtime수치를최종사용한다. 근거out/KoukuMario20260914/{bern_runtime_compare.log,navigation-publish.log,world-publish.log,measure_bern_stair.py}.

Server실제남쪽경로·ridge높이·기존통행회귀검사를추가했다. 최종실행결과는통합캐릭터/아레나RESULT G06에기록한다. 전베른모든위치의사용자보행화면확인을완료했다고판정하지않는다.
