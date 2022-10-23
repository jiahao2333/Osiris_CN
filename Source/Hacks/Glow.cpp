#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
#include <imgui/imgui.h>

#include "../ConfigStructs.h"
#include "../InputUtil.h"
#include "Glow.h"
#include "../Helpers.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include <SDK/Constants/ClassId.h>
#include "../SDK/ClientClass.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/GlowObjectManager.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Utils.h"
#include "../SDK/UtlVector.h"
#include "../SDK/Vector.h"
#include "../imguiCustom.h"

#if OSIRIS_GLOW()

struct GlowItem : Color4 {
    bool enabled = false;
    bool healthBased = false;
    int style = 0;
};

struct PlayerGlow {
    GlowItem all, visible, occluded;
};

static std::unordered_map<std::string, PlayerGlow> playerGlowConfig;
static std::unordered_map<std::string, GlowItem> glowConfig;
static KeyBindToggle glowToggleKey;
static KeyBind glowHoldKey;

static std::vector<std::pair<int, int>> customGlowEntities;

void Glow::render(const EngineInterfaces& engineInterfaces, const ClientInterfaces& clientInterfaces, const Interfaces& interfaces, const Memory& memory) noexcept
{
    if (!localPlayer)
        return;

    auto& glow = glowConfig;

    Glow::clearCustomObjects(memory);

    if (glowToggleKey.isSet()) {
        if (!glowToggleKey.isToggled() && !glowHoldKey.isDown())
            return;
    } else if (glowHoldKey.isSet() && !glowHoldKey.isDown()) {
        return;
    }

    const auto highestEntityIndex = clientInterfaces.getEntityList().getHighestEntityIndex();
    for (int i = engineInterfaces.getEngine().getMaxClients() + 1; i <= highestEntityIndex; ++i) {
        const Entity entity{ retSpoofGadgets.client, clientInterfaces.getEntityList().getEntity(i) };
        if (entity.getThis() == 0 || entity.getNetworkable().isDormant())
            continue;

        switch (entity.getNetworkable().getClientClass()->classId) {
        case ClassId::EconEntity:
        case ClassId::BaseCSGrenadeProjectile:
        case ClassId::BreachChargeProjectile:
        case ClassId::BumpMineProjectile:
        case ClassId::DecoyProjectile:
        case ClassId::MolotovProjectile:
        case ClassId::SensorGrenadeProjectile:
        case ClassId::SmokeGrenadeProjectile:
        case ClassId::SnowballProjectile:
        case ClassId::Hostage:
        case ClassId::CSRagdoll:
            if (!memory.glowObjectManager->hasGlowEffect(entity.getThis())) {
                if (auto index{ memory.glowObjectManager->registerGlowObject(entity.getThis()) }; index != -1)
                    customGlowEntities.emplace_back(i, index);
            }
            break;
        default:
            break;
        }
    }

    for (int i = 0; i < memory.glowObjectManager->glowObjectDefinitions.size; i++) {
        GlowObjectDefinition& glowobject = memory.glowObjectManager->glowObjectDefinitions[i];

        const Entity entity{ retSpoofGadgets.client, glowobject.entity };

        if (glowobject.isUnused() || entity.getThis() == 0 || entity.getNetworkable().isDormant())
            continue;

        auto applyGlow = [&glowobject, &memory](const GlowItem& glow, int health = 0) noexcept
        {
            if (glow.enabled) {
                glowobject.renderWhenOccluded = true;
                glowobject.glowAlpha = glow.color[3];
                glowobject.glowStyle = glow.style;
                glowobject.glowAlphaMax = 0.6f;
                if (glow.healthBased && health) {
                    Helpers::healthColor(std::clamp(health / 100.0f, 0.0f, 1.0f), glowobject.glowColor.x, glowobject.glowColor.y, glowobject.glowColor.z);
                } else if (glow.rainbow) {
                    const auto [r, g, b] { rainbowColor(memory.globalVars->realtime, glow.rainbowSpeed) };
                    glowobject.glowColor = { r, g, b };
                } else {
                    glowobject.glowColor = { glow.color[0], glow.color[1], glow.color[2] };
                }
            }
        };

        auto applyPlayerGlow = [applyGlow, &memory, &engineInterfaces](const std::string& name, const Entity& entity) noexcept {
            const auto& cfg = playerGlowConfig[name];
            if (cfg.all.enabled) 
                applyGlow(cfg.all, entity.health());
            else if (cfg.visible.enabled && entity.visibleTo(engineInterfaces, memory, localPlayer.get()))
                applyGlow(cfg.visible, entity.health());
            else if (cfg.occluded.enabled && !entity.visibleTo(engineInterfaces, memory, localPlayer.get()))
                applyGlow(cfg.occluded, entity.health());
        };

        switch (entity.getNetworkable().getClientClass()->classId) {
        case ClassId::CSPlayer:
            if (!entity.isAlive())
                break;
            if (const Entity activeWeapon{ retSpoofGadgets.client, entity.getActiveWeapon() }; activeWeapon.getThis() != 0 && activeWeapon.getNetworkable().getClientClass()->classId == ClassId::C4 && activeWeapon.c4StartedArming())
                applyPlayerGlow("正在安放", entity);
            else if (entity.isDefusing())
                applyPlayerGlow("正在拆除", entity);
            else if (entity.getThis() == localPlayer.get().getThis())
                applyGlow(glow["自己"], entity.health());
            else if (entity.isOtherEnemy(memory, localPlayer.get()))
                applyPlayerGlow("敌人", entity);
            else
                applyPlayerGlow("队友", entity);
            break;
        case ClassId::C4: applyGlow(glow["C4 炸弹"]); break;
        case ClassId::PlantedC4: applyGlow(glow["已安放的 C4 炸弹"]); break;
        case ClassId::Chicken: applyGlow(glow["鸡"]); break;
        case ClassId::EconEntity: applyGlow(glow["拆弹器"]); break;

        case ClassId::BaseCSGrenadeProjectile:
        case ClassId::BreachChargeProjectile:
        case ClassId::BumpMineProjectile:
        case ClassId::DecoyProjectile:
        case ClassId::MolotovProjectile:
        case ClassId::SensorGrenadeProjectile:
        case ClassId::SmokeGrenadeProjectile:
        case ClassId::SnowballProjectile:
            applyGlow(glow["已投掷物"]); break;

        case ClassId::Hostage: applyGlow(glow["人质"]); break;
        case ClassId::CSRagdoll: applyGlow(glow["已死亡人物"]); break;
        default:
           if (entity.isWeapon()) {
                applyGlow(glow["Weapons"]);
                if (!glow["Weapons"].enabled) glowobject.renderWhenOccluded = false;
            }
        }
    }
}

