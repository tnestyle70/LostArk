// Headless product CShader verification; isolated numeric draw readback, no window/capture.
#include "Shader.h"
#include "Engine_VertexTypes.h"
#include <array>
#include <cmath>
#include <type_traits>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>

using namespace Engine;

static void Require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

static bool ContainsBytes(ID3D11Device* device, ID3D11DeviceContext* context,
    ID3D11Buffer* buffer, const void* expected, size_t size)
{
    if (!buffer) return false;
    D3D11_BUFFER_DESC descriptor{};
    buffer->GetDesc(&descriptor);
    if (size > descriptor.ByteWidth) return false;
    descriptor.Usage = D3D11_USAGE_STAGING;
    descriptor.BindFlags = 0;
    descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    descriptor.MiscFlags = 0;
    ComPtr<ID3D11Buffer> staging;
    Require(SUCCEEDED(device->CreateBuffer(&descriptor, nullptr, &staging)), "staging buffer creation failed");
    context->CopyResource(staging.Get(), buffer);
    D3D11_MAPPED_SUBRESOURCE mapped{};
    Require(SUCCEEDED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)), "constant readback failed");
    bool found = false;
    for (size_t offset = 0; offset + size <= descriptor.ByteWidth; offset += sizeof(float))
        if (0 == std::memcmp(static_cast<const uint8_t*>(mapped.pData) + offset, expected, size)) { found = true; break; }
    context->Unmap(staging.Get(), 0);
    return found;
}

static bool ConstantsBound(ID3D11Device* device, ID3D11DeviceContext* context,
    bool vertex, const void* expected, size_t size)
{
    ID3D11Buffer* buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT]{};
    if (vertex) context->VSGetConstantBuffers(0, std::size(buffers), buffers);
    else context->PSGetConstantBuffers(0, std::size(buffers), buffers);
    bool found = false;
    for (auto* buffer : buffers)
    {
        if (buffer) { found |= ContainsBytes(device, context, buffer, expected, size); buffer->Release(); }
    }
    return found;
}

static bool TextureBound(ID3D11DeviceContext* context, ID3D11ShaderResourceView* expected)
{
    ID3D11ShaderResourceView* resources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    context->PSGetShaderResources(0, std::size(resources), resources);
    bool found = false;
    for (auto* resource : resources)
    {
        found |= resource && resource == expected;
        if (resource) resource->Release();
    }
    return found;
}

template<class Function> static double Measure(Function&& function)
{
    constexpr uint32_t iterations = 2000u;
    for (uint32_t i = 0; i < 100u; ++i) function(i);
    const auto start = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < iterations; ++i) function(i);
    return std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start).count() / iterations;
}


