"""Regress Character time consumption and the actual installed navigation follower.

Run after the matching Product build, for example:
  python Tools/MovementRegression/test_local_move_following.py --configuration Release

The production Character update method is compiled unchanged with its prediction
header. Navigation, Transform and NavPathFollower are real Engine.dll calls. Only
the Character shell and wall clock are substituted; no Client, window, graphics
device, network, or product build is started. Outputs and frozen binaries stay in
out, including source/binary hashes and per-case numerical results.
"""
from pathlib import Path
import argparse
import hashlib
import json
import shutil
import struct
import subprocess

from test_async_picking import method

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--configuration', choices=['Debug', 'Release'], default='Release')
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()
    out = (args.out or ROOT / 'out/LocalMoveFollowingRegression' / args.configuration).resolve()
    out.mkdir(parents=True, exist_ok=True)
    installed = ROOT / 'Client/Bin' / args.configuration
    engine_build = ROOT / 'Engine/Bin' / args.configuration
    tracked = [ROOT / p for p in (
        'Client/Private/Character.cpp', 'Client/Public/LocalMovePrediction.h',
        'Engine/Public/NavPathFollower.h', 'Engine/Private/NavPathFollower.cpp',
        'Engine/Public/Navigation.h', 'Engine/Private/Navigation.cpp',
        'Engine/Public/Transform.h', 'Engine/Private/Transform.cpp')]
    tracked += [installed / 'Engine.dll', engine_build / 'Engine.dll', engine_build / 'Engine.lib']
    before = {str(p): sha(p) for p in tracked}
    if before[str(installed / 'Engine.dll')] != before[str(engine_build / 'Engine.dll')]:
        raise RuntimeError('Installed Engine.dll differs from the Engine build. Finish Product deployment first.')
    dependencies = []
    for name in ('Engine.dll', 'fmod.dll', 'PhysX_64.dll', 'PhysXCommon_64.dll',
                 'PhysXFoundation_64.dll', 'assimp-vc143-mtd.dll' if args.configuration == 'Debug'
                 else 'assimp-vc143-mt.dll'):
        src = installed / name
        shutil.copyfile(src, out / name)
        dependencies.append({'source': str(src), 'sha256': sha(out / name)})
    shutil.copyfile(engine_build / 'Engine.lib', out / 'Engine.lib')
    shutil.copyfile(ROOT / 'Client/Public/LocalMovePrediction.h', out / 'LocalMovePrediction.h')
    if sha(out / 'Engine.dll') != before[str(installed / 'Engine.dll')] or \
            sha(out / 'Engine.lib') != before[str(engine_build / 'Engine.lib')]:
        raise RuntimeError('Engine changed while freezing probe inputs; retry after the build finishes.')
    for name, obstacle in [('open.navgrid', False), ('corner.navgrid', True)]:
        walk = bytearray([1] * (32 * 32))
        if obstacle:
            walk[2] = walk[34] = 0
        (out / name).write_bytes(struct.pack('<IIfff', 32, 32, 1., 0., 0.) + walk + bytes(4 * 32 * 32))
    source = (ROOT / 'Client/Private/Character.cpp').read_text(encoding='utf-8-sig')
    update = method(source, 'bool_t CCharacter::Update_LocalMovePrediction(')
    cpp = r'''
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cmath>
#include <string>
#include "Navigation.h"
#include "NavPathFollower.h"
#include "Transform.h"
#include "LocalMovePrediction.h"
static double testNow=0;
static double LocalMoveClockSeconds(){return testNow;}
namespace Client {
class CCharacter {
public:
    bool m_isLocalMovePredictionEnabled=true, m_isLocallyControlled=true, moving=true;
    std::shared_ptr<CTransform> m_pTransformCom=CTransform::Create(nullptr,nullptr);
    std::shared_ptr<CNavigation> m_pNavigationCom;
    CNavPathFollower m_PathFollower;
    CLocalMovePrediction m_LocalMovePrediction;
    CLocalMovePrediction::Pose Get_LocalMovePose() const {
        auto v=m_pTransformCom->Get_State(STATE::POSITION);
        return {{XMVectorGetX(v),XMVectorGetY(v),XMVectorGetZ(v)},90,moving};
    }
    void Apply_LocalMovePose(const CLocalMovePrediction::Pose& pose,float) {
        m_pTransformCom->Set_State(STATE::POSITION,XMVectorSet(pose.position.x,pose.position.y,pose.position.z,1));
        moving=pose.isMoving;
    }
    bool_t Update_LocalMovePrediction(f32_t);
};
''' + update + r'''
}
using P=Client::CLocalMovePrediction;
unsigned checks=0;
static void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
static double distance(P::Pose a,P::Pose b){return std::hypot(a.position.x-b.position.x,a.position.z-b.position.z);}
static void initialize(Client::CCharacter& c,std::shared_ptr<CNavigation> nav,P::Vec3 position,
    P::Vec3 waypoint,float speed) {
    c.m_pNavigationCom=nav;
    c.m_pTransformCom->Set_State(STATE::POSITION,XMVectorSet(position.x,position.y,position.z,1));
    P::Snapshot s;s.serverTick=100;s.moveSpeed=speed;s.canPredictMove=true;s.hasMoveGoal=true;
    s.position=position;s.nextWaypoint=waypoint;
    require(c.m_LocalMovePrediction.ApplySnapshot(s,0,c.Get_LocalMovePose())==P::SnapshotDisposition::RESET,"initial snapshot");
}
static void submit(Client::CCharacter& c,P::Vec3 goal,unsigned sequence) {
    if(!c.m_LocalMovePrediction.IsSameMoveGoal(goal)) {
        CNavPathFollower staged;
        require(staged.Request_Path(c.m_pNavigationCom,c.m_pTransformCom->Get_State(STATE::POSITION),
            XMVectorSet(goal.x,goal.y,goal.z,1))==PATH_RESULT_CODE::SUCCESS,"stage actual navigation path");
        require(c.m_LocalMovePrediction.SubmitMove(sequence,.001,c.Get_LocalMovePose(),&goal),"submit prediction");
        c.m_PathFollower=std::move(staged);
    } else require(c.m_LocalMovePrediction.SubmitMove(sequence,.001,c.Get_LocalMovePose(),&goal),"same-goal sequence");
}
static void clockCase(std::shared_ptr<CNavigation> nav,float dt,const char* mode) {
    Client::CCharacter c;P::Vec3 goal{10.5f,0,.5f};initialize(c,nav,{.5f,0,.5f},goal,2.95f);submit(c,goal,1);
    if(std::string(mode)=="retarget")goal.x=20.5f;
    if(std::string(mode)=="unacked_turn")goal={.5f,0,10.5f};
    submit(c,goal,2);const auto before=c.Get_LocalMovePose();testNow=dt;
    require(c.Update_LocalMovePrediction(dt),"actual Character update active");
    const double travel=distance(before,c.Get_LocalMovePose()),expected=2.95*dt;
    std::cout<<"{\"case\":\"clock\",\"mode\":\""<<mode<<"\",\"dt\":"<<dt<<",\"travel\":"<<travel<<",\"expected\":"<<expected<<"}\n";
    require(std::abs(travel-expected)<.00001,"Character discarded helper-authorized frame time");
    require(c.moving,"long goal unexpectedly ended");
}
static void cornerCase(std::shared_ptr<CNavigation> nav,float dt) {
    auto start=XMVectorSet(.5f,0,.5f,1),goal=XMVectorSet(4.5f,0,.5f,1);
    std::vector<float3_t> path;
    require(nav->Find_Path(start,goal,.6f,path)==PATH_RESULT_CODE::SUCCESS&&path.size()>2,"actual rounded obstacle path");
    const auto nearCorner=XMVectorSetW(XMLoadFloat3(&path[1])-
        XMVector3Normalize(XMLoadFloat3(&path[1])-XMLoadFloat3(&path[0]))*.019f,1);
    Client::CCharacter c;initialize(c,nav,{XMVectorGetX(nearCorner),0,XMVectorGetZ(nearCorner)},
        {path[1].x,0,path[1].z},2.95f);
    require(c.m_PathFollower.Request_Path(nav,start,goal)==PATH_RESULT_CODE::SUCCESS,"corner path stage");
    const P::Vec3 target{4.5f,0,.5f};
    require(c.m_LocalMovePrediction.SubmitMove(1,.001,c.Get_LocalMovePose(),&target),"corner prediction");
    const auto before=c.Get_LocalMovePose();testNow=dt;
    require(c.Update_LocalMovePrediction(dt),"corner Character update");
    const double travel=distance(before,c.Get_LocalMovePose()),budget=2.95*dt;
    std::cout<<"{\"case\":\"corner19mm\",\"dt\":"<<dt<<",\"travel\":"<<travel<<",\"budget\":"<<budget<<"}\n";
    require(travel>0&&travel<=budget+.00001,"short waypoint added free distance before the next segment");
}
static void arrivalCase(std::shared_ptr<CNavigation> nav,float dt,float speed) {
    Client::CCharacter c;P::Vec3 goal{.519f,0,.5f};initialize(c,nav,{.5f,0,.5f},goal,speed);submit(c,goal,1);
    const auto before=c.Get_LocalMovePose();testNow=dt;require(c.Update_LocalMovePrediction(dt),"arrival Character update");
    const double travel=distance(before,c.Get_LocalMovePose()),expected=(std::min)(.019,double(speed*dt));
    std::cout<<"{\"case\":\"arrival19mm\",\"dt\":"<<dt<<",\"speed\":"<<speed<<",\"travel\":"<<travel<<",\"expected\":"<<expected<<",\"moving\":"<<c.moving<<"}\n";
    require(std::abs(travel-expected)<.00001,"short goal violated its horizontal movement budget");
    require(c.moving==(speed*dt<.019f),"follower ended before the displayed pose arrived");
}
int wmain(int argc,wchar_t** argv){try{
    Set_NonInteractiveErrorMode(true);require(argc==3,"two fixture paths required");std::cout<<std::setprecision(10);
    std::shared_ptr<CNavigation> open=CNavigation::Create_NavGrid(nullptr,nullptr,argv[1]);
    std::shared_ptr<CNavigation> corner=CNavigation::Create_NavGrid(nullptr,nullptr,argv[2]);
    require(bool(open)&&bool(corner),"headless nav fixture load");
    for(float dt:{1.f/8,1.f/20,1.f/40,1.f/60,.132401f})
        for(const char* mode:{"same_goal","retarget","unacked_turn"})clockCase(open,dt,mode);
    for(float dt:{1.f/60,.025f,.05f,.132401f}){cornerCase(corner,dt);arrivalCase(open,dt,2.95f);arrivalCase(open,dt,.5f);}
    std::cout<<"{\"checks\":"<<checks<<",\"passed\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
'''
    path = out / 'probe.cpp'
    path.write_text(cpp, encoding='utf-8')
    vswhere = Path('C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe')
    install = subprocess.check_output([str(vswhere), '-latest', '-prerelease', '-products', '*',
        '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'], text=True).strip()
    flags = '/Od /MDd /RTC1 /D_DEBUG' if args.configuration == 'Debug' else '/O2 /MD /DNDEBUG'
    command = out / 'compile.cmd'
    command.write_text(f'@echo off\ncall "{install}/VC/Auxiliary/Build/vcvars64.bat" >nul\n'
        f'if errorlevel 1 exit /b %errorlevel%\ncl /nologo /std:c++20 /EHsc /utf-8 {flags} '
        f'/DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /DNOMINMAX /I"{ROOT}/Engine/Public" '
        f'"{path}" "{out}/Engine.lib" /Fe:"{out}/probe.exe" /Fo:"{out}/probe.obj"\n', encoding='utf-8')
    compiled = subprocess.run(['cmd', '/c', str(command)], capture_output=True, text=True, encoding='utf-8', errors='replace')
    (out / 'compile.log').write_text(compiled.stdout + compiled.stderr, encoding='utf-8')
    result = {'configuration': args.configuration, 'sourceAndBinarySha256': before,
              'dependencies': dependencies, 'characterMethodSha256': hashlib.sha256(update.encode()).hexdigest(),
              'compileExitCode': compiled.returncode, 'runtimeExitCode': None}
    if compiled.returncode == 0:
        executed = subprocess.run([str(out / 'probe.exe'), str(out / 'open.navgrid'), str(out / 'corner.navgrid')],
                                  capture_output=True, text=True, encoding='utf-8', errors='replace')
        (out / 'results.jsonl').write_text(executed.stdout, encoding='utf-8')
        (out / 'stderr.log').write_text(executed.stderr, encoding='utf-8')
        result['runtimeExitCode'] = executed.returncode
        result['cases'] = [json.loads(row) for row in executed.stdout.splitlines() if row.strip()]
    result['inputsUnchanged'] = all(sha(p) == before[str(p)] for p in tracked)
    result['passed'] = result['compileExitCode'] == 0 and result['runtimeExitCode'] == 0 and result['inputsUnchanged']
    (out / 'result.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps({'passed': result['passed'], 'configuration': args.configuration, 'result': str(out / 'result.json')}))
    if not result['passed']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
