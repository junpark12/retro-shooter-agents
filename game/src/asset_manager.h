#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>

namespace galaxy {

// ─── AssetManager ─────────────────────────────────────────────────────────────
// Loads and caches SDL_Texture objects from the assets/sprites/ directory.
// Falls back gracefully if a file is not found (returns nullptr and renders
// primitive shapes instead).
//
// Usage:
//   AssetManager am;
//   am.init(renderer);
//   SDL_Texture* tex = am.get("player/ship_bagon");
//   am.shutdown();

struct AssetManager {
    SDL_Renderer* renderer = nullptr;
    std::unordered_map<std::string, SDL_Texture*> cache;

    // Initialise with a renderer reference and pre-load all game sprites.
    // Returns false if SDL2_image could not be initialised.
    bool init(SDL_Renderer* r);

    // Retrieve a cached texture by logical key (e.g., "player/ship_bagon").
    // Returns nullptr if the asset was not found or failed to load.
    SDL_Texture* get(const std::string& key) const;

    // Load a PNG file from disk and cache it under the given key.
    // Idempotent: calling twice with the same key is a no-op.
    SDL_Texture* load(const std::string& key, const std::string& path);

    // Release all textures and shut down SDL2_image.
    void shutdown();
};

// ─── Sprite key constants ─────────────────────────────────────────────────────
// Player ships (Kenney assets)
constexpr const char* SPR_PLAYER_BAGON  = "player/ship_bagon_k";
constexpr const char* SPR_PLAYER_DAMUL  = "player/ship_damul_k";
constexpr const char* SPR_PLAYER_GUNEX  = "player/ship_gunex_k";
// Player damage states
constexpr const char* SPR_PLAYER_DAMAGE_1 = "player/ship_damage1";
constexpr const char* SPR_PLAYER_DAMAGE_2 = "player/ship_damage2";
constexpr const char* SPR_PLAYER_DAMAGE_3 = "player/ship_damage3";

// Enemies (Kenney assets)
constexpr const char* SPR_ENEMY_SMALL   = "enemies/enemy_small_k";
constexpr const char* SPR_ENEMY_MEDIUM  = "enemies/enemy_medium_k";
constexpr const char* SPR_ENEMY_LARGE   = "enemies/enemy_large_k";
constexpr const char* SPR_ENEMY_FAST    = "enemies/enemy_fast_k";
constexpr const char* SPR_ENEMY_ARMORED = "enemies/enemy_armored_k";
constexpr const char* SPR_ENEMY_UFO_BLUE = "enemies/enemy_ufo_blue";
constexpr const char* SPR_ENEMY_UFO_RED  = "enemies/enemy_ufo_red";
// Extra enemy ship variants (Kenney Space Shooter Extension)
constexpr const char* SPR_ENEMY_SHIP_1  = "enemies/spaceShips_001";
constexpr const char* SPR_ENEMY_SHIP_2  = "enemies/spaceShips_002";
constexpr const char* SPR_ENEMY_SHIP_3  = "enemies/spaceShips_003";
constexpr const char* SPR_ENEMY_SHIP_4  = "enemies/spaceShips_004";
constexpr const char* SPR_ENEMY_SHIP_5  = "enemies/spaceShips_005";
constexpr const char* SPR_ENEMY_SHIP_6  = "enemies/spaceShips_006";
constexpr const char* SPR_ENEMY_SHIP_7  = "enemies/spaceShips_007";
constexpr const char* SPR_ENEMY_SHIP_8  = "enemies/spaceShips_008";
constexpr const char* SPR_ENEMY_SHIP_9  = "enemies/spaceShips_009";
// Enemy color variants (Kenney Space Shooter Redux — 5 per color)
// Green variants → SMALL enemy
constexpr const char* SPR_ENEMY_GREEN_1 = "enemies/enemyGreen1";
constexpr const char* SPR_ENEMY_GREEN_2 = "enemies/enemyGreen2";
constexpr const char* SPR_ENEMY_GREEN_3 = "enemies/enemyGreen3";
constexpr const char* SPR_ENEMY_GREEN_4 = "enemies/enemyGreen4";
constexpr const char* SPR_ENEMY_GREEN_5 = "enemies/enemyGreen5";
// Blue variants → MEDIUM enemy
constexpr const char* SPR_ENEMY_BLUE_1  = "enemies/enemyBlue1";
constexpr const char* SPR_ENEMY_BLUE_2  = "enemies/enemyBlue2";
constexpr const char* SPR_ENEMY_BLUE_3  = "enemies/enemyBlue3";
constexpr const char* SPR_ENEMY_BLUE_4  = "enemies/enemyBlue4";
constexpr const char* SPR_ENEMY_BLUE_5  = "enemies/enemyBlue5";
// Red variants → FAST enemy
constexpr const char* SPR_ENEMY_RED_1   = "enemies/enemyRed1";
constexpr const char* SPR_ENEMY_RED_2   = "enemies/enemyRed2";
constexpr const char* SPR_ENEMY_RED_3   = "enemies/enemyRed3";
constexpr const char* SPR_ENEMY_RED_4   = "enemies/enemyRed4";
constexpr const char* SPR_ENEMY_RED_5   = "enemies/enemyRed5";
// Black variants → ARMORED enemy
constexpr const char* SPR_ENEMY_BLACK_1 = "enemies/enemyBlack1";
constexpr const char* SPR_ENEMY_BLACK_2 = "enemies/enemyBlack2";
constexpr const char* SPR_ENEMY_BLACK_3 = "enemies/enemyBlack3";
constexpr const char* SPR_ENEMY_BLACK_4 = "enemies/enemyBlack4";
constexpr const char* SPR_ENEMY_BLACK_5 = "enemies/enemyBlack5";
// Missile projectile sprites (spaceMissiles — used for MISSILE bullet type)
constexpr const char* SPR_MISSILE_1     = "enemies/spaceMissiles_001";
constexpr const char* SPR_MISSILE_2     = "enemies/spaceMissiles_010";
constexpr const char* SPR_MISSILE_3     = "enemies/spaceMissiles_020";
constexpr const char* SPR_MISSILE_4     = "enemies/spaceMissiles_030";
constexpr const char* SPR_MISSILE_5     = "enemies/spaceMissiles_040";

// Bosses (200 Starships / Wisedawn CC0 — large-scale boss candidates)
constexpr const char* SPR_BOSS_1        = "bosses/ship_20";
constexpr const char* SPR_BOSS_2        = "bosses/ship_80";
constexpr const char* SPR_BOSS_3        = "bosses/ship_180";
// Fallback boss sprites (procedural, Python/Pillow)
constexpr const char* SPR_BOSS_1_FB     = "bosses/boss_1";
constexpr const char* SPR_BOSS_2_FB     = "bosses/boss_2";
constexpr const char* SPR_BOSS_3_FB     = "bosses/boss_3";
// Boss missile sprite (boss_missiles directory)
constexpr const char* SPR_BOSS_MISSILE  = "boss_missiles/missile";

// Bullets / Lasers (Kenney assets)
constexpr const char* SPR_BULLET_PLAYER = "bullets/laser_player";
constexpr const char* SPR_BULLET_ENEMY  = "bullets/laser_enemy";
constexpr const char* SPR_BULLET_LASER  = "bullets/laser_weapon";
constexpr const char* SPR_BULLET_PLAYER_WIDE = "bullets/laser_player_wide";
// Bullet-strip sprites (Bonsaiheldin / CC0): each strip is a single glow-orb bullet.
// spr_bullet_strip  → blue/violet glow
// spr_bullet_strip02 → green glow
// spr_bullet_strip03 → yellow/gold glow
// spr_bullet_strip04 → red/orange glow (if different)
constexpr const char* SPR_BULLET_STRIP_BLUE   = "bullets/spr_bullet_strip";
constexpr const char* SPR_BULLET_STRIP_GREEN  = "bullets/spr_bullet_strip02";
constexpr const char* SPR_BULLET_STRIP_YELLOW = "bullets/spr_bullet_strip03";
constexpr const char* SPR_BULLET_STRIP_RED    = "bullets/spr_bullet_strip04";
// Kenney Space Shooter Redux laser sprites (varied color lasers for enemy/player bullets)
constexpr const char* SPR_LASER_BLUE_1   = "bullets/laserBlue01";
constexpr const char* SPR_LASER_BLUE_9   = "bullets/laserBlue09";
constexpr const char* SPR_LASER_GREEN_1  = "bullets/laserGreen01";
constexpr const char* SPR_LASER_GREEN_9  = "bullets/laserGreen09";
constexpr const char* SPR_LASER_RED_1    = "bullets/laserRed01";
constexpr const char* SPR_LASER_RED_9    = "bullets/laserRed09";

// Effects
constexpr const char* SPR_EXPLOSION_0   = "effects/explosion_0";
constexpr const char* SPR_EXPLOSION_1   = "effects/explosion_1";
constexpr const char* SPR_EXPLOSION_2   = "effects/explosion_2";
constexpr const char* SPR_EXPLOSION_3   = "effects/explosion_3";
// M484ExplosionSet1 sprite sheet (Master484 / CC0).
// Layout: 3 sections wide × 3 sections tall; each section is one animation type.
//   Columns: yellow/orange (0), blue/purple (1), red/pink (2)
//   Rows:    large-ring (0), medium (1), small (2)
// Frame size: 48×48 px; 8 frames per animation row.
constexpr const char* SPR_EXPLOSION_SHEET  = "effects/M484ExplosionSet1";
// Engine exhaust fire animation frames (8 frames, index 0-7)
constexpr const char* SPR_ENGINE_FIRE_0 = "effects/engine_fire00";
constexpr const char* SPR_ENGINE_FIRE_1 = "effects/engine_fire01";
constexpr const char* SPR_ENGINE_FIRE_2 = "effects/engine_fire02";
constexpr const char* SPR_ENGINE_FIRE_3 = "effects/engine_fire03";
constexpr const char* SPR_ENGINE_FIRE_4 = "effects/engine_fire04";
constexpr const char* SPR_ENGINE_FIRE_5 = "effects/engine_fire05";
constexpr const char* SPR_ENGINE_FIRE_6 = "effects/engine_fire06";
constexpr const char* SPR_ENGINE_FIRE_7 = "effects/engine_fire07";
// Meteor obstacles
constexpr const char* SPR_METEOR_BROWN_BIG  = "effects/meteor_brown_big";
constexpr const char* SPR_METEOR_BROWN_MED  = "effects/meteor_brown_med";
constexpr const char* SPR_METEOR_GREY_BIG   = "effects/meteor_grey_big";
constexpr const char* SPR_METEOR_GREY_SMALL = "effects/meteor_grey_small";

// Power-ups (Kenney assets)
constexpr const char* SPR_POWERUP_SPREAD  = "powerups/powerup_spread_k";
constexpr const char* SPR_POWERUP_LASER   = "powerups/powerup_laser_k";
constexpr const char* SPR_POWERUP_MISSILE = "powerups/powerup_missile_k";
constexpr const char* SPR_POWERUP_SHIELD  = "powerups/powerup_shield_k";
constexpr const char* SPR_POWERUP_BOMB    = "powerups/powerup_bomb_k";
constexpr const char* SPR_POWERUP_POWER   = "powerups/powerup_power_k";
constexpr const char* SPR_POWERUP_SPEEDUP = "powerups/speed";

// Backgrounds
constexpr const char* SPR_BG_SPACE        = "backgrounds/space_bg";

// UI
constexpr const char* SPR_UI_LIFE         = "ui/life_icon";
constexpr const char* SPR_UI_BOMB         = "ui/bomb_icon";
constexpr const char* SPR_UI_POWER_BAR    = "ui/power_bar";

} // namespace galaxy