// A closed slab isolates hull coverage, depth occlusion, alpha masks and transient
// hit reset. Numeric readback is not a claim about any game's final appearance.
template<class Vertex> static uint32_t CheckCombatDraw(ID3D11Device* device,
    ID3D11DeviceContext* context, CShader& shader, uint32_t outlinePass, bool silhouette = false)
{
    constexpr UINT side = 64u;
    D3D11_TEXTURE2D_DESC td{};
    td.Width = td.Height = side; td.MipLevels = td.ArraySize = 1u;
    td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; td.SampleDesc.Count = 1u;
    td.BindFlags = D3D11_BIND_RENDER_TARGET;
    std::array<ComPtr<ID3D11Texture2D>, 8> targets;
    std::array<ComPtr<ID3D11RenderTargetView>, 8> views;
    ID3D11RenderTargetView* rawViews[8]{};
    for (UINT i = 0; i < 8u; ++i)
    {
        Require(SUCCEEDED(device->CreateTexture2D(&td, nullptr, &targets[i])) &&
            SUCCEEDED(device->CreateRenderTargetView(targets[i].Get(), nullptr, &views[i])), "combat MRT creation failed");
        rawViews[i] = views[i].Get();
    }
    td.BindFlags = 0u; td.Usage = D3D11_USAGE_STAGING; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> readback;
    Require(SUCCEEDED(device->CreateTexture2D(&td, nullptr, &readback)), "combat readback creation failed");
    td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    td.Usage = D3D11_USAGE_DEFAULT; td.CPUAccessFlags = 0u;
    ComPtr<ID3D11Texture2D> depth;
    ComPtr<ID3D11DepthStencilView> dsv;
    Require(SUCCEEDED(device->CreateTexture2D(&td, nullptr, &depth)) &&
        SUCCEEDED(device->CreateDepthStencilView(depth.Get(), nullptr, &dsv)), "combat depth creation failed");
    std::array<Vertex, 8> vertices{};
    for (UINT i = 0; i < 8u; ++i)
    {
        const float x = (i % 4u >= 2u) ? .45f : -.45f;
        const float y = (i % 4u == 1u || i % 4u == 2u) ? .45f : -.45f;
        vertices[i].vPosition = {x, y, i < 4u ? .4f : .6f};
        vertices[i].vNormal = {x, y, i < 4u ? -1.f : 1.f};
        vertices[i].vTangent = {1.f,0.f,0.f}; vertices[i].vBinormal = {0.f,1.f,0.f};
        vertices[i].vTexcoord = {.5f,.5f};
        if constexpr (std::is_same_v<Vertex,VTXANIMMESH>) vertices[i].vBlendWeights = {1.f,0.f,0.f,0.f};
    }
    const UINT indices[] = {0,1,2,0,2,3,4,6,5,4,7,6};
    D3D11_BUFFER_DESC bd{}; bd.ByteWidth = sizeof(vertices); bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA data{vertices.data(),0u,0u};
    ComPtr<ID3D11Buffer> vb, ib;
    Require(SUCCEEDED(device->CreateBuffer(&bd, &data, &vb)), "combat vertices failed");
    bd.ByteWidth = sizeof(indices); bd.BindFlags = D3D11_BIND_INDEX_BUFFER; data.pSysMem = indices;
    Require(SUCCEEDED(device->CreateBuffer(&bd, &data, &ib)), "combat indices failed");
    UINT stride = sizeof(Vertex), offset = 0u; ID3D11Buffer* rawVB = vb.Get();
    context->IASetVertexBuffers(0u,1u,&rawVB,&stride,&offset);
    context->IASetIndexBuffer(ib.Get(),DXGI_FORMAT_R32_UINT,0u);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    const D3D11_VIEWPORT viewport{0.f,0.f,float(side),float(side),0.f,1.f};
    context->RSSetViewports(1u,&viewport); context->OMSetRenderTargets(8u,rawViews,dsv.Get());
    float4x4_t identity; DirectX::XMStoreFloat4x4(&identity,DirectX::XMMatrixIdentity());
    for (const char* name : {"g_WorldMatrix","g_ViewMatrix","g_ProjMatrix"})
        Require(SUCCEEDED(shader.Bind_Matrix(name,&identity)), "combat transform bind failed");
    if constexpr (std::is_same_v<Vertex,VTXANIMMESH>)
    {
        std::array<float4x4_t,512> bones; bones.fill(identity);
        Require(SUCCEEDED(shader.Bind_Matrices("g_BoneMatrices",bones.data(),UINT(bones.size()))), "combat bones failed");
    }
    else Require(SUCCEEDED(shader.Bind_Matrix("g_WorldInvTransposeMatrix",&identity)), "combat normal matrix failed");
    const uint32_t zero = 0u, one = 1u; const float intensity = 4.f;
    const float4_t yellow{1.f,.72f,.08f,1.f}; const float2_t width{(silhouette?8.f:4.f)/side,(silhouette?8.f:4.f)/side};
    for (const char* name : {"g_SourceCharacterProgram","g_SurfaceProgram","g_HasNormalTexture",
        "g_HasEmissiveTexture","g_HasFullSurfaceEmissiveOverride","g_CombatHoverReflected"})
        Require(SUCCEEDED(shader.Bind_RawValue(name,&zero,sizeof(zero))), "combat reset bind failed");
    Require(SUCCEEDED(shader.Bind_RawValue("g_CombatHoverNdcWidth",&width,sizeof(width))), "combat width bind failed");
    const auto texture = [&](bool opaque) {
        D3D11_TEXTURE2D_DESC desc{}; desc.Width=desc.Height=desc.MipLevels=desc.ArraySize=1u;
        desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count=1u; desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        const UINT pixel=opaque?0xffffffffu:0x00ffffffu; D3D11_SUBRESOURCE_DATA input{&pixel,4u,0u};
        ComPtr<ID3D11Texture2D> tex; ComPtr<ID3D11ShaderResourceView> srv;
        Require(SUCCEEDED(device->CreateTexture2D(&desc,&input,&tex)) &&
            SUCCEEDED(device->CreateShaderResourceView(tex.Get(),nullptr,&srv)) &&
            SUCCEEDED(shader.Bind_Texture("g_DiffuseTexture",srv)), "combat diffuse bind failed");
    };
    const auto clear = [&](float z) { const float black[4]{};
        for (const auto& view:views) context->ClearRenderTargetView(view.Get(),black);
        context->ClearDepthStencilView(dsv.Get(),D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,z,5u); };
    uint32_t drawCount = 0u;
    const auto draw = [&](uint32_t pass) { Require(SUCCEEDED(shader.Begin(pass)),"combat pass failed"); context->DrawIndexed(12u,0u,0); ++drawCount; };
    const auto count = [&](bool rimOnly) {
        context->CopyResource(readback.Get(),targets[4].Get()); D3D11_MAPPED_SUBRESOURCE mapped{};
        Require(SUCCEEDED(context->Map(readback.Get(),0u,D3D11_MAP_READ,0u,&mapped)), "combat map failed");
        uint32_t lit=0u, inside=0u; bool finite=true;
        for(UINT y=0;y<side;++y) for(UINT x=0;x<side;++x) {
            const float* rgb=reinterpret_cast<const float*>(static_cast<const uint8_t*>(mapped.pData)+y*mapped.RowPitch)+x*4u;
            finite &= std::isfinite(rgb[0]) && std::isfinite(rgb[1]) && std::isfinite(rgb[2]);
            if(rgb[0]>.001f) { ++lit; if(x>=20u && x<44u && y>=20u && y<44u) ++inside; }
        }
        context->Unmap(readback.Get(),0u); Require(finite,"combat RGB was not finite");
        if(rimOnly) Require(inside==0u,"hover painted over the body interior"); return lit;
    };
    if (silhouette)
    {
        const uint32_t mask = 0x80u;
        const auto maskDraw = [&](uint32_t reference) {
            Require(SUCCEEDED(shader.Bind_RawValue("g_CombatHoverStencilReference", &reference, sizeof(reference))), "mask reference bind failed");
            draw(20u);
        };
        const auto background = [&] {
            float4x4_t world; XMStoreFloat4x4(&world, XMMatrixScaling(3.f,3.f,1.f)*XMMatrixTranslation(0,0,.4f));
            Require(SUCCEEDED(shader.Bind_Matrix("g_WorldMatrix",&world)),"background transform failed");
            draw(0u);
            Require(SUCCEEDED(shader.Bind_Matrix("g_WorldMatrix",&identity)),"body transform restore failed");
        };
        // Reproduce the former order: read-only hull is erased by later opaque
        // geometry because no body depth exists at the exterior border pixels.
        texture(true); clear(1.f); draw(0u); draw(18u);
        Require(count(true)>0u,"old-order fixture has no initial border");
        background(); Require(count(false)==0u,"old-order overwrite was not reproduced");
        D3D11_TEXTURE2D_DESC depthReadDesc; depth->GetDesc(&depthReadDesc);
        depthReadDesc.Usage=D3D11_USAGE_STAGING; depthReadDesc.BindFlags=0; depthReadDesc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
        ComPtr<ID3D11Texture2D> depthRead;
        Require(SUCCEEDED(device->CreateTexture2D(&depthReadDesc,nullptr,&depthRead)),"stencil readback creation failed");
        const auto depthBytes = [&] {
            std::array<uint32_t,side*side> values;
            context->CopyResource(depthRead.Get(),depth.Get()); D3D11_MAPPED_SUBRESOURCE mapped{};
            Require(SUCCEEDED(context->Map(depthRead.Get(),0,D3D11_MAP_READ,0,&mapped)),"stencil map failed");
            for(UINT y=0;y<side;++y) std::memcpy(values.data()+y*side,
                static_cast<const uint8_t*>(mapped.pData)+y*mapped.RowPitch,side*sizeof(uint32_t));
            context->Unmap(depthRead.Get(),0); return values;
        };
        uint32_t least=UINT32_MAX, most=0;
        for (const float shift : {-.012f,-.006f,0.f,.006f,.012f})
        {
            float4x4_t view; XMStoreFloat4x4(&view,XMMatrixTranslation(shift,0,0));
            Require(SUCCEEDED(shader.Bind_Matrix("g_ViewMatrix",&view)),"moving camera bind failed");
            clear(1.f); draw(0u); background(); const auto before=depthBytes();
            maskDraw(mask); draw(outlinePass); const auto pixels=count(true);
            Require(pixels>=360u,"four-pixel silhouette was missing/thinned during camera motion");
            least=(std::min)(least,pixels); most=(std::max)(most,pixels);
            maskDraw(0u); Require(before==depthBytes(),"hover changed depth or failed to return its stencil bit");
        }
        Require(SUCCEEDED(shader.Bind_Matrix("g_ViewMatrix",&identity)),"view restore failed");
        clear(0.f); maskDraw(mask); draw(outlinePass); maskDraw(0u);
        Require(count(false)==0u,"silhouette ignored scene occlusion");
        texture(false); clear(1.f); maskDraw(mask); draw(outlinePass); maskDraw(0u);
        Require(count(false)==0u,"silhouette ignored alpha coverage");
        std::printf("silhouette moving-camera pixels=%u..%u old-order-repro/interior/occlusion/alpha/stencil-depth PASS\n",least,most);
        context->ClearState(); return drawCount;
    }
    texture(true); clear(1.f); draw(0u); draw(outlinePass);
    const UINT rimPixels=count(true); Require(rimPixels>0u,"hover emitted no border pixels");
    clear(0.f); draw(outlinePass); Require(count(false)==0u,"hover ignored occluding depth");
    texture(false); clear(1.f); draw(outlinePass); Require(count(false)==0u,"hover ignored material alpha mask");
    texture(true); clear(1.f);
    Require(SUCCEEDED(shader.Bind_RawValue("g_HasFullSurfaceEmissiveOverride",&one,sizeof(one))) &&
        SUCCEEDED(shader.Bind_RawValue("g_FullSurfaceEmissiveMaskMode",&one,sizeof(one))) &&
        SUCCEEDED(shader.Bind_RawValue("g_FullSurfaceEmissiveColor",&yellow,sizeof(yellow))) &&
        SUCCEEDED(shader.Bind_RawValue("g_FullSurfaceEmissiveIntensity",&intensity,sizeof(intensity))), "combat hit bind failed");
    draw(0u); Require(count(false)>0u,"hit emitted no finite radiance");
    clear(1.f); Require(SUCCEEDED(shader.Bind_RawValue("g_HasFullSurfaceEmissiveOverride",&zero,sizeof(zero))),"hit reset failed");
    draw(0u); Require(count(false)==0u,"expired hit leaked to following draw");
    std::printf("combat outline pass=%u rimPixels=%u depth/mask/hit-reset PASS\n",outlinePass,rimPixels);
    context->ClearState(); return drawCount;
}

