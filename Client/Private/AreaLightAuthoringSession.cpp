#include "AreaLightAuthoringSession.h"
#include "LightResourceCatalog.h"
#include "ProjectDataRoot.h"
#include "MapAssetCatalog.h"
#include "DataJson.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace
{
std::string ReadMapLightSource(const std::filesystem::path& path)
{
    std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Cannot read map lights: "+path.string());
    std::ostringstream out;out<<in.rdbuf();if(in.bad())throw std::runtime_error("Cannot read complete map lights.");return out.str();
}
}
NS_BEGIN(Client)
bool_t CAreaLightAuthoringSession::Open(const std::string& areaId,std::string& status)
{
    if(areaId.empty() || !std::all_of(areaId.begin(),areaId.end(),[](char c){return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_';}))
    {status="Map light Area ID is invalid.";return false;}
    const auto path=CProjectDataRoot::Get()/L"Maps/Authoring"/std::filesystem::path(areaId)/std::filesystem::path(areaId+".maplights.json");
    try
    {
        const auto bytes=ReadMapLightSource(path);CMapLightDocument staged;
        if(!staged.Parse(bytes,areaId,status))return false;
        auto preview=std::make_shared<CMapLightPresentationRuntime>();
        if(!preview->Replace_Document(staged)){status=preview->Get_Status();return false;}
        m_Path=path;m_SourceBytes=bytes;m_Document=std::move(staged);m_Preview=std::move(preview);return true;
    }catch(const std::exception& e){status=e.what();return false;}
}
bool_t CAreaLightAuthoringSession::Reload(std::string& status)
{const auto area=Get_AreaId();return Open(area,status);}
bool_t CAreaLightAuthoringSession::Commit(std::vector<MAP_POINT_LIGHT_RECORD> rows,uint32_t next,std::string& status)
{
    CMapLightDocument staged=m_Document;
    if(!staged.Replace_Authored(rows,next,status))return false;
    if(!m_Preview || !m_Preview->Replace_Document(staged)){status="Map light preview is unavailable.";return false;}
    m_Document=std::move(staged);status="Map light draft updated.";return true;
}
bool_t CAreaLightAuthoringSession::Create(const std::string& name,LIGHT type,const float3_t& pos,std::string& outId,std::string& status)
{
    if(!Is_Open()||Is_ReadOnly()||m_Document.Get_NextLightOrdinal()>=UINT32_MAX-1){status="Map light source is not writable.";return false;}
    auto rows=m_Document.Get_Lights();MAP_POINT_LIGHT_RECORD r;
    r.lightId="light."+Get_AreaId()+"."+std::to_string(m_Document.Get_NextLightOrdinal());
    r.displayName=name;r.kind=type;r.position=pos;r.radiusMeters=16;r.falloffExponent=2;r.brightness=4;
    if(type==LIGHT::SPOT){r.rotationDegrees={90,0,0};r.innerConeDegrees=12;r.outerConeDegrees=26;}
    else if(type==LIGHT::DIRECTIONAL){r.position={0,0,0};r.rotationDegrees={90,0,0};r.radiusMeters=0;}
    rows.push_back(r);
    if(!Commit(std::move(rows),m_Document.Get_NextLightOrdinal()+1,status))return false;
    outId=r.lightId;return true;
}
bool_t CAreaLightAuthoringSession::Update(const MAP_POINT_LIGHT_RECORD& r,std::string& status)
{
    auto rows=m_Document.Get_Lights();
    const auto it=std::find_if(rows.begin(),rows.end(),[&r](const auto& row){return row.lightId==r.lightId;});
    if(it==rows.end()){status="Map light is missing.";return false;}*it=r;
    return Commit(std::move(rows),m_Document.Get_NextLightOrdinal(),status);
}
bool_t CAreaLightAuthoringSession::Delete(const std::string& id,std::string& status)
{
    try
    {
        DATA_JSON_VALUE composition;std::string error;
        const auto path=CProjectDataRoot::Resolve(L"KoukuSaydon/Gate1/KoukuSaydonComposition.json");
        if(!CDataJson::Parse(ReadMapLightSource(path),composition,error))
        {status="Cannot verify Composition references; map light preserved: "+error;return false;}
        const auto* resources=composition.Find("presentationResources");
        if(resources && !resources->Is_Array()){status="Composition resources are invalid; map light preserved.";return false;}
        if(resources)for(const auto& row:resources->Get_Array())
        {const auto* asset=row.Find("assetId");if(asset&&asset->Is_String()&&asset->Get_String()==id)
            {status="Map light is referenced by Composition; remove that reference before deleting.";return false;}}
    }catch(const std::exception& e){status=e.what();return false;}
    auto rows=m_Document.Get_Lights();const auto it=std::find_if(rows.begin(),rows.end(),[&id](const auto& row){return row.lightId==id;});
    if(it==rows.end()){status="Map light is missing.";return false;}rows.erase(it);
    return Commit(std::move(rows),m_Document.Get_NextLightOrdinal(),status);
}
bool_t CAreaLightAuthoringSession::Save_Authored(std::string& status)
{
    if(!Is_Open()||Is_ReadOnly()){status="Imported map lights are read-only.";return false;}
    auto temp=m_Path;temp+=L".lighting-"+std::to_wstring(GetCurrentProcessId())+L".tmp";
    try
    {
        const auto bytes=m_Document.Serialize();CMapLightDocument verified;
        if(!verified.Parse(bytes,Get_AreaId(),status))return false;
        if(ReadMapLightSource(m_Path)!=m_SourceBytes){status="Map light source changed on disk; draft preserved. Reload first.";return false;}
        {std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<bytes;if(!out)throw std::runtime_error("Map light staging write failed.");}
        if(!verified.Load(temp,Get_AreaId(),status))throw std::runtime_error(status);
        if(ReadMapLightSource(m_Path)!=m_SourceBytes)throw std::runtime_error("Map light source changed during save; draft preserved.");
        if(!MoveFileExW(temp.c_str(),m_Path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Map light atomic save failed.");
        m_SourceBytes=bytes;status="Map lights saved. Publish then Reload Runtime to apply.";return true;
    }catch(const std::exception& e){std::error_code ec;std::filesystem::remove(temp,ec);status=e.what();return false;}
}
bool_t CAreaLightAuthoringSession::Publish_Runtime(std::string& status) const
{
    if(!Is_Open()||Is_ReadOnly()){status="Map light source is not writable.";return false;}
    const std::wstring area(Get_AreaId().begin(),Get_AreaId().end());
    return CLightResourceCatalog::Run_Publisher(L"Tools/MapPipeline/Publish-MapAuthoring.ps1",L"-AreaId "+area+L" -Mode Publish",status);
}
NS_END
