#include "GuideCatalog.h"
#include "Network/PacketMessages.h"
#include <Windows.h>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <set>
#include <string_view>
#include <variant>

namespace
{
 // Bounded JSON reader for the independently published Guide domain. Parsing and
 // validation fill a candidate; a failed load never replaces a live generation.
 struct Json
 {
  using Object = std::map<std::string, Json>; using Array = std::vector<Json>;
  std::variant<std::nullptr_t, bool, double, std::string, Array, Object> Value;
  const Json& at(const std::string& key) const { return std::get<Object>(Value).at(key); }
  const Json* find(const std::string& key) const { const auto& o=std::get<Object>(Value); auto i=o.find(key); return i==o.end()?nullptr:&i->second; }
  const Array& array() const { return std::get<Array>(Value); }
  std::string text() const { return std::get<std::string>(Value); }
  double number() const { return std::get<double>(Value); }
  bool boolean() const { return std::get<bool>(Value); }
 };
 class Parser
 {
  std::string_view Text; std::size_t Cursor=0, Nodes=0;
  [[noreturn]] void fail() const { throw std::runtime_error("malformed Guide JSON at byte "+std::to_string(Cursor)); }
  void space() { while(Cursor<Text.size() && (Text[Cursor]==' '||Text[Cursor]=='\r'||Text[Cursor]=='\n'||Text[Cursor]=='\t')) ++Cursor; }
  unsigned hex4() { unsigned x=0; for(int n=0;n<4;++n) { if(Cursor==Text.size()) fail(); char c=Text[Cursor++]; unsigned v=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:16; if(v==16) fail(); x=(x<<4)|v; } return x; }
  static void utf8(std::string& s,unsigned c) { if(c<0x80)s+=char(c); else if(c<0x800){s+=char(0xc0|(c>>6));s+=char(0x80|(c&63));} else if(c<0x10000){s+=char(0xe0|(c>>12));s+=char(0x80|((c>>6)&63));s+=char(0x80|(c&63));} else {s+=char(0xf0|(c>>18));s+=char(0x80|((c>>12)&63));s+=char(0x80|((c>>6)&63));s+=char(0x80|(c&63));} }
  std::string string()
  {
   if(Cursor==Text.size()||Text[Cursor++]!='"') fail(); std::string out;
   while(Cursor<Text.size()) { unsigned char c=Text[Cursor++]; if(c=='"')return out; if(c<32)fail(); if(c!='\\'){out+=char(c);continue;} if(Cursor==Text.size())fail();
    switch(Text[Cursor++]) {case '"':out+='"';break;case '\\':out+='\\';break;case '/':out+='/';break;case 'b':out+='\b';break;case 'f':out+='\f';break;case 'n':out+='\n';break;case 'r':out+='\r';break;case 't':out+='\t';break;case 'u': {unsigned v=hex4(); if(v>=0xd800&&v<=0xdbff){if(Text.substr(Cursor,2)!="\\u")fail();Cursor+=2;unsigned low=hex4();if(low<0xdc00||low>0xdfff)fail();v=0x10000+((v-0xd800)<<10)+low-0xdc00;}else if(v>=0xdc00&&v<=0xdfff)fail();utf8(out,v);break;}default:fail();}
   } fail();
  }
  Json value(unsigned depth)
  {
   if(depth>32||++Nodes>100000)fail(); space(); if(Cursor==Text.size())fail(); char c=Text[Cursor];
   if(c=='"')return Json{string()};
   if(c=='{'||c=='[') {++Cursor;space();Json::Object object;Json::Array list;char end=c=='{'?'}':']';if(Cursor<Text.size()&&Text[Cursor]==end){++Cursor;return c=='{'?Json{object}:Json{list};}
    for(;;){space();if(c=='{'){auto key=string();space();if(Cursor==Text.size()||Text[Cursor++]!=':')fail();if(!object.emplace(key,value(depth+1)).second)fail();}else list.push_back(value(depth+1));space();if(Cursor==Text.size())fail();char next=Text[Cursor++];if(next==end)break;if(next!=',')fail();}return c=='{'?Json{object}:Json{list};}
   for(auto literal:{std::string_view("true"),std::string_view("false"),std::string_view("null")})if(Text.substr(Cursor,literal.size())==literal){Cursor+=literal.size();if(literal=="null")return Json{nullptr};return Json{literal=="true"};}
   std::size_t start=Cursor;if(Text[Cursor]=='-')++Cursor;if(Cursor==Text.size())fail();if(Text[Cursor]=='0')++Cursor;else {if(Text[Cursor]<'1'||Text[Cursor]>'9')fail();while(Cursor<Text.size()&&Text[Cursor]>='0'&&Text[Cursor]<='9')++Cursor;}
   if(Cursor<Text.size()&&Text[Cursor]=='.'){++Cursor;auto begin=Cursor;while(Cursor<Text.size()&&Text[Cursor]>='0'&&Text[Cursor]<='9')++Cursor;if(begin==Cursor)fail();}
   if(Cursor<Text.size()&&(Text[Cursor]=='e'||Text[Cursor]=='E')){++Cursor;if(Cursor<Text.size()&&(Text[Cursor]=='+'||Text[Cursor]=='-'))++Cursor;auto begin=Cursor;while(Cursor<Text.size()&&Text[Cursor]>='0'&&Text[Cursor]<='9')++Cursor;if(begin==Cursor)fail();}
   double result=0;auto read=std::from_chars(Text.data()+start,Text.data()+Cursor,result);if(read.ec!=std::errc{}||!std::isfinite(result))fail();return Json{result};
  }
 public: explicit Parser(std::string_view text):Text(text){} Json parse(){Json j=value(0);space();if(Cursor!=Text.size())fail();return j;}
 };
 std::string text(const Json& j,const char* key,std::string fallback={}) {auto v=j.find(key);return v?v->text():fallback;}
 double number(const Json& j,const char* key,double fallback) {auto v=j.find(key);return v?v->number():fallback;}
 bool jsonBoolean(const Json& j,const char* key,bool fallback) {auto v=j.find(key);return v?v->boolean():fallback;}
 std::uint32_t uint(const Json& j,const char* key,std::uint32_t fallback,std::uint32_t maximum=3600000){auto v=number(j,key,fallback);if(v<0||v>maximum||std::floor(v)!=v)throw std::runtime_error(std::string("invalid integer: ")+key);return static_cast<std::uint32_t>(v);}
 std::array<float,3> position(const Json& j){const auto&a=j.array();if(a.size()!=3)throw std::runtime_error("Guide vector must have 3 values");std::array<float,3>p{};for(int i=0;i<3;++i){auto n=a[i].number();if(std::abs(n)>100000)throw std::runtime_error("Guide vector outside world bounds");p[i]=static_cast<float>(n);}return p;}
 std::filesystem::path path()
 {
  wchar_t buffer[32768]{}; auto size=GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",buffer,32768);
  if(size&&size<32768)return std::filesystem::path(buffer)/L"Guide"/L"Guide.runtime.json";
  size=GetModuleFileNameW(nullptr,buffer,32768);if(!size||size>=32768)return {};
  return std::filesystem::path(buffer).parent_path().parent_path()/L"DataFiles"/L"Guide"/L"Guide.runtime.json";
 }
 LostArk::Shared::WORLD_ID world(std::string name)
 {
  using LostArk::Shared::WORLD_ID;
  if(name=="BERN")return WORLD_ID::BERN;if(name=="VALTAN_ARENA")return WORLD_ID::VALTAN_ARENA;if(name=="KAKULSAYDON_ARENA")return WORLD_ID::KAKULSAYDON_ARENA;
  throw std::runtime_error("unknown Guide world: "+name);
 }
}
bool LostArk::Server::CGuideCatalog::Load(std::string& status)
{
 try
 {
  std::ifstream file(path(),std::ios::binary);if(!file)throw std::runtime_error("Guide.runtime.json is missing; run Guide Publish");
  file.seekg(0,std::ios::end);if(file.tellg()>4*1024*1024)throw std::runtime_error("Guide runtime exceeds 4 MiB");file.seekg(0);
  std::string source((std::istreambuf_iterator<char>(file)),{});if(source.starts_with("\xef\xbb\xbf"))source.erase(0,3);
  const auto root=Parser(source).parse();if(text(root,"schema")!="lostark.guide-runtime"||uint(root,"formatVersion",0)!=1)throw std::runtime_error("unsupported Guide schema");
  CGuideCatalog candidate;const double revision=root.at("revision").number();if(revision<1||revision>9007199254740991.0||std::floor(revision)!=revision)throw std::runtime_error("invalid Guide revision");candidate.Revision=static_cast<std::uint64_t>(revision);candidate.Id=root.at("guideId").text();candidate.Name=root.at("displayName").text();
  if(!candidate.Revision||candidate.Id.empty()||!LostArk::Shared::Is_Valid_PlayerNickname(candidate.Name)||text(root,"characterClass")!="DIMENSIONMASTER")throw std::runtime_error("invalid Guide identity");
  std::set<std::string> categoryIds;
  for(const auto& c:root.at("categories").array()){const auto id=c.at("categoryId").text();if(id.empty()||id.size()>128||!categoryIds.insert(id).second||!candidate.Categories.emplace(world(c.at("worldId").text()),id).second)throw std::runtime_error("duplicate or invalid Guide category");}
  const auto& p=root.at("placement");candidate.PlacementId=p.at("placementId").text();candidate.Position=position(p.at("position"));candidate.Yaw=static_cast<float>(p.at("yawDegrees").number());candidate.AnchorPolicy=text(p,"anchorPolicy","INVITER_THEN_LEADER");
  for(const auto& pmt:root.at("prompts").array()) { GUIDE_PROMPT prompt;prompt.Id=pmt.at("promptId").text();for(const auto& s:pmt.at("segments").array()){auto value=s.at("text").text();auto duration=uint(s,"durationMs",5000,120000);if(value.empty()||value.size()>512||duration<1000||duration>20000)throw std::runtime_error("invalid Guide prompt segment");prompt.Segments.push_back({value,duration});}if(prompt.Id.empty()||prompt.Segments.empty()||candidate.Find_Prompt(prompt.Id))throw std::runtime_error("duplicate or empty Guide prompt");candidate.Prompts.push_back(std::move(prompt));}
  for(const auto& t:root.at("triggers").array()) { GUIDE_TRIGGER trigger;trigger.Id=t.at("triggerId").text();trigger.Category=t.at("categoryId").text();trigger.Enabled=jsonBoolean(t,"enabled",true);trigger.PromptId=text(t,"promptId");trigger.ComboId=text(t,"comboId");trigger.CooldownMs=uint(t,"cooldownMs",10000);trigger.Priority=static_cast<int>(uint(t,"priority",0,255));const auto&e=t.at("event");trigger.Type=e.at("type").text();trigger.PatternId=text(e,"patternId",text(e,"commandId"));
   if(const auto* box=trigger.Type=="SPACE_ENTER"?&e:nullptr){auto pos=position(box->at("position"));auto ext=position(box->at("halfExtents"));if(ext[0]<=0||ext[1]<=0||ext[2]<=0)throw std::runtime_error("Guide box extents must be positive");trigger.Box.fPositionX=pos[0];trigger.Box.fPositionY=pos[1];trigger.Box.fPositionZ=pos[2];trigger.Box.fHalfExtentX=ext[0];trigger.Box.fHalfExtentY=ext[1];trigger.Box.fHalfExtentZ=ext[2];trigger.Box.fYawDegrees=static_cast<float>(number(*box,"yawDegrees",0));}
   if(trigger.Type!="PARTY_JOINED"&&trigger.Type!="SPACE_ENTER"&&trigger.Type!="BOSS_PATTERN_STARTED"&&trigger.Type!="HELP_COMMAND")throw std::runtime_error("unknown Guide trigger type");
   if(trigger.Type!="HELP_COMMAND"&&!trigger.ComboId.empty())throw std::runtime_error("only help triggers can start a combo");
   if(!trigger.PromptId.empty()&&!candidate.Find_Prompt(trigger.PromptId))throw std::runtime_error("Guide trigger has an unknown prompt");candidate.Triggers.push_back(std::move(trigger));}
  const auto& combat=root.at("combat");const auto& follow=combat.at("follow");candidate.DesiredDistance=static_cast<float>(number(follow,"desiredDistanceM",3));candidate.MinimumDistance=static_cast<float>(number(follow,"minimumDistanceM",2));candidate.MaximumDistance=static_cast<float>(number(follow,"maximumDistanceM",4));candidate.ResumeDistance=static_cast<float>(number(follow,"resumeDistanceM",6));candidate.RecoverDistance=static_cast<float>(number(follow,"recoverDistanceM",30));candidate.RecoverDelay=uint(follow,"recoverDelayMs",3000)/1000.f;
  if(candidate.MinimumDistance<0||candidate.MinimumDistance>candidate.DesiredDistance||candidate.DesiredDistance>candidate.MaximumDistance||candidate.MaximumDistance>candidate.ResumeDistance||candidate.ResumeDistance>candidate.RecoverDistance)throw std::runtime_error("invalid Guide follow distance ordering");
  const auto& decision=combat.at("decision");candidate.ThinkSeconds=uint(decision,"thinkIntervalMs",100,10000)/1000.f;candidate.HorizonSeconds=uint(decision,"horizonMs",1000,10000)/1000.f;candidate.SwitchMargin=static_cast<float>(number(decision,"switchMargin",.15));candidate.MinimumHoldSeconds=uint(decision,"minimumHoldMs",250,10000)/1000.f;candidate.LethalHpFraction=static_cast<float>(number(decision,"lethalHpFraction",.25));
  auto weights=[](const Json& row){GUIDE_WEIGHTS w;w.Attack=static_cast<float>(number(row,"attack",1));w.Avoid=static_cast<float>(number(row,"avoid",3));w.Follow=static_cast<float>(number(row,"follow",5));if(w.Attack<0||w.Avoid<0||w.Follow<0)throw std::runtime_error("negative Guide weight");return w;};candidate.FollowWeights=weights(combat.at("weights").at("FOLLOW"));candidate.AssistWeights=weights(combat.at("weights").at("ASSIST"));
  for(const auto& c:combat.at("combos").array()) {GUIDE_COMBO combo;combo.Id=c.at("comboId").text();combo.TimeoutMs=uint(c,"timeoutMs",45000);combo.StepWaitMs=uint(c,"stepWaitMs",5000);combo.Repeat=jsonBoolean(c,"repeat",false);for(const auto& s:c.at("inputSlots").array())combo.Slots.push_back(s.text());if(const auto* ids=c.find("resolvedSkillIds"))for(const auto& id:ids->array()){const auto n=id.number();if(n<1||n>4294967295.0||std::floor(n)!=n)throw std::runtime_error("invalid resolved skill id");combo.Skills.push_back(static_cast<std::uint32_t>(n));}if(combo.Id.empty()||combo.Slots.empty()||combo.Slots.size()>64||candidate.Find_Combo(combo.Id))throw std::runtime_error("invalid Guide combo");candidate.Combos.push_back(std::move(combo));}
  for(const auto& c:combat.at("commands").array()){GUIDE_COMMAND command;command.Id=c.at("commandId").text();command.ComboId=text(c,"comboId");command.Stop=jsonBoolean(c,"stop",false);command.Enabled=jsonBoolean(c,"enabled",true);command.CooldownMs=uint(c,"cooldownMs",3000);for(const auto&a:c.at("aliases").array())command.Aliases.push_back(a.text());if(!command.Stop&&!candidate.Find_Combo(command.ComboId))throw std::runtime_error("Guide command has unknown combo");candidate.Commands.push_back(std::move(command));}
  for(const auto& trigger:candidate.Triggers){if(!trigger.ComboId.empty()&&!candidate.Find_Combo(trigger.ComboId))throw std::runtime_error("Guide trigger has unknown combo");if(trigger.Type=="HELP_COMMAND"&&std::none_of(candidate.Commands.begin(),candidate.Commands.end(),[&](const auto& command){return command.Id==trigger.PatternId;}))throw std::runtime_error("Guide trigger has unknown command");}
  auto bounded=[](float value,float low,float high){return std::isfinite(value)&&value>=low&&value<=high;};
  if(candidate.Id!="guide.dimensionmaster"||candidate.PlacementId.empty()||candidate.PlacementId.size()>128||!categoryIds.contains(p.at("categoryId").text())||!bounded(candidate.Yaw,-36000,36000)||(candidate.AnchorPolicy!="INVITER_THEN_LEADER"&&candidate.AnchorPolicy!="PARTY_LEADER"))throw std::runtime_error("invalid Guide placement contract");
  if(candidate.Prompts.size()>512||candidate.Combos.size()>128||!bounded(candidate.MinimumDistance,.1f,200)||!bounded(candidate.RecoverDistance,.1f,200)||candidate.MaximumDistance>=candidate.ResumeDistance||candidate.ResumeDistance>=candidate.RecoverDistance||!bounded(candidate.RecoverDelay,1,60)||!bounded(candidate.ThinkSeconds,.033f,1)||!bounded(candidate.HorizonSeconds,.1f,5)||!bounded(candidate.SwitchMargin,0,1)||!bounded(candidate.MinimumHoldSeconds,0,3)||!bounded(candidate.LethalHpFraction,.1f,1))throw std::runtime_error("invalid Guide decision bounds");
  for(const auto& weights:{candidate.FollowWeights,candidate.AssistWeights})if(!bounded(weights.Attack,0,1)||!bounded(weights.Avoid,0,1)||!bounded(weights.Follow,0,1)||std::abs(weights.Attack+weights.Avoid+weights.Follow-1)>.001f)throw std::runtime_error("invalid normalized Guide weights");
  if(candidate.FollowWeights.Attack!=0)throw std::runtime_error("follow mode cannot attack");
  for(const auto& prompt:candidate.Prompts){std::size_t bytes=0;std::uint32_t duration=0;for(const auto& segment:prompt.Segments){bytes+=segment.Text.size();duration+=segment.DurationMs;if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,segment.Text.data(),static_cast<int>(segment.Text.size()),nullptr,0))throw std::runtime_error("invalid prompt UTF-8");}if(prompt.Id.size()>128||prompt.Segments.size()>16||bytes>4096||duration>60000)throw std::runtime_error("Guide prompt exceeds published limits");}
  std::set<std::string> triggerIds,commandIds,aliases;
  for(const auto& trigger:candidate.Triggers)if(trigger.Id.empty()||trigger.Id.size()>128||!triggerIds.insert(trigger.Id).second||!categoryIds.contains(trigger.Category)||(trigger.Type=="BOSS_PATTERN_STARTED"&&trigger.PatternId.empty()))throw std::runtime_error("invalid Guide trigger identity/category");
  for(const auto& combo:candidate.Combos)if(combo.Id.size()>128||combo.Slots.size()>32||combo.Skills.size()!=combo.Slots.size()||combo.TimeoutMs<1000||combo.TimeoutMs>120000||combo.StepWaitMs>30000)throw std::runtime_error("invalid Guide combo bounds");
  for(const auto& command:candidate.Commands){if(command.Id.empty()||command.Id.size()>128||!commandIds.insert(command.Id).second||command.Aliases.empty()||command.Aliases.size()>32||command.CooldownMs>60000||(command.Stop&&!command.ComboId.empty()))throw std::runtime_error("invalid Guide command");for(const auto& alias:command.Aliases)if(alias.empty()||alias.size()>128||(command.Enabled&&!aliases.insert(alias).second))throw std::runtime_error("invalid or duplicate Guide alias");}
  candidate.Loaded=true;*this=std::move(candidate);status="Guide revision "+std::to_string(Revision)+" loaded";return true;
 }
 catch(const std::exception& error){status=error.what();return false;}
}
const LostArk::Server::GUIDE_PROMPT* LostArk::Server::CGuideCatalog::Find_Prompt(const std::string& id)const{auto i=std::find_if(Prompts.begin(),Prompts.end(),[&](const auto& p){return p.Id==id;});return i==Prompts.end()?nullptr:&*i;}
const LostArk::Server::GUIDE_COMBO* LostArk::Server::CGuideCatalog::Find_Combo(const std::string& id)const{auto i=std::find_if(Combos.begin(),Combos.end(),[&](const auto& p){return p.Id==id;});return i==Combos.end()?nullptr:&*i;}
