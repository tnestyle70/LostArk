#include "ClientReplication.h"

#include "Character.h"
#include "CharacterCatalog.h"
#include "GameInstance.h"
#include "NetworkManager.h"
#include "Transform.h"

namespace
{
	Client::NET_PLAYER_RECORD Make_Record(
		const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned)
	{
		Client::NET_PLAYER_RECORD record{};

		record.iPlayerId = spawned.iPlayerId;

		record.iNetEntityId = spawned.iNetEntityId;

		record.eCharacterClass = spawned.eCharacterClass;

		record.strNickName = spawned.strNickName;

		record.fPositionX = spawned.fPositionX;
		record.fPositionY = spawned.fPositionY;
		record.fPositionZ = spawned.fPositionZ;

		record.fYawDegrees = spawned.fYawDegrees;

		return record;
	}
	// TCP 재전송이나 중복 Event가 같은 Spawn을 다시 만들지 않게 하고,
	// 같은 NetEntityId에 다른 내용이 오면 Protocol 충돌로 판단한다.
	bool Is_Same_Record(
		const Client::NET_PLAYER_RECORD& left,
		const Client::NET_PLAYER_RECORD& right)
	{
		return
			left.iPlayerId == right.iPlayerId &&
			left.iNetEntityId == right.iNetEntityId &&
			left.eCharacterClass == right.eCharacterClass &&
			left.strNickName == right.strNickName &&
			left.fPositionX == right.fPositionX &&
			left.fPositionY == right.fPositionY &&
			left.fPositionZ == right.fPositionZ &&
			left.fYawDegrees == right.fYawDegrees;
	}
}

bool Client::CClientReplication::Initialize(const DESC& desc)
{
	//Layer 정보가 유한한지 검사하고, 설정을 저장한다.
	//현재 network 연결 상태도 기억해 두어, 이후 연결이 끊겼는지 감지할 수 있게 한다.
	if (desc.strPlayerLayerTag.empty())
		return false;

	m_Desc = desc;
	m_isInitialized = true;
	m_wasConnected =
		CNetworkManager::Get().Is_Connected();

	return true;
}

