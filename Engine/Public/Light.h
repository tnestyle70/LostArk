#pragma once

// 빛 렌더에 전달할 shader/buffer 소유 참조와 엔진 공통 수치 타입을 사용한다.
#include "Engine_Defines.h"
// LIGHT_DESC와 빛 종류/수신 대상/지연 렌더 pass 계약을 소비한다.
#include "Engine_RenderTypes.h"

NS_BEGIN(Engine)

// 빛 설명 한 개를 값으로 소유한다. 장면 전체 목록과 일시적인 빛의 배치 제출은 Light_Manager가 담당한다.
class CLight final 
{
private:
	CLight();
public:
	~CLight();

public:
	// 기하·감쇠 조건을 확인한 뒤 설명을 복사한다. 거부 시 기존 설명은 유지한다.
	HRESULT Initialize(const LIGHT_DESC& LightDesc);
	// 보관한 설명을 단일 빛 pass로 그린다. shader/buffer는 호출 동안 소유 참조를 유지하며 유효한 입력이 필요하다.
	HRESULT Render(
		shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer);
	// 종류·수신 대상·감쇠·위치/방향 조건을 검사한다. diffuse/ambient/specular 색상 전체 검증은 장면 관리자 경로가 더 수행한다.
	static bool_t Is_ValidDesc(const LIGHT_DESC& LightDesc);
	// 검증 후 shader 값을 바인딩하고 quad를 그린다. 도중 실패는 이전에 바인딩한 shader 상태를 되돌리지 않는다.
	static HRESULT Render_Desc(
		const LIGHT_DESC& LightDesc,
		shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer,
		bool_t bApplyDirectionalShadow = false, bool_t bApplyStaticShadow = false,
		bool_t bSourceLightMask = false);

private:
	// 실제 빛 데이터의 소유 복사본이다. 초기화 성공 후 Render가 소비한다.
	LIGHT_DESC				m_LightDesc;

public:
	// 입력을 거부하거나 초기화가 실패하면 nullptr다. 완성한 객체만 호출자에게 단독 소유로 넘긴다.
	static unique_ptr<CLight> Create(const LIGHT_DESC& LightDesc);
};

NS_END
