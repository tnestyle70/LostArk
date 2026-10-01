"""Exercise production CPU move dispatch without a Client, UI, GPU or socket.

The actual dispatch/admission functions are compiled unchanged. Only camera input,
CPU surface resolution and typed-send results are substituted. No GPU API stub is
provided, so reintroducing a GPU dependency also fails this focused regression.
"""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
from test_async_picking import method

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=Path, default=ROOT / 'out/CpuMoveDispatchRegression')
    parser.add_argument('--configuration', choices=['Debug', 'Release'], default='Release')
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'Client/Private/PlayerController.cpp').read_text(encoding='utf-8-sig')
    methods = '\n'.join(method(source, signature) for signature in (
        'void Client::CPlayerController::Update_MovePicking(',
        'bool_t Client::CPlayerController::Should_SendMoveGoal('))
    cpp = r'''
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
using bool_t=bool; using f32_t=float; using std::shared_ptr;
struct float3_t {float x{},y{},z{};};
struct float4_t {float x{},y{},z{},w{};};
using vector_t=float4_t;
enum class STATE { POSITION };
inline float XMVectorGetX(float4_t v){return v.x;}
inline float XMVectorGetY(float4_t v){return v.y;}
inline float XMVectorGetZ(float4_t v){return v.z;}
inline void XMStoreFloat3(float3_t* out,float4_t v){*out={v.x,v.y,v.z};}
constexpr std::chrono::milliseconds MOVE_GOAL_RESEND_INTERVAL{50};
constexpr float MOVE_GOAL_DEADZONE_RADIUS=.5f, MOVE_GOAL_RESEND_EPSILON=.25f;
constexpr float SHIP_WATER_BELOW_ROOT_M=.15f;
struct CTransform {float4_t position{0,2,0,1}; float4_t Get_State(STATE) const{return position;}};
struct CCharacter {shared_ptr<CTransform> transform=std::make_shared<CTransform>();
    shared_ptr<CTransform> Get_Transform()const{return transform;}};
struct CCombatHUDViewModel {
    struct Player {unsigned iVehicleId=0;} player;
    static CCombatHUDViewModel& Get(){static CCombatHUDViewModel hud;return hud;}
    const Player& Get_Player()const{return player;}
};
struct VEHICLE_ACTOR_ENTRY {bool isShip=true;};
struct CActorCatalog {static const VEHICLE_ACTOR_ENTRY* Find_Vehicle(unsigned id){
    static VEHICLE_ACTOR_ENTRY ship;return id==1?&ship:nullptr;}};
namespace Client {
class CPlayerController {
public:
    struct SENT {float3_t goal;bool effect,exact;}; std::vector<SENT> sent;
    std::function<bool(const float3_t&,const float3_t&,float3_t&)> m_MovementSurfaceResolver;
    std::chrono::steady_clock::time_point m_LastMoveGoalSentAt{};
    std::chrono::steady_clock::time_point m_LastMoveSurfaceQueryAt{};
    bool rayAvailable=true,planeAvailable=true,sendSucceeds=true;
    float3_t m_LastSentMoveGoal{},plane{30,2,40};
    float4_t origin{1,10,2,1},direction{0,-1,0,0};
    void Update_MovePicking(bool_t,bool_t,bool_t,const shared_ptr<CCharacter>&);
    bool_t Should_SendMoveGoal(bool_t,f32_t,f32_t,const float3_t&)const;
    bool Try_PickWorldRay(float4_t& o,float4_t& d){o=origin;d=direction;return rayAvailable;}
    bool Try_PickGroundPlane(float y,float3_t& goal){goal=plane;goal.y=y;return planeAvailable;}
    bool Request_MoveToPointResolved(const float3_t& goal,bool effect,const float3_t* exact){
        if(!sendSucceeds)return false;
        sent.push_back({goal,effect,exact!=nullptr});m_LastSentMoveGoal=goal;
        m_LastMoveGoalSentAt=std::chrono::steady_clock::now();return true;}
};
}
''' + methods + r'''
unsigned checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
struct Fixture {
    Client::CPlayerController c;shared_ptr<CCharacter> actor=std::make_shared<CCharacter>();
    unsigned queries{};bool hit=true;float3_t surface{8,4,9},queriedOrigin{},queriedDirection{};
    Fixture(){CCombatHUDViewModel::Get().player={};
        c.m_MovementSurfaceResolver=[this](const float3_t& o,const float3_t& d,float3_t& out){
            ++queries;queriedOrigin=o;queriedDirection=d;out=surface;return hit;};}
    void click(){c.Update_MovePicking(true,true,true,actor);}
    void hold(){c.Update_MovePicking(true,true,false,actor);}
    void release(){c.Update_MovePicking(true,false,false,actor);}
    void expire(){c.m_LastMoveGoalSentAt-=std::chrono::milliseconds(51);
        c.m_LastMoveSurfaceQueryAt-=std::chrono::milliseconds(51);}
};
int main(){try{
    {Fixture f;f.click();require(f.queries==1&&f.c.sent.size()==1&&f.c.sent[0].goal.x==8&&
        f.c.sent[0].effect&&f.c.sent[0].exact,"fresh click resolves and submits in the same input frame");
     require(f.queriedOrigin.x==1&&f.queriedOrigin.y==10&&f.queriedDirection.y==-1,"resolver receives this occurrence's camera ray");
     f.release();f.release();require(f.queries==1&&f.c.sent.size()==1,"release has no delayed command to resurrect");
     f.surface={17,3,18};f.click();require(f.c.sent.size()==2&&f.c.sent.back().goal.x==17,"new press immediately replaces the goal even within 50ms");}
    {Fixture f;f.c.Update_MovePicking(false,true,true,f.actor);require(!f.queries&&f.c.sent.empty(),"disabled input does no query or command");}
    {Fixture f;f.actor->transform.reset();f.click();require(!f.queries&&f.c.sent.empty(),"missing transform does no query");}
    {Fixture f;f.actor.reset();f.click();require(!f.queries&&f.c.sent.empty(),"missing character does no query");}
    {Fixture f;f.c.m_MovementSurfaceResolver={};f.click();require(f.c.sent.empty(),"missing map resolver cannot fall back to a GPU or invented plane");}
    {Fixture f;f.c.rayAvailable=false;f.click();require(!f.queries&&f.c.sent.empty(),"invalid viewport ray never queries a surface");}
    {Fixture f;f.click();const auto old=f.c.m_LastSentMoveGoal;f.hit=false;f.click();
     require(f.c.sent.size()==1&&f.c.m_LastSentMoveGoal.x==old.x,"surface miss preserves the previous submitted move");
     f.hit=true;f.surface={20,3,20};f.release();require(f.c.sent.size()==1,"a later surface cannot submit a released failed click");}
    {Fixture f;f.surface.x=std::numeric_limits<float>::quiet_NaN();f.click();require(f.c.sent.empty(),"non-finite surface is rejected");}
    {Fixture f;f.c.sendSucceeds=false;f.click();require(f.c.sent.empty()&&f.c.m_LastSentMoveGoal.x==0,"typed-send failure preserves command state");
     f.c.sendSucceeds=true;f.hold();require(f.queries==1,"failed send still throttles repeated held queries");
     f.expire();f.hold();require(f.c.sent.size()==1&&!f.c.sent[0].effect,"held retry is allowed without replaying a fresh-click effect");}
    {Fixture f;f.click();f.hold();require(f.queries==1,"held resend throttle avoids a CPU geometry query");
     f.expire();f.hold();require(f.c.sent.size()==1,"unchanged held goal is not resent");
     const auto queried=f.queries;f.hold();f.hold();require(f.queries==queried,"unchanged goal still throttles CPU queries independently of the last send");
     f.surface={11,4,12};f.expire();f.hold();require(f.c.sent.size()==2&&!f.c.sent.back().effect,"changed held goal is submitted in this frame without a press marker");}
    {Fixture f;f.hit=false;f.click();f.hold();f.hold();require(f.queries==1,"surface miss cannot trigger a full search on every held frame");
     f.expire();f.hit=true;f.hold();require(f.queries==2&&f.c.sent.size()==1,"held miss retries after the regular interval");
     f.click();require(f.queries==3&&f.c.sent.size()==2,"fresh press bypasses the held query interval");}
    {Fixture f;f.click();f.expire();f.surface={.1f,2,.1f};f.hold();require(f.c.sent.size()==1,"held arrival deadzone is retained");
     f.click();require(f.c.sent.size()==2&&f.c.sent.back().effect,"fresh press retains explicit near-goal admission");}
    {Fixture f;CCombatHUDViewModel::Get().player.iVehicleId=1;f.click();
     require(!f.queries&&f.c.sent.size()==1&&f.c.sent[0].effect,"ship retains immediate CPU water plane");
     require(f.c.sent[0].goal.y==2-SHIP_WATER_BELOW_ROOT_M,"ship water height is preserved");}
    {Fixture f;CCombatHUDViewModel::Get().player.iVehicleId=1;f.c.planeAvailable=false;f.click();
     require(!f.queries&&f.c.sent.empty(),"ship plane failure preserves current movement");}
    std::cout<<"{\"checks\":"<<checks<<",\"source\":\"production Update_MovePicking and Should_SendMoveGoal\"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
'''
    path=args.out/'probe.cpp';path.write_text(cpp,encoding='utf-8')
    vswhere=Path('C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe')
    install=subprocess.check_output([str(vswhere),'-latest','-prerelease','-products','*',
        '-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip()
    command=args.out/'compile.cmd'
    flags='/Od /MDd /RTC1 /D_DEBUG' if args.configuration=='Debug' else '/O2 /MD /DNDEBUG'
    command.write_text(f'@echo off\ncall "{install}/VC/Auxiliary/Build/vcvars64.bat" >nul\n'
        f'cl /nologo /std:c++20 /EHsc {flags} "{path}" /Fe:"{args.out}/probe.exe" /Fo:"{args.out}/probe.obj"\n',encoding='utf-8')
    compiled=subprocess.run(['cmd','/c',str(command)],capture_output=True,text=True,encoding='utf-8',errors='replace')
    (args.out/'compile.log').write_text(compiled.stdout+compiled.stderr,encoding='utf-8')
    if compiled.returncode:raise RuntimeError(f'Compile failed: {args.out / "compile.log"}')
    result=json.loads(subprocess.check_output([str(args.out/'probe.exe')],text=True))
    result['configuration']=args.configuration
    result['methodsSha256']=hashlib.sha256(methods.encode()).hexdigest()
    result['boundLevels']=[]
    for level in ('Bern','ValtanArena','KakulSaydonArena','CharacterSelect','Development'):
        code=(ROOT/f'Client/Private/Level_{level}.cpp').read_text(encoding='utf-8-sig')
        assert 'Set_MovementSurfaceResolver' in code and 'm_MapRuntime.Try_PickMovementSurface' in code, level
        result['boundLevels'].append(level)
    (args.out/'result.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps(result))


if __name__=='__main__':main()
