#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <unordered_map>

#include "fnv.h"
#include "GameData.h"
#include "Interfaces.h"
#include "Memory.h"

#include "Resources/avatar_ct.h"
#include "Resources/avatar_tt.h"

#include <stb_image.h>

#include "SDK/Constants/ClassId.h"
#include "SDK/ClientClass.h"
#include "SDK/Engine.h"
#include "SDK/Entity.h"
#include "SDK/EntityList.h"
#include "SDK/GlobalVars.h"
#include "SDK/Localize.h"
#include "SDK/LocalPlayer.h"
#include "SDK/ModelInfo.h"
#include "SDK/ModelRender.h"
#include "SDK/NetworkChannel.h"
#include "SDK/PlantedC4.h"
#include "SDK/PlayerResource.h"
#include "SDK/Sound.h"
#include "SDK/Steam.h"
#include "SDK/UtlVector.h"
#include "SDK/WeaponId.h"
#include "SDK/WeaponData.h"

auto operator<(const BaseData& a, const BaseData& b) noexcept
{
    return a.distanceToLocal > b.distanceToLocal;
}

static Matrix4x4 viewMatrix;
static LocalPlayerData localPlayerData;
static std::vector<PlayerData> playerData;
static std::vector<ObserverData> observerData;
static std::vector<WeaponData> weaponData;
static std::vector<EntityData> entityData;
static std::vector<LootCrateData> lootCrateData;
static std::forward_list<ProjectileData> projectileData;
static BombData bombData;
static std::vector<InfernoData> infernoData;
static std::atomic_int netOutgoingLatency;

static auto playerByHandleWritable(int handle) noexcept
{
    const auto it = std::ranges::find(playerData, handle, &PlayerData::handle);
    return it != playerData.end() ? &(*it) : nullptr;
}

static void updateNetLatency(const Engine& engine) noexcept
{
    if (const auto networkChannel = engine.getNetworkChannel())
        netOutgoingLatency = (std::max)(static_cast<int>(networkChannel->getLatency(0) * 1000.0f), 0);
    else
        netOutgoingLatency = 0;
}

constexpr auto playerVisibilityUpdateDelay = 0.1f;
static float nextPlayerVisibilityUpdateTime = 0.0f;

static bool shouldUpdatePlayerVisibility(const Memory& memory) noexcept
{
    return nextPlayerVisibilityUpdateTime <= memory.globalVars->realtime;
}

