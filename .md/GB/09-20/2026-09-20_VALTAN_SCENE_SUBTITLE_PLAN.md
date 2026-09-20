# 발탄 원본 연출 자막 연결

## G1. 원본과 기존 소유자

설치 GameMsg DB와 기존 `ValtanSequences` 원본 파싱본에서 SCENE06A 입장 2행·버러지들 2행, SCENE04A 사망 2행, SCENE07A 흰/검 늑대 말풍선 2행이 확인됐다. 원본 Matinee 시간과 배우를 이미 소유한 WorldSequence에 자막을 붙인다. 발탄 stage와 World에 같은 타이밍을 중복 저장하지 않는다.

## G2. 구현 계약

기존 `WORLD_SEQUENCE_TEMPLATE`에 optional `subtitleTracks`를 추가한다. 각 행은 `subtitleTrackId`, `stringId`, `text`, `position`, `slotId`, `startMs`, `durationMs`를 가진다. `NORMAL`/`UPPER`는 screen-space 위치이며 빈 slot을 사용하고, `BALLOON`은 실제 ObjectResource actor slot을 요구한다. C++ document와 Map publisher가 같은 필드·UTF-8·시간 경계를 검증하고 Serialize와 저작 저장에 보존한다.

기존 WorldSequencePlayer가 성공적으로 적용한 source clock을 읽어 자막 sample을 만든다. Stop/Seek/Pause와 template 교체는 기존 player 소유권을 따른다. 말풍선은 해당 actor의 실제 visible model/world bounds 상단을 사용하고, 숨김·모델 누락·비유한 좌표이면 해당 말풍선만 표시하지 않는다. MainApp은 기존 제품 font renderer로 투영한다. 기존 MapTool 연출 World 배우 화면에 자막 행 목록·문구/시간/위치 편집과 사운드 행 시간/volume 편집을 연결하며 동일 document 검증·Save 경로를 사용한다. 신규 C++ 파일이나 프로젝트 항목은 추가하지 않는다.

## G3. 검증·적용 경계

source 8행과 실제 instance/slot/time 결합, codec 저장·재로드·invalid input rollback, player의 시간/종료/숨김 경계, C++ TU와 publisher를 검사한다. Data는 baseline/candidate로 준비하고 통합 담당이 최신 저장본에 stable leaf CAS로 반영한다. Client 실행과 화면 판정은 하지 않는다. 기존 자동 연출은 입장·버러지들·사망이며 두 늑대는 현재 MapTool source preview다. 자동 1관문 입장 연출 생성과 camera/animation 변경은 별도 범위다.
