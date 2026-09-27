#pragma once
#include "WorldBootstrap.h"
#include <array>
#include <map>
#include <string>
#include <vector>

namespace LostArk::Server
{
 struct GUIDE_PROMPT_SEGMENT { std::string Text; std::uint32_t DurationMs = 5000; };
 struct GUIDE_PROMPT { std::string Id; std::vector<GUIDE_PROMPT_SEGMENT> Segments; };
 struct GUIDE_TRIGGER
 {
  std::string Id, Type, Category, PatternId, PromptId, ComboId;
  WORLD_BOOTSTRAP_PLACEMENT Box;
  bool Enabled = true;
  std::uint32_t CooldownMs = 10000;
  int Priority = 0;
 };
 struct GUIDE_COMBO
 {
  std::string Id;
  std::vector<std::string> Slots;
  std::vector<LostArk::Shared::SKILL_ID> Skills;
  std::uint32_t TimeoutMs = 45000, StepWaitMs = 5000;
  bool Repeat = false;
 };
 struct GUIDE_COMMAND
 {
  std::string Id, ComboId;
  std::vector<std::string> Aliases;
  bool Stop = false, Enabled = true;
  std::uint32_t CooldownMs = 3000;
 };
 struct GUIDE_WEIGHTS { float Attack = 1.f, Avoid = 3.f, Follow = 5.f; };
 class CGuideCatalog final
 {
 public:
  bool Load(std::string& status);
  bool Loaded = false;
  std::uint64_t Revision = 0;
  std::string Id, Name, PlacementId, AnchorPolicy;
  std::array<float, 3> Position{};
  float Yaw = 0.f;
  float DesiredDistance = 3.f, MinimumDistance = 2.f, MaximumDistance = 4.f, ResumeDistance = 6.f;
  float RecoverDistance = 30.f, RecoverDelay = 3.f;
  float ThinkSeconds = .1f, HorizonSeconds = 1.f, SwitchMargin = .15f, MinimumHoldSeconds = .25f, LethalHpFraction = .25f;
  GUIDE_WEIGHTS FollowWeights, AssistWeights;
  std::map<LostArk::Shared::WORLD_ID, std::string> Categories;
  std::vector<GUIDE_PROMPT> Prompts;
  std::vector<GUIDE_TRIGGER> Triggers;
  std::vector<GUIDE_COMBO> Combos;
  std::vector<GUIDE_COMMAND> Commands;
  const GUIDE_PROMPT* Find_Prompt(const std::string& id) const;
  const GUIDE_COMBO* Find_Combo(const std::string& id) const;
 };
}
