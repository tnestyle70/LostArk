# 콜로세움 매치 로딩 화면(섬멸전 - 콜로세움) 복원 RESULT

## 한 줄 요약
원본 `colosseumLoadingS3` 무비를 파싱해 좌표·텍스처를 뽑고, 디버그 로비의 Colosseum 버튼으로 들어갈 때 이 화면이 뜨도록 `CLevel_Loading`에 연결했다. 실제 게임 화면 확인은 아직 하지 않았다(사용자 몫).

## 원본에서 찾은 것
- 무비: `EFUI_COLOSSEUMLOADINGS3`(구성 좌표 1920x1080). 공용 조각은 `EFUI_LOCALRESOURCE`(VS의 V/S 글자), `EFUI_COMPONENTS`/`componentsv2`(클래스 심볼, 티어 아이콘, 라벨).
- 기존 GFX 파서가 PlaceObject3을 잘못 읽는 버그(f2&0x10만 켜지고 클래스명 없음)를 확인하고 `Tools/LpkPipeline/gfx_scene_extract.py`로 새로 작성했다.
- 참조 캡처(41.jpg)와 무비 좌표의 대응: `캡처 = 0.8966 x 스테이지 + (102, 29)`(균일 스케일). 무비 좌표를 정본으로 삼았고, 이 식으로 텍스트·슬롯 앵커를 맞췄다.

## 만든 것
- `Tools/LpkPipeline/build_colosseum_match_loading_ui.py`: TGA -> PNG(`Client/Bin/Resources/UI/Colosseum/MatchLoading/`, 28장, 약 2.0 MB), 레이아웃 JSON, 비교용 미리보기 생성.
- `Data/UI/Colosseum/MatchLoading_Layout.json`: 슬롯 41개(stable id `MatchLoading_*`), 1280x720 기준.
- `Client/Private/Level_Loading.cpp`, `Client/Public/Level_Loading.h`: COLOSSEUM일 때만 공용 크롬 대신 이 화면을 쓴다. 제목·팁·팀 패널 텍스트, 슬롯 6개 텍스트, 실제 로딩 진행률에 연동된 하단 진행 바(진행 머리 포함), 좌측 주목 캐릭터 = 입장 플레이어의 직업 일러스트(`UI/ClassSelect/<직업>/Illustration.png`).

## 진짜 값과 샘플 값
- 진짜: 제목, 팁 문구(캡처에 보이는 문장), 입장 플레이어의 직업·닉네임(좌측 팀 3번 슬롯), 진행률.
- 샘플(매치메이킹이 없어 채울 수 없음): 나머지 5명의 직업·단·닉네임·서버·KDA, 우측 주목 캐릭터의 직업 일러스트. 표는 `Level_Loading.cpp`의 `MATCH_ROSTER` 한 곳에 있다.

## 원본과 다른 점 / 못한 것
- 3D 캐릭터 렌더는 못 넣었다. 로딩 중에는 캐릭터 모델이 아직 안 올라와서, 대신 클래스 선택 화면의 2D 일러스트를 썼다.
- VS 엠블럼 주변 파티클·빛줄기는 애니메이션 없이 정지 이미지로 근사 배치했다. 새싹 마스코트, 관전자 수 표시, 클래스 심볼/티어 아이콘/랭크 라벨 컴포넌트는 넣지 않았다.
- 슬롯 안 텍스트 위치는 캡처 대조로 잡았고, 원본은 ActionScript가 정하므로 몇 px 어긋날 수 있다.

## 검증(실행한 것만)
- `cl /Zs` Level_Loading.cpp, MainApp.cpp: rc=0(인코딩 경고만).
- 레이아웃 JSON parse, slot id 41개 중복 없음, 참조 PNG 27개 전부 존재.
- CY_Resources에 `UI/Colosseum/MatchLoading` 28개 복사, sha1 일치.
- 미리보기 합성과 캡처 나란히 비교로 배치 확인(정지 이미지 기준).
- 실행/화면 확인 안 함: VS에서 Debug 빌드 후 로비 Colosseum 버튼 -> 로딩 화면 확인이 필요하다.
