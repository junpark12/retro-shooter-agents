#include "game.h"

#include "asset_manager.h"
#include "audio.h"
#include "background.h"
#include "boss.h"
#include "bullet.h"
#include "collision.h"
#include "enemy.h"
#include "hud.h"
#include "menu.h"
#include "player.h"
#include "powerup.h"
#include "stage.h"

#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <cstdlib>
#include <ctime>
#include <string>

namespace galaxy {

namespace {
constexpr float STAGE_CLEAR_DURATION = 2.5f;

const char* stageBgmKey(int stageNum) {
    switch (stageNum) {
        case 1: return BGM_STAGE_1;
        case 2: return BGM_STAGE_2;
        default: return BGM_STAGE_3;
    }
}
} // namespace

Game::Game() = default;
Game::~Game() = default;

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
        return false;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        SDL_Quit();
        return false;
    }

    if (TTF_Init() != 0) {
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    // Mixer init is optional for gameplay; manager handles fallback.
    Mix_Init(0);

    window_ = SDL_CreateWindow("Galaxy Storm",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    if (!window_) {
        shutdown();
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        shutdown();
        return false;
    }

    font_ = TTF_OpenFont("assets/fonts/PressStart2P-Regular.ttf", 12);
    if (!font_) {
        font_ = TTF_OpenFont("game/assets/fonts/PressStart2P-Regular.ttf", 12);
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    player_ = new Player();
    bullets_ = new BulletPool();
    enemies_ = new EnemyPool();
    boss_ = new Boss();
    powerUps_ = new PowerUpPool();
    stage_ = new Stage();
    background_ = new Background();
    menu_ = new Menu();
    shipSelect_ = new ShipSelect();
    assets_ = new AssetManager();
    audio_ = new AudioManager();

    assets_->init(renderer_);
    audio_->init();
    initBackground(*background_);
    initPlayer(*player_, selectedShip_);

    for (auto& b : bullets_->pool) b.active = false;
    for (auto& e : enemies_->pool) e.active = false;
    for (auto& p : powerUps_->pool) p.active = false;
    boss_->active = false;

    state_ = GameState::TITLE;
    running_ = true;
    stageNum_ = 1;
    bossMusicPlaying_ = false;
    return true;
}

void Game::run() {
    if (!renderer_) return;

    Uint32 previous = SDL_GetTicks();
    float accumulatorMs = 0.0f;
    float stageClearTimer = 0.0f;

    while (running_) {
        const Uint32 now = SDL_GetTicks();
        const Uint32 elapsed = now - previous;
        previous = now;
        accumulatorMs += static_cast<float>(elapsed);

        handleEvents();

        while (accumulatorMs >= FRAME_MS) {
            const float dt = 1.0f / static_cast<float>(FPS);
            update(dt);

            if (state_ == GameState::STAGE_CLEAR) {
                stageClearTimer += dt;
                if (stageClearTimer >= STAGE_CLEAR_DURATION) {
                    stageClearTimer = 0.0f;
                    startStage(stageNum_);
                    state_ = GameState::PLAYING;
                }
            } else {
                stageClearTimer = 0.0f;
            }

            accumulatorMs -= FRAME_MS;
        }

        render();

        const Uint32 frameTime = SDL_GetTicks() - now;
        if (frameTime < static_cast<Uint32>(FRAME_MS)) {
            SDL_Delay(static_cast<Uint32>(FRAME_MS) - frameTime);
        }
    }
}

void Game::shutdown() {
    if (audio_) {
        audio_->shutdown();
        delete audio_;
        audio_ = nullptr;
    }
    if (assets_) {
        assets_->shutdown();
        delete assets_;
        assets_ = nullptr;
    }

    delete shipSelect_; shipSelect_ = nullptr;
    delete menu_; menu_ = nullptr;
    delete background_; background_ = nullptr;
    delete stage_; stage_ = nullptr;
    delete powerUps_; powerUps_ = nullptr;
    delete boss_; boss_ = nullptr;
    delete enemies_; enemies_ = nullptr;
    delete bullets_; bullets_ = nullptr;
    delete player_; player_ = nullptr;

    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running_ = false;
            return;
        }

        if (state_ == GameState::TITLE) {
            GameState next = handleMenuEvent(*menu_, e);
            if (next == GameState::SHIP_SELECT) {
                state_ = GameState::SHIP_SELECT;
            } else if (next == GameState::QUIT) {
                running_ = false;
                return;
            }
        } else if (state_ == GameState::SHIP_SELECT) {
            ShipType outShip = selectedShip_;
            GameState next = handleShipSelectEvent(*shipSelect_, e, outShip);
            if (next == GameState::PLAYING) {
                selectedShip_ = outShip;
                initPlayer(*player_, selectedShip_);
                stageNum_ = 1;
                startStage(stageNum_);
                state_ = GameState::PLAYING;
            } else if (next == GameState::TITLE) {
                state_ = GameState::TITLE;
            }
        } else if (state_ == GameState::GAMEOVER || state_ == GameState::VICTORY) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                state_ = GameState::TITLE;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                running_ = false;
            }
        }
    }
}

