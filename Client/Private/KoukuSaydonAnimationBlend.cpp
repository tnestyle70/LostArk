#include "KoukuSaydonAnimationBlend.h"
#include "DataJson.h"
#include <algorithm>
#include <cmath>
#include <set>

using namespace Client;
namespace
{
bool Shape(const DATA_JSON_VALUE& value, std::initializer_list<const char*> keys)
{
    if (!value.Is_Object() || value.Get_Object().size() != keys.size()) return false;
    return std::all_of(keys.begin(),keys.end(),[&](const char* key){return value.Find(key)!=nullptr;});
}
bool Text(const DATA_JSON_VALUE& value,const char* key,std::string& out)
{
    const auto* item=value.Find(key);
    if(!item||!item->Is_String()||item->Get_String().empty()||item->Get_String().size()>256u)return false;
    out=item->Get_String();return true;
}
bool UInt(const DATA_JSON_VALUE& value,const char* key,uint32_t minimum,uint32_t maximum,uint32_t& out)
{
    const auto* item=value.Find(key);
    if(!item||!item->Is_Number()||!std::isfinite(item->Get_Number())||std::floor(item->Get_Number())!=item->Get_Number()||
        item->Get_Number()<minimum||item->Get_Number()>maximum)return false;
    out=uint32_t(item->Get_Number());return true;
}
bool Source(const DATA_JSON_VALUE& value,KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& row)
{
    if(!Shape(value,{"occurrenceId","runtimeClip","startMs","poseStartMs","sourceStartMs","sourceEndMs","playMs","playRate","endPolicy"})||
        !Text(value,"occurrenceId",row.strOccurrenceId)||!Text(value,"runtimeClip",row.strRuntimeClip)||
        !Text(value,"endPolicy",row.strEndPolicy)||!UInt(value,"startMs",0u,600000u,row.iStartOffsetMs)||
        !UInt(value,"poseStartMs",0u,row.iStartOffsetMs,row.iPoseStartMs)||!UInt(value,"sourceStartMs",0u,600000u,row.iSourceStartMs)||
        !UInt(value,"sourceEndMs",0u,600000u,row.iSourceEndMs)||!UInt(value,"playMs",1u,600000u,row.iPlayMs)||
        uint64_t(row.iStartOffsetMs)+row.iPlayMs>600000u||
        (row.iSourceEndMs&&row.iSourceEndMs<=row.iSourceStartMs)||
        (row.strEndPolicy!="EXACT"&&row.strEndPolicy!="HOLD_LAST_POSE"&&row.strEndPolicy!="LOOP_TO_WINDOW"))return false;
    const auto* rate=value.Find("playRate");
    if(!rate||!rate->Is_Number()||!std::isfinite(rate->Get_Number())||rate->Get_Number()<.01||rate->Get_Number()>16.0)return false;
    row.fPlayRate=float(rate->Get_Number());return true;
}
bool SampleSource(const Engine::CModel& model,const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& row,
    const double clock,uint32_t& index,float& ticks)
{
    index=UINT32_MAX;
    for(uint32_t i=0u;i<model.Get_NumAnimations();++i)
        if(const char* name=model.Get_AnimationName(i);name&&row.strRuntimeClip==name)
        {if(index!=UINT32_MAX)return false;index=i;}
    float cursor=0.f,duration=0.f;
    if(index==UINT32_MAX||!model.Get_AnimationProgress(index,cursor,duration))return false;
    const float tps=model.Get_AnimationTickPerSecond(index);
    if(!std::isfinite(tps)||tps<=0.f||!std::isfinite(duration)||duration<=0.f)return false;
    double sourceMs=0.0;
    if(!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(row.iSourceStartMs,row.iSourceEndMs,
        (std::clamp)(clock-row.iStartOffsetMs,0.0,double(row.iPlayMs)),row.fPlayRate,
        duration*1000.0/tps,row.strEndPolicy=="LOOP_TO_WINDOW",sourceMs))return false;
    ticks=(std::min)(duration,float(sourceMs*tps*.001));return true;
}
}

