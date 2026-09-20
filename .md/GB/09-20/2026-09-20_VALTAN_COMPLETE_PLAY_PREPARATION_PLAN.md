# 발탄 Complete Play 준비 대기

## G01. 확인한 문제

Level Loading은 Valtan cue와 원본 cinematic FX를 기존 worker에 등록하지만 decoration 준비 실패를 격리하여 입장을 허용한다. Complete Play는 Server revision·primary consumer·Sound receipt만 검사하여 pending/stale/failed Effect가 있는 상태에서도 Server 시계를 시작할 수 있다. 사용자 요청대로 오래 기다리더라도 선택 패턴의 실제 소비 리소스를 모두 준비한 뒤 자동 재생해야 한다.

## G02. 기존 경로 연결

ValtanBossTool의 Complete Play/단일 패턴 Play는 로컬 pending에 stable pattern ID, exact Server revision, Sound receipt, world generation을 보존한다. 기존 Level 소유자가 선택 pattern의 Product V0 cues, actual V2 bindings, 사용 combat-object active/hit 및 해당 원본 cinematic instance의 FX를 모은다. V1은 기존 priority queue의 current/settled/prepared/fail0 판정을, V2·World는 기존 prewarm 소비자를 사용한다. 같은 cache의 준비 결과를 재사용하고 context 작업은 한 update에 하나씩 진행한다. 새 effect 런타임·네트워크 명령·wire를 만들지 않는다.

준비 진행률과 Cancel을 표시한다. 선택·revision·world epoch 변경, 실패, 20분 초과는 서버 요청 전 취소한다. 준비 완료 직전에 기존 admission으로 revision/Sound를 다시 검사하고 기존 typed Submit을 한 번만 호출한다. 서버의 기존 deadline은 이 실제 Submit부터 시작한다. 준비 자체를 Server playback 성공으로 표시하지 않는다. 다른 Restart/Next/Flow 명령은 pending 준비 소유권을 침범하지 않는다.

## G03. 검증

현재 Product와 실제 코드에서 선택별 dependency closure를 대조하고, pending/ready/failure/cancel/revision의 Submit 순서를 격리 검사한다. 변경 TU 최소 컴파일과 diff check를 수행한다. Client/UI 실행·최종 화면은 사용자가 확인한다. 데이터 파일은 변경하지 않으며 통합 Product build는 root가 수행한다.