void GameData::update(const ClientInterfaces& clientInterfaces, const EngineInterfaces& engineInterfaces, const Interfaces& interfaces, const Memory& memory) noexcept
{
    static int lastFrame;
    if (lastFrame == memory.globalVars->framecount)
        return;
    lastFrame = memory.globalVars->framecount;

    updateNetLatency(engineInterfaces.getEngine());

    Lock lock;
    observerData.clear();
    weaponData.clear();
    entityData.clear();
    lootCrateData.clear();
    infernoData.clear();

    localPlayerData.update(engineInterfaces.getEngine());
    bombData.update(memory);

    if (!localPlayer) {
        playerData.clear();
        projectileData.clear();
        return;
    }

    viewMatrix = engineInterfaces.getEngine().worldToScreenMatrix();

    const auto observerTarget = localPlayer->getObserverMode() == ObsMode::InEye ? localPlayer->getObserverTarget() : nullptr;

    const auto highestEntityIndex = clientInterfaces.getEntityList().getHighestEntityIndex();
    for (int i = 1; i <= highestEntityIndex; ++i) {
        const auto entity = clientInterfaces.getEntityList().getEntity(i);
        if (!entity)
            continue;

        if (entity->isPlayer()) {
            if (entity == localPlayer.get() || entity == observerTarget)
                continue;

            if (const auto player = playerByHandleWritable(entity->handle())) {
                player->update(engineInterfaces, interfaces, memory, entity);
            } else {
                playerData.emplace_back(engineInterfaces, interfaces, memory, entity);
            }

            if (!entity->isDormant() && !entity->isAlive()) {
                if (const auto obs = entity->getObserverTarget())
                    observerData.emplace_back(entity, obs, obs == localPlayer.get());
            }
        } else {
            if (entity->isDormant())
                continue;

            if (entity->isWeapon()) {
                if (entity->ownerEntity() == -1)
                    weaponData.emplace_back(interfaces, entity);
            } else {
                switch (entity->getClientClass()->classId) {
                case ClassId::BaseCSGrenadeProjectile:
                    if (!entity->shouldDraw()) {
                        if (const auto it = std::ranges::find(projectileData, entity->handle(), &ProjectileData::handle); it != projectileData.end())
                            it->exploded = true;
                        break;
                    }
                    [[fallthrough]];
                case ClassId::BreachChargeProjectile:
                case ClassId::BumpMineProjectile:
                case ClassId::DecoyProjectile:
                case ClassId::MolotovProjectile:
                case ClassId::SensorGrenadeProjectile:
                case ClassId::SmokeGrenadeProjectile:
                case ClassId::SnowballProjectile:
                    if (const auto it = std::ranges::find(projectileData, entity->handle(), &ProjectileData::handle); it != projectileData.end())
                        it->update(memory, entity);
                    else
                        projectileData.emplace_front(clientInterfaces, memory, entity);
                    break;
                case ClassId::DynamicProp:
                    if (const auto model = entity->getModel(); !model || !std::strstr(model->name, "challenge_coin"))
                        break;
                    [[fallthrough]];
                case ClassId::EconEntity:
                case ClassId::Chicken:
                case ClassId::PlantedC4:
                case ClassId::Hostage:
                case ClassId::Dronegun:
                case ClassId::Cash:
                case ClassId::AmmoBox:
                case ClassId::RadarJammer:
                case ClassId::SnowballPile:
                    entityData.emplace_back(entity);
                    break;
                case ClassId::LootCrate:
                    lootCrateData.emplace_back(entity);
                    break;
                case ClassId::Inferno:
                    infernoData.emplace_back(entity);
                    break;
                default:
                    break;
                }
            }
        }
    }

    std::sort(playerData.begin(), playerData.end());
    std::sort(weaponData.begin(), weaponData.end());
    std::sort(entityData.begin(), entityData.end());
    std::sort(lootCrateData.begin(), lootCrateData.end());

    std::ranges::for_each(projectileData, [&clientInterfaces](auto& projectile) {
        if (clientInterfaces.getEntityList().getEntityFromHandle(projectile.handle) == nullptr)
            projectile.exploded = true;
    });

    std::erase_if(projectileData, [&memory, &clientInterfaces](const auto& projectile) { return clientInterfaces.getEntityList().getEntityFromHandle(projectile.handle) == nullptr
        && (projectile.trajectory.size() < 1 || projectile.trajectory[projectile.trajectory.size() - 1].first + 60.0f < memory.globalVars->realtime); });

    std::erase_if(playerData, [&clientInterfaces](const auto& player) { return clientInterfaces.getEntityList().getEntityFromHandle(player.handle) == nullptr; });

    if (shouldUpdatePlayerVisibility(memory))
        nextPlayerVisibilityUpdateTime = memory.globalVars->realtime + playerVisibilityUpdateDelay;
}

void GameData::clearProjectileList() noexcept
{
    Lock lock;
    projectileData.clear();
}

static void clearAvatarTextures() noexcept;

struct PlayerAvatar {
    mutable Texture texture;
    std::unique_ptr<std::uint8_t[]> rgba;
};

static std::unordered_map<int, PlayerAvatar> playerAvatars;

void GameData::clearTextures() noexcept
{
    Lock lock;

    clearAvatarTextures();
    for (const auto& [handle, avatar] : playerAvatars)
        avatar.texture.clear();
}

void GameData::clearUnusedAvatars() noexcept
{
    Lock lock;
    std::erase_if(playerAvatars, [](const auto& pair) { return std::ranges::find(std::as_const(playerData), pair.first, &PlayerData::handle) == playerData.cend(); });
}

int GameData::getNetOutgoingLatency() noexcept
{
    return netOutgoingLatency;
}

const Matrix4x4& GameData::toScreenMatrix() noexcept
{
    return viewMatrix;
}

const LocalPlayerData& GameData::local() noexcept
{
    return localPlayerData;
}

const std::vector<PlayerData>& GameData::players() noexcept
{
    return playerData;
}

const PlayerData* GameData::playerByHandle(int handle) noexcept
{
    return playerByHandleWritable(handle);
}

const std::vector<ObserverData>& GameData::observers() noexcept
{
    return observerData;
}

const std::vector<WeaponData>& GameData::weapons() noexcept
{
    return weaponData;
}

