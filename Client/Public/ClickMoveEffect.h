#pragma once

// 표시를 허용하는 LEVEL과 Client 공통 선언을 사용한다.
#include "Client_Defines.h"
// Layer에서 Late_Update를 받는 CGameObject·Prototype 경로를 사용한다.
#include "GameObject.h"
// 실제 World 이펙트 생성·중지에 쓰는 handle과 공용 표현 서비스를 사용한다.
#include "Effect_PresentationService.h"

NS_BEGIN(Client)
// 플레이어의 생존 여부만 weak_ptr로 확인하므로 이름을 먼저 선언한다.
class CCharacter;

// 이동 명령을 제출한 지점과 방 핑의 시각 피드백이다. Server의 경로·목표 승인 표시는 아니다.
class CClickMoveEffect final : public CGameObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG =
		L"Prototype_GameObject_ClickMoveEffect";
	// Level 진입 준비에서 이동·핑 장식의 사용 가능 여부를 확인한다.
	static bool_t Uses_LevelMarkers(LEVEL level);
	// 지원 Level의 선택적 이펙트를 각각 준비 큐에 넣고 수락된 target 목록을 반환한다.
	static std::vector<std::string> Queue_LevelResources(LEVEL level);

private:
	CClickMoveEffect(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	// 기반 객체만 복사한다. 클릭·핑 handle과 재생 시간은 새 인스턴스의 초기 상태로 시작한다.
	CClickMoveEffect(const CClickMoveEffect& prototype);

public:
	// Clear로 이 객체가 시작한 클릭·핑 이펙트를 중지한 뒤 보유 참조를 정리한다.
	virtual ~CClickMoveEffect();
	// 원형은 추가 GPU 자원을 만들지 않는다. 실제 이펙트 준비는 표현 서비스가 담당한다.
	virtual HRESULT Initialize_Prototype() override;
	// Clone 입력으로 기반 GameObject만 초기화한다.
	virtual HRESULT Initialize(void* pArg) override;
	// Level·플레이어 상태와 초 단위 만료 시간을 확인해 클릭·핑을 정리한다.
	virtual void Late_Update(f32_t fTimeDelta) override;
	// 이 객체는 직접 그리지 않는다. 생성한 World 이펙트의 기존 렌더 경로가 그린다.
	virtual HRESULT Render() override;
	// 표시할 Level을 저장한다. 범위 밖 또는 미지원 Level이면 기존 값을 유지하고 false다.
	bool_t Initialize_Effects(uint32_t levelIndex);
	// PlayerController의 이동 명령 송신 성공 뒤 클릭 지점에 피드백을 표시한다.
	void Play(const float3_t& worldPosition,
		const shared_ptr<CCharacter>& character);
	// 발신자·순번으로 핑 배치 ID를 만들고 최대 64개를 시간 제한으로 표시한다.
	void Play_Ping(const float3_t& worldPosition, std::uint32_t senderId, std::uint32_t sequence);
	// 클릭만 중지하거나 Clear로 방 핑까지 함께 정리한다.
	void Clear_Move();
	void Clear();

private:
	// 같은 실패 문자열의 반복 로그를 억제한다.
	void Report_Failure(const std::string& status);
	// 새 World 이펙트 생성이 성공한 뒤 전달받은 기존 handle을 중지·교체한다.
	bool_t Spawn_Marker(const char* effectId, const char* placementId,
		const float3_t& position, EFFECT_WORLD_ROOT_HANDLE& handle, bool_t sustained = false);
	uint32_t m_iLevelIndex = ETOUI(LEVEL::END);
	// 표현 서비스가 만든 이펙트를 중지할 handle이다. 이 클래스가 mesh·shader 객체를 직접 소유하지 않는다.
	EFFECT_WORLD_ROOT_HANDLE m_ClickHandle;
	struct ROOM_PING_MARKER
	{
		EFFECT_WORLD_ROOT_HANDLE Handle;
		f32_t fSeconds = 0.f;
	};
	std::vector<ROOM_PING_MARKER> m_RoomPings;
	// 플레이어 수명을 연장하지 않고 종료 여부만 확인한다.
	weak_ptr<CCharacter> m_pCharacter;
	f32_t m_fClickSeconds = 0.f;
	std::string m_strLastFailure;

public:
	// MainApp이 STATIC 원형으로 등록할 객체를 만든다.
	static unique_ptr<CClickMoveEffect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	// PlayerController가 Layer_ClickMoveEffect 등록 시 복제한다. Layer와 Controller가 함께 참조한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};
NS_END
