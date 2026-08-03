# LostArk `.lpk` 데이터 아카이브 디코더 결과

작성자: JS · 2026-07-30

일반 전투 스킬의 이펙트/사운드/타이밍 결선은 UPK가 아니라 `EFGame/*.lpk`
독자 암호화 아카이브에 있다는 [스킬 결선 조사](2026-07-30_LOSTARK_LANCEMASTER_SKILL_WIRING_RESULT.md)
결론에 따라, `.lpk` 컨테이너 포맷을 리버싱했다.

## 1. 먼저 내릴 결론

1. `.lpk`의 **외피(컨테이너) 층은 완전히 해독했다.** 파일 프레이밍은 전역 8바이트
   반복 XOR이고, 그 아래 count·헤더·TOC·페이로드 구조를 확정했다(§3). 6개
   아카이브(`data1~4`, `leveldata1`, `font`)에서 동일 구조로 검증했다.
2. **내용(TOC 레코드 본문 + 페이로드)은 XOR 해독 후에도 바이트 분포가 완전 균일
   (≈1/256)한 강암호(AES급)다.** 압축도 아니고(압축 매직·해독 실패), 약한 XOR도
   아니다. 모든 `.lpk`에서 동일하게 확인.
3. 이 2차 암호 키는 LostArk **클라이언트 바이너리에 내장**돼 있고 안티치트로
   보호된다. 키 없이는 스킬 데이터 테이블을 못 읽는다. **여기가 확정적 벽이다.**
4. 참고: `umodel_lostark_v7`은 `.upk`만 복호화하고 `.lpk`는 건드리지 않는다.
   즉 커뮤니티 도구도 `.lpk` 2차 암호는 풀지 않았다. UPK 경로로 얻을 수 있는
   것과 `.lpk`에 갇힌 것은 다른 lane이다.

## 2. 실측 방법

표준 파이썬이 없어 **Blender 5.0 번들 파이썬**으로 분석했다.
가장 작은 `config.lpk`(1,432B, count=2)로 포맷을 잡고 `data1.lpk`(count=35,027)로
TOC 구조를 확정, 나머지로 교차 검증했다.

XOR 키 복원 원리:

- 첫 dword(count)를 제외한 본문에서, 사용 안 하는 대량 0-패딩 영역이 8바이트
  주기로 동일 바이트열(`49 da 82 8b b4 d4 f0 2c` …)로 나타났다.
- 동일 평문(0)이 주기 8로 반복 → **8바이트 반복키 XOR**. 반복 영역이 곧 키다.
- 키 = `B4 D4 F0 2C 49 DA 82 8B`, phase = `파일오프셋 mod 8`.
- 검증: 이 키로 XOR하니 대량 영역이 정확히 0으로 떨어지고, `data1`·`config` 모두
  통했다(전역 키).

## 3. 확정한 `.lpk` 포맷

```text
0x000  u32   count                         (평문, XOR 대상 아님)
0x004  40B   header                        (XOR 해독됨, 이후 강암호 / 용도 미상)
0x204  TOC   count 개 레코드
              레코드 슬롯 stride = 0x210 (528 bytes)
              슬롯 내 실제 사용 64~104 bytes, 나머지 0 패딩
              레코드 본문 = 강암호 (오프셋/크기/이름이 평문으로 안 보임)
DATA0  이후  payload = 강암호 컨텐츠
              DATA0 = 0x204 + count * 0x210
```

전역 XOR 해독 후: 헤더·패딩은 평문 구조(0)로 떨어지지만, **레코드 본문과
페이로드는 균일 난수**다. 즉 XOR은 프레이밍 난독화일 뿐이고 실제 데이터는 2차
암호로 보호된다.

레코드가 강암호라는 근거: 서로 다른 레코드가 record+0x18 위치에서 동일한 16~24B
블록을 공유(공통 상수/해시가 암호화돼 반복). 오프셋·크기 필드를 아무 위치에서도
평문 단조증가로 찾지 못함. 페이로드 바이트 분포 spread ≈ 0.001~0.005(완전 균일).

아카이브별 실측:

| 파일 | count | payload 크기 | 성격(추정) |
|---|---:|---:|---|
| config.lpk | 2 | (특수·소형) | 설정 |
| data1.lpk | 35,027 | 25.1 MB | 소형 레코드 다수 = 데이터 테이블/인덱스 |
| data2.lpk | 792 | 851.6 MB | 대형 블롭 소수 = 벌크 컨텐츠 |
| data3.lpk | 12,262 | 47.1 MB | 데이터 테이블 |
| data4.lpk | 59,793 | 15.4 MB | 초소형 레코드 = 매니페스트/인덱스 추정 |
| leveldata1.lpk | 4,693 | 16.8 MB | 레벨 데이터 |
| font.lpk | 27 | 31.3 MB | 폰트 |

`data4`의 59,793 엔트리는 게임 파일 수(~33,836)를 웃돌아, 물리↔논리 이름 매핑
매니페스트일 가능성이 있다(이전에 필요했던 그 맵). 다만 이것도 2차 암호라
지금은 못 읽는다.

## 4. 벽과 현실적 선택지

스킬 데이터 테이블은 이 페이로드 안에 있고, 2차 AES급 키 없이는 못 연다.
키를 얻는 길과 그 성격:

1. **클라이언트 바이너리 정적 RE로 `.lpk` 로더의 키 추출.**
   게임 실행 파일/DLL에서 `.lpk`를 여는 코드를 디스어셈블해 AES 키·모드를 찾는
   별도의 큰 작업. 실행 중인 게임을 건드리지 않는 정적 분석이라 안티치트와는
   무관하지만, 난도가 높고 시간이 든다.
