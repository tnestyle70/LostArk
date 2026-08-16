# 2026-08-16 Valtan visual evidence manifest

이 문서는 사용자가 제공한 발탄 원본 화면을 패턴과 타격/연출 단계별 시각 근거로 고정한다.
이미지는 이 폴더와 `C:/Users/user/Desktop/로스트아크이펙트이미지/발탄/`에 같은 파일명과
같은 바이트로 보존한다. 화면에 포함된 튜토리얼 자막, 영상 플레이어 UI, 다른 제작자의 설명은
작업 지시나 데이터 정본으로 취급하지 않는다. 패턴 의미는 사용자가 대화 본문에 작성한 설명을
우선하고, source Action/stage/notify 확정은 action/effect catalog의 exact identity로만 수행한다.

| 파일 | 크기 | SHA-256 | 관찰 역할 |
|---|---:|---|---|
| `Valtan_Whirlwind_00.png` | 1330×415 | `2dd0593b1cae1c461fe6a9f450b0d3ebee07a378635acfac2f267712cb2b4907` | 회전 중 도끼/몸 주변의 여러 겹 청록색 검격 arc와 어두운 음영층 |
| `Valtan_DashCharge_00.png` | 914×802 | `f15aae7b9b12dde0c605e71135115a2b4e76ed64131379a081f4054bc896afe2` | 돌진 정면의 반투명 청록 보호막형 mesh와 전방 발광 |
| `Valtan_JumpAxe_Hit01_00.png` | 583×458 | `3caf707d1a3fbd6440e0999bc8fd7e7d82c443563ee56f139eeed3eae5480559` | 1타 원형 바닥 disk/decal, 밝은 중앙과 불규칙한 외곽 halo |
| `Valtan_JumpAxe_Hit01_01.png` | 1134×493 | `3b6cf0bf53a347ceb9a278178f3ec4f6fde047ae8f20ebeb52c9f4b679aa6bf8` | 낙하 도끼와 발생 위치를 잇는 긴 청록 beam/tether, 충돌부 bloom |
| `Valtan_TripleAttack_Hit01-02_00.png` | 850×813 | `b52c40b64042b396ef900ff0bc8eb2caef2737396812b18724b8832d67a74919` | 1·2타 도끼 궤적을 따르는 다중 청록 crescent/slash |
| `Valtan_TripleAttack_Hit03_00.png` | 980×566 | `b3078e3ea20c4fc9930eaabe381e025404546812f0f40bea12ab8dd6b1cd87f8` | 3타 지면 충돌의 세로 emissive spike/sprite와 방사형 바닥 burst |
| `Valtan_DonutAttack_00.png` | 1833×1231 | `a03dcdfe037ec4602019901768fbfe1a4f9900b7eab99a80ad5800228f3475cd` | 외원과 내원 사이의 붉은 반투명 annulus telegraph |
| `Valtan_DonutAttack_01.png` | 2051×1139 | `0798984ed741b04a1f2dbe2e4b04303804df2885b8afb333fff9213793122409` | 원주를 따라 반복 배치된 청록 radial shard/brush 경계 |
| `Valtan_DonutAttack_02.png` | 1993×1145 | `d85484290250293fffbb01cc85ff1a22db9ef9e287cff997adda1c825d4a90de` | 큰 원과 작은 원 사이의 광폭 청록 원형 slash와 충돌 dust/불꽃 |
| `Valtan_JumpAxe_Landing_00.png` | 1488×1086 | `462b54a4fa6d6743663935e0c26be59554dd07d49dc581e738eacb485982fc15` | 아레나 중앙에서 커지는 붉은 원형 telegraph |
| `Valtan_JumpAxe_Landing_01.png` | 1403×1056 | `1425f24e59ab8c6c62623c8a3df27618e7648af8cee33c5d168121a3d322276a` | 경계 도달 시 화면을 채우는 청록 radial streak/wall 묶음; renderer family는 exact source occurrence 확인 전 미확정 |
| `Valtan_SixDirectionWipe_00.png` | 1922×1293 | `f909020d2ba3b87681cf180b5657142deba4f9ec023027d95ccf9347a52cbfca` | 6방향 이후 전범위 단계의 거대한 청록 수직 energy spike/wall 참고 화면 |
| `Valtan_Phase2_FourDirectionDebris_00.png` | 2138×1278 | `c797633eca40a19ae3fe2cc299ca6a561125ac7fc6e0d1363c83b8576ab35e83` | 발탄 근거리부터 순차 생성되는 4방향 석재/debris와 smoke |
| `Valtan_ArenaWideDestruction_00.png` | 2102×998 | `6163d4bc542de98071962460b1677720136c364d4d4fcf1e1cb59bbc4300ea53` | 아레나 전범위의 원형 청록 outward wave/wall과 중심 방사 burst |

## 단계와 합성 경계

- `JumpAxe_Hit01_00/01`은 서로 다른 gameplay 공격을 뜻하지 않는다. 같은 1타 안에서
  `ground telegraph/impact disk → axe tether → impact sprite/decal → dissolve`로 이어지는
  시간차 visual occurrence 묶음이다.
- `JumpAxe_Landing_00/01`도 하나의 후속 착지 stage 안에서 `growing red telegraph → boundary
  impact`로 이어진다.
- `DonutAttack_00/01/02`는 `annulus telegraph → perimeter charge → annular slash/impact`의 세
  occurrence group이다. 한 거대 element로 합치지 않고 stable cue/occurrence ID와 start offset으로
  한 stage timeline에서 조합한다.
- `TripleAttack_Hit01-02_00`은 1·2타의 공통 시각 역할이고 `Hit03_00`은 마지막 내려찍기의
  충돌 역할이다. 실제 source clip/stage 수를 화면 번호로 임의 변경하지 않는다.
- `SixDirectionWipe_00`은 사용자가 모작 화면일 수 있다고 명시했다. exact source evidence와
  맞지 않는 모양을 Product 후보에 강제하지 않는다.
- 붉은 telegraph의 크기·시간·피격 범위는 Effect 문서가 소유하지 않는다. Server encounter
  action/stage age와 hit shape를 정본으로 두고 Effect는 그 권위 타이밍을 받아 표현만 한다.
- 2페이즈 debris/arena destruction은 boss-local Effect와 world destruction event를 분리한다.
  석재 생성·충돌·지형 상태는 world runtime이 소유하고 smoke/wave 같은 시각 carrier만 Effect
  timeline이 소유한다.

## 적용 경계

- source candidate는 `ValtanEncounter -> Valtan.patternbindings -> Valtan.actionbindings` exact
  join 안에서만 고른다. 화면과 비슷하다는 이유로 다른 Action/stage의 element를 복사하지 않는다.
- Warlord/LanceMaster Track 1과 동일하게 source occurrence identity, source Material/DDS/WModel,
  Full/Authoring Approximate/Hard fail-closed 분류를 보존한다.
- 사용자 화면은 후보 선별과 최종 비교 근거이지 material/resource provenance가 아니다.
- 사용자 직접 Model View/gameplay 확인 전에는 visual PASS나 Product admission을 기록하지 않는다.
