"""Compile production Deploy CPU picking and Level resolver composition unchanged.

The native probe uses real DirectXMath and the current object/runtime methods,
including their suppression predicate. CModel is a deterministic geometry seam:
it records live matrices, ray normalization, mesh/cull selection and distance
limits. This verifies carrier dispatch and visibility, not installed WModel
triangles, GPU pixels, Server navigation or interactive Client behavior.
"""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess

from test_async_picking import method

ROOT = Path(__file__).resolve().parents[2]


def resolver(source):
    """Return the actual movement resolver lambda, retaining its complete body."""
    start = source.index('[', source.index('Set_MovementSurfaceResolver('))
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=Path, default=ROOT / 'out/DeployMoveSurfaceRegression')
    parser.add_argument('--configuration', choices=['Debug', 'Release'], default='Release')
    args = parser.parse_args()
    args.out = args.out.resolve()
    args.out.mkdir(parents=True, exist_ok=True)
    object_source = (ROOT / 'Client/Private/DeployPropObject.cpp').read_text(encoding='utf-8-sig')
    runtime_source = (ROOT / 'Client/Private/DeployPropRuntime.cpp').read_text(encoding='utf-8-sig')
    methods = '\n'.join((
        method(object_source, 'bool_t CDeployPropObject::Try_PickMovementSurface('),
        method(object_source, 'bool_t CDeployPropObject::Is_BasePresentationSuppressed('),
        method(runtime_source, 'bool_t CDeployPropRuntime::Try_PickMovementSurface(')))
    lambdas = {}
    for level in ('ValtanArena', 'KakulSaydonArena', 'Bern', 'CharacterSelect', 'Development'):
        source = (ROOT / f'Client/Private/Level_{level}.cpp').read_text(encoding='utf-8-sig')
        candidate = resolver(source)
        if 'm_DeployRuntime.Try_PickMovementSurface' in candidate:
            lambdas[level] = candidate
    assert 'ValtanArena' in lambdas and 'KakulSaydonArena' in lambdas, 'Deploy floors need both arena consumers'
    cpp = r'''
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
using namespace DirectX;
using bool_t=bool; using f32_t=float; using float3_t=XMFLOAT3;
using float4x4_t=XMFLOAT4X4; using vector_t=XMVECTOR;
using std::shared_ptr;
bool IsFinite(const float3_t& v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
enum class LEVEL { VALTAN_ARENA, END };
unsigned ETOUI(LEVEL v){return static_cast<unsigned>(v);}
struct CGameInstance {
    bool replaced=false;
    static CGameInstance& Get(){static CGameInstance game;return game;}
    bool Is_SceneEnvironmentReplaced()const{return replaced;}
    void* Get_Profiler()const{return nullptr;}
};
namespace Engine {struct CProfilerScope{CProfilerScope(void*,const char*){}};}
enum class DEPLOY_PROP_MODEL_KIND { STATIC, ANIM };
enum class DEPLOY_PROP_STATE { INTACT, FRACTURED, DESPAWNED };
struct CTransform {
    float4x4_t world;
    CTransform(){XMStoreFloat4x4(&world,XMMatrixIdentity());}
    const float4x4_t* Get_WorldMatrixPtr()const{return &world;}
};
struct CModel {
    enum class PICK_CULL_MODE { NONE, BACK, FRONT };
    struct Query {unsigned mesh;float4x4_t world;float3_t origin,direction;float limit;PICK_CULL_MODE cull;};
    // Each fixture mesh is a deterministic horizontal CPU plane. This is a
    // geometry seam, deliberately not a replacement for production BVH tests.
    std::vector<float> heights{0.f}; bool skinned=false,hasGeometry=true,poseHit=true;
    float poseDistance=3.f;
    mutable std::vector<Query> queries; mutable unsigned poses=0;
    mutable float4x4_t poseWorld{};
    unsigned Get_NumMeshes()const{return static_cast<unsigned>(heights.size());}
    bool Is_Skinned()const{return skinned;}
    bool Try_PickStaticSurface(unsigned mesh,const float4x4_t& world,
        const float3_t& origin,const float3_t& direction,float limit,PICK_CULL_MODE cull,float& out)const{
        queries.push_back({mesh,world,origin,direction,limit,cull});
        if(!hasGeometry||skinned||mesh>=heights.size()||!std::isfinite(limit)||limit<0||
            !IsFinite(origin)||!IsFinite(direction)||std::abs(direction.y)<1.e-8f)return false;
        const float len=std::sqrt(direction.x*direction.x+direction.y*direction.y+direction.z*direction.z);
        const float candidate=(world._42+heights[mesh]-origin.y)/(direction.y/len);
        if(!std::isfinite(candidate)||candidate<0||candidate>limit)return false;
        out=candidate;return true;
    }
    bool Try_PickCurrentPose(const float4x4_t& world,const float3_t&,const float3_t&,float& out)const{
        ++poses;poseWorld=world;if(!poseHit)return false;out=poseDistance;return true;
    }
};
struct CDeployPropObject {
    enum class DEBRIS_PRESENTATION_OWNER { NONE, MAPTOOL_PREVIEW, PRODUCT_DESTRUCTION };
    DEPLOY_PROP_MODEL_KIND m_ModelKind=DEPLOY_PROP_MODEL_KIND::STATIC;
    DEPLOY_PROP_STATE m_State=DEPLOY_PROP_STATE::INTACT;
    struct {float fOpacity=1.f;} m_SurfacePresentation;
    bool m_bTransientDestructionSuppressed=false,m_bCameraPreviewSuppressed=false;
    bool m_bDebrisPreviewActive=false,m_bDebrisSuppressSource=false,m_bPhysicsPreviewActive=false;
    bool m_bAnimationAuthoringRevealHidden=false;
    DEBRIS_PRESENTATION_OWNER m_eDebrisPresentationOwner=DEBRIS_PRESENTATION_OWNER::NONE;
    shared_ptr<CTransform> m_pTransformCom=std::make_shared<CTransform>();
    shared_ptr<CModel> m_pIntactModelCom=std::make_shared<CModel>(),m_pFracturedModelCom;
    bool_t Try_PickMovementSurface(const float3_t&,const float3_t&,f32_t,f32_t&)const;
    bool_t Is_BasePresentationSuppressed()const;
};
struct DEPLOY_RUNTIME_ENTRY {shared_ptr<CDeployPropObject> object;};
struct CDeployPropRuntime {
    unsigned m_iLevelIndex=ETOUI(LEVEL::VALTAN_ARENA);
    std::vector<DEPLOY_RUNTIME_ENTRY> m_Entries;
    bool Is_Loaded()const{return m_iLevelIndex<ETOUI(LEVEL::END);}
    bool_t Try_PickMovementSurface(const float3_t&,const float3_t&,f32_t,float3_t&)const;
};
''' + methods + r'''
struct MapRuntime {
    bool hit=false;float3_t position{0,0,0};unsigned queries=0;
    bool Try_PickMovementSurface(const float3_t&,const float3_t&,float3_t& out){
        ++queries;if(!hit)return false;out=position;return true;}
};
'''
    for level, callback in lambdas.items():
        cpp += f'''struct Level_{level} {{
    MapRuntime m_MapRuntime; CDeployPropRuntime m_DeployRuntime;
    auto Resolver(){{return {callback};}}
}};
'''
    cpp += r'''
unsigned checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
bool near(float a,float b){return std::abs(a-b)<.0001f;}
bool equal(const float3_t& a,const float3_t& b){return near(a.x,b.x)&&near(a.y,b.y)&&near(a.z,b.z);}
const float3_t ORIGIN{1,10,2}, DOWN{0,-1,0}, SENTINEL{91,92,93};
shared_ptr<CDeployPropObject> makeFloor(float height){
    auto object=std::make_shared<CDeployPropObject>();object->m_pTransformCom->world._42=height;return object;}
bool pick(CDeployPropObject& object,float& out,float limit=100){return object.Try_PickMovementSurface(ORIGIN,DOWN,limit,out);}
void miss(CDeployPropObject& object,const char* label){
    float out=91;require(!pick(object,out)&&out==91,label);}
template<class Level> void composition(){
    Level level;auto resolve=level.Resolver();float3_t out=SENTINEL;
    auto object=makeFloor(4);level.m_DeployRuntime.m_Entries={{object}};
    require(resolve(ORIGIN,DOWN,out)&&equal(out,{1,4,2}),"Deploy-only floor resolves when Map misses");
    level.m_MapRuntime.hit=true;level.m_MapRuntime.position={1,0,2};out=SENTINEL;
    require(resolve(ORIGIN,DOWN,out)&&equal(out,{1,4,2}),"nearer Deploy floor overrides Map");
    require(near(object->m_pIntactModelCom->queries.back().limit,10),"Map distance bounds Deploy geometry query");
    level.m_MapRuntime.position={1,8,2};out=SENTINEL;
    require(resolve(ORIGIN,DOWN,out)&&equal(out,{1,8,2}),"nearer Map floor wins over farther Deploy");
    level.m_MapRuntime.position={1,4,2};out=SENTINEL;
    require(resolve(ORIGIN,DOWN,out)&&equal(out,{1,4,2}),"equal-depth Map/Deploy surface remains valid");
    object->m_State=DEPLOY_PROP_STATE::DESPAWNED;level.m_MapRuntime.position={1,0,2};
    require(resolve(ORIGIN,DOWN,out)&&equal(out,{1,0,2}),"despawned Deploy allows Map beneath");
    level.m_MapRuntime.hit=false;out=SENTINEL;
    require(!resolve(ORIGIN,DOWN,out)&&equal(out,SENTINEL),"both misses preserve caller output");
    object->m_State=DEPLOY_PROP_STATE::INTACT;out=SENTINEL;
    require(resolve(ORIGIN,{0,-7,0},out)&&equal(out,{1,4,2}),"composition handles non-unit ray in world distance");
}
int main(){try{
    {auto object=makeFloor(4);float out=91;require(pick(*object,out)&&near(out,6),"visible intact Deploy floor picked");
     const auto& q=object->m_pIntactModelCom->queries.back();
     require(q.cull==CModel::PICK_CULL_MODE::BACK&&near(q.world._42,4),"static rendering uses BACK cull and current world matrix");
     object->m_pTransformCom->world._42=7;require(pick(*object,out)&&near(out,3),"live transform is re-read after presentation movement");}
    {auto object=makeFloor(0);object->m_pIntactModelCom->heights={1,8,3};float out=91;
     require(pick(*object,out)&&near(out,2),"nearest of all static submeshes selected");
     const auto& q=object->m_pIntactModelCom->queries;
     require(q.size()==3&&q[0].mesh==0&&q[1].mesh==1&&q[2].mesh==2,"all current model submeshes considered");
     require(near(q[0].limit,100)&&near(q[1].limit,9)&&near(q[2].limit,2),"nearest hit narrows subsequent mesh limits");}
    {auto object=makeFloor(4);object->m_State=DEPLOY_PROP_STATE::FRACTURED;
     object->m_pFracturedModelCom=std::make_shared<CModel>();object->m_pFracturedModelCom->heights={3};float out=91;
     require(pick(*object,out)&&near(out,3)&&object->m_pIntactModelCom->queries.empty(),"fractured state queries rendered fractured mesh only");
     object->m_pFracturedModelCom.reset();miss(*object,"missing fractured model cannot pick invisible intact geometry");}
    {auto object=makeFloor(4);object->m_State=DEPLOY_PROP_STATE::DESPAWNED;miss(*object,"despawned source never picked");
     object->m_bAnimationAuthoringRevealHidden=true;float out=91;
     require(pick(*object,out)&&near(out,6),"explicit authoring reveal matches source-visible render predicate");}
    for(float opacity:{0.f,.0001f}){auto object=makeFloor(4);object->m_SurfacePresentation.fOpacity=opacity;
        miss(*object,"invisible opacity rejected without output mutation");}
    {auto object=makeFloor(4);object->m_SurfacePresentation.fOpacity=.00011f;float out=91;
     require(pick(*object,out),"visible fade above render threshold remains pickable");}
    {auto object=makeFloor(4);object->m_bCameraPreviewSuppressed=true;miss(*object,"camera-hidden source rejected");}
    {auto object=makeFloor(4);object->m_bTransientDestructionSuppressed=true;miss(*object,"transient destruction suppression respected");}
    {auto object=makeFloor(4);object->m_bDebrisPreviewActive=true;object->m_bDebrisSuppressSource=true;
     object->m_eDebrisPresentationOwner=CDeployPropObject::DEBRIS_PRESENTATION_OWNER::PRODUCT_DESTRUCTION;
     miss(*object,"product debris source suppression respected");
     object->m_eDebrisPresentationOwner=CDeployPropObject::DEBRIS_PRESENTATION_OWNER::MAPTOOL_PREVIEW;
     object->m_bPhysicsPreviewActive=true;miss(*object,"active physics preview source suppression respected");
     object->m_bPhysicsPreviewActive=false;float out=91;require(pick(*object,out),"inactive physics preview keeps its source pickable");}
    {auto object=makeFloor(4);object->m_pTransformCom.reset();miss(*object,"missing transform rejected");}
    {auto object=makeFloor(4);object->m_pIntactModelCom.reset();miss(*object,"missing intact model rejected");}
    {auto object=makeFloor(4);object->m_pIntactModelCom->heights.clear();miss(*object,"empty static model rejected");}
    {auto object=makeFloor(4);object->m_pIntactModelCom->hasGeometry=false;miss(*object,"unsupported geometry preserves output");}
    {auto object=makeFloor(4);object->m_ModelKind=DEPLOY_PROP_MODEL_KIND::ANIM;object->m_State=DEPLOY_PROP_STATE::FRACTURED;
     auto model=object->m_pIntactModelCom;model->skinned=true;model->poseDistance=2;float out=91;
     require(pick(*object,out,3)&&near(out,2)&&model->poses==1&&model->queries.empty(),"animated fractured prop queries current pose on rendered intact carrier");
     require(near(model->poseWorld._42,4),"animated pose receives current root transform");
     model->poseDistance=4;out=91;require(!pick(*object,out,3)&&out==91,"animated current-pose hit beyond Map limit rejected");
     model->poseDistance=3;out=91;require(!pick(*object,out,3)&&out==91,"animated tie cannot replace nearest existing surface");
     for(float distance:{-1.f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}){
         model->poseDistance=distance;miss(*object,"malformed animated pose distance rejected");}
     model->poseDistance=2;model->poseHit=false;miss(*object,"animated pose miss preserves output");}
    {CDeployPropRuntime runtime;auto far=makeFloor(2),close=makeFloor(8);runtime.m_Entries={{far},{nullptr},{close}};float3_t out=SENTINEL;
     require(runtime.Try_PickMovementSurface(ORIGIN,{0,-9,0},100,out)&&equal(out,{1,8,2}),"runtime normalizes and selects nearest non-null entry");
     require(near(far->m_pIntactModelCom->queries.back().direction.y,-1)&&near(close->m_pIntactModelCom->queries.back().limit,8),"runtime passes unit ray and bounded nearest distance");
     runtime.m_Entries={{makeFloor(6)}};out=SENTINEL;
     require(runtime.Try_PickMovementSurface(ORIGIN,DOWN,100,out)&&equal(out,{1,6,2}),"runtime uses current entry set after replacement");
     runtime.m_Entries.clear();out=SENTINEL;
     require(!runtime.Try_PickMovementSurface(ORIGIN,DOWN,100,out)&&equal(out,SENTINEL),"cleared runtime preserves output");}
    {CDeployPropRuntime runtime;auto object=makeFloor(4);runtime.m_Entries={{object}};float3_t out=SENTINEL;
     require(!runtime.Try_PickMovementSurface(ORIGIN,DOWN,5,out)&&equal(out,SENTINEL),"runtime maximum world distance excludes farther floor");
     require(!runtime.Try_PickMovementSurface(ORIGIN,DOWN,6,out)&&equal(out,SENTINEL),"runtime tie preserves nearest existing surface");
     CGameInstance::Get().replaced=true;out=SENTINEL;const auto queries=object->m_pIntactModelCom->queries.size();
     require(!runtime.Try_PickMovementSurface(ORIGIN,DOWN,100,out)&&equal(out,SENTINEL)&&queries==object->m_pIntactModelCom->queries.size(),"replacement scene rejects original level Deploy");
     CGameInstance::Get().replaced=false;}
    {CDeployPropRuntime runtime;auto object=makeFloor(4);runtime.m_Entries={{object}};
     const float nan=std::numeric_limits<float>::quiet_NaN(),inf=std::numeric_limits<float>::infinity();
     for(const float3_t direction:std::vector<float3_t>{{0,0,0},{0,1.e-10f,0},{nan,-1,0},{0,-inf,0},{inf,0,0}}){
         float3_t out=SENTINEL;require(!runtime.Try_PickMovementSurface(ORIGIN,direction,100,out)&&equal(out,SENTINEL),"invalid ray direction preserves caller output");}
     for(const float3_t origin:std::vector<float3_t>{{nan,10,2},{1,inf,2},{1,10,nan}}){
         float3_t out=SENTINEL;require(!runtime.Try_PickMovementSurface(origin,DOWN,100,out)&&equal(out,SENTINEL),"non-finite ray origin rejected");}
     for(const float limit:{-1.f,nan,inf}){float3_t out=SENTINEL;
         require(!runtime.Try_PickMovementSurface(ORIGIN,DOWN,limit,out)&&equal(out,SENTINEL),"invalid runtime distance limit rejected");}}
'''
    cpp += '\n'.join(f'composition<Level_{level}>();' for level in lambdas)
    cpp += r'''
    std::cout<<"{\"checks\":"<<checks<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
'''
    path = args.out / 'probe.cpp'
    path.write_text(cpp, encoding='utf-8')
    vswhere = Path('C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe')
    install = subprocess.check_output([str(vswhere), '-latest', '-prerelease', '-products', '*',
        '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property',
        'installationPath'], text=True).strip()
    command = args.out / 'compile.cmd'
    flags = '/Od /MDd /RTC1 /D_DEBUG' if args.configuration == 'Debug' else '/O2 /MD /DNDEBUG'
    command.write_text(f'@echo off\ncall "{install}/VC/Auxiliary/Build/vcvars64.bat" >nul\n'
        f'cl /nologo /std:c++20 /EHsc {flags} "{path}" /Fe:"{args.out}/probe.exe" '
        f'/Fo:"{args.out}/probe.obj"\n', encoding='utf-8')
    compiled = subprocess.run(['cmd', '/c', str(command)], capture_output=True, text=True,
        encoding='utf-8', errors='replace')
    (args.out / 'compile.log').write_text(compiled.stdout + compiled.stderr, encoding='utf-8')
    if compiled.returncode:
        raise RuntimeError(f'Compile failed: {args.out / "compile.log"}')
    result = json.loads(subprocess.check_output([str(args.out / 'probe.exe')], text=True))
    result.update(configuration=args.configuration,
        methodsSha256=hashlib.sha256(methods.encode()).hexdigest(),
        boundLevels=list(lambdas),
        resolverSha256={level: hashlib.sha256(code.encode()).hexdigest() for level, code in lambdas.items()},
        scope='Production Deploy carrier/runtime/resolver; deterministic CModel geometry seam; no Client/GPU/Server execution')
    (args.out / 'result.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