const std::vector<EntityData>& GameData::entities() noexcept
{
    return entityData;
}

const std::vector<LootCrateData>& GameData::lootCrates() noexcept
{
    return lootCrateData;
}

const std::forward_list<ProjectileData>& GameData::projectiles() noexcept
{
    return projectileData;
}

const BombData& GameData::plantedC4() noexcept
{
    return bombData;
}

const std::vector<InfernoData>& GameData::infernos() noexcept
{
    return infernoData;
}

void LocalPlayerData::update(const Engine& engine) noexcept
{
    if (!localPlayer) {
        exists = false;
        return;
    }

    exists = true;
    alive = localPlayer->isAlive();

    if (const auto activeWeapon = localPlayer->getActiveWeapon()) {
        inReload = activeWeapon->isInReload();
        shooting = localPlayer->shotsFired() > 1;
        noScope = activeWeapon->isSniperRifle() && !localPlayer->isScoped();
        nextWeaponAttack = activeWeapon->nextPrimaryAttack();
    }
    fov = localPlayer->fov() ? localPlayer->fov() : localPlayer->defaultFov();
    handle = localPlayer->handle();
    flashDuration = localPlayer->flashDuration();

    aimPunch = localPlayer->getEyePosition() + Vector::fromAngle(engine.getViewAngles() + localPlayer->getAimPunch()) * 1000.0f;

    const auto obsMode = localPlayer->getObserverMode();
    if (const auto obs = localPlayer->getObserverTarget(); obs && obsMode != ObsMode::Roaming && obsMode != ObsMode::Deathcam)
        origin = obs->getAbsOrigin();
    else
        origin = localPlayer->getAbsOrigin();
}

BaseData::BaseData(Entity* entity) noexcept
{
    distanceToLocal = entity->getAbsOrigin().distTo(localPlayerData.origin);
 
    if (entity->isPlayer()) {
        const auto collideable = entity->getCollideable();
        obbMins = collideable->obbMins();
        obbMaxs = collideable->obbMaxs();
    } else if (const auto model = entity->getModel()) {
        obbMins = model->mins;
        obbMaxs = model->maxs;
    }

    coordinateFrame = entity->toWorldTransform();
}

EntityData::EntityData(Entity* entity) noexcept : BaseData{ entity }
{
    name = [](Entity* entity) {
        switch (entity->getClientClass()->classId) {
        case ClassId::EconEntity: return "拆弹器";
        case ClassId::Chicken: return "鸡";
        case ClassId::PlantedC4: return "已安装的 C4 炸弹";
        case ClassId::Hostage: return "人质";
        case ClassId::Dronegun: return "自动哨兵";
        case ClassId::Cash: return "金钱";
        case ClassId::AmmoBox: return "弹药盒";
        case ClassId::RadarJammer: return "雷达干扰器";
        case ClassId::SnowballPile: return "雪球桩";
        case ClassId::DynamicProp: return "收藏币";
        default: assert(false); return "未知";
        }
    }(entity);
}

ProjectileData::ProjectileData(const ClientInterfaces& clientInterfaces, const Memory& memory, Entity* projectile) noexcept : BaseData { projectile }
{
    name = [](Entity* projectile) {
        switch (projectile->getClientClass()->classId) {
        case ClassId::BaseCSGrenadeProjectile:
            if (const auto model = projectile->getModel(); model && strstr(model->name, "flashbang"))
                return "闪光震撼弹";
            else
                return "高爆手雷";
        case ClassId::BreachChargeProjectile: return "遥控炸弹";
        case ClassId::BumpMineProjectile: return "弹射地雷";
        case ClassId::DecoyProjectile: return "诱饵手雷";
        case ClassId::MolotovProjectile: return "燃烧瓶";
        case ClassId::SensorGrenadeProjectile: return "战术探测手雷";
        case ClassId::SmokeGrenadeProjectile: return "烟雾弹";
        case ClassId::SnowballProjectile: return "雪球";
        default: assert(false); return "未知";
        }
    }(projectile);

    if (const auto thrower = clientInterfaces.getEntityList().getEntityFromHandle(projectile->thrower()); thrower && localPlayer) {
        if (thrower == localPlayer.get())
            thrownByLocalPlayer = true;
        else
            thrownByEnemy = localPlayer->isOtherEnemy(memory, thrower);
    }

    handle = projectile->handle();
}