void Game::update(float dt) {
    updateBackground(*background_, dt);

    if (state_ == GameState::TITLE) {
        updateMenu(*menu_, dt);
        return;
    }
    if (state_ == GameState::SHIP_SELECT) {
        updateShipSelect(*shipSelect_, dt);
        return;
    }
    if (state_ != GameState::PLAYING) {
        return;
    }

    updatePlayer(*player_, dt, *bullets_, *enemies_);
    updateStage(*stage_, dt, *enemies_, *boss_);
    updateEnemies(*enemies_, dt, *bullets_, player_->center());
    updateBoss(*boss_, dt, *bullets_, player_->center());
    updateBullets(*bullets_, dt);
    updatePowerUps(*powerUps_, dt);

    checkBulletEnemyCollision(*bullets_, *enemies_, *player_, *powerUps_, audio_);
    checkBulletPlayerCollision(*bullets_, *player_, audio_);
    checkPlayerEnemyCollision(*player_, *enemies_, audio_);
    checkBulletBossCollision(*bullets_, *boss_, *player_, audio_);
    checkPowerUpPickup(*player_, *powerUps_, audio_);

    if (player_->score >= player_->nextLifeScore) {
        player_->lives++;
        player_->nextLifeScore += SCORE_PER_EXTRA_LIFE;
        if (audio_) audio_->playSFX(SFX_1UP);
    }

    if (!bossMusicPlaying_ && boss_->active) {
        if (audio_) audio_->playBGM(BGM_BOSS);
        bossMusicPlaying_ = true;
    }

    if (!player_->active || player_->lives <= 0) {
        onGameOver();
        return;
    }
    if (stage_->stageCleared) {
        onStageClear();
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    renderBackground(renderer_, *assets_, *background_);

    if (state_ == GameState::TITLE) {
        renderMenu(renderer_, *assets_, font_, *menu_);
        SDL_RenderPresent(renderer_);
        return;
    }
    if (state_ == GameState::SHIP_SELECT) {
        renderShipSelect(renderer_, *assets_, font_, *shipSelect_);
        SDL_RenderPresent(renderer_);
        return;
    }

    renderEnemies(renderer_, *enemies_);
    renderBoss(renderer_, *boss_);
    renderPowerUps(renderer_, *assets_, *powerUps_);
    renderPlayer(renderer_, *player_);
    renderBullets(renderer_, *bullets_);

    if (state_ == GameState::PLAYING) {
        renderHUD(renderer_, *assets_, font_, *player_, stageNum_);
        if (boss_->active) {
            renderBossHP(renderer_, font_, boss_->hp, boss_->maxHp, boss_->phase);
        }
    } else if (state_ == GameState::STAGE_CLEAR) {
        renderStageClear(renderer_, font_, stageNum_ - 1, player_->score);
    } else if (state_ == GameState::GAMEOVER) {
        renderGameOver(renderer_, font_, player_->score);
    } else if (state_ == GameState::VICTORY) {
        renderVictory(renderer_, font_, player_->score);
    }

    SDL_RenderPresent(renderer_);
}

void Game::startStage(int num) {
    if (!stage_ || !enemies_ || !bullets_ || !powerUps_ || !boss_) return;

    for (auto& e : enemies_->pool) e.active = false;
    for (auto& b : bullets_->pool) b.active = false;
    for (auto& p : powerUps_->pool) p.active = false;
    boss_->active = false;
    bossMusicPlaying_ = false;

    initStage(*stage_, num);
    if (audio_) audio_->playBGM(stageBgmKey(num));
}

void Game::onStageClear() {
    stageNum_++;
    if (stageNum_ > 3) {
        state_ = GameState::VICTORY;
        if (audio_) audio_->playBGM(BGM_VICTORY);
    } else {
        state_ = GameState::STAGE_CLEAR;
    }
}

void Game::onGameOver() {
    state_ = GameState::GAMEOVER;
    if (audio_) audio_->playBGM(BGM_GAMEOVER);
}

} // namespace galaxy
