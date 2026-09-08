# 09-08 pull 이후 카드미로 리소스 전달

기준: 2026-09-08 10:28:29 KST, Git `f92178f0` 이후 코드/Data 변경과 현재 물리 파일을 비교했다.
Resources는 Git 비추적이므로 과거 바이너리와의 정확한 내용 비교는 불가능하다.
현재 Resources 전체의 수정 시각 조사에서는 기준 이후 파일이 아래 카드 병사 28개뿐이었다.
Drive에 이미 올렸는지, 팀원이 이미 받았는지는 확인하지 않았다. 이번 작업은 목록 안내이며 업로드하지 않았다.

## 먼저 Drive로 보낼 신규 리소스

물리 루트: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/`

| Resources 상대 폴더 | 파일 | 크기 |
|---|---:|---:|
| `Character/KoukuSaton/CardMiro_Monster_Heart/` | 7 | 9,828,636 bytes |
| `Character/KoukuSaton/CardMiro_Monster_Diamond/` | 7 | 9,828,636 bytes |
| `Character/KoukuSaton/CardMiro_Monster_Clover/` | 7 | 9,828,636 bytes |
| `Character/KoukuSaton/CardMiro_Monster_Spade/` | 7 | 9,541,108 bytes |

합계 28파일, 39,027,016 bytes(약 37.2 MiB). 각 폴더 전체를 보낸다.
각각 같은 폴더명 `.wmodel` 1개와 `textures/` 안의 DDS 6개가 있다. 하위 경로를 유지한다.
`Data/Actors/MonsterCatalog.json`에 오늘 추가한 네 `MONSTER_KOUKU_CARD_*`가 직접 참조한다.
CLUB 문양의 실제 물리 폴더명은 `Clover`이며 `Club`으로 바꾸면 안 된다.

## 이번 기능의 기존 리소스 의존성

- 세토: `Character/KoukuSaton/MN_PPCT_00/` 전체 12파일, 38,450,700 bytes(약 36.7 MiB).
  `MN_PPCT_00.wmodel`과 텍스처이며 파일 수정일은 09-07, 오늘 추가한
  `cardmiro.march.seto` object resource가 처음 참조한다. 팀에 아직 전달하지 않았다면 반드시 같이 보낸다.
  카드 병사와 세토 합계는 40파일, 77,477,716 bytes(약 73.9 MiB).
- 플레이어·몬스터·출구 바닥 문양: 아래 기존 DDS 4개(각 262,272 bytes).
  새로 추출/변경하지 않았고 기존 dance 효과와 공유한다. 팀원이 없을 때만 추가 전달한다.

```text
Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47.dds
Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_1.dds
Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_2.dds
Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_3.dds
```

각각 heart/spade/club/diamond. 네 DDS까지 모두 보낼 경우 44파일, 78,526,804 bytes(약 74.9 MiB).

## Git으로 전달하는 항목

모델 카탈로그/프로파일, 문양 effect JSON 4개와 group JSON 8개, 카메라,
36개 행진/트리거, CardMiro navsource/navpaint/navregions, Client/Shared/Server 코드,
publisher와 검증 코드, PLAN/RESULT. 네비게이션 저작 데이터는 Resources가 아니므로 Git에 포함한다.
원본 PSK/PSA/glTF나 전체 Resources 팩은 이 기능 전달에 새로 요구하지 않는다.

## 팀원 적용

1. PR이 main에 병합된 후 pull한다. 위 리소스를 자기 `Client/Bin/Resources`에 같은 상대 경로로 받는다.
2. 변경 도메인의 공식 publisher를 사용해 런타임 데이터를 생성한다. 생성 bootstrap을 직접 편집하지 않는다.
3. Engine → Shared → Server → Client를 같은 소스로 빌드한다. protocol 70이므로 공유 Server도 재시작해야 한다.
4. 이 PC는 팀 LAN client 설정이다. 공유 `192.168.0.4:7777` Server가 실행 중이어야 한다.
   개인 `.vcxproj.user`, localhost 우회 설정은 PR에 포함하지 않는다.
5. Debug 한 명일 때만 망원경 담당이 본인 문양도 받는 시험 경로가 있다. 실제 다인에서는 담당자 제외 최대 세 명에게 배정한다.

아직 사용자 확인이 필요한 것: 실제 카메라/이펙트 구도와 4인 동시 진행.
일반 플레이어의 망치 애니메이션과 중앙 망원경 상자의 시각 모델은 이번 PR에서 추가하지 않았다.
최종 복귀는 기존 2관문 좌표를 쓰며 `cardmaze.return`에서 추후 조정한다.
