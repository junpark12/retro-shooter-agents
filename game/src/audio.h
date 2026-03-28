#pragma once
#include <SDL_mixer.h>
#include <string>
#include <unordered_map>

namespace galaxy {

// ─── AudioManager ─────────────────────────────────────────────────────────────
// Wraps SDL2_mixer for BGM (Mix_Music) and SFX (Mix_Chunk) playback.
// Falls back silently when audio files are missing.
//
// Usage:
//   AudioManager am;
//   am.init();
//   am.playBGM("bgm_title");
//   am.playSFX("sfx_shoot");
//   am.shutdown();

struct AudioManager {
    std::unordered_map<std::string, Mix_Music*> music;
    std::unordered_map<std::string, Mix_Chunk*> sounds;
    bool audioAvailable = false;
    int  bgmVolume      = 80;  // 0–128
    int  sfxVolume      = 100; // 0–128

    // Initialise SDL2_mixer and pre-load all audio assets.
    // Returns false if mixer could not be opened (audio disabled, game still runs).
    bool init();

    // Play BGM by key (loops indefinitely). Fades in over 1 second.
    // If the key is not found, currently playing BGM continues.
    void playBGM(const std::string& key);

    // Stop the currently playing BGM (fade out over 0.5 seconds).
    void stopBGM();

    // Play a sound effect by key on any available channel.
    // Silently ignored if the key is not found.
    void playSFX(const std::string& key);

    // Load a music file (OGG/WAV) and cache it under key.
    void loadMusic(const std::string& key, const std::string& path);

    // Load a sound effect (WAV) and cache it under key.
    void loadSound(const std::string& key, const std::string& path);

    // Set BGM volume (0–128).
    void setBGMVolume(int vol);

    // Set SFX volume (0–128).
    void setSFXVolume(int vol);

    // Release all audio resources and shut down SDL2_mixer.
    void shutdown();
};

// ─── Audio key constants ──────────────────────────────────────────────────────
// BGM
constexpr const char* BGM_TITLE       = "bgm_title";
constexpr const char* BGM_STAGE_1     = "bgm_stage1";
constexpr const char* BGM_STAGE_2     = "bgm_stage2";
constexpr const char* BGM_STAGE_3     = "bgm_stage3";
constexpr const char* BGM_BOSS        = "bgm_boss";
constexpr const char* BGM_GAMEOVER    = "bgm_gameover";
constexpr const char* BGM_VICTORY     = "bgm_victory";

// SFX
constexpr const char* SFX_SHOOT       = "sfx_shoot";
constexpr const char* SFX_SHOOT_LASER = "sfx_shoot_laser";
constexpr const char* SFX_SHOOT_MISSILE = "sfx_shoot_missile";
constexpr const char* SFX_EXPLODE_SMALL = "sfx_explode_small";
constexpr const char* SFX_EXPLODE_BIG   = "sfx_explode_big";
constexpr const char* SFX_POWERUP       = "sfx_powerup";
constexpr const char* SFX_PLAYER_HIT    = "sfx_player_hit";
constexpr const char* SFX_BOMB          = "sfx_bomb";
constexpr const char* SFX_LOCKON        = "sfx_lockon";
constexpr const char* SFX_CHARGE        = "sfx_charge";
constexpr const char* SFX_1UP           = "sfx_1up";
constexpr const char* SFX_MENU_MOVE     = "sfx_menu_move";
constexpr const char* SFX_MENU_SELECT   = "sfx_menu_select";

} // namespace galaxy