void Glow::clearCustomObjects(const Memory& memory) noexcept
{
    for (const auto& [entityIndex, glowObjectIndex] : customGlowEntities)
        memory.glowObjectManager->unregisterGlowObject(glowObjectIndex);

    customGlowEntities.clear();
}

void Glow::updateInput() noexcept
{
    glowToggleKey.handleToggle();
}

static bool glowWindowOpen = false;

void Glow::menuBarItem() noexcept
{
    if (ImGui::MenuItem("发光")) {
        glowWindowOpen = true;
        ImGui::SetWindowFocus("Glow");
        ImGui::SetWindowPos("Glow", { 100.0f, 100.0f });
    }
}

void Glow::tabItem() noexcept
{
    if (ImGui::BeginTabItem("发光")) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void Glow::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!glowWindowOpen)
            return;
        ImGui::SetNextWindowSize({ 450.0f, 0.0f });
        ImGui::Begin("发光", &glowWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    ImGui::hotkey("切换按键", glowToggleKey, 80.0f);
    ImGui::hotkey("保持按键", glowHoldKey, 80.0f);
    ImGui::Separator();

    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    constexpr std::array categories{ "队友", "敌人", "正在安放", "正在拆除", "自己", "武器", "C4 炸弹", "已安装的 C4 炸弹", "鸡", "拆弹器", "已投掷物", "人质", "已死亡人物" };
    ImGui::Combo("", &currentCategory, categories.data(), categories.size());
    ImGui::PopID();
    GlowItem* currentItem;
    if (currentCategory <= 3) {
        ImGui::SameLine();
        static int currentType{ 0 };
        ImGui::PushID(1);
        ImGui::Combo("", &currentType, "全部\0可见时\0不可见时\0");
        ImGui::PopID();
        auto& cfg = playerGlowConfig[categories[currentCategory]];
        switch (currentType) {
        case 0: currentItem = &cfg.all; break;
        case 1: currentItem = &cfg.visible; break;
        case 2: currentItem = &cfg.occluded; break;
        }
    } else {
        currentItem = &glowConfig[categories[currentCategory]];
    }

    ImGui::SameLine();
    ImGui::Checkbox("启用", &currentItem->enabled);
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 150.0f);
    ImGui::Checkbox("根据生命值", &currentItem->healthBased);

    ImGuiCustom::colorPicker("颜色", *currentItem);

    ImGui::NextColumn();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::Combo("样式", &currentItem->style, "默认\0边缘3D\0边缘\0边缘脉冲\0");

    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

static void to_json(json& j, const GlowItem& o, const GlowItem& dummy = {})
{
    to_json(j, static_cast<const Color4&>(o), dummy);
    WRITE("Enabled", enabled);
    WRITE("Health based", healthBased);
    WRITE("Style", style);
}

static void to_json(json& j, const PlayerGlow& o, const PlayerGlow& dummy = {})
{
    WRITE("All", all);
    WRITE("Visible", visible);
    WRITE("Occluded", occluded);
}

json Glow::toJson() noexcept
{
    json j;
    j["Items"] = glowConfig;
    j["Players"] = playerGlowConfig;
    to_json(j["Toggle Key"], glowToggleKey, {});
    to_json(j["Hold Key"], glowHoldKey, {});
    return j;
}

static void from_json(const json& j, GlowItem& g)
{
    from_json(j, static_cast<Color4&>(g));

    read(j, "Enabled", g.enabled);
    read(j, "Health based", g.healthBased);
    read(j, "Style", g.style);
}

static void from_json(const json& j, PlayerGlow& g)
{
    read<value_t::object>(j, "All", g.all);
    read<value_t::object>(j, "Visible", g.visible);
    read<value_t::object>(j, "Occluded", g.occluded);
}

void Glow::fromJson(const json& j) noexcept
{
    read(j, "Items", glowConfig);
    read(j, "Players", playerGlowConfig);
    read(j, "Toggle Key", glowToggleKey);
    read(j, "Hold Key", glowHoldKey);
}

void Glow::resetConfig() noexcept
{
    glowConfig = {};
    playerGlowConfig = {};
    glowToggleKey = {};
    glowHoldKey = {};
}

#else

void Glow::render() noexcept {}
void Glow::clearCustomObjects() noexcept {}
void Glow::updateInput() noexcept {}

// GUI
void Glow::menuBarItem() noexcept {}
void Glow::tabItem() noexcept {}
void Glow::drawGUI(bool contentOnly) noexcept {}

// Config
json Glow::toJson() noexcept { return {}; }
void Glow::fromJson(const json& j) noexcept {}
void Glow::resetConfig() noexcept {}

#endif
