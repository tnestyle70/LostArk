"""Compile the production readback methods against headless D3D11 WARP.

No Client, window creation, cursor movement or network is used. Cursor APIs are
stubbed; production Request/Poll/Cancel use real WARP texture copies and Map,
including an unfinished GPU copy and stale-request cancellation.
"""
from pathlib import Path
import json
import subprocess
import argparse

ROOT = Path(__file__).resolve().parents[2]


def method(source, signature):
    start = source.index(signature)
    opening = source.index('\n{', start) + 1
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=Path, default=ROOT / 'out/AsyncPickingRegression')
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'Engine/Private/Picking.cpp').read_text(encoding='utf-8-sig')
    methods = '\n'.join(method(source, signature) for signature in (
        'CPicking::CPicking(', 'CPicking::~CPicking()', 'HRESULT CPicking::Initialize(',
        'uint64_t CPicking::Request_Picking(', 'HRESULT CPicking::Poll_Picking(', 'void CPicking::Cancel_Picking(',
        'bool_t CPicking::Read_Pixel('))
    cpp = '''#include "Engine_Defines.h"
#define private public
#include "Picking.h"
#undef private
#include <cmath>
#include <cstring>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
static ::POINT testCursor{17,23};
static bool testCursorAvailable=true;
static BOOL TestGetCursorPos(::POINT* cursor) { *cursor=testCursor; return testCursorAvailable; }
static BOOL TestScreenToClient(HWND,::POINT*) { return TRUE; }
namespace Engine {
enum class EProfilerCounter { PickingReadbacks, PickingReadbackBytes };
class CProfiler { public: void Add_Counter(EProfilerCounter,unsigned long long=1) {} };
class CProfilerScope { public: CProfilerScope(CProfiler*,const char*) {} };
class CProfilerGpuScope { public: CProfilerGpuScope(CProfiler*,const char*) {} };
class CGameInstance {
public:
    ComPtr<ID3D11ShaderResourceView> source;
    static CGameInstance& Get() { static CGameInstance game; return game; }
    ComPtr<ID3D11ShaderResourceView> Get_RT_SRV(const wchar_t*) const { return source; }
    CProfiler* Get_Profiler() const { return nullptr; }
};
}
#define GetCursorPos TestGetCursorPos
#define ScreenToClient TestScreenToClient
''' + methods + r'''
#undef GetCursorPos
#undef ScreenToClient
static void require(bool value, const char* label) { if (!value) throw std::runtime_error(label); }
int main() {
    try {
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        require(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            nullptr, 0, D3D11_SDK_VERSION, &device, nullptr, &context)), "WARP device");
        CPicking picking(device, context);
        require(SUCCEEDED(picking.Initialize(nullptr)), "Initialize");
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width=512; desc.Height=512; desc.MipLevels=1; desc.ArraySize=1;
        desc.Format=DXGI_FORMAT_R32G32B32A32_FLOAT;
        desc.SampleDesc.Count=1; desc.Usage=D3D11_USAGE_DEFAULT;
        desc.BindFlags=D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        ComPtr<ID3D11Texture2D> target;
        ComPtr<ID3D11RenderTargetView> view;
        require(SUCCEEDED(device->CreateTexture2D(&desc,nullptr,&target)), "target");
        require(SUCCEEDED(device->CreateRenderTargetView(target.Get(),nullptr,&view)), "view");
        D3D11_BOX box{17,23,0,18,24,1};
        float4_t output{91,92,93,94};
        require(picking.Poll_Picking(1,output)==E_ABORT && output.x==91, "unrequested output unchanged");
        unsigned pending=0, checks=1;
        double maxPollMs=0;
        for (unsigned pass=0;pass<12;++pass) {
            float values[]{float(pass+1),-2,3,1};
            for (int busy=0;busy<256;++busy) context->ClearRenderTargetView(view.Get(),values);
            context->CopySubresourceRegion(picking.m_pAsyncTexture.Get(),0,0,0,0,target.Get(),0,&box);
            picking.m_iPendingRequestId=pass+1;
            const auto started=std::chrono::steady_clock::now();
            HRESULT hr=picking.Poll_Picking(pass+1,output);
            maxPollMs=(std::max)(maxPollMs,std::chrono::duration<double,std::milli>(
                std::chrono::steady_clock::now()-started).count());
            if (hr==S_FALSE) { ++pending; require(output.x==float(pass ? pass : 91), "pending preserves output"); }
            // Headless tests have no Present; only this test submits queued work.
            context->Flush();
            const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
            while(hr==S_FALSE && std::chrono::steady_clock::now()<deadline) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                hr=picking.Poll_Picking(pass+1,output);
            }
            require(hr==S_OK && output.x==values[0] && output.y==-2 && output.z==3 && output.w==1,
                "exact captured pixel");
            require(picking.Poll_Picking(pass+1,output)==E_ABORT, "one-shot result");
            checks+=2;
        }
        picking.m_iPendingRequestId=101;
        picking.Cancel_Picking(100);
        require(picking.m_iPendingRequestId==101,"stale cancel preserves newer request");
        picking.Cancel_Picking(101);
        require(picking.Poll_Picking(101,output)==E_ABORT,"canceled result rejected");
        checks+=2;
        for (int invalid=0;invalid<2;++invalid) {
            float values[]{invalid ? NAN : 0.f,2,3,invalid ? 1.f : 0.f};
            context->ClearRenderTargetView(view.Get(),values);
            context->CopySubresourceRegion(picking.m_pAsyncTexture.Get(),0,0,0,0,target.Get(),0,&box);
            picking.m_iPendingRequestId=200+invalid;
            context->Flush();
            HRESULT hr=S_FALSE;
            const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
            while(hr==S_FALSE && std::chrono::steady_clock::now()<deadline) {
                hr=picking.Poll_Picking(200+invalid,output);
                if(hr==S_FALSE) std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            require(hr==E_FAIL,"empty/nonfinite rejected"); ++checks;
        }
        // Request uses fake cursor APIs only; texture copies and Map use real WARP.
        picking.m_hWnd=reinterpret_cast<HWND>(1);
        require(SUCCEEDED(device->CreateShaderResourceView(target.Get(),nullptr,
            &CGameInstance::Get().source)),"source SRV");
        float captured[]{27,28,29,1};
        context->ClearRenderTargetView(view.Get(),captured);
        const auto requested=picking.Request_Picking();
        require(requested!=0,"production request accepted");++checks;
        testCursor={111,222};
        float later[]{71,72,73,1};
        context->ClearRenderTargetView(view.Get(),later);
        context->Flush();
        HRESULT requestedResult=S_FALSE;
        const auto requestDeadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
        while(requestedResult==S_FALSE&&std::chrono::steady_clock::now()<requestDeadline) {
            requestedResult=picking.Poll_Picking(requested,output);
            if(requestedResult==S_FALSE)std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        require(requestedResult==S_OK&&output.x==27&&output.y==28&&output.z==29,
            "request captures original cursor/RT occurrence before later render");++checks;
        const auto previous=picking.Request_Picking();
        const auto latest=picking.Request_Picking();
        require(latest!=previous&&picking.Poll_Picking(previous,output)==E_ABORT,
            "latest request rejects previous occurrence");++checks;
        picking.Cancel_Picking(previous);
        require(picking.m_iPendingRequestId==latest,"stale request cancel preserves latest");++checks;
        testCursor={-1,23};
        require(picking.Request_Picking()==0&&picking.Poll_Picking(latest,output)==E_ABORT,
            "invalid new cursor supersedes stale pending result");++checks;
        testCursor={512,23};require(picking.Request_Picking()==0,"out-of-bounds cursor rejected");++checks;
        testCursor={17,23};testCursorAvailable=false;
        require(picking.Request_Picking()==0,"cursor read failure rejected");++checks;
        testCursorAvailable=true;
        ComPtr<ID3D11Device> otherDevice;ComPtr<ID3D11DeviceContext> otherContext;
        require(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,
            nullptr,0,D3D11_SDK_VERSION,&otherDevice,nullptr,&otherContext)),"other WARP device");
        ComPtr<ID3D11Texture2D> otherTarget;
        require(SUCCEEDED(otherDevice->CreateTexture2D(&desc,nullptr,&otherTarget)),"other target");
        CGameInstance::Get().source.Reset();
        require(SUCCEEDED(otherDevice->CreateShaderResourceView(otherTarget.Get(),nullptr,
            &CGameInstance::Get().source)),"other source");
        require(picking.Request_Picking()==0,"foreign device source rejected before GPU copy");++checks;
        require(pending>0,"unfinished GPU copy exercised");
        CGameInstance::Get().source.Reset();
        require(SUCCEEDED(device->CreateShaderResourceView(target.Get(),nullptr,
            &CGameInstance::Get().source)),"restore source");
        // Same WARP rendering backlog for the production legacy and async paths.
        // Timing is evidence, not a scheduler-sensitive pass/fail threshold.
        std::string comparison;
        for (int repeats : {128,256,512}) {
            const float values[]{42,43,44,1};
            for(int i=0;i<repeats;++i) context->ClearRenderTargetView(view.Get(),values);
            const auto syncAt=std::chrono::steady_clock::now();
            require(picking.Read_Pixel(target.Get(),17,23,output,nullptr),"legacy exact readback");
            const double syncMs=std::chrono::duration<double,std::milli>(
                std::chrono::steady_clock::now()-syncAt).count();
            for(int i=0;i<repeats;++i) context->ClearRenderTargetView(view.Get(),values);
            const auto asyncAt=std::chrono::steady_clock::now();
            const auto id=picking.Request_Picking();
            require(id!=0,"comparison request accepted");
            HRESULT hr=picking.Poll_Picking(id,output);
            const double asyncMs=std::chrono::duration<double,std::milli>(
                std::chrono::steady_clock::now()-asyncAt).count();
            const bool wasPending=hr==S_FALSE;
            context->Flush();
            const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
            while(hr==S_FALSE&&std::chrono::steady_clock::now()<deadline) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                hr=picking.Poll_Picking(id,output);
            }
            require(hr==S_OK&&output.x==42&&output.y==43&&output.z==44,"comparison same pixel");
            checks+=3;
            if(!comparison.empty())comparison+=",";
            comparison+="{\"renderClears\":"+std::to_string(repeats)+
                ",\"legacyCpuWaitMs\":"+std::to_string(syncMs)+
                ",\"asyncRequestAndPollMs\":"+std::to_string(asyncMs)+
                ",\"asyncPending\":"+(wasPending?"true":"false")+"}";
        }
        std::cout<<"{\"checks\":"<<checks<<",\"pendingCopies\":"<<pending
            <<",\"maximumFirstPollMs\":"<<maxPollMs
            <<",\"driver\":\"D3D11 WARP, synthetic render backlog\",\"comparison\":["<<comparison<<"]}\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
'''
    path = args.out / 'probe.cpp'
    path.write_text(cpp, encoding='utf-8')
    vswhere = Path('C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe')
    install = subprocess.check_output([str(vswhere), '-latest', '-prerelease', '-products', '*',
        '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property',
        'installationPath'], text=True).strip()
    command = args.out / 'compile.cmd'
    command.write_text(f'@echo off\ncall "{install}/VC/Auxiliary/Build/vcvars64.bat" >nul\n'
        f'cl /nologo /std:c++20 /EHsc /O2 /MD /DNDEBUG /DUNICODE /D_UNICODE '
        f'/I"{ROOT}/Engine/Public" "{path}" /Fe:"{args.out}/probe.exe" '
        f'/Fo:"{args.out}/probe.obj" /link d3d11.lib user32.lib\n', encoding='utf-8')
    compiled = subprocess.run(['cmd', '/c', str(command)], capture_output=True, text=True,
        encoding='utf-8', errors='replace')
    (args.out / 'compile.log').write_text(compiled.stdout + compiled.stderr, encoding='utf-8')
    if compiled.returncode:
        raise RuntimeError(f'Compile failed: {args.out / "compile.log"}')
    result = subprocess.check_output([str(args.out / 'probe.exe')], text=True)
    json.loads(result)
    (args.out / 'result.json').write_text(result, encoding='utf-8')
    print(result.strip())


if __name__ == '__main__':
    main()
