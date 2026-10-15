#pragma once

// 화면 오차에 따른 LOD 선택 입력을 사용한다.
#include "MeshLod.h"
// 원본 VTXMESH의 위치/법선/UV/색상 채널을 단순화 입력으로 읽는다.
#include "Engine_VertexTypes.h"
// 고정 개수 범위 저장소와 호출 중에만 차용하는 정점/인덱스 span을 사용한다.
#include <array>
#include <span>

NS_BEGIN(Engine)

// 추가 불변 인덱스 범위가 원본 vertex buffer를 공유한다.
// 기본 경로와 shadow는 원본 mesh의 index buffer를 계속 사용한다.
class CStaticMeshLod final
{
public:
    // 중간 크기 건물도 원경 LOD를 준비하되 원본 정점/재질과 보더는 유지한다.
    static constexpr size_t MINIMUM_INDICES = 512u * 3u;
    static constexpr size_t MAXIMUM_RANGES = 5u;
    static constexpr size_t ATTRIBUTE_MINIMUM_INDICES = 128u * 3u;
    // 결합 인덱스 버퍼의 범위다. firstIndex/indexCount는 바이트가 아니라 uint32 인덱스 원소 단위다.
    struct SELECTION final
    {
        uint32_t indexCount = 0u;
        uint32_t firstIndex = 0u;
        uint32_t level = 0u;
    };

    // 원본 vertex/index는 호출 중에만 차용하고 파생 GPU 인덱스 버퍼를 소유한다.
    // 진입 시 result를 비운다. 생성 생략은 S_FALSE이며 실패 시 이전 result를 보존하는 함수는 아니다.
    static HRESULT Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
        std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result,
        bool_t preserveAttributes = false);
    // 뷰 공간 bounds와 화면 오차로 범위를 선택한다. 입력 오류는 출력 보존, 정상 입력에서 축소가 불가능하면 level 0을 반환한다.
    HRESULT Select_Range(const MESH_SCREEN_LOD_DESC& view, SELECTION& result) const;
    // 소유 COM 참조를 늘리지 않는 조회다. 반환 포인터 사용 동안 이 LOD 객체를 유지해야 한다.
    ID3D11Buffer* Get_IndexBuffer() const
    {
        return m_IndexBuffer.Get();
    }
    // GPU bin은 이 불변 범위를 args에 넣고 instanceCount만 갱신한다. 실패 시 출력 보존.
    bool_t Get_Range(uint32_t level, SELECTION& result, f32_t& localError) const;

private:
    // count/first는 인덱스 원소 단위, error는 원본 로컬 공간 길이다. Select_Range가 배율과 투영으로 픽셀 오차를 계산한다.
    struct RANGE final
    {
        uint32_t count = 0u, first = 0u;
        f32_t error = 0.f;
    };
    std::array<RANGE, MAXIMUM_RANGES> m_Ranges{};
    uint32_t m_RangeCount = 0u;
    ComPtr<ID3D11Buffer> m_IndexBuffer;
};

NS_END