2. **실행 중 게임 메모리에서 복호화본 덤프.** LostArk는 커널 안티치트로 보호되며
   보호된 게임 프로세스에 붙는 행위는 하지 않는다. **이 경로는 배제한다.**
3. **우회: `.upk` 경로로 얻을 수 있는 것에 집중.** 궁극기 연출은 이미
   `STANDARD_SKILLCAM_LANCEMASTER`(.upk)에서 구조를 확보했다. 일반 스킬 타이밍은
   `.lpk`에 갇혀 있으므로, 원본 복원 대신 우리 엔진에서 저작하는 편이 현실적이다.

**권고:** `.lpk` 2차 암호는 정적 바이너리 RE(1번) 없이는 못 푼다. 이건 별도
프로젝트로 떼는 게 맞다. 지금 진행 중인 창술사 작업은 `.upk`로 확보한 애니·연출
골격(스킬 결선 문서)으로 이어가는 것이 빠르다.

## 4-B. 팀원 파이프라인 문서 대조 + base16 위치 확정 (갱신)

팀원(tnestyle70) 저장소 문서 `에셋 추출 파이프라인.md`를 대조해 **복호화 레시피
전체와 남은 미지수 하나를 확정**했다.

레시피(현재 KR 빌드):

```text
per-table key 파생:
  digest  = MD5(table_stem.encode("utf-16le"))          # stem 예: "SkillEffect"
  mixed   = bytes(base16[i] ^ digest[15 - i] for i in 16)
  aes_key = SHA256(mixed.hex().encode("ascii"))          # 32B = AES-256 키
payload:
  AES-256 CBC, IV는 1024바이트 chunk마다 zero로 리셋
검증:
  복호화 결과가 "SQLite format 3\0" 매직 + PRAGMA quick_check == ok
```

즉 각 데이터 테이블은 **SQLite DB**이고, 복호화하면 스킬/이펙트/애니 노티파이가
그대로 읽힌다(문서가 "Action notify attachment transforms", "trail timing
RelativeTime ~1/60s"까지 이 경로로 복원했다고 명시).

**남은 미지수는 `base16`(16바이트 로더 키) 하나뿐이다.** 컨테이너 XOR(§3)과
per-table 파생·CBC는 다 안다. base16만 있으면 디렉터리와 페이로드 모두 풀린다.

base16 위치도 확정했다:

- 내 `EFEngine.dll`의 SHA-256이 문서가 지정한 빌드
  `CC09E083C21BDE705E4B8BC7A07D7014C618EF3F3A3ECF533E5B1FA326AA10E7`
  (v1.0.8767.0)과 **정확히 일치**. 레시피·RVA가 이 파일에 그대로 유효.
- 로더 함수: `FPackingPackageFileCache::Decrypt` @ RVA `0x2192E0`,
  `FFileManagerWindows::InitializeAltAESKeyMap` @ `0x2631B0`(키 등록),
  `FArchiveFileReaderWindowsEncrypted::DecryptBuffer` @ `0x219310`.

### 그런데 base16을 정적으로 못 뽑는다 — 새 벽

`EFEngine.dll`은 **WinLicense(Oreans)로 보호**돼 있다. 확증:

- 섹션에 `.winlice`, `.vm_sec`, `.boot`(약 6MB) 존재.
- `.text` 전 구간 엔트로피 7.88~7.91 bits/byte = 암호화/가상화.
- 위 RVA를 capstone으로 디스어셈블하면 전부 쓰레기 명령
  (`call 0x6f91...`, `jmp 0xffffff...`). 디스크의 코드는 암호화돼 있고,
  진짜 명령은 **런타임 언팩 후에만** 존재한다.

따라서 문서의 RVA는 런타임 언팩 이미지 기준이며, 디스크 파일 정적 분석으로는
base16을 못 읽는다. base16을 얻으려면 보호된 게임 모듈을 **런타임에 언팩·덤프**해야
하는데, LostArk는 XIGNCODE 커널 안티치트로 보호되므로 **그 경로(실행 중 보호
프로세스 접근)는 배제한다.**

### 현실적 결론

- "로더 키(base16)만 뽑으면 데이터 다 읽히나?" → **그렇다.** 그게 유일한 미지수다.
- 그러나 base16은 WinLicense 보호 안에 있어 정적 추출 불가, 런타임 언팩은
  안티치트 영역이라 하지 않는다.
- **팀원(tnestyle70)이 이미 이 RE를 끝냈다.** 문서가 정확한 레시피와 추출 산출물
  (SQLite, notify transform, trail timing)을 설명한다. 즉 그쪽이 base16 값 또는
  복호화된 `.db`를 이미 가지고 있다. **재추출하지 말고 팀원에게 base16 16바이트
  (또는 복호화된 EFTable `.db`)를 받는 것이 정답이다.** 받으면 위 레시피 +
  이 문서 §3 컨테이너 코드로 즉시 전량 복호화 가능하다.

## 5. 산출물

```text
scratchpad/lpk_decoder.py     .lpk 외피 XOR 디코더 + 구조 요약/검증
                              (python lpk_decoder.py <file.lpk> [dumpxor out.bin])
```

전역 XOR 키: `B4 D4 F0 2C 49 DA 82 8B` (phase = offset mod 8, count 4바이트 제외).
원본 데이터는 Smilegate 저작물이다. 산출물·해독 이미지는 Git에 올리지 않는다.
