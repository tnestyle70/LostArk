#pragma once

// Windows보다 Winsock2를 먼저 포함하는 Client 공통 경계다.
#include "Client_Defines.h"
// NS_BEGIN/NS_END와 기존 Engine 공통 선언을 사용한다.
#include "Engine_Defines.h"

// Run의 장치 입력과 HANDLE·HRESULT 선언을 사용한다.
#include <d3d11.h>
// 작업 배분·최초 실패·외부 취소를 스레드 간에 공개한다.
#include <atomic>
// 작업 수와 실패 index를 size_t로 표현한다.
#include <cstddef>
// worker 수와 중첩 깊이에 고정 폭 정수를 사용한다.
#include <cstdint>
// 호출자가 준비하는 각 작업의 callback을 참조로 차용한다.
#include <functional>
// 특정 작업을 가리키지 않는 실패 index의 최댓값 표식을 정의한다.
#include <limits>
// 같은 batch의 재진입과 자식 HANDLE 목록의 취소·닫기를 동기화한다.
#include <mutex>
// 생성한 자식 worker HANDLE을 batch가 소유한다.
#include <vector>

NS_BEGIN(Client)

// Loader, PlayableCharacterAssetService, EffectPresentationService의 독립 준비 작업을 나누는 도우미다.
// 준비 결과의 Prototype 등록이나 화면 교체는 각 호출자가 join 이후에 처리한다.
// Loader·비동기 준비 상태의 값 멤버 또는 준비 함수의 지역 객체다. 스레드는 Run에서 시작한다.
// owner는 Run이 끝난 뒤 이 객체를 파괴해야 한다. 객체 소멸자 자체가 실행 중인 Run을 중지하지 않는다.
class CAssetPreparationBatch final
{
public:
	// 첫 실패 HRESULT와 해당 작업 index를 전달한다. 작업 밖 실패는 failedTask가 max이며 취소도 그 값일 수 있다.
	// workerCount는 참여시킨 호출자와 생성한 자식 수다. 실제 callback 동시 실행 수를 측정한 값은 아니다.
	struct RESULT final
	{
		HRESULT result = S_OK;
		size_t failedTask = (std::numeric_limits<size_t>::max)();
		uint32_t workerCount = 0u;
	};

	// 동기 호출이며 반환 전에 생성한 자식 스레드를 모두 join한다. callback은 여러 스레드에서 실행될 수 있다.
	// 각 task는 서로 다른 출력 슬롯을 사용해야 하며 공유 자료 접근의 동기화는 callback 책임이다.
	// 호출자는 Run이 끝날 때까지 batch, device, callback이 참조하는 값과 cancellation flag를 살려 둔다.
	// 모든 인스턴스가 최상위 callback 실행 허가와 자식 스레드 예산을 공유한다. 각각 최대 4개와 3개다.
	// SINGLETHREADED 독점은 이 batch들 사이의 보장이다. 외부 장치 호출까지 동기화하지 않는다.
	// 작은 CPU에서는 한도가 더 낮다. 실행 허가를 기다리는 호출자는 활성 허가 수에 포함하지 않는다.
	// 중첩 batch는 부모 callback의 허가를 빌려 현재 스레드에서 순차 실행한다.
	// 일반 부모의 허가를 SINGLETHREADED device용 독점 허가로 승격하는 중첩 호출은 거부한다.
	RESULT Run(ID3D11Device* device, size_t taskCount,
		const std::atomic_bool* cancellation,
		const std::function<HRESULT(size_t)>& prepare);

	// 추적 중인 자식 스레드의 동기 I/O 취소를 요청한다. cancellation flag 설정과 호출자 스레드 I/O 취소는 호출자 책임이다.
	void Cancel_SynchronousIo() noexcept;

private:
	struct RUN_STATE;
	// Run 정상 종료와 RUN_STATE 정리에서 자식 종료를 확인하고 HANDLE을 닫는다.
	void Join_Workers() noexcept;

	// Run 재진입 상태와 자식 HANDLE 목록의 등록/닫기/취소를 보호한다. callback의 출력 자료를 보호하는 lock은 아니다.
	std::mutex m_WorkerMutex;
	// Run이 등록하고 Join_Workers가 닫는 자식 HANDLE의 소유 목록이다.
	std::vector<HANDLE> m_Workers;
	bool m_bRunning = false;
};

NS_END
