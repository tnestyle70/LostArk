# G01. 마리오 광대 얼굴 폭탄 발사 적용 결과

## 후속 조정: 발사 간격·속도·얼굴 방향

사용자는 원본 교체 후 몬스터/오브젝트 배치와 로켓 발사를 확인했고, 로켓이 너무 자주/빠르게
나오며 일부 방향에서는 뒤로 날아가는 것처럼 보인다고 보고했다. 첨부 202415 이미지를
분석하고 위치 track에 회전이 없었던 코드를 확인했다.

- 발사 지점별 간격 2,000 → 4,000ms, 속도 6 → 3m/s.
- cooked ClownFaceBall 코의 앞축인 +X를 실제 비행 방향으로 정렬한다.
  world-up을 유지하며 앞/뒤 두 방향 모두 진행 방향으로 얼굴이 향한다.
- 비대칭 모델의 바닥 중심 보정 offset도 함께 회전한다. 모델과 발사 중심의 어긋남을 막는다.
- 발사 marker, 진행선, 머리/발 높이 선택, 크기, 끝점 소멸과 stage 종료 정리는 유지했다.
- 마리오 측면 카메라 기준 옆모습으로 통과하도록 한 것으로, 자유 카메라를 돌려도 항상
  옆모습이 되게 하는 billboard는 아니다. 피해 없는 Client 표현 경계도 유지했다.

7개 발사 방향의 변환된 +X와 비행 방향 내적=1, 회전 후 바닥 중심 오차 1e-6m 이하,
source/published marker 일치, JSON/XML parse, 120초 범위 49,413회 slot-clock 비교를 통과했다.
새 비행 시간은 marker 순서대로 10568/9870/13220/5829/5129/5606/13159ms이며,
최대 slot은 기존처럼 3/3/4/2/2/2/4개다. 구조·수치 검증이지 사용자 화면 판정은 아니다.

이 후속 작업의 수치가 아래 최초 적용 기록보다 우선한다. 추가 Resources/Server/protocol
변경은 없고 Client만 갱신한다. 최종 빌드 결과는 작업 종료 시 아래에 기록한다.

후속 빌드 상태: 코드 컴파일 뒤 링크에서 LNK1201(Client.pdb 쓰기 실패)이 발생했다.
디스크 여유 공간 약 235GB이며, 기존 PDB의 배타적 접근 검사에서 다른 process 사용 중임을
확인했다. Visual Studio 종료를 사용자에게 요청했다. 링크가 실패하면서 기존 Client.exe가
없는 상태이므로 재링크 완료 전에는 실행 준비 완료로 안내하지 않는다.
로그: `out/MarioOriginalReplacement/client-bomb-facing-build.log`.
수정 CPP의 UTF-8 BOM 없음, PLAN 전문 일치, 관련 git diff --check는 통과했다.
독립 read-only 검토에서도 +X 방향/yaw와 WorldSequence의 Scale * Rotation * Translation
합성, 중심 보정, 양방향 비행, slot 상한에 구체적 결함은 발견되지 않았다.

최종 후속 상태: PDB 배타적 접근 재검사에서 잠금 해제를 확인했다. `/t:Link` 단독 실행은
성공 코드만 반환하고 EXE를 생성하지 않아 완료로 취급하지 않았다. 정식 `/t:Build`로 재실행해
Client Debug 빌드 성공과 Client.exe 생성을 확인했다. binary에도 새 4000ms/3m/s/face follows flight
상태 문자열이 포함된다. `out/MarioOriginalReplacement/client-bomb-facing-final-build.log`에 기록했다.
이 최종 상태가 위의 일시적 링크 차단 상태보다 우선한다. 사용자 화면 재확인은 아직 남아 있다.

## 구현 상태

사용자가 저장한 7개 Mario2/3/4_Boom marker를 해당 Server iMarioStage 진입 중 자동 발사 위치로 연결했다. marker는 enabled=false/events=[] 그대로이며 바깥으로 걸어가 밟거나 G를 누를 필요가 없다. 기존 CWorldSequencePlayer/WorldSequenceObject/CModel/CMaterial 경로만 사용한다. 소규모 메모리 재생 문서를 published marker와 기존 clown_face_ball show 리소스에서 생성하며 원본 문서를 덮어쓰지 않는다.

