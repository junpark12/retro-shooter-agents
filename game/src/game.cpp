// Galaxy Storm — Game class implementation

#include "game.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "boss.h"
#include "collision.h"
#include "stage.h"
#include "powerup.h"
#include "background.h"
#include "menu.h"
#include "sprites.h"
#include "hud.h"

#include <SDL.h>
#include <cstdlib>
#include <ctime>

namespace galaxy {

// How long to display STAGE_CLEAR / GAMEOVER / VICTORY before advancing (seconds)
static constexpr float OVERLAY_DURATION = 3.0f;

// ─── Constructor / Destructor ─────────────────────────────────────────────────

Game::Game() = default;

Game::~Game() {
    shutdown();
}

// ─── init ─────────────────────────────────────────────────────────────────────

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    window_ = SDL_CreateWindow(
        "Galaxy Storm",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H,
        SDL_WINDOW_SHOWN);
    if (!window_) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        return false;
    }

    // Seed RNG
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Allocate subsystems
    player_     = new Player();
    bullets_    = new BulletPool();
    enemies_    = new EnemyPool();
    boss_       = new Boss();
    powerUps_   = new PowerUpPool();
    stage_      = new Stage();
    background_ = new Background();
    menu_       = new Menu();

    // Initialise stateless subsystems
    initBackground(*background_);

    state_    = GameState::TITLE;
    running_  = true;
    stageNum_ = 1;

    return true;
}

// ─── run ──────────────────────────────────────────────────────────────────────

void Game::run() {
    Uint32 prevTicks  = SDL_GetTicks();
    // Overlay timer used for STAGE_CLEAR / GAMEOVER / VICTORY transitions
    float  overlayTimer = 0.0f;

    while (running_) {
        Uint32 now  = SDL_GetTicks();
        float  dt   = static_cast<float>(now - prevTicks) / 1000.0f;
        prevTicks   = now;

        // Cap dt to avoid spiral-of-death after a stall
        if (dt > 0.05f) dt = 0.05f;

        handleEvents();

        // Manage overlay transition timers
        if (state_ == GameState::STAGE_CLEAR ||
            state_ == GameState::GAMEOVER    ||
            state_ == GameState::VICTORY) {
            overlayTimer += dt;
            if (overlayTimer >= OVERLAY_DURATION) {
                overlayTimer = 0.0f;
                if (state_ == GameState::STAGE_CLEAR) {
                    if (stageNum_ < 3) {
                        ++stageNum_;
                        startStage(stageNum_);
                    } else {
                        state_ = GameState::VICTORY;
                        overlayTimer = 0.0f;
                    }
                } else if (state_ == GameState::GAMEOVER ||
                           state_ == GameState::VICTORY) {
                    // Return to title after showing the overlay
                    state_    = GameState::TITLE;
                    stageNum_ = 1;
                    *menu_    = Menu{};
                }
            }
        } else {
            update(dt);
        }

        render();
    }
}

// ─── shutdown ─────────────────────────────────────────────────────────────────

void Game::shutdown() {
    delete player_;     player_     = nullptr;
    delete bullets_;    bullets_    = nullptr;
    delete enemies_;    enemies_    = nullptr;
    delete boss_;       boss_       = nullptr;
    delete powerUps_;   powerUps_   = nullptr;
    delete stage_;      stage_      = nullptr;
    delete background_; background_ = nullptr;
    delete menu_;       menu_       = nullptr;

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

// ─── handleEvents ─────────────────────────────────────────────────────────────

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running_ = false;
            return;
        }

        if (state_ == GameState::TITLE) {
            GameState next = handleMenuEvent(*menu_, e);
            if (next == GameState::PLAYING) {
                stageNum_ = 1;
                startStage(stageNum_);
            }
        }

        // ESC always returns to title (or quits from title)
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            if (state_ == GameState::TITLE) {
                running_ = false;
            } else {
                state_    = GameState::TITLE;
                stageNum_ = 1;
                *menu_    = Menu{};
            }
        }
    }
}

// ─── update ───────────────────────────────────────────────────────────────────

void Game::update(float dt) {
    if (state_ != GameState::PLAYING) {
        // Update menu blink in TITLE state
        if (state_ == GameState::TITLE) {
            updateMenu(*menu_, dt);
        }
        return;
    }

    updateBackground(*background_, dt);
    updatePlayer(*player_, dt, *bullets_);
    updateStage(*stage_, dt, *enemies_, *boss_);
    updateEnemies(*enemies_, dt, *bullets_, player_->pos);
    updateBoss(*boss_, dt, *bullets_, player_->pos);
    updateBullets(*bullets_, dt);
    updatePowerUps(*powerUps_, dt);

    // Collision detection
    checkBulletEnemyCollision(*bullets_, *enemies_, *player_, *powerUps_);
    checkBulletPlayerCollision(*bullets_, *player_);
    checkPlayerEnemyCollision(*player_, *enemies_);
    checkBulletBossCollision(*bullets_, *boss_, *player_);
    checkPowerUpPickup(*player_, *powerUps_);

    // Check for player death
    if (player_->lives <= 0) {
        onGameOver();
        return;
    }

    // Check for stage cleared
    if (stage_->stageCleared) {
        onStageClear();
    }
}

// ─── render ───────────────────────────────────────────────────────────────────

void Game::render() {
    // Clear with black
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    switch (state_) {
    case GameState::TITLE:
        renderBackground(renderer_, *background_);
        renderMenu(renderer_, *menu_);
        break;

    case GameState::PLAYING:
        renderBackground(renderer_, *background_);
        renderEnemies(renderer_, *enemies_);
        renderBoss(renderer_, *boss_);
        renderPowerUps(renderer_, *powerUps_);
        renderPlayer(renderer_, *player_);
        renderBullets(renderer_, *bullets_);
        renderHUD(renderer_, *player_, stageNum_);
        break;

    case GameState::STAGE_CLEAR:
        renderBackground(renderer_, *background_);
        renderStageClear(renderer_, stageNum_, player_->score);
        break;

    case GameState::GAMEOVER:
        renderBackground(renderer_, *background_);
        renderGameOver(renderer_, player_->score);
        break;

    case GameState::VICTORY:
        renderBackground(renderer_, *background_);
        renderVictory(renderer_, player_->score);
        break;
    }

    SDL_RenderPresent(renderer_);
}

// ─── startStage ───────────────────────────────────────────────────────────────

void Game::startStage(int num) {
    stageNum_ = num;

    // Reset player position for new stage (keep score/lives)
    player_->pos  = { static_cast<float>(SCREEN_W) * 0.5f - 14.0f,
                      static_cast<float>(SCREEN_H) - 80.0f };
    player_->active = true;

    // Clear all pools
    for (int i = 0; i < MAX_BULLETS;  ++i) bullets_->pool[i].active  = false;
    for (int i = 0; i < MAX_ENEMIES;  ++i) enemies_->pool[i].active  = false;
    for (int i = 0; i < MAX_POWERUPS; ++i) powerUps_->pool[i].active = false;
    boss_->active = false;

    initStage(*stage_, num);

    // If it's the very first stage, also initialise player completely
    if (num == 1) {
        initPlayer(*player_);
    }

    state_ = GameState::PLAYING;
}

// ─── onStageClear ─────────────────────────────────────────────────────────────

void Game::onStageClear() {
    state_ = GameState::STAGE_CLEAR;
}

// ─── onGameOver ───────────────────────────────────────────────────────────────

void Game::onGameOver() {
    state_ = GameState::GAMEOVER;
}

} // namespace galaxy
