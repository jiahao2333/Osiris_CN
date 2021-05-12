#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "../imgui/imgui.h"

#include "../SDK/WeaponId.h"
#include "../JsonForward.h"

enum class FrameStage;
class Entity;
class GameEvent;

namespace SkinChanger
{
    void run(FrameStage) noexcept;
    void scheduleHudUpdate() noexcept;
    void overrideHudIcon(GameEvent& event) noexcept;
    void updateStatTrak(GameEvent& event) noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;

    struct PaintKit {
        PaintKit(int id, const std::string& name, int rarity = 0) noexcept;
        PaintKit(int id, std::string&& name, int rarity = 0) noexcept;
        PaintKit(int id, std::wstring&& name, std::string&& iconPath, int rarity = 0) noexcept;
        PaintKit(int id, std::wstring&& name, int rarity = 0) noexcept;

        int id;
        int rarity;
        std::string name;
        std::wstring nameUpperCase;
        std::string iconPath;

        auto operator<(const PaintKit& other) const noexcept
        {
            return nameUpperCase < other.nameUpperCase;
        }
    };

    struct Quality {
        Quality(int index, const char* name) : index{ index }, name{ name } {}
        int index = 0;
        std::string name;
    };

    struct Item {
        Item(WeaponId id, const char* name) : id(id), name(name) {}

        WeaponId id;
        std::string name;
    };

    const std::vector<PaintKit>& getSkinKits() noexcept;
    const std::vector<PaintKit>& getGloveKits() noexcept;
    const std::vector<PaintKit>& getStickerKits() noexcept;
    const std::vector<Quality>& getQualities() noexcept;
    const std::vector<Item>& getGloveTypes() noexcept;
    const std::vector<Item>& getKnifeTypes() noexcept;

    ImTextureID getItemIconTexture(const std::string& iconpath) noexcept;
    void clearItemIconTextures() noexcept;
    void clearUnusedItemIconTextures() noexcept;

    void fixKnifeAnimation(Entity* viewModelWeapon, long& sequence) noexcept;

    struct weapon_name {
        constexpr weapon_name(WeaponId definition_index, const char* name) : definition_index(definition_index), name(name) {}

        WeaponId definition_index;
        const char* name;
    };

    constexpr auto weapon_names = std::to_array<weapon_name>({
        {WeaponId::Knife, "匕首"},
        {WeaponId::GloveT, "手套"},
        {WeaponId::Ak47, "AK-47"},
        {WeaponId::Aug, "AUG"},
        {WeaponId::Awp, "AWP"},
        {WeaponId::Cz75a, "CZ75"},
        {WeaponId::Deagle, "沙漠之鹰"},
        {WeaponId::Elite, "双持贝瑞塔"},
        {WeaponId::Famas, "法玛斯"},
        {WeaponId::Fiveseven, "FN57"},
        {WeaponId::G3SG1, "G3SG1"},
        {WeaponId::GalilAr, "加利尔 AR"},
        {WeaponId::Glock, "格洛克 18 型"},
        {WeaponId::M249, "M249"},
        {WeaponId::M4a1_s, "M4A1 消音型"},
        {WeaponId::M4A1, "M4A4"},
        {WeaponId::Mac10, "MAC-10"},
        {WeaponId::Mag7, "MAG-7"},
        {WeaponId::Mp5sd, "MP5-SD"},
        {WeaponId::Mp7, "MP7"},
        {WeaponId::Mp9, "MP9"},
        {WeaponId::Negev, "内格夫"},
        {WeaponId::Nova, "新星"},
        {WeaponId::Hkp2000, "P2000"},
        {WeaponId::P250, "P250"},
        {WeaponId::P90, "P90"},
        {WeaponId::Bizon, "PP-野牛"},
        {WeaponId::Revolver, "R8 左轮手枪"},
        {WeaponId::Sawedoff, "截短霰弹枪"},
        {WeaponId::Scar20, "SCAR-20"},
        {WeaponId::Ssg08, "SSG 08"},
        {WeaponId::Sg553, "SG 553"},
        {WeaponId::Tec9, "Tec-9"},
        {WeaponId::Ump45, "UMP-45"},
        {WeaponId::Usp_s, "USP 消音型"},
        {WeaponId::Xm1014, "XM1014"}
    });
}
