#include "game.h"

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

#include <cstdlib>
#include <ctime>

namespace galaxy {

namespace {
constexpr float STAGE_CLEAR_DURATION = 2.5f;
}

Game::Game() = default;

Game::~Game() = default;

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        return false;
    }

    window_ = SDL_CreateWindow("Galaxy Storm",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    if (!window_) {
        SDL_Quit();
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return false;
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

    initBackground(*background_);
    initPlayer(*player_);
    stageNum_ = 1;
    state_ = GameState::TITLE;
    running_ = true;

    // Ensure pools start inactive.
    for (auto& b : bullets_->pool) b.active = false;
    for (auto& e : enemies_->pool) e.active = false;
    for (auto& p : powerUps_->pool) p.active = false;
    boss_->active = false;

    return true;
}

void Game::run() {
    if (!window_ || !renderer_) return;

    Uint32 previous = SDL_GetTicks();
    float accumulatorMs = 0.0f;
    float stageClearTimer = 0.0f;

    while (running_) {
        Uint32 now = SDL_GetTicks();
        Uint32 elapsed = now - previous;
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

        // Hard cap to 60fps in case VSYNC is unavailable.
        Uint32 frameTime = SDL_GetTicks() - now;
        if (frameTime < static_cast<Uint32>(FRAME_MS)) {
            SDL_Delay(static_cast<Uint32>(FRAME_MS) - frameTime);
        }
    }
}

void Game::shutdown() {
    delete player_;
    player_ = nullptr;
    delete bullets_;
    bullets_ = nullptr;
    delete enemies_;
    enemies_ = nullptr;
    delete boss_;
    boss_ = nullptr;
    delete powerUps_;
    powerUps_ = nullptr;
    delete stage_;
    stage_ = nullptr;
    delete background_;
    background_ = nullptr;
    delete menu_;
    menu_ = nullptr;

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
                initPlayer(*player_);
                startStage(stageNum_);
                state_ = GameState::PLAYING;
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

    if (state_ != GameState::PLAYING) {
        return;
    }

    updatePlayer(*player_, dt, *bullets_);
    updateStage(*stage_, dt, *enemies_, *boss_);
    updateEnemies(*enemies_, dt, *bullets_, player_->pos);
    updateBoss(*boss_, dt, *bullets_, player_->pos);
    updateBullets(*bullets_, dt);
    updatePowerUps(*powerUps_, dt);

    checkBulletEnemyCollision(*bullets_, *enemies_, *player_, *powerUps_);
    checkBulletPlayerCollision(*bullets_, *player_);
    checkPlayerEnemyCollision(*player_, *enemies_);
    checkBulletBossCollision(*bullets_, *boss_, *player_);
    checkPowerUpPickup(*player_, *powerUps_);

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

    renderBackground(renderer_, *background_);

    if (state_ == GameState::TITLE) {
        renderMenu(renderer_, *menu_);
        SDL_RenderPresent(renderer_);
        return;
    }

    renderEnemies(renderer_, *enemies_);
    renderBoss(renderer_, *boss_);
    renderPowerUps(renderer_, *powerUps_);
    renderPlayer(renderer_, *player_);
    renderBullets(renderer_, *bullets_);

    if (state_ == GameState::PLAYING) {
        renderHUD(renderer_, *player_, stageNum_);
    } else if (state_ == GameState::STAGE_CLEAR) {
        renderStageClear(renderer_, stageNum_ - 1, player_->score);
    } else if (state_ == GameState::GAMEOVER) {
        renderGameOver(renderer_, player_->score);
    } else if (state_ == GameState::VICTORY) {
        renderVictory(renderer_, player_->score);
    }

    SDL_RenderPresent(renderer_);
}

void Game::startStage(int num) {
    if (!stage_ || !enemies_ || !bullets_ || !powerUps_ || !boss_) return;

    for (auto& e : enemies_->pool) e.active = false;
    for (auto& b : bullets_->pool) b.active = false;
    for (auto& p : powerUps_->pool) p.active = false;
    boss_->active = false;

    initStage(*stage_, num);
}

void Game::onStageClear() {
    stageNum_++;
    if (stageNum_ > 3) {
        state_ = GameState::VICTORY;
    } else {
        state_ = GameState::STAGE_CLEAR;
    }
}

void Game::onGameOver() {
    state_ = GameState::GAMEOVER;
}

} // namespace galaxy