void ProjectileData::update(const Memory& memory, Entity* projectile) noexcept
{
    static_cast<BaseData&>(*this) = { projectile };

    if (const auto& pos = projectile->getAbsOrigin(); trajectory.size() < 1 || trajectory[trajectory.size() - 1].second != pos)
        trajectory.emplace_back(memory.globalVars->realtime, pos);
}

PlayerData::PlayerData(const EngineInterfaces& engineInterfaces, const Interfaces& interfaces, const Memory& memory, Entity* entity) noexcept : BaseData{ entity }, handle{ entity->handle() }
{
    if (const auto steamID = entity->getSteamId(engineInterfaces.getEngine())) {
        const auto ctx = engineInterfaces.getEngine().getSteamAPIContext();
        const auto avatar = ctx->steamFriends->getSmallFriendAvatar(steamID);
        constexpr auto rgbaDataSize = 4 * 32 * 32;

        PlayerAvatar playerAvatar;
        playerAvatar.rgba = std::make_unique<std::uint8_t[]>(rgbaDataSize);
        if (ctx->steamUtils->getImageRGBA(avatar, playerAvatar.rgba.get(), rgbaDataSize))
            playerAvatars[handle] = std::move(playerAvatar);
    }

    update(engineInterfaces, interfaces, memory, entity);
}

void PlayerData::update(const EngineInterfaces& engineInterfaces, const Interfaces& interfaces, const Memory& memory, Entity* entity) noexcept
{
    name = entity->getPlayerName(interfaces, memory);

    dormant = entity->isDormant();
    if (dormant)
        return;

    team = entity->getTeamNumber();
    static_cast<BaseData&>(*this) = { entity };
    origin = entity->getAbsOrigin();
    inViewFrustum = !engineInterfaces.getEngine().cullBox(obbMins + origin, obbMaxs + origin);
    alive = entity->isAlive();
    lastContactTime = alive ? memory.globalVars->realtime : 0.0f;

    if (localPlayer) {
        enemy = localPlayer->isOtherEnemy(memory, entity);

        if (!inViewFrustum || !alive)
            visible = false;
        else if (shouldUpdatePlayerVisibility(memory))
            visible = entity->visibleTo(engineInterfaces, memory, localPlayer.get());
    }

    auto isEntityAudible = [&memory](int entityIndex) noexcept {
        for (int i = 0; i < memory.activeChannels->count; ++i)
            if (memory.channels[memory.activeChannels->list[i]].soundSource == entityIndex)
                return true;
        return false;
    };

    audible = isEntityAudible(entity->index());
    spotted = entity->spotted();
    health = entity->health();
    immune = entity->gunGameImmunity();
    flashDuration = entity->flashDuration();

    if (const auto weapon = entity->getActiveWeapon()) {
        audible = audible || isEntityAudible(weapon->index());
        if (const auto weaponInfo = weapon->getWeaponData())
            activeWeapon = interfaces.localize->findAsUTF8(weaponInfo->name);
    }

    if (!alive || !inViewFrustum)
        return;

    const auto model = entity->getModel();
    if (!model)
        return;

    const auto studioModel = engineInterfaces.getModelInfo().getStudioModel(model);
    if (!studioModel)
        return;

    matrix3x4 boneMatrices[MAXSTUDIOBONES];
    if (!entity->setupBones(memory, boneMatrices, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, memory.globalVars->currenttime))
        return;

    bones.clear();
    bones.reserve(20);

    for (int i = 0; i < studioModel->numBones; ++i) {
        const auto bone = studioModel->getBone(i);

        if (!bone || bone->parent == -1 || !(bone->flags & BONE_USED_BY_HITBOX))
            continue;

        bones.emplace_back(boneMatrices[i].origin(), boneMatrices[bone->parent].origin());
    }

    const auto set = studioModel->getHitboxSet(entity->hitboxSet());
    if (!set)
        return;

    const auto headBox = set->getHitbox(0);

    headMins = transform(headBox->bbMin, boneMatrices[headBox->bone]);
    headMaxs = transform(headBox->bbMax, boneMatrices[headBox->bone]);

    if (headBox->capsuleRadius > 0.0f) {
        headMins -= headBox->capsuleRadius;
        headMaxs += headBox->capsuleRadius;
    }
}

