// Headless bundled ImGui A/B: this fixture creates no native window or Client.

#include "imgui.h"
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
void TestImGuiAssert(const char* condition, const char* file, int line) { throw std::runtime_error(std::string(condition) + " at " + file + ":" + std::to_string(line)); }
namespace Client {
struct Transfer { std::string label = "summon"; };
using COMPOSITION_TRANSFER = std::shared_ptr<Transfer>;
struct CCompositionClipboard { static auto& Get(){ static CCompositionClipboard x; return x; } void Write(COMPOSITION_TRANSFER) {} };
void Offer_CompositionResourceDrag(const char*, const std::function<COMPOSITION_TRANSFER()>&);
}
using namespace Client;
namespace {
constexpr const char* TransferPayload = "COMPOSITION_RESOURCE_V1";
COMPOSITION_TRANSFER DragTransfer;
std::uint64_t DragToken = 0;
}
// @PRODUCTION_DRAG@
std::array<char,160> m_SummonResourceSearch{};
std::string m_strStatus;
ImVec2 searchPosition, copyPosition;
bool textActive = false;
void content(bool original, bool logic = false) {
 ImGui::SetNextWindowPos({0,0}); ImGui::SetNextWindowSize({650,400});
 ImGui::Begin("Summon resource fixture", nullptr, ImGuiWindowFlags_NoSavedSettings);
 ImGui::SetNextItemWidth(-1.f);
 // @PRODUCTION_SEARCH@
 textActive = ImGui::IsItemActive();
 searchPosition = {ImGui::GetItemRectMin().x + 15.f, ImGui::GetItemRectMin().y + 5.f};
 struct Object { std::string strCombatObjectArchetypeId="combat.valtan.ground-roar.rock"; unsigned iSpawnValue=4, iLifetimeMs=1000; } object;
 const auto capture = [&](std::string&) { return std::make_shared<Transfer>(); };
 ImGui::PushID(object.strCombatObjectArchetypeId.c_str());
 if(original) {
  ImGui::TextDisabled("Spawn count %u | lifetime %u ms", object.iSpawnValue, object.iLifetimeMs);
  Offer_CompositionResourceDrag(object.strCombatObjectArchetypeId.c_str(), [&]() { return capture(m_strStatus); });
  if (ImGui::SmallButton("Copy Resource")) CCompositionClipboard::Get().Write(capture(m_strStatus));
 } else if (logic) {
  struct Action { std::string strKind="SET_FLAG", strTargetId="fixture", strTrigger="ENTER"; } action;
  const std::string key="logic.fixture";
  // @PRODUCTION_LOGIC_ROWS@
 } else {
  // @PRODUCTION_ROWS@
 }
 ImGui::PopID(); ImGui::End();
}
void setup() {
 ImGui::CreateContext(); auto& io = ImGui::GetIO(); io.IniFilename=nullptr; io.LogFilename=nullptr; io.DisplaySize={800,600}; io.DeltaTime=1.f/60.f;
 unsigned char* pixels; int width,height; io.Fonts->GetTexDataAsRGBA32(&pixels,&width,&height);
}
void frame(bool original, bool logic = false) { ImGui::NewFrame(); content(original, logic); ImGui::Render(); }
int main() {
 setup(); frame(true);
 auto& beforeIo=ImGui::GetIO(); beforeIo.AddMousePosEvent(searchPosition.x,searchPosition.y); beforeIo.AddMouseButtonEvent(0,true);
 bool reproduced=false; try { frame(true); } catch(const std::exception& e) { reproduced=true; std::cout << "BASELINE " << e.what() << '\n'; }
 ImGui::DestroyContext(); if(!reproduced) { std::cerr << "baseline search click failed to reproduce\n"; return 1; }

 for (bool logic : {false,true}) {
  m_SummonResourceSearch.fill(0); DragTransfer.reset();
  setup(); frame(false,logic);
  auto& io=ImGui::GetIO(); io.AddMousePosEvent(searchPosition.x,searchPosition.y); io.AddMouseButtonEvent(0,true); frame(false,logic);
  if(!textActive) { std::cerr << "candidate search was not activated\n"; return 2; }
  io.AddMouseButtonEvent(0,false); frame(false,logic); io.AddInputCharactersUTF8("ground-roar.rock"); frame(false,logic);
  if(std::string(m_SummonResourceSearch.data())!="ground-roar.rock") { std::cerr << "candidate input was not retained\n"; return 3; }
  io.AddMousePosEvent(copyPosition.x,copyPosition.y); io.AddMouseButtonEvent(0,true); frame(false,logic);
  io.AddMousePosEvent(copyPosition.x+30.f,copyPosition.y+10.f); frame(false,logic);
  const auto* payload=ImGui::GetDragDropPayload();
  if(!payload || !payload->IsDataType(TransferPayload)) { std::cerr << "candidate resource button drag did not yield payload\n"; return 4; }
  ImGui::DestroyContext();
 }
 std::cout << "PASS real ImGui Summon/Logic search click, text input and resource drag; original null-ID assertion reproduced\n";
}
