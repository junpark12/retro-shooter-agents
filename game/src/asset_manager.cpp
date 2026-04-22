#include "asset_manager.h"

namespace galaxy {

bool AssetManager::init(SDL_Renderer* r) {
    renderer = r;
    const int flags = IMG_Init(IMG_INIT_PNG);
    if ((flags & IMG_INIT_PNG) == 0) {
        return false;
    }

    // Player ships (Kenney)
    load(SPR_PLAYER_BAGON,  "assets/sprites/player/ship_bagon_k.png");
    load(SPR_PLAYER_DAMUL,  "assets/sprites/player/ship_damul_k.png");
    load(SPR_PLAYER_GUNEX,  "assets/sprites/player/ship_gunex_k.png");
    load(SPR_PLAYER_DAMAGE_1, "assets/sprites/player/ship_damage1.png");
    load(SPR_PLAYER_DAMAGE_2, "assets/sprites/player/ship_damage2.png");
    load(SPR_PLAYER_DAMAGE_3, "assets/sprites/player/ship_damage3.png");

    // Enemies (Kenney)
    load(SPR_ENEMY_SMALL,   "assets/sprites/enemies/enemy_small_k.png");
    load(SPR_ENEMY_MEDIUM,  "assets/sprites/enemies/enemy_medium_k.png");
    load(SPR_ENEMY_LARGE,   "assets/sprites/enemies/enemy_large_k.png");
    load(SPR_ENEMY_FAST,    "assets/sprites/enemies/enemy_fast_k.png");
    load(SPR_ENEMY_ARMORED, "assets/sprites/enemies/enemy_armored_k.png");
    load(SPR_ENEMY_UFO_BLUE, "assets/sprites/enemies/enemy_ufo_blue.png");
    load(SPR_ENEMY_UFO_RED,  "assets/sprites/enemies/enemy_ufo_red.png");

    // Bosses (200 Starships + fallback set)
    load(SPR_BOSS_1,        "assets/sprites/bosses_extra/og_001_0.png");
    load(SPR_BOSS_2,        "assets/sprites/bosses_extra/og_002_1.png");
    load(SPR_BOSS_3,        "assets/sprites/bosses_extra/Titan_0.png");
    load(SPR_BOSS_1_FB,     "assets/sprites/bosses/boss_1.png");
    load(SPR_BOSS_2_FB,     "assets/sprites/bosses/boss_2.png");
    load(SPR_BOSS_3_FB,     "assets/sprites/bosses/boss_3.png");
    load(SPR_BOSS_MISSILE,  "assets/sprites/boss_missiles/missile.png");

    // Extra enemy ship variants
    load(SPR_ENEMY_SHIP_1,  "assets/sprites/enemies/spaceShips_001.png");
    load(SPR_ENEMY_SHIP_2,  "assets/sprites/enemies/spaceShips_002.png");
    load(SPR_ENEMY_SHIP_3,  "assets/sprites/enemies/spaceShips_003.png");
    load(SPR_ENEMY_SHIP_4,  "assets/sprites/enemies/spaceShips_004.png");
    load(SPR_ENEMY_SHIP_5,  "assets/sprites/enemies/spaceShips_005.png");
    load(SPR_ENEMY_SHIP_6,  "assets/sprites/enemies/spaceShips_006.png");
    load(SPR_ENEMY_SHIP_7,  "assets/sprites/enemies/spaceShips_007.png");
    load(SPR_ENEMY_SHIP_8,  "assets/sprites/enemies/spaceShips_008.png");
    load(SPR_ENEMY_SHIP_9,  "assets/sprites/enemies/spaceShips_009.png");
    // Enemy Green variants (SMALL enemies)
    load(SPR_ENEMY_GREEN_1, "assets/sprites/enemies/enemyGreen1.png");
    load(SPR_ENEMY_GREEN_2, "assets/sprites/enemies/enemyGreen2.png");
    load(SPR_ENEMY_GREEN_3, "assets/sprites/enemies/enemyGreen3.png");
    load(SPR_ENEMY_GREEN_4, "assets/sprites/enemies/enemyGreen4.png");
    load(SPR_ENEMY_GREEN_5, "assets/sprites/enemies/enemyGreen5.png");
    // Enemy Blue variants (MEDIUM enemies)
    load(SPR_ENEMY_BLUE_1,  "assets/sprites/enemies/enemyBlue1.png");
    load(SPR_ENEMY_BLUE_2,  "assets/sprites/enemies/enemyBlue2.png");
    load(SPR_ENEMY_BLUE_3,  "assets/sprites/enemies/enemyBlue3.png");
    load(SPR_ENEMY_BLUE_4,  "assets/sprites/enemies/enemyBlue4.png");
    load(SPR_ENEMY_BLUE_5,  "assets/sprites/enemies/enemyBlue5.png");
    // Enemy Red variants (FAST enemies)
    load(SPR_ENEMY_RED_1,   "assets/sprites/enemies/enemyRed1.png");
    load(SPR_ENEMY_RED_2,   "assets/sprites/enemies/enemyRed2.png");
    load(SPR_ENEMY_RED_3,   "assets/sprites/enemies/enemyRed3.png");
    load(SPR_ENEMY_RED_4,   "assets/sprites/enemies/enemyRed4.png");
    load(SPR_ENEMY_RED_5,   "assets/sprites/enemies/enemyRed5.png");
    // Enemy Black variants (ARMORED enemies)
    load(SPR_ENEMY_BLACK_1, "assets/sprites/enemies/enemyBlack1.png");
    load(SPR_ENEMY_BLACK_2, "assets/sprites/enemies/enemyBlack2.png");
    load(SPR_ENEMY_BLACK_3, "assets/sprites/enemies/enemyBlack3.png");
    load(SPR_ENEMY_BLACK_4, "assets/sprites/enemies/enemyBlack4.png");
    load(SPR_ENEMY_BLACK_5, "assets/sprites/enemies/enemyBlack5.png");
    // Missile projectile sprites
    load(SPR_MISSILE_1, "assets/sprites/enemies/spaceMissiles_001.png");
    load(SPR_MISSILE_2, "assets/sprites/enemies/spaceMissiles_010.png");
    load(SPR_MISSILE_3, "assets/sprites/enemies/spaceMissiles_020.png");
    load(SPR_MISSILE_4, "assets/sprites/enemies/spaceMissiles_030.png");
    load(SPR_MISSILE_5, "assets/sprites/enemies/spaceMissiles_040.png");

    // Bullets / Lasers (Kenney)
    load(SPR_BULLET_PLAYER,      "assets/sprites/bullets/laser_player.png");
    load(SPR_BULLET_PLAYER_WIDE, "assets/sprites/bullets/laser_player_wide.png");
    load(SPR_BULLET_ENEMY,       "assets/sprites/bullets/laser_enemy.png");
    load(SPR_BULLET_LASER,       "assets/sprites/bullets/laser_weapon.png");
    load(SPR_BULLET_STRIP_BLUE,   "assets/sprites/bullets/spr_bullet_strip.png");
    load(SPR_BULLET_STRIP_GREEN,  "assets/sprites/bullets/spr_bullet_strip02.png");
    load(SPR_BULLET_STRIP_YELLOW, "assets/sprites/bullets/spr_bullet_strip03.png");
    load(SPR_BULLET_STRIP_RED,    "assets/sprites/bullets/spr_bullet_strip04.png");
    load(SPR_LASER_BLUE_1,  "assets/sprites/bullets/laserBlue01.png");
    load(SPR_LASER_BLUE_9,  "assets/sprites/bullets/laserBlue09.png");
    load(SPR_LASER_GREEN_1, "assets/sprites/bullets/laserGreen01.png");
    load(SPR_LASER_GREEN_9, "assets/sprites/bullets/laserGreen09.png");
    load(SPR_LASER_RED_1,   "assets/sprites/bullets/laserRed01.png");
    load(SPR_LASER_RED_9,   "assets/sprites/bullets/laserRed09.png");

    // Explosions
    load(SPR_EXPLOSION_0, "assets/sprites/effects/explosion_0.png");
    load(SPR_EXPLOSION_1, "assets/sprites/effects/explosion_1.png");
    load(SPR_EXPLOSION_2, "assets/sprites/effects/explosion_2.png");
    load(SPR_EXPLOSION_3, "assets/sprites/effects/explosion_3.png");
    load(SPR_EXPLOSION_SHEET, "assets/sprites/effects/M484ExplosionSet1.png");

    // Engine fire animation frames (Kenney)
    load(SPR_ENGINE_FIRE_0, "assets/sprites/effects/engine_fire00.png");
    load(SPR_ENGINE_FIRE_1, "assets/sprites/effects/engine_fire01.png");
    load(SPR_ENGINE_FIRE_2, "assets/sprites/effects/engine_fire02.png");
    load(SPR_ENGINE_FIRE_3, "assets/sprites/effects/engine_fire03.png");
    load(SPR_ENGINE_FIRE_4, "assets/sprites/effects/engine_fire04.png");
    load(SPR_ENGINE_FIRE_5, "assets/sprites/effects/engine_fire05.png");
    load(SPR_ENGINE_FIRE_6, "assets/sprites/effects/engine_fire06.png");
    load(SPR_ENGINE_FIRE_7, "assets/sprites/effects/engine_fire07.png");

    // Meteors
    load(SPR_METEOR_BROWN_BIG,  "assets/sprites/effects/meteor_brown_big.png");
    load(SPR_METEOR_BROWN_MED,  "assets/sprites/effects/meteor_brown_med.png");
    load(SPR_METEOR_GREY_BIG,   "assets/sprites/effects/meteor_grey_big.png");
    load(SPR_METEOR_GREY_SMALL, "assets/sprites/effects/meteor_grey_small.png");

    // Power-ups (Kenney)
    load(SPR_POWERUP_SPREAD,  "assets/sprites/powerups/powerup_spread_k.png");
    load(SPR_POWERUP_LASER,   "assets/sprites/powerups/powerup_laser_k.png");
    load(SPR_POWERUP_MISSILE, "assets/sprites/powerups/powerup_missile_k.png");
    load(SPR_POWERUP_SHIELD,  "assets/sprites/powerups/powerup_shield_k.png");
    load(SPR_POWERUP_BOMB,    "assets/sprites/powerups/powerup_bomb_k.png");
    load(SPR_POWERUP_POWER,   "assets/sprites/powerups/powerup_power_k.png");
    load(SPR_POWERUP_SPEEDUP, "assets/sprites/powerups/speed.png");

    // Background + UI
    load(SPR_BG_SPACE,     "assets/sprites/backgrounds/space_bg.png");
    load(SPR_UI_LIFE,      "assets/sprites/ui/life_icon.png");
    load(SPR_UI_BOMB,      "assets/sprites/ui/bomb_icon.png");
    load(SPR_UI_POWER_BAR, "assets/sprites/ui/power_bar.png");

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