struct PNGTexture {
    template <std::size_t N>
    PNGTexture(const std::array<char, N>& png) noexcept : pngData{ png.data() }, pngDataSize{ png.size() } {}

    ImTextureID getTexture() const noexcept
    {
        if (!texture.get()) {
            int width, height;
            stbi_set_flip_vertically_on_load_thread(false);

            if (const auto data = stbi_load_from_memory((const stbi_uc*)pngData, pngDataSize, &width, &height, nullptr, STBI_rgb_alpha)) {
                texture.init(width, height, data);
                stbi_image_free(data);
            } else {
                assert(false);
            }
        }

        return texture.get();
    }

    void clearTexture() const noexcept { texture.clear(); }

private:
    const char* pngData;
    std::size_t pngDataSize;

    mutable Texture texture;
};

static const PNGTexture avatarTT{ Resource::avatar_tt };
static const PNGTexture avatarCT{ Resource::avatar_ct };

static void clearAvatarTextures() noexcept
{
    avatarTT.clearTexture();
    avatarCT.clearTexture();
}

ImTextureID PlayerData::getAvatarTexture() const noexcept
{
    const auto it = std::as_const(playerAvatars).find(handle);
    if (it == playerAvatars.cend())
        return team == csgo::Team::TT ? avatarTT.getTexture() : avatarCT.getTexture();

    const auto& avatar = it->second;
    if (!avatar.texture.get())
        avatar.texture.init(32, 32, avatar.rgba.get());
    return avatar.texture.get();
}

float PlayerData::fadingAlpha(const Memory& memory) const noexcept
{
    constexpr float fadeTime = 1.50f;
    return std::clamp(1.0f - (memory.globalVars->realtime - lastContactTime - 0.25f) / fadeTime, 0.0f, 1.0f);
}

WeaponData::WeaponData(const Interfaces& interfaces, Entity* entity) noexcept : BaseData{ entity }
{
    clip = entity->clip();
    reserveAmmo = entity->reserveAmmoCount();

    if (const auto weaponInfo = entity->getWeaponData()) {
        group = [](WeaponType type, WeaponId weaponId) {
            switch (type) {
            case WeaponType::Pistol: return "手枪";
            case WeaponType::SubMachinegun: return "微型冲锋枪";
            case WeaponType::Rifle: return "步枪";
            case WeaponType::SniperRifle: return "狙击步枪";
            case WeaponType::Shotgun: return "霰弹枪";
            case WeaponType::Machinegun: return "重型武器";
            case WeaponType::Grenade: return "手雷";
            case WeaponType::Melee: return "近战武器";
            default:
                switch (weaponId) {
                case WeaponId::C4:
                case WeaponId::Healthshot:
                case WeaponId::BumpMine:
                case WeaponId::ZoneRepulsor:
                case WeaponId::Shield:
                    return "其他";
                default: return "All";
                }
            }
        }(weaponInfo->type, entity->itemDefinitionIndex());
        name = [](WeaponId weaponId) {
            switch (weaponId) {
            default: return "All";

            case WeaponId::Glock: return "格洛克 18 型";
            case WeaponId::Hkp2000: return "P2000";
            case WeaponId::Usp_s: return "USP 消音型";
            case WeaponId::Elite: return "双持贝瑞塔";
            case WeaponId::P250: return "P250";
            case WeaponId::Tec9: return "Tec-9";
            case WeaponId::Fiveseven: return "FN57";
            case WeaponId::Cz75a: return "CZ75";
            case WeaponId::Deagle: return "沙漠之鹰";
            case WeaponId::Revolver: return "R8 左轮手枪";

            case WeaponId::Mac10: return "MAC-10";
            case WeaponId::Mp9: return "MP9";
            case WeaponId::Mp7: return "MP7";
            case WeaponId::Mp5sd: return "MP5-SD";
            case WeaponId::Ump45: return "UMP-45";
            case WeaponId::P90: return "P90";
            case WeaponId::Bizon: return "PP-野牛";

            case WeaponId::GalilAr: return "加利尔 AR";
            case WeaponId::Famas: return "法玛斯";
            case WeaponId::Ak47: return "AK-47";
            case WeaponId::M4A1: return "M4A4";
            case WeaponId::M4a1_s: return "M4A1 消音型";
            case WeaponId::Sg553: return "SG 553";
            case WeaponId::Aug: return "AUG";

            case WeaponId::Ssg08: return "SSG 08";
            case WeaponId::Awp: return "AWP";
            case WeaponId::G3SG1: return "G3SG1";
            case WeaponId::Scar20: return "SCAR-20";

            case WeaponId::Nova: return "新星";
            case WeaponId::Xm1014: return "XM1014";
            case WeaponId::Sawedoff: return "截短霰弹枪";
            case WeaponId::Mag7: return "MAG-7";

            case WeaponId::M249: return "M249";
            case WeaponId::Negev: return "内格夫";

            case WeaponId::Flashbang: return "闪光震撼弹";
            case WeaponId::HeGrenade: return "高爆手雷";
            case WeaponId::SmokeGrenade: return "烟雾弹";
            case WeaponId::Molotov: return "燃烧瓶";
            case WeaponId::Decoy: return "诱饵弹";
            case WeaponId::IncGrenade: return "燃烧弹";
            case WeaponId::TaGrenade: return "战术探测手雷";
            case WeaponId::Firebomb: return "火焰弹";
            case WeaponId::Diversion: return "干扰型武器";
            case WeaponId::FragGrenade: return "破片手雷";
            case WeaponId::Snowball: return "雪球";

            case WeaponId::Axe: return "斧头";
            case WeaponId::Hammer: return "锤子";
            case WeaponId::Spanner: return "扳手";

            case WeaponId::C4: return "C4 炸弹";
            case WeaponId::Healthshot: return "医疗针";
            case WeaponId::BumpMine: return "弹射地雷";
            case WeaponId::ZoneRepulsor: return "排斥装置";
            case WeaponId::Shield: return "防弹盾";
            }
        }(entity->itemDefinitionIndex());

        displayName = interfaces.localize->findAsUTF8(weaponInfo->name);
    }
}