bool CKoukuSaydonAnimationBlend::Read_ProductWindows(const DATA_JSON_VALUE& value,
    std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW>& output,std::string& status)
{
    if(!value.Is_Array()||value.Get_Array().size()>4096u){status="Animation blend windows require a bounded array.";return false;}
    std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> staged;std::set<std::string> ids;
    for(const auto& item:value.Get_Array())
    {
        KOUKU_SAYDON_ANIMATION_BLEND_WINDOW window;
        if(!Shape(item,{"logicOccurrenceId","startMs","durationMs","source","target"})||
            !Text(item,"logicOccurrenceId",window.strLogicOccurrenceId)||!ids.insert(window.strLogicOccurrenceId).second||
            !UInt(item,"startMs",0u,600000u,window.iStartMs)||!UInt(item,"durationMs",1u,1000u,window.iDurationMs)||
            uint64_t(window.iStartMs)+window.iDurationMs>600000u||
            !Source(*item.Find("source"),window.Source)||!Source(*item.Find("target"),window.Target)||
            window.Source.strOccurrenceId==window.Target.strOccurrenceId||
            window.Source.iPoseStartMs>=window.Target.iPoseStartMs||window.iStartMs<window.Source.iPoseStartMs||
            window.iStartMs>window.Target.iPoseStartMs||uint64_t(window.iStartMs)+window.iDurationMs<window.Target.iPoseStartMs||
            (!staged.empty()&&uint64_t(staged.back().iStartMs)+staged.back().iDurationMs>window.iStartMs))
        {status="Product animation blend identity, source range, boundary or ordering is invalid.";return false;}
        staged.push_back(std::move(window));
    }
    output=std::move(staged);status.clear();return true;
}

bool CKoukuSaydonAnimationBlend::Validate_ModelWindows(const Engine::CModel& model,
    std::span<const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> windows,std::string& status)
{
    for(const auto& window:windows)
    {
        if(!window.iDurationMs||window.iDurationMs>1000u){status="Animation blend must remain within 1..1000 ms.";return false;}
        for(const auto* row:{&window.Source,&window.Target})
        {
            uint32_t index;float ticks;
            if(!SampleSource(model,*row,window.iStartMs,index,ticks)||
                !SampleSource(model,*row,double(window.iStartMs)+window.iDurationMs,index,ticks))
            {status="Animation blend source clip/range is unavailable: "+row->strRuntimeClip;return false;}
        }
    }
    status.clear();return true;
}

bool CKoukuSaydonAnimationBlend::Sample_Pose(const Engine::CModel& model,
    std::span<const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> windows,const double patternMs,
    Engine::CModel::ANIMATION_TRANSITION_POSE& output,bool& active,std::string& status)
{
    if(!std::isfinite(patternMs)){status="Animation blend clock is not finite.";return false;}
    const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW* selected=nullptr;
    for(const auto& window:windows)
        if(patternMs>=window.iStartMs&&patternMs<double(window.iStartMs)+window.iDurationMs)
        {if(selected){status="Animation blend windows overlap.";return false;}selected=&window;}
    if(!selected){active=false;return true;}
    Engine::CModel::ANIMATION_TRANSITION_POSE pose;
    if(!SampleSource(model,selected->Source,patternMs,pose.sourceIndex,pose.sourceTicks)||
        !SampleSource(model,selected->Target,patternMs,pose.targetIndex,pose.targetTicks))
    {status="Animation blend cannot sample both source windows at the Pattern clock.";return false;}
    pose.durationSeconds=selected->iDurationMs*.001f;
    pose.elapsedSeconds=float((patternMs-selected->iStartMs)*.001);
    pose.playRate=selected->Target.fPlayRate;
    output=pose;active=true;status.clear();return true;
}