- 2000ms 간격, 약 6m/s. stable marker hash별 위상으로 지점마다 발사 시각을 엇갈리게 했다.
- 발사당 낮음/높음을 Server 출생 tick 기반 hash로 선택한다. 기본 바닥 정렬 키에 +0.05m 또는 +0.90m. 1.5m 광대의 발/머리 부근을 위한 초기 튜닝이며 본을 추적하지 않는다.
- marker의 정확한 XZ에서 해당 진행선의 먼 끝점 XZ로 수평 직진한다. 원래 marker가 진행선 축에서 0.125~1.243m 벗어나 있으므로 시작 위치를 임의로 옮기지 않고 끝점 쪽으로 직진하게 했다. Y는 marker의 바닥 높이를 유지한다.
- 비행 종료 시 객체를 제거한다. stage 변경/퇴장/사망도 잔류 폭탄을 제거한다. 매 프레임 born-time에서 절대 샘플하며 누적 이동/무한 생성이 없다.
- 모델 크기는 resource scale 1 유지. box halfExtents를 모델 크기로 해석하지 않는다. 기존 줄무늬 공의 1.5배 크기/4m 바운스와 마리오 광대 1.5m, 일반 플레이어는 변경하지 않았다.
- 피해 없는 Client 비행 표현이다. 접촉 판정, 폭발, 피해, Server combat-object 생성은 구현 범위에 포함하지 않았다.

## 수치 검증

| marker | 거리 m | 비행 ms | 최대 slot |
|---|---:|---:|---:|
| Mario2_Boom | 31.702 | 5284 | 3 |
| Mario2_Boom_1 | 29.609 | 4935 | 3 |
| Mario2_Boom_2 | 39.658 | 6610 | 4 |
| Mario3_Boom | 17.485 | 2915 | 2 |
| Mario3_Boom_1 | 15.384 | 2565 | 2 |
| Mario3_Boom_2 | 16.816 | 2803 | 2 |
| Mario4_Boom | 39.474 | 6580 | 4 |

실행한 검증:

- 기존 WorldGameplay publisher Validate/Publish 성공(exit0). 사용자 Gameplay revision 8792의 112 placements를 내보냈다. publisher는 기존 계약에 따라 모든 등록 world 생성물을 검증·재출력했다. 생성물을 직접 편집하지 않았다.
- source/published marker 및 참조 진행선 좌표 일치 확인, JSON parse 성공. 각 emitter 256회 출생에서 두 높이 모두 선택됨. 120초를 17ms 간격으로 검사한 49,413회 slot 비교에서 brute-force 활성 출생 목록과 일치했다. 시작/중간/끝 직진 위치와 일정 Y, endpoint, slot 상한 검사 성공. 이는 수치 검사이며 화면 재생 증거가 아니다.
- 독립 read-only 검토에서 소유권/slot 재사용/정리/7개 MARIO_LANES 참조 확인. PLAN의 위상 설명을 실제 hash 방식에 맞게 고쳤고, 후속으로 폭탄 전용 tick 보간을 분리해 바운스 실패 여부와 독립시켰다.
- Client Debug x64 Build 성공(exit0), 최종 변경 후 증분 Build 재확인 성공(exit0). Client/Bin/Debug/Client.exe 수정 시각 2026-09-09 18:09:48, 크기 48,238,080 bytes. 로그 out/MarioBounce/client-bomb-build.log 및 client-bomb-final-build.log. 기존 코드페이지 경고는 유지된다.
- 기존 Client 프로젝트/filters XML parse 및 관련 git diff --check 성공. 새 C++ 파일/프로젝트 등록 없음. 기존 C++ UTF-8 BOM 없음 유지.
- unrelated dirty worktree 보존, stage/commit/push하지 않음. git fetch 후 HEAD...origin/main은 0/4이며 pull/merge하지 않았다.
- 로그 읽기의 잘못된 Unicode 지정으로 지연된 read-only shell만 취소했다. 성공한 빌드나 사용자 프로그램을 종료한 것이 아니다.

## 사용자 확인과 배포

Client/Server는 사용자 종료 후 실행하지 않았다. 현재 PC는 Team LAN client이며 endpoint는 192.168.0.14:7777이다. Server가 실행된 상태에서 Debug Client를 실행하고 Lobby KoukuSaydon -> F1 -> 2/3/4마리오로 이동한다. 진입 후 2초 이내 첫 발사가 나오고 이후 반복된다. F1을 닫고 발/머리 높이·직진·끝점 소멸을 확인한다. 다른 마리오로 이동하거나 퇴장하면 이전 폭탄은 사라져야 한다. Client/UI 조작·캡처·최종 육안 판정은 수행하지 않았다.

트리거 위치를 다시 바꾸면 World Gameplay Save -> Publish-WorldGameplay.ps1 -Mode Publish -> Client 재진입 순서다. marker 이름은 연결 ID이므로 유지하며 Enabled를 켜지 않는다. 속도/주기/두 높이는 현재 Level_KakulSaydonArena_WorldObjects.cpp의 MARIO_BOMB 상수에서 조절한다.

재사용 Resources 상대 asset ID: Map/LV_LUT_MIDNIGHTC_ED/MarioProps/ClownFaceBall/ClownFaceBall.wmodel. 물리 폴더 Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/MarioProps/ClownFaceBall/ 및 기존 textures가 필요하다. 기존 설치본을 그대로 사용했으므로 이번 작업의 추가 Drive payload는 없다. 다른 PC에는 기존 MarioProps 팩이 있어야 한다.
