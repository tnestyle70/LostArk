# NPC 카탈로그 이름·용도 목록

## 이 문서의 용도

`Data/Actors/NpcCatalog.json`에 `runtimeStatus=supported`로 등록된 75개 NPC를 Map Tool에서
고를 때 사용할 식별표다.

- 기존 `NPC_숫자` 64개는 해당 숫자를 Lost Ark Codex의 NPC ID와 대조해 원작 영문 이름과 역할을 확인했다.
- target catalog에 추가된 숫자형 7개는 현재 보유한 근거만으로 원작 이름·용도를 확정하지 않고
  아래 별도 표에서 `미분류`로 표시한다.
- 역할은 알아보기 쉽게 한국어로 옮겼다. 링크를 누르면 원문 이름, 역할, 등장 지역을 다시 확인할 수 있다.
- 이 목록의 `상인`, `교환원`, `요리사`는 **원작에서 어떤 NPC 외형인지 설명하는 분류**다. 현재 프로젝트에서 이 NPC를 배치한다고 상점·대화·교환 기능이 자동으로 구현되는 것은 아니다.
- `NPC_BEDA`, `NPC_AYLARA`, `NPC_FORMAN`, `NPC_SCHMIDT`는 숫자형 원작 DB ID가 없는 이름형 프로젝트 archetype이므로, 원작 직업을 임의로 추정하지 않았다.

## 이름형 프로젝트 NPC 4개

| Archetype ID | 이름 | 이 NPC는 어떤 NPC인가 | 현재 프로젝트 상태 |
|---|---|---|---|
| `NPC_BEDA` | Beda | 프로젝트에 이름으로 등록된 일반 인간형 NPC. 원작 직업은 현재 자료만으로 확정할 수 없다. | Bern에 `npc.bern.beda.guide`로 배치되어 있으나 초기 behavior는 `null` |
| `NPC_AYLARA` | Aylara | 프로젝트에 이름으로 등록된 여성 인간형 NPC. 원작 직업은 현재 자료만으로 확정할 수 없다. | Bern에 `npc.bern.aylara`로 배치되어 있으나 초기 behavior는 `null` |
| `NPC_FORMAN` | Forman | 프로젝트에 이름으로 등록된 남성 인간형 NPC. 원작 직업은 현재 자료만으로 확정할 수 없다. | 카탈로그 선택 가능, 현재 Bern 배치 없음 |
| `NPC_SCHMIDT` | Schmidt | 프로젝트에 이름으로 등록된 남성 인간형 NPC. 원작 직업은 현재 자료만으로 확정할 수 없다. | 카탈로그 선택 가능, 현재 Bern 배치 없음 |

## 숫자형 원작 NPC 64개

