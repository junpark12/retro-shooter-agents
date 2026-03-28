#include "asset_manager.h"

namespace galaxy {

bool AssetManager::init(SDL_Renderer* r) {
    renderer = r;
    const int flags = IMG_Init(IMG_INIT_PNG);
    if ((flags & IMG_INIT_PNG) == 0) {
        return false;
    }

    load(SPR_PLAYER_BAGON,  "assets/sprites/player/ship_bagon.png");
    load(SPR_PLAYER_DAMUL,  "assets/sprites/player/ship_damul.png");
    load(SPR_PLAYER_GUNEX,  "assets/sprites/player/ship_gunex.png");
    load(SPR_ENEMY_SMALL,   "assets/sprites/enemies/enemy_small.png");
    load(SPR_ENEMY_MEDIUM,  "assets/sprites/enemies/enemy_medium.png");
    load(SPR_ENEMY_LARGE,   "assets/sprites/enemies/enemy_large.png");
    load(SPR_ENEMY_FAST,    "assets/sprites/enemies/enemy_fast.png");
    load(SPR_ENEMY_ARMORED, "assets/sprites/enemies/enemy_armored.png");
    load(SPR_BOSS_1,        "assets/sprites/bosses/boss_1.png");
    load(SPR_BOSS_2,        "assets/sprites/bosses/boss_2.png");
    load(SPR_BOSS_3,        "assets/sprites/bosses/boss_3.png");
    load(SPR_BULLET_PLAYER, "assets/sprites/bullets/bullet_player.png");
    load(SPR_BULLET_ENEMY,  "assets/sprites/bullets/bullet_enemy.png");
    load(SPR_BULLET_LASER,  "assets/sprites/bullets/bullet_laser.png");
    load(SPR_POWERUP_SPREAD,  "assets/sprites/powerups/powerup_spread.png");
    load(SPR_POWERUP_LASER,   "assets/sprites/powerups/powerup_laser.png");
    load(SPR_POWERUP_MISSILE, "assets/sprites/powerups/powerup_missile.png");
    load(SPR_POWERUP_SHIELD,  "assets/sprites/powerups/powerup_shield.png");
    load(SPR_POWERUP_BOMB,    "assets/sprites/powerups/powerup_bomb.png");
    load(SPR_POWERUP_POWER,   "assets/sprites/powerups/powerup_power.png");
    load(SPR_BG_SPACE,         "assets/sprites/backgrounds/space_bg.png");
    load(SPR_UI_LIFE,          "assets/sprites/ui/life_icon.png");
    load(SPR_UI_BOMB,          "assets/sprites/ui/bomb_icon.png");
    load(SPR_UI_POWER_BAR,     "assets/sprites/ui/power_bar.png");

    return true;
}

SDL_Texture* AssetManager::get(const std::string& key) const {
    auto it = cache.find(key);
    if (it == cache.end()) return nullptr;
    return it->second;
}

SDL_Texture* AssetManager::load(const std::string& key, const std::string& path) {
    if (!renderer) return nullptr;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;

    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        cache[key] = nullptr;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    cache[key] = tex;
    return tex;
}

void AssetManager::shutdown() {
    for (auto& kv : cache) {
        if (kv.second) {
            SDL_DestroyTexture(kv.second);
        }
    }
    cache.clear();
    IMG_Quit();
    renderer = nullptr;
}

} // namespace galaxy
