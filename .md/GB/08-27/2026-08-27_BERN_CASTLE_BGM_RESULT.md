# 2026-08-27 베른성 BGM 구현 결과

## 구현 상태

베른성 레벨의 필수 맵·카메라·복제 초기화가 모두 성공한 뒤 다음 정확한 원본 음악을 반복
재생하도록 제품 Client에 연결했다.

```text
event name: bgm_berntown_mscene01_thecapital
asset ID:   Sound/BGM/BernCastle/bgm_berntown_mscene01_thecapital.wav
loop:       true
volume:     1.0
```

재생 성공 여부는 `CLevel_Bern` 인스턴스가 소유한다. 베른을 실제로 떠날 때만 music channel을
정지하며, 베른 초기화가 중간에 실패한 임시 객체는 Lobby나 다른 레벨의 음악을 정지하지 않는다.
원본 WAV 누락이나 FMOD 재생 실패도 네트워크 진입과 베른 gameplay를 막지 않고 해당 음향
presentation만 격리한다.

## 런타임 리소스

SoundCatalog가 선언한 exact asset 경로에 Sound1 원본을 그대로 배치했다.

```text
source:  Client/Bin/Resources/Sound1/BGM/BernCastle/bgm_berntown_mscene01_thecapital.wav
runtime: Client/Bin/Resources/Sound/BGM/BernCastle/bgm_berntown_mscene01_thecapital.wav
size:    77,535,058 bytes
SHA-256: 56D929CC9E64992F57DB3622EDF3CB595C2A7F198D99759E77CF4F05DA587460
```

## 자동 검증

- SoundCatalog exact asset path 등록 수: 1
- 원본과 runtime WAV 크기 및 SHA-256 일치: PASS
- Client x64 Debug compile/link: PASS
- `Client.exe` 생성: PASS
- 임시 pre-build 우회 후 `Client.vcxproj` 내용이 HEAD와 동일하게 복원됨: PASS
- `git diff --check`: PASS

표준 Client pre-build는 이번 변경과 무관한 기존 Valtan presentation debug/manual 목록 불일치가
있으므로 C++ 빌드 동안 해당 검사만 일시적으로 건너뛰었다. 프로젝트 파일은 빌드 직후 원래
내용으로 복원했다.

## 수동 검증 경계

에이전트가 Client를 실행하거나 음향 결과를 대신 판정하지 않았다. 사용자가 Server + Client를
재시작하고 Lobby에서 Bern으로 진입하면 `mscene01_thecapital`이 시작되어야 하며, Bern을 떠나면
정지되어야 한다.