| Archetype ID | 원작 이름 | 이 NPC는 어떤 NPC인가 | 원작 등장 지역 |
|---|---|---|---|
| `NPC_11592` | [Hely](https://lostarkcodex.com/us/npc/11592/) | 요리사 NPC | Rethramis - Prideholme |
| `NPC_11748` | [Wilhelm](https://lostarkcodex.com/us/npc/11748/) | 장비 상인 NPC | Yudia - Saland Hill |
| `NPC_11749` | [Vilma](https://lostarkcodex.com/us/npc/11749/) | 물약 상인 NPC | Yudia - Saland Hill |
| `NPC_11752` | [Armin](https://lostarkcodex.com/us/npc/11752/) | 장비 상인 NPC | Yudia - Saland Hill |
| `NPC_11816` | [Edur](https://lostarkcodex.com/us/npc/11816/) | 장비 상인 NPC | Yudia - Ozhorn Hill |
| `NPC_11831` | [Hella](https://lostarkcodex.com/us/npc/11831/) | 요리사 NPC | Yudia - Saland Hill |
| `NPC_12025` | [Helen](https://lostarkcodex.com/us/npc/12025/) | 요리사 NPC | West Luterra - Lakebar |
| `NPC_12249` | [Nickel](https://lostarkcodex.com/us/npc/12249/) | 일반 상인 NPC | West Luterra - Lakebar |
| `NPC_12250` | [Ethan](https://lostarkcodex.com/us/npc/12250/) | 물약 상인 NPC | West Luterra - Lakebar |
| `NPC_12312` | [Clika](https://lostarkcodex.com/us/npc/12312/) | 물약 상인 NPC | Rethramis - Loghill |
| `NPC_12905` | [Liane](https://lostarkcodex.com/us/npc/12905/) | 물약 상인 NPC | West Luterra - Medrick Monastery |
| `NPC_12911` | [Jedin](https://lostarkcodex.com/us/npc/12911/) | 물약 상인 NPC | East Luterra - Dyorika Plain |
| `NPC_12913` | [Gent](https://lostarkcodex.com/us/npc/12913/) | 물약 상인 NPC | East Luterra - Leyar Terrace |
| `NPC_12917` | [Caros](https://lostarkcodex.com/us/npc/12917/) | 물약 상인 NPC | East Luterra - Croconys Seashore |
| `NPC_13203` | [Shine](https://lostarkcodex.com/us/npc/13203/) | 연마 재료 교환 NPC | East Luterra - Luterra Castle |
| `NPC_16773` | [Helena](https://lostarkcodex.com/us/npc/16773/) | 요리사 NPC | Shushire - Frozen Sea |
| `NPC_18394` | [Sophie](https://lostarkcodex.com/us/npc/18394/) | 물약 상인 NPC | Arthetine - Arid Path |
| `NPC_19244` | [Luigi](https://lostarkcodex.com/us/npc/19244/) | 물약 상인 NPC | Open Seas - Freedom Isle |
| `NPC_19249` | [Madof](https://lostarkcodex.com/us/npc/19249/) | 일반 상인 NPC | Open Seas - Freedom Isle / Opportunity Isle |
| `NPC_19364` | [Volta](https://lostarkcodex.com/us/npc/19364/) | 바텐더 NPC | Open Seas - Peyto |
| `NPC_19368` | [Broccoli](https://lostarkcodex.com/us/npc/19368/) | 요리사 NPC | Open Seas - Peyto |
| `NPC_19377` | [Collector Terion](https://lostarkcodex.com/us/npc/19377/) | 균열 조각 교환 NPC | Arthetine - Origins of Stern |
| `NPC_19545` | [Illicit Marketeer](https://lostarkcodex.com/us/npc/19545/) | 해적 암시장 상인 NPC | Open Seas - Atlas |
| `NPC_19546` | [Illicit Marketeer](https://lostarkcodex.com/us/npc/19546/) | 해적 암시장 상인 NPC. 이름은 같지만 별도 원작 ID와 별도 모델이다. | Open Seas - Atlas |
| `NPC_19571` | [Aldridge](https://lostarkcodex.com/us/npc/19571/) | 결정화된 비늘 교환 NPC | Open Seas - Crescent Isle |
| `NPC_19645` | [Traveler Herodot](https://lostarkcodex.com/us/npc/19645/) | 정령 토큰 교환 NPC | Open Seas - Monte Island |
| `NPC_19741` | [Flash Refiner Everfoss](https://lostarkcodex.com/us/npc/19741/) | 별 조각 교환 NPC | Open Seas - Astella |
| `NPC_19815` | [Shandars](https://lostarkcodex.com/us/npc/19815/) | Chicking 교환 NPC | Open Seas - Alakkir |
| `NPC_20630` | [Sojin](https://lostarkcodex.com/us/npc/20630/) | 물약 상인 NPC | Anikka - Twilight Mists |
| `NPC_25008` | [Kessili](https://lostarkcodex.com/us/npc/25008/) | 생활 재료 교환 NPC | North Vern - Vern Castle |
| `NPC_25016` | [Stelia](https://lostarkcodex.com/us/npc/25016/) | 요리사 NPC | North Vern - Vern Castle |
| `NPC_43998` | [Evan](https://lostarkcodex.com/us/npc/43998/) | 떠돌이 상인 NPC | South Vern - Bellion Ruins / Candaria Territory |
| `NPC_44908` | [Garento](https://lostarkcodex.com/us/npc/44908/) | 물약 상인 NPC | Rowen - Fang River |
| `NPC_44923` | [Rodaru](https://lostarkcodex.com/us/npc/44923/) | 물약 상인 NPC | Rowen - Regarbank Great Plains |
| `NPC_44929` | [Bren](https://lostarkcodex.com/us/npc/44929/) | 물약 상인 NPC | Rowen - Regarbank Great Plains |
| `NPC_44931` | [Avedo](https://lostarkcodex.com/us/npc/44931/) | 일반 상인 NPC | Rowen - Regarbank Great Plains |
| `NPC_44935` | [Lemma](https://lostarkcodex.com/us/npc/44935/) | 일반 상인 NPC | Codex에 지역 표기 없음 |
| `NPC_44936` | [Tiago](https://lostarkcodex.com/us/npc/44936/) | 물약 상인 NPC | Codex에 지역 표기 없음 |
| `NPC_50460` | [Granca](https://lostarkcodex.com/us/npc/50460/) | 일반 상인 NPC | Open Seas - Azure Wind Island |
| `NPC_50576` | [Tortello](https://lostarkcodex.com/us/npc/50576/) | 몬테 섬 보상 교환 NPC | Open Seas - Atropos |
| `NPC_50583` | [Greedy Stavros](https://lostarkcodex.com/us/npc/50583/) | 죽음의 협곡 보상 교환 NPC | Open Seas - Atropos |
| `NPC_50589` | [Dennis](https://lostarkcodex.com/us/npc/50589/) | 물약 상인 NPC | Open Seas - Atropos |
| `NPC_50593` | [Alicio](https://lostarkcodex.com/us/npc/50593/) | 생활 재료 교환 NPC | Open Seas - Atropos |
| `NPC_50738` | [Summers](https://lostarkcodex.com/us/npc/50738/) | 교환 상인 NPC | Open Seas - Liebeheim |
| `NPC_50739` | [Ramer](https://lostarkcodex.com/us/npc/50739/) | 일반 상인 NPC | Open Seas - Golden Wave Island |
| `NPC_50780` | [Dennis](https://lostarkcodex.com/us/npc/50780/) | 바텐더 NPC. `NPC_50589`와 이름은 같지만 역할·원작 ID·모델이 다르다. | Open Seas - Cradle of the Sea Fermata |
| `NPC_50944` | [Ritarre](https://lostarkcodex.com/us/npc/50944/) | 교환 상인 NPC | Open Seas - Meteora |
| `NPC_62014` | [Observer Anderson](https://lostarkcodex.com/us/npc/62014/) | 물약 상인 NPC | Codex에 지역 표기 없음 |
| `NPC_71751` | [Sellitall](https://lostarkcodex.com/us/npc/71751/) | 생활 도구 상인 NPC | Codex에 지역 표기 없음 |
| `NPC_80002` | [Ben](https://lostarkcodex.com/us/npc/80002/) | 떠돌이 상인 NPC | Rethramis 지역 여러 필드 |
| `NPC_80003` | [Burt](https://lostarkcodex.com/us/npc/80003/) | 떠돌이 상인 NPC | East Luterra 지역 여러 필드 |
| `NPC_80013` | [Jeffrey](https://lostarkcodex.com/us/npc/80013/) | 떠돌이 상인 NPC | Shushire 지역 여러 필드 |
| `NPC_80014` | [Nox](https://lostarkcodex.com/us/npc/80014/) | 떠돌이 상인 NPC | Arthetine 지역 여러 필드 |
| `NPC_80015` | [Lucas](https://lostarkcodex.com/us/npc/80015/) | 떠돌이 상인 NPC | Yudia 지역 여러 필드 |
| `NPC_80016` | [Mac](https://lostarkcodex.com/us/npc/80016/) | 떠돌이 상인 NPC | Anikka 지역 여러 필드 |
| `NPC_80017` | [Malone](https://lostarkcodex.com/us/npc/80017/) | 떠돌이 상인 NPC | West Luterra 지역 여러 필드 |
| `NPC_80018` | [Morris](https://lostarkcodex.com/us/npc/80018/) | 떠돌이 상인 NPC | East Luterra 지역 여러 필드 |
| `NPC_80019` | [Oliver](https://lostarkcodex.com/us/npc/80019/) | 떠돌이 상인 NPC | Tortoyk 지역 여러 필드 |
| `NPC_80020` | [Peter](https://lostarkcodex.com/us/npc/80020/) | 떠돌이 상인 NPC | North Vern 지역 여러 필드 |
| `NPC_80021` | [Erin](https://lostarkcodex.com/us/npc/80021/) | 교환 상인 NPC | Open Seas - Little Luck Island |
| `NPC_80059` | [Rayni](https://lostarkcodex.com/us/npc/80059/) | 떠돌이 상인 NPC | Punika 지역 여러 필드 |
| `NPC_82090` | [Noblewoman Pavellin](https://lostarkcodex.com/us/npc/82090/) | Clear Medal 교환 NPC | Arthetine - Origins of Stern |
| `NPC_83307` | [Jeniff](https://lostarkcodex.com/us/npc/83307/) | 제작법 교환 NPC | Codex에 지역 표기 없음 |
| `NPC_59030` | [Thirain](https://lostarkcodex.com/us/npc/59030/) | 고유 인물 NPC. Codex에는 상인·교환원 같은 직업 표기가 없다. | Codex에 지역 표기 없음 |

## target catalog 추가 숫자형 NPC 7개 — 이름·용도 미분류

아래 7개는 model/animation runtime 등록은 확인되지만, 이 문서가 사용하는 원작 이름·직업 근거는
확보하지 못했다. 파일명이나 외형만 보고 이름·용도를 추정하지 않는다.

| Archetype ID | 이름 | 이 NPC는 어떤 NPC인가 | 현재 프로젝트 상태 |
|---|---|---|---|
| `NPC_25001` | 미분류 | 이름·용도 근거 미확보 | supported, Map Tool 선택 가능 |
| `NPC_25002` | 미분류 | 이름·용도 근거 미확보 | supported, Map Tool 선택 가능 |
| `NPC_25029` | 미분류 | 이름·용도 근거 미확보 | supported, Map Tool 선택 가능 |
| `NPC_25184` | 미분류 | 이름·용도 근거 미확보 | supported, Map Tool 선택 가능 |
| `NPC_25287` | 미분류 | 이름·용도 근거 미확보 | supported, Map Tool 선택 가능 |
| `NPC_58700` | 미분류 | 이름·용도 근거 미확보 | supported, Map Tool 선택 가능 |
| `NPC_59060` | 미분류 | 이름·용도 근거 미확보 | supported, Map Tool 선택 가능 |

## Map Tool에서 빠르게 고르는 기준

- 도시를 사람답게 채우는 용도라면 `요리사`, `일반 상인`, `물약 상인`, `장비 상인`, `바텐더`를 섞는다.
- 장터나 광장에는 `떠돌이 상인`을 1~2명만 두고, 나머지는 일반 상인 계열로 채우면 역할이 덜 중복돼 보인다.
- 섬·교환소 분위기에는 `Exchange` 계열 NPC를 사용한다.
- `NPC_59030`은 일반 상점 군중보다 고유 인물 또는 중요 NPC 자리에 쓰는 편이 자연스럽다.
- 현재 NPC runtime은 배치·모델·제자리 생활 행동·순찰·범위 배회를 Server 권위로 지원한다. 실제 대화, 상점, 교환 기능은 별도 gameplay 수직 슬라이스가 필요하다.