bool Client::CClientReplication::Update()
{
	if (!m_isInitialized)
		return false;
	//network manager가 만들어놓은 replication event를 실제 engine 변경으로 적용한다.
	//baren main thread에서 매프레임 호출한다.
	//초기화 여부 검사 -> 현재 network 연결 상태 확인
	CNetworkManager& networkManager =
		CNetworkManager::Get();

	const bool isConnected =
		networkManager.Is_Connected();
	//연결이 끊어진 경우 : 이전 프레임에는 연결돼 있었는가? -> 그렇다면 reset world()
	//남아 있는 event queue 비우기 -> 이전 서버의 spawn이 재접속 후 적용되지 않게 함
	if (!isConnected)
	{
		if(m_wasConnected)
			Reset_World();

		CLIENT_REPLICATION_EVENT ignored{};
		while (networkManager.Try_Consume_ReplicationEvent(ignored))
		{

		}
		m_wasConnected = false;
		return true;
	}

	m_wasConnected = true;
	bool allSucceeded = true;

	CLIENT_REPLICATION_EVENT event{};

	while (networkManager.Try_Consume_ReplicationEvent(event))
	{
		switch (event.eType)
		{
		case CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED:
			allSucceeded =
				Apply_Spawn(event.PlayerSpawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PLAYER_DESPAWNED:
			allSucceeded =
				Apply_Despawn(event.PlayerDespawned) &&
				allSucceeded;
			break;
		}
	}

	return allSucceeded;
}

std::shared_ptr<CCharacter> Client::CClientReplication::Get_LocalCharacter() const
{
	return m_Registry.Resolve(m_LocalCharacterHandle);
}

bool Client::CClientReplication::Apply_Spawn(
	const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned)
{
	//서버 player를 client character로 생성한다.
	//spawn message -> net player record 변환
	//동일  entity id가 이미 있는지를 검사한다.
	//character catalog에서 class spec 검색 ->
	//character desc 구성 -> local remote 판정
	//add gameobject to layer -> character 캐스팅 및 transform 검사
	//yaw 반영 -> registry 등록 -> local이면 localcharacterhandle 저장

	const NET_PLAYER_RECORD stagedRecord =
		Make_Record(spawned);

	if (const NET_PLAYER_RECORD* existing =
		m_Registry.Find_Record(spawned.iNetEntityId))
	{
		//같은 event 재전송은 no-op, 다른 내용의 같은 id는  protocol conflict이다.
		return Is_Same_Record(*existing, stagedRecord);
	}

	const CHARACTER_SPEC* spec =
		CCharacterCatalog::Find_Spec(spawned.eCharacterClass);

	if (nullptr == spec)
		return false;

	CCharacter::CHARACTER_DESC desc{};
	desc.iPrototypeLevelIndex =
		m_Desc.iPrototypeLevelIndex;
	desc.pSpec = spec;
	desc.pNavigationPrototypeTag = nullptr;
	desc.fSpeedPerSec = 6.f;
	desc.fRotationPerSec = 180.f;
	desc.vPosition = float3_t(
		spawned.fPositionX,
		spawned.fPositionY,
		spawned.fPositionZ);
	desc.strNickName = spawned.strNickName;
	desc.isLocallyControlled =
		spawned.iPlayerId ==
		CNetworkManager::Get().Get_LocalPlayerId();

	std::shared_ptr<CGameObject> gameObject;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		m_Desc.iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Character"),
		m_Desc.iLayerLevelIndex,
		m_Desc.strPlayerLayerTag,
		&desc,
		&gameObject)))
	{
		return false;
	}

	const std::shared_ptr<CCharacter> character =
		std::dynamic_pointer_cast<CCharacter>(gameObject);

	if (nullptr == character ||
		nullptr == character->Get_Transform())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			gameObject);
		return false;
	}

	character->Get_Transform()->Rotation(
		0.f,
		spawned.fYawDegrees,
		0.f);

	OBJECT_HANDLE handle{};

	if (!m_Registry.Register(
		stagedRecord,
		character,
		handle))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			gameObject);
		return false;
	}

	if (desc.isLocallyControlled)
		m_LocalCharacterHandle = handle;

	return true;
}

bool Client::CClientReplication::Apply_Despawn(
	const LostArk::Shared::S2C_PLAYER_DESPAWNED& despawned)
{
	OBJECT_HANDLE handle{};

	if (!m_Registry.Find_Handle(
		despawned.iNetEntityId, handle))
	{
		//이미 제거된 중복 despawn은 no-op으로 취급
		return true;
	}

	const std::shared_ptr<CCharacter> character =
		m_Registry.Resolve(handle);
	//gameobject layer에서 제거
	if (nullptr != character &&
		FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			character)))
	{
		return false;
	}

	if (!m_Registry.Unregister(despawned.iNetEntityId))
		return false;

	if (m_LocalCharacterHandle.iSlotIndex == handle.iSlotIndex &&
		m_LocalCharacterHandle.iGeneration == handle.iGeneration)
	{
		m_LocalCharacterHandle = {};
	}

	return true;
}

void Client::CClientReplication::Reset_World()
{
	//접속이 끊겼을 때 현재 registry의 살아있는 character를 모두 layer에서 제거하고,
	//registry와 local handle을 초기화한다.
	//파괴자에서 호출하지 않는 이유도 맞다. 현재 engine은 레벨 전환 시 layer를
	//먼저 제거하므로, level 파괴자에서 다시 layer 제거를 시도하면, 이미
	//사라진 layer를 건드릴 수 있다.
	const std::vector<std::shared_ptr<CCharacter>> characters =
		m_Registry.Get_LiveObjects();

	for (const auto& character : characters)
	{
		if (nullptr == character)
			continue;

		//Layer에서 GameObject 제거하기
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			character);
	}
	m_Registry.Reset();
	m_LocalCharacterHandle = {};
}
