"""Exercise production deferred-move dispatch with deterministic renderer/input stubs.

No Client, window, UI input or network is started. Dispatch and goal admission
are extracted unchanged; only renderer readiness and typed-send outcomes are stubbed.
"""
from pathlib import Path
import argparse
import json
import subprocess
from test_async_picking import method

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=Path, default=ROOT / 'out/AsyncMoveDispatchRegression')
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'Client/Private/PlayerController.cpp').read_text(encoding='utf-8-sig')
    methods = '\n'.join(method(source, signature) for signature in (
        'void Client::CPlayerController::Cancel_MovePicking(',
        'void Client::CPlayerController::Update_MovePicking(',
        'bool_t Client::CPlayerController::Should_SendMoveGoal('))
    cpp = r"""
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using bool_t=bool; using f32_t=float; using HRESULT=long;
using std::shared_ptr;
constexpr HRESULT S_OK=0, S_FALSE=1, E_ABORT=-1, E_FAIL=-2;
struct float3_t {float x{},y{},z{};};
struct float4_t {float x{},y{},z{},w{};};
enum class STATE { POSITION };
inline float XMVectorGetX(float4_t v){return v.x;}
inline float XMVectorGetY(float4_t v){return v.y;}
inline float XMVectorGetZ(float4_t v){return v.z;}
constexpr std::chrono::milliseconds MOVE_GOAL_RESEND_INTERVAL{50};
constexpr float MOVE_GOAL_DEADZONE_RADIUS=.5f, MOVE_GOAL_RESEND_EPSILON=.25f;
constexpr float SHIP_WATER_BELOW_ROOT_M=.15f;
struct CTransform {float4_t position{0,2,0,1}; float4_t Get_State(STATE) const{return position;}};
struct CCharacter {shared_ptr<CTransform> transform=std::make_shared<CTransform>();
    shared_ptr<CTransform> Get_Transform()const{return transform;}};
struct CGameInstance {
    static CGameInstance& Get(){static CGameInstance game;return game;}
    uint64_t next{}, pending{}; unsigned requests{},polls{},cancels{};
    bool requestSucceeds=true; HRESULT ready=S_FALSE; float4_t surface{8,4,9,1};
    uint64_t Request_Picking(){++requests;return pending=requestSucceeds?++next:0;}
    HRESULT Poll_Picking(uint64_t id,float4_t& out){++polls;
        if(id!=pending)return E_ABORT;if(ready==S_OK)out=surface;return ready;}
    void Cancel_Picking(uint64_t id){++cancels;if(pending==id)pending=0;}
};
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
    uint64_t m_iMovePickRequest=0;
    uint32_t m_iMovePickActionSequence=0,m_iMovePickMoveSequence=0;
    uint32_t m_iNextActionSequence=1,m_iNextMoveSequence=1;
    std::chrono::steady_clock::time_point m_MovePickRequestedAt{},m_LastMoveGoalSentAt{};
    bool m_bMovePickHasFallback=false,m_bMovePickFreshPress=false,planeAvailable=true;
    float3_t m_MovePickFallback{},m_LastSentMoveGoal{},plane{30,2,40};
    void Cancel_MovePicking();
    void Update_MovePicking(bool_t,bool_t,bool_t,const shared_ptr<CCharacter>&);
    bool_t Should_SendMoveGoal(bool_t,f32_t,f32_t,const float3_t&)const;
    bool Try_PickGroundPlane(float y,float3_t& goal){goal=plane;goal.y=y;return planeAvailable;}
    bool Request_MoveToPointResolved(const float3_t& goal,bool effect,const float3_t* exact){
        sent.push_back({goal,effect,exact!=nullptr});m_LastSentMoveGoal=goal;
        m_LastMoveGoalSentAt=std::chrono::steady_clock::now();++m_iNextMoveSequence;return true;}
};
}
""" + methods + r"""
unsigned checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
struct Fixture {
    Client::CPlayerController c;shared_ptr<CCharacter> actor=std::make_shared<CCharacter>();
    Fixture(){CGameInstance::Get()={};CCombatHUDViewModel::Get().player={};}
    CGameInstance& gpu(){return CGameInstance::Get();}
    void click(){c.Update_MovePicking(true,true,true,actor);}
    void poll(bool held=false){c.Update_MovePicking(true,held,false,actor);}
};
int main(){try{
    {Fixture f;f.click();require(f.gpu().requests==1&&f.c.sent.empty(),"request never sends unresolved point");
     f.poll();require(f.c.sent.empty()&&f.gpu().requests==1,"pending release preserves occurrence");
     f.c.plane={99,99,99};f.gpu().ready=S_OK;f.poll();
     require(f.c.sent.size()==1&&f.c.sent[0].goal.x==8&&f.c.sent[0].effect&&f.c.sent[0].exact,"released click commits exact captured point once");
     f.poll();require(f.c.sent.size()==1,"consumed click is one-shot");}
    {Fixture f;f.click();const auto old=f.c.m_iMovePickRequest;f.click();
     require(f.gpu().cancels==1&&f.c.m_iMovePickRequest!=old,"new press replaces older sample");
     f.gpu().surface={17,3,18,1};f.gpu().ready=S_OK;f.poll();
     require(f.c.sent.size()==1&&f.c.sent[0].goal.x==17,"only newest press is submitted");}
    {Fixture f;f.click();f.gpu().ready=S_OK;f.c.Update_MovePicking(false,false,false,f.actor);
     require(!f.c.m_iMovePickRequest&&f.c.sent.empty(),"UI/dead/capture/focus-disabled rejects ready old sample");}
    {Fixture f;f.click();f.gpu().ready=S_OK;f.actor->transform.reset();f.poll();
     require(!f.c.m_iMovePickRequest&&f.c.sent.empty(),"missing character transform cancels");}
    {Fixture f;f.click();++f.c.m_iNextActionSequence;f.gpu().ready=S_OK;f.poll();
     require(f.c.sent.empty()&&!f.c.m_iMovePickRequest,"newer skill or interact wins over delayed move");}
    {Fixture f;f.click();++f.c.m_iNextMoveSequence;f.gpu().ready=S_OK;f.poll();
     require(f.c.sent.empty()&&!f.c.m_iMovePickRequest,"newer external movement wins over delayed move");}
    {Fixture f;f.click();f.c.m_MovePickRequestedAt-=std::chrono::milliseconds(351);f.gpu().ready=S_OK;f.poll();
     require(f.c.sent.empty()&&!f.c.m_iMovePickRequest,"timeout does not resurrect released click");}
    {Fixture f;f.click();f.c.plane={99,99,99};f.gpu().ready=E_FAIL;f.poll();
     require(f.c.sent.size()==1&&f.c.sent[0].goal.x==30&&!f.c.sent[0].effect&&!f.c.sent[0].exact,"fallback stays bound to original occurrence without marker");}
    {Fixture f;f.click();f.gpu().ready=E_ABORT;f.poll();require(f.c.sent.empty(),"superseded GPU occurrence never falls back");}
    {Fixture f;f.gpu().requestSucceeds=false;f.click();require(f.c.sent.size()==1&&!f.c.sent[0].effect,"unavailable renderer uses captured plane without marker");}
    {Fixture f;f.gpu().requestSucceeds=false;f.c.planeAvailable=false;f.click();require(f.c.sent.empty(),"no renderer or plane means no move");}
    {Fixture f;f.click();f.gpu().ready=S_OK;f.poll();f.poll(true);
     require(f.gpu().requests==1,"held resend throttle avoids needless readback");
     f.c.m_LastMoveGoalSentAt-=std::chrono::milliseconds(51);f.poll(true);f.poll(true);
     require(f.c.sent.size()==1,"unchanged held goal is not resent");
     f.poll(true);f.gpu().surface={11,4,12,1};f.poll(true);
     require(f.c.sent.size()==2&&!f.c.sent.back().effect,"changed held goal resends without fresh-press marker");}
    {Fixture f;f.c.m_LastMoveGoalSentAt=std::chrono::steady_clock::now();f.click();f.gpu().surface={0,2,0,1};f.gpu().ready=S_OK;f.poll();
     require(f.c.sent.size()==1&&f.c.sent[0].effect,"fresh press keeps former immediate admission semantics");}
    {Fixture f;CCombatHUDViewModel::Get().player.iVehicleId=1;f.click();
     require(f.gpu().requests==0&&f.c.sent.size()==1&&f.c.sent[0].effect,"ship keeps direct plane path");
     require(f.c.sent[0].goal.y==2-SHIP_WATER_BELOW_ROOT_M,"ship water height unchanged");}
    {Fixture f;f.click();f.c.Cancel_MovePicking();f.gpu().ready=S_OK;f.poll();
     require(f.c.sent.empty(),"explicit rebind/sink cancel discards deferred reply");}
    std::cout<<"{\"checks\":"<<checks<<",\"source\":\"production Update_MovePicking, Cancel_MovePicking, Should_SendMoveGoal\"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
"""
    path=args.out/'probe.cpp';path.write_text(cpp,encoding='utf-8')
    vswhere=Path('C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe')
    install=subprocess.check_output([str(vswhere),'-latest','-prerelease','-products','*',
        '-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip()
    command=args.out/'compile.cmd'
    command.write_text(f'@echo off\ncall "{install}/VC/Auxiliary/Build/vcvars64.bat" >nul\n'
        f'cl /nologo /std:c++20 /EHsc /O2 /MD "{path}" /Fe:"{args.out}/probe.exe" /Fo:"{args.out}/probe.obj"\n',encoding='utf-8')
    compiled=subprocess.run(['cmd','/c',str(command)],capture_output=True,text=True,encoding='utf-8',errors='replace')
    (args.out/'compile.log').write_text(compiled.stdout+compiled.stderr,encoding='utf-8')
    if compiled.returncode:raise RuntimeError(f'Compile failed: {args.out / "compile.log"}')
    result=subprocess.check_output([str(args.out/'probe.exe')],text=True)
    json.loads(result);(args.out/'result.json').write_text(result,encoding='utf-8');print(result.strip())

if __name__=='__main__':main()
