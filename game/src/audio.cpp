#include "audio.h"

#include <algorithm>

namespace galaxy {

bool AudioManager::init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        audioAvailable = false;
        return false;
    }

    audioAvailable = true;
    loadMusic(BGM_TITLE,    "assets/bgm/bgm_title.ogg");
    loadMusic(BGM_STAGE_1,  "assets/bgm/bgm_stage1.ogg");
    loadMusic(BGM_STAGE_2,  "assets/bgm/bgm_stage2.ogg");
    loadMusic(BGM_STAGE_3,  "assets/bgm/bgm_stage3.ogg");
    loadMusic(BGM_BOSS,     "assets/bgm/bgm_boss.ogg");
    loadMusic(BGM_GAMEOVER, "assets/bgm/bgm_gameover.ogg");
    loadMusic(BGM_VICTORY,  "assets/bgm/bgm_victory.ogg");

    loadSound(SFX_SHOOT,         "assets/sfx/sfx_shoot.wav");
    loadSound(SFX_SHOOT_LASER,   "assets/sfx/sfx_shoot_laser.wav");
    loadSound(SFX_SHOOT_MISSILE, "assets/sfx/sfx_shoot_missile.wav");
    loadSound(SFX_EXPLODE_SMALL, "assets/sfx/sfx_explode_small.wav");
    loadSound(SFX_EXPLODE_BIG,   "assets/sfx/sfx_explode_big.wav");
    loadSound(SFX_POWERUP,       "assets/sfx/sfx_powerup.wav");
    loadSound(SFX_PLAYER_HIT,    "assets/sfx/sfx_player_hit.wav");
    loadSound(SFX_BOMB,          "assets/sfx/sfx_bomb.wav");
    loadSound(SFX_LOCKON,        "assets/sfx/sfx_lockon.wav");
    loadSound(SFX_CHARGE,        "assets/sfx/sfx_charge.wav");
    loadSound(SFX_1UP,           "assets/sfx/sfx_1up.wav");
    loadSound(SFX_MENU_MOVE,     "assets/sfx/sfx_menu_move.wav");
    loadSound(SFX_MENU_SELECT,   "assets/sfx/sfx_menu_select.wav");

    setBGMVolume(bgmVolume);
    setSFXVolume(sfxVolume);
    return true;
}

void AudioManager::playBGM(const std::string& key) {
    if (!audioAvailable) return;
    auto it = music.find(key);
    if (it == music.end() || !it->second) return;
    Mix_VolumeMusic(bgmVolume);
    Mix_FadeInMusic(it->second, -1, 1000);
}

void AudioManager::stopBGM() {
    if (!audioAvailable) return;
    Mix_FadeOutMusic(500);
}

void AudioManager::playSFX(const std::string& key) {
    if (!audioAvailable) return;
    auto it = sounds.find(key);
    if (it == sounds.end() || !it->second) return;
    Mix_VolumeChunk(it->second, sfxVolume);
    Mix_PlayChannel(-1, it->second, 0);
}

void AudioManager::loadMusic(const std::string& key, const std::string& path) {
    if (!audioAvailable) return;
    if (music.find(key) != music.end()) return;
    music[key] = Mix_LoadMUS(path.c_str());
}

void AudioManager::loadSound(const std::string& key, const std::string& path) {
    if (!audioAvailable) return;
    if (sounds.find(key) != sounds.end()) return;
    sounds[key] = Mix_LoadWAV(path.c_str());
}

void AudioManager::setBGMVolume(int vol) {
    bgmVolume = std::clamp(vol, 0, 128);
    if (audioAvailable) Mix_VolumeMusic(bgmVolume);
}

void AudioManager::setSFXVolume(int vol) {
    sfxVolume = std::clamp(vol, 0, 128);
    if (!audioAvailable) return;
    for (auto& kv : sounds) {
        if (kv.second) Mix_VolumeChunk(kv.second, sfxVolume);
    }
}

void AudioManager::shutdown() {
    for (auto& kv : music) {
        if (kv.second) Mix_FreeMusic(kv.second);
    }
    music.clear();

    for (auto& kv : sounds) {
        if (kv.second) Mix_FreeChunk(kv.second);
    }
    sounds.clear();

    if (audioAvailable) {
        Mix_CloseAudio();
    }
    audioAvailable = false;
}

} // namespace galaxy