int wmain(int argc, wchar_t** argv)
{
    try
    {
        Require(argc == 2 || argc == 3, "expected isolated compiled shader directory [--silhouette-only]");
        const std::filesystem::path directory = argv[1];
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL level{};
        Require(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            nullptr, 0, D3D11_SDK_VERSION, &device, &level, &context)), "WARP device creation failed");
        if (argc == 3 && std::wstring(argv[2]) == L"--silhouette-only")
        {
            auto shader=CShader::Create(device,context,L"HoverSilhouette.hlsl",VTXANIMMESH::Elements,VTXANIMMESH::iNumElements);
            Require(shader != nullptr,"focused silhouette shader failed");
            CheckCombatDraw<VTXANIMMESH>(device.Get(),context.Get(),*shader,21u,true);
            return 0;
        }
        D3D11_TEXTURE2D_DESC textureDesc{};
        textureDesc.Width = textureDesc.Height = textureDesc.MipLevels = textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11ShaderResourceView> view;
        Require(SUCCEEDED(device->CreateTexture2D(&textureDesc, nullptr, &texture)) &&
            SUCCEEDED(device->CreateShaderResourceView(texture.Get(), nullptr, &view)), "texture fixture creation failed");
        struct Fixture { const wchar_t* name; const D3D11_INPUT_ELEMENT_DESC* elements; uint32_t count; uint32_t pass; bool light; bool bones; };
        const Fixture fixtures[] = {
            {L"Shader_VtxAnimMeshBinary", VTXANIMMESH::Elements, VTXANIMMESH::iNumElements, 0u, false, true},
            {L"Shader_VtxMeshBinary", VTXMESH::Elements, VTXMESH::iNumElements, 0u, false, false},
            {L"Shader_Deferred", VTXTEX::Elements, VTXTEX::iNumElements, 19u, true, false}
        };
        uint32_t programsChecked = 0u, clonesChecked = 0u, failureCases = 0u, lightPassesChecked = 0u;
        uint32_t nativePassesChecked = 0u, unavailablePassesChecked = 0u;
        uint32_t afterimagePassesChecked = 0u, combatPassesChecked = 0u, combatDraws = 0u;
        for (const auto& fixture : fixtures)
        {
            const std::wstring logical = std::wstring(fixture.name) + L".hlsl";
            auto prototype = CShader::Create(device, context, logical.c_str(), fixture.elements, fixture.count);
            Require(prototype != nullptr, "product CShader creation / cohort ABI verification failed");
            auto first = std::dynamic_pointer_cast<CShader>(prototype->Clone(nullptr));
            auto second = std::dynamic_pointer_cast<CShader>(prototype->Clone(nullptr));
            Require(first && second, "product CShader clone failed");
            prototype.reset();
            const char* constantsName = fixture.light ? "g_SourceCharacterLightConstants" : "g_SourceCharacterBaseConstants";
            std::array<float, 256> constants{};
            std::vector<float4x4_t> bones(512);
            if (fixture.bones)
            {
                for (size_t i = 0; i < bones.size(); ++i)
                    for (auto& row : bones[i].m) for (float& value : row) value = 20.25f + static_cast<float>(i);
                Require(SUCCEEDED(first->Bind_Matrices("g_BoneMatrices", bones.data(), static_cast<uint32_t>(bones.size()))), "bone matrix array bind failed");
            }
            ComPtr<ID3D11PixelShader> cohortShader;
            for (uint32_t program = 1u; program <= 89u; ++program)
            {
                if (program > 32u && program < 80u) continue;
                for (size_t i = 0; i < constants.size(); ++i) constants[i] = 1000.125f + static_cast<float>(program * 300u + i);
                Require(SUCCEEDED(first->Bind_RawValue(constantsName, constants.data(), sizeof(constants))) &&
                    SUCCEEDED(first->Bind_Texture("g_SourceCharacterTexture0", view)) &&
                    SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &program, sizeof(program))), "source material bind failed");
                Require(SUCCEEDED(first->Begin(fixture.pass)), "source cohort pass failed");
                Require(ConstantsBound(device.Get(), context.Get(), false, constants.data(), sizeof(constants)), "native constants were not transferred to selected PS");
                Require(TextureBound(context.Get(), view.Get()), "native texture was not transferred to selected PS");
                if (fixture.bones) Require(ConstantsBound(device.Get(), context.Get(), true, bones.data(), sizeof(float4x4_t) * bones.size()), "bone array was not transferred to selected VS");
                if (fixture.light)
                {
                    for (uint32_t pass : {1u, 2u, 16u, 19u, 20u, 21u, 22u, 23u, 24u, 25u, 26u, 27u})
                    {
                        Require(SUCCEEDED(first->Begin(pass)), "legacy / instanced / mask fallback light pass failed");
                        ++lightPassesChecked;
                    }
                    Require(SUCCEEDED(first->Begin(fixture.pass)), "restore primary light pass failed");
                }
                if (fixture.bones && (program == 18u || program == 88u))
                {
                    Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterLightConstants", constants.data(), sizeof(constants))), "translucent hair light constants bind failed");
                    for (uint32_t pass : {9u, 10u})
                        Require(SUCCEEDED(first->Begin(pass)) && ConstantsBound(device.Get(), context.Get(), false, constants.data(), sizeof(constants)), "translucent hair light pass lost its constants");
                    Require(SUCCEEDED(first->Begin(fixture.pass)), "restore primary geometry pass failed");
                }
                if (!fixture.light && (program == 21u || program == 25u || program == 84u))
                {
                    const uint32_t firstPass = fixture.bones ? 18u : 28u;
                    for (uint32_t pass : {firstPass, firstPass + 1u})
                    {
                        Require(SUCCEEDED(first->Begin(pass)), "combat hover cohort pass failed");
                        ComPtr<ID3D11DepthStencilState> state; UINT reference = 0u;
                        context->OMGetDepthStencilState(&state,&reference);
                        D3D11_DEPTH_STENCIL_DESC desc{}; Require(state != nullptr,"combat depth state missing"); state->GetDesc(&desc);
                        Require(desc.DepthEnable && desc.DepthWriteMask == D3D11_DEPTH_WRITE_MASK_ZERO && !desc.StencilEnable,
                            "combat hover modified scene depth/stencil contract");
                        ++combatPassesChecked;
                    }
                    Require(SUCCEEDED(first->Begin(fixture.pass)), "combat primary pass restore failed");
                }
                ComPtr<ID3D11PixelShader> active;
                context->PSGetShader(&active, nullptr, nullptr);
                Require(active != nullptr, "selected pass has no PS");
                Require(SUCCEEDED(second->Begin(fixture.pass)), "clone did not observe shared source program / constants");
                context->PSGetShader(&cohortShader, nullptr, nullptr);
                Require(active == cohortShader, "clones selected different FX state for the same cohort");
                ++programsChecked;
            }
            uint32_t program = 9u;
            for (size_t i = 0; i < constants.size(); ++i) constants[i] = 80000.25f + static_cast<float>(i);
            Require(SUCCEEDED(second->Bind_RawValue(constantsName, constants.data(), sizeof(constants))) &&
                SUCCEEDED(second->Bind_RawValue("g_SourceCharacterProgram", &program, sizeof(program))) &&
                SUCCEEDED(first->Begin(fixture.pass)), "clone cohort switch failed");
            Require(ConstantsBound(device.Get(), context.Get(), false, constants.data(), sizeof(constants)), "older clone revision hid another clone's update");
            clonesChecked += 2u;
            if (fixture.bones)
            {
                auto directA = CShader::Create(device, context, L"Shader_VtxAnimMeshBinary_SourceGroup001.hlsl", fixture.elements, fixture.count);
                auto directB = CShader::Create(device, context, L"Shader_VtxAnimMeshBinary_SourceGroup009.hlsl", fixture.elements, fixture.count);
                Require(directA && directB, "direct cohort benchmark creation failed");

                // Source groups have no base-owned native cue or afterimage PS. Direct callers must fail
                // before applying an invisible pass, while the owning base FX
                // must select its native PS despite a stale surface program.
                for (uint32_t pass : {7u, 8u, 12u, 13u, 14u})
                {
                    Require(FAILED(directA->Begin(pass)) && FAILED(directB->Begin(pass)),
                        "direct source group accepted a base-owned native cue pass");
                    unavailablePassesChecked += 2u;
                }
                std::array<float, 128> nativeConstants{};
                for (size_t i = 0; i < nativeConstants.size(); ++i)
                    nativeConstants[i] = 30000.25f + static_cast<float>(i);
                Require(SUCCEEDED(first->Bind_RawValue("g_ArtistSourceMaterialParameters",
                    nativeConstants.data(), sizeof(nativeConstants))) &&
                    SUCCEEDED(first->Bind_Texture("g_SourceTexture0", view)), "native cue fixture binding failed");
                for (uint32_t nativeProfile : {178u, 460u, 461u, 772u, 1360u, 3828u, 3831u, 3832u, 3833u})
                {
                    Require(SUCCEEDED(first->Bind_RawValue("g_ArtistModelCueProfile",
                        &nativeProfile, sizeof(nativeProfile))), "native cue profile bind failed");
                    uint32_t surfaceProgram = 0u;
                    Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &surfaceProgram,
                        sizeof(surfaceProgram))) && SUCCEEDED(first->Begin(7u)), "base native cue pass failed");
                    ComPtr<ID3D11PixelShader> expectedNative;
                    context->PSGetShader(&expectedNative, nullptr, nullptr);
                    Require(expectedNative != nullptr, "base native cue PS missing");
                    for (uint32_t staleProgram : {1u, 9u, 18u, 88u})
                    {
                        Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &staleProgram,
                            sizeof(staleProgram))), "stale source program fixture bind failed");
                        for (uint32_t pass : {7u, 8u, 12u, 13u})
                        {
                            Require(SUCCEEDED(second->Begin(pass)), "shared clone native cue pass failed");
                            ComPtr<ID3D11PixelShader> actualNative;
                            context->PSGetShader(&actualNative, nullptr, nullptr);
                            Require(actualNative == expectedNative, "native cue selected a source-group PS");
                            Require(ConstantsBound(device.Get(), context.Get(), false, nativeConstants.data(),
                                sizeof(nativeConstants)) && TextureBound(context.Get(), view.Get()),
                                "native cue lost base-owned constants or textures");
                            Require(ConstantsBound(device.Get(), context.Get(), true, bones.data(),
                                sizeof(float4x4_t) * bones.size()), "native cue lost base-owned bone matrices");
                            ++nativePassesChecked;
                        }
                    }
                }
                const std::array<float, 4> afterimageColor{1.8125f, 1.6875f, 1.5625f, .3125f};
                uint32_t surfaceProgram = 0u;
                Require(SUCCEEDED(first->Bind_RawValue("g_ChargeAfterimageColor", afterimageColor.data(),
                    sizeof(afterimageColor))) && SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram",
                    &surfaceProgram, sizeof(surfaceProgram))) && SUCCEEDED(first->Begin(14u)),
                    "base afterimage pass failed");
                ComPtr<ID3D11PixelShader> expectedAfterimage;
                context->PSGetShader(&expectedAfterimage, nullptr, nullptr);
                Require(expectedAfterimage != nullptr, "base afterimage PS missing");
                for (uint32_t staleProgram : {1u, 9u, 18u, 88u})
                {
                    Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &staleProgram,
                        sizeof(staleProgram))) && SUCCEEDED(second->Begin(14u)),
                        "shared clone afterimage pass failed with a stale source program");
                    ComPtr<ID3D11PixelShader> actualAfterimage;
                    context->PSGetShader(&actualAfterimage, nullptr, nullptr);
                    Require(actualAfterimage == expectedAfterimage, "afterimage selected a source-group PS");
                    Require(ConstantsBound(device.Get(), context.Get(), false, afterimageColor.data(),
                        sizeof(afterimageColor)), "afterimage lost base-owned color/fade constants");
                    Require(ConstantsBound(device.Get(), context.Get(), true, bones.data(),
                        sizeof(float4x4_t) * bones.size()), "afterimage lost base-owned bone matrices");
                    ++afterimagePassesChecked;
                }
                for (bool updates : {false, true}) for (bool switching : {false, true})
                {
                    const auto execute = [&](CShader& shader, uint32_t i)
                    {
                        const uint32_t selected = switching && (i & 1u) ? 9u : 1u;
                        Require(SUCCEEDED(shader.Bind_RawValue("g_SourceCharacterProgram", &selected, sizeof(selected))), "benchmark program bind failed");
                        if (updates)
                        {
                            constants[0] = static_cast<float>(i);
                            Require(SUCCEEDED(shader.Bind_RawValue(constantsName, constants.data(), sizeof(constants))) &&
                                SUCCEEDED(shader.Bind_Matrices("g_BoneMatrices", bones.data(), static_cast<uint32_t>(bones.size()))), "benchmark constant/bone bind failed");
                        }
                        Require(SUCCEEDED(shader.Begin(fixture.pass)), "benchmark pass failed");
                    };
                    const double directUs = Measure([&](uint32_t i) { execute(switching && (i & 1u) ? *directB : *directA, i); });
                    const double groupedUs = Measure([&](uint32_t i) { execute(*first, i); });
                    std::printf("{\"microbench\":true,\"updates\":%s,\"switching\":%s,\"directUs\":%.3f,\"groupedUs\":%.3f}\n",
                        updates ? "true" : "false", switching ? "true" : "false", directUs, groupedUs);
                }
                // Restore the expected shared clone fixture after the benchmark.
                for (size_t i = 0; i < constants.size(); ++i) constants[i] = 80000.25f + static_cast<float>(i);
                Require(SUCCEEDED(first->Bind_RawValue(constantsName, constants.data(), sizeof(constants))), "post-benchmark fixture restore failed");
            }
            program = 66u;
            Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &program, sizeof(program))) &&
                FAILED(first->Begin(fixture.pass)), "unknown program did not fail closed");
            ++failureCases;
            program = 9u;
            Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &program, sizeof(program))), "restore valid program failed");
            const auto shard = directory / (std::wstring(fixture.name) + L"_SourceGroup001.cso");
            const auto saved = directory / (std::wstring(fixture.name) + L"_SourceGroup001.saved");
            std::filesystem::rename(shard, saved);
            const auto missing = CShader::Create(device, context, logical.c_str(), fixture.elements, fixture.count);
            const HRESULT reloaded = first->Initialize_Prototype(logical.c_str(), fixture.elements, fixture.count);
            std::filesystem::rename(saved, shard);
            Require(missing == nullptr && FAILED(reloaded), "missing cohort was accepted");
            Require(SUCCEEDED(first->Begin(fixture.pass)) &&
                ConstantsBound(device.Get(), context.Get(), false, constants.data(), sizeof(constants)), "failed reload replaced existing valid shader state");
            failureCases += 2u;
            if (!fixture.light)
                combatDraws += fixture.bones ? CheckCombatDraw<VTXANIMMESH>(device.Get(), context.Get(), *first, 18u) :
                    CheckCombatDraw<VTXMESH>(device.Get(), context.Get(), *first, 28u);
                if (fixture.bones) combatDraws += CheckCombatDraw<VTXANIMMESH>(device.Get(),context.Get(),*first,21u,true);
            std::printf("checked source shader %ls\n", fixture.name);
        }
        std::printf("{\"programsChecked\":%u,\"clonesChecked\":%u,\"failureCases\":%u,\"lightPassesChecked\":%u,\"nativePassesChecked\":%u,\"afterimagePassesChecked\":%u,\"unavailablePassesChecked\":%u,\"combatPassesChecked\":%u,\"windowsCreated\":0,\"draws\":%u}\n", programsChecked, clonesChecked, failureCases, lightPassesChecked, nativePassesChecked, afterimagePassesChecked, unavailablePassesChecked, combatPassesChecked, combatDraws);
        context->ClearState();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "SourceCharacter shader variant probe: %s\n", error.what());
        return 1;
    }
}