LootCrateData::LootCrateData(Entity* entity) noexcept : BaseData{ entity }
{
    const auto model = entity->getModel();
    if (!model)
        return;

    name = [](const char* modelName) -> const char* {
        switch (fnv::hashRuntime(modelName)) {
        case fnv::hash("models/props_survival/cases/case_pistol.mdl"): return "手枪盒";
        case fnv::hash("models/props_survival/cases/case_light_weapon.mdl"): return "闪光盒";
        case fnv::hash("models/props_survival/cases/case_heavy_weapon.mdl"): return "重型盒";
        case fnv::hash("models/props_survival/cases/case_explosive.mdl"): return "炸药盒";
        case fnv::hash("models/props_survival/cases/case_tools.mdl"): return "工具盒";
        case fnv::hash("models/props_survival/cash/dufflebag.mdl"): return "现金行李袋";
        default: return nullptr;
        }
    }(model->name);
}

ObserverData::ObserverData(Entity* entity, Entity* obs, bool targetIsLocalPlayer) noexcept : playerHandle{ entity->handle() }, targetHandle{ obs->handle() }, targetIsLocalPlayer{ targetIsLocalPlayer } {}

void BombData::update(const Memory& memory) noexcept
{
    if (memory.plantedC4s->size > 0 && (!*memory.gameRules || (*memory.gameRules)->mapHasBombTarget())) {
        if (const auto bomb = (*memory.plantedC4s)[0]; bomb && bomb->c4Ticking()) {
            blowTime = bomb->c4BlowTime();
            timerLength = bomb->c4TimerLength();
            defuserHandle = bomb->c4Defuser();
            if (defuserHandle != -1) {
                defuseCountDown = bomb->c4DefuseCountDown();
                defuseLength = bomb->c4DefuseLength();
            }

            if (*memory.playerResource) {
                const auto& bombOrigin = bomb->origin();
                bombsite = bombOrigin.distTo((*memory.playerResource)->bombsiteCenterA()) > bombOrigin.distTo((*memory.playerResource)->bombsiteCenterB());
            }
            return;
        }
    }
    blowTime = 0.0f;
}

InfernoData::InfernoData(Entity* inferno) noexcept
{
    const auto& origin = inferno->getAbsOrigin();

    points.reserve(inferno->fireCount());
    for (int i = 0; i < inferno->fireCount(); ++i) {
        if (inferno->fireIsBurning()[i])
            points.emplace_back(inferno->fireXDelta()[i] + origin.x, inferno->fireYDelta()[i] + origin.y, inferno->fireZDelta()[i] + origin.z);
    }
}
