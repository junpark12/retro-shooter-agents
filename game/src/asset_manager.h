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
// Player ships
constexpr const char* SPR_PLAYER_BAGON  = "player/ship_bagon";
constexpr const char* SPR_PLAYER_DAMUL  = "player/ship_damul";
constexpr const char* SPR_PLAYER_GUNEX  = "player/ship_gunex";

// Enemies
constexpr const char* SPR_ENEMY_SMALL   = "enemies/enemy_small";
constexpr const char* SPR_ENEMY_MEDIUM  = "enemies/enemy_medium";
constexpr const char* SPR_ENEMY_LARGE   = "enemies/enemy_large";
constexpr const char* SPR_ENEMY_FAST    = "enemies/enemy_fast";
constexpr const char* SPR_ENEMY_ARMORED = "enemies/enemy_armored";

// Bosses
constexpr const char* SPR_BOSS_1        = "bosses/boss_1";
constexpr const char* SPR_BOSS_2        = "bosses/boss_2";
constexpr const char* SPR_BOSS_3        = "bosses/boss_3";

// Bullets
constexpr const char* SPR_BULLET_PLAYER = "bullets/bullet_player";
constexpr const char* SPR_BULLET_ENEMY  = "bullets/bullet_enemy";
constexpr const char* SPR_BULLET_LASER  = "bullets/bullet_laser";

// Effects
constexpr const char* SPR_EXPLOSION_0   = "effects/explosion_0";
constexpr const char* SPR_EXPLOSION_1   = "effects/explosion_1";
constexpr const char* SPR_EXPLOSION_2   = "effects/explosion_2";
constexpr const char* SPR_EXPLOSION_3   = "effects/explosion_3";

// Power-ups
constexpr const char* SPR_POWERUP_SPREAD  = "powerups/powerup_spread";
constexpr const char* SPR_POWERUP_LASER   = "powerups/powerup_laser";
constexpr const char* SPR_POWERUP_MISSILE = "powerups/powerup_missile";
constexpr const char* SPR_POWERUP_SHIELD  = "powerups/powerup_shield";
constexpr const char* SPR_POWERUP_BOMB    = "powerups/powerup_bomb";
constexpr const char* SPR_POWERUP_POWER   = "powerups/powerup_power";

// Backgrounds
constexpr const char* SPR_BG_SPACE        = "backgrounds/space_bg";

// UI
constexpr const char* SPR_UI_LIFE         = "ui/life_icon";
constexpr const char* SPR_UI_BOMB         = "ui/bomb_icon";
constexpr const char* SPR_UI_POWER_BAR    = "ui/power_bar";

} // namespace galaxy
