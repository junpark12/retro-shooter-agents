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
#include "particles.h"
#include "player.h"
#include "powerup.h"
#include "sprites.h"
#include "stage.h"

#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <string>

namespace galaxy {

namespace {
constexpr float STAGE_CLEAR_DURATION = 2.5f;

const char* stageBgmKey(int stageNum) {
    switch (stageNum) {
        case 1: return BGM_STAGE_1;
        case 2: return BGM_STAGE_2;
        case 3: return BGM_STAGE_3;
        case 4: return BGM_STAGE_2;
        default: return BGM_STAGE_3;
    }
}
} // namespace

Game::Game() = default;
Game::~Game() = default;

void Game::spawnFloatingText(Vec2 pos, int value) {
    for (auto& ft : floatingTexts_) {
        if (!ft.active) {
            ft.pos      = pos;
            ft.vel      = -60.0f;
            ft.lifetime = 1.0f;
            ft.age      = 0.0f;
            ft.value    = value;
            ft.active   = true;
            return;
        }
    }
}

void Game::loadHiScore() {
    std::ifstream in("hi_score.dat");
    if (in && (in >> hiScore_)) {
        return;
    }
    hiScore_ = 0;
}

void Game::saveHiScore() {
    std::ofstream out("hi_score.dat");
    if (out) {
        out << hiScore_;
    }
}

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
    particles_ = new ParticleSystem();
    assets_ = new AssetManager();
    audio_ = new AudioManager();

    assets_->init(renderer_);
    audio_->init();
    initBackground(*background_);
    initPlayer(*player_, selectedShip_);
    loadHiScore();

    for (auto& b : bullets_->pool) b.active = false;
    for (auto& e : enemies_->pool) e.active = false;
    for (auto& p : powerUps_->pool) p.active = false;
    for (auto& p : particles_->pool) p.active = false;
    boss_->active = false;

    state_ = GameState::TITLE;
    running_ = true;
    stageNum_ = 1;
    bossMusicPlaying_ = false;
    prevBossWarningActive_ = false;
    displayScore_ = 0;
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
    delete particles_; particles_ = nullptr;

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
            } else if (next == GameState::HIGH_SCORE) {
                state_ = GameState::HIGH_SCORE;
            } else if (next == GameState::QUIT) {
                running_ = false;
                return;
            }
        } else if (state_ == GameState::HIGH_SCORE) {
            GameState next = handleHighScoreEvent(e);
            if (next == GameState::TITLE) {
                state_ = GameState::TITLE;
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
    if (state_ == GameState::HIGH_SCORE) {
        return;
    }
    if (state_ == GameState::SHIP_SELECT) {
        updateShipSelect(*shipSelect_, dt);
        return;
    }
    // Update particles during STAGE_CLEAR so explosion effects keep animating
    // on the clear screen; other gameplay systems are paused.
    if (state_ == GameState::STAGE_CLEAR) {
        updateParticles(*particles_, dt);
        return;
    }
    if (state_ != GameState::PLAYING) {
        return;
    }

    updatePlayer(*player_, dt, *bullets_, *enemies_);
    // Animate display score toward actual score
    if (displayScore_ < player_->score) {
        int delta = std::max(1, (player_->score - displayScore_) / 8);
        displayScore_ = std::min(player_->score, displayScore_ + delta);
    }
    // Detect bomb activation edge: start screen shake
    if (player_->bombActive && !prevBombActive_) {
        screenShakeTimer_ = 0.35f;
    }
    prevBombActive_ = player_->bombActive;
    if (screenShakeTimer_ > 0.0f) {
        screenShakeTimer_ -= dt;
        if (screenShakeTimer_ < 0.0f) screenShakeTimer_ = 0.0f;
    }
    updateStage(*stage_, dt, *enemies_, *boss_);
    updateEnemies(*enemies_, dt, *bullets_, player_->center());
    updateBoss(*boss_, dt, *bullets_, player_->center());
    updateBullets(*bullets_, dt);
    updateParticles(*particles_, dt);
    updatePowerUps(*powerUps_, dt, player_->center());

    // Update floating score texts
    for (auto& ft : floatingTexts_) {
        if (!ft.active) continue;
        ft.age += dt;
        ft.pos.y += ft.vel * dt;
        if (ft.age >= ft.lifetime) ft.active = false;
    }

    bool prevActive[MAX_ENEMIES];
    Vec2 prevPos[MAX_ENEMIES];
    int prevPointValue[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        prevActive[i] = enemies_->pool[i].active;
        prevPos[i] = enemies_->pool[i].pos;
        prevPointValue[i] = enemies_->pool[i].pointValue;
    }

    checkBulletEnemyCollision(*bullets_, *enemies_, *player_, *powerUps_, audio_, particles_);
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (prevActive[i] && !enemies_->pool[i].active) {
            spawnFloatingText(prevPos[i], prevPointValue[i]);
        }
    }
    const bool bossWarningStarted = stage_->bossWarningActive && !prevBossWarningActive_;
    if (bossWarningStarted) {
        if (audio_) audio_->playSFX(SFX_BOSS_WARNING);
    }
    prevBossWarningActive_ = stage_->bossWarningActive;
    checkBulletPlayerCollision(*bullets_, *player_, audio_);
    checkPlayerEnemyCollision(*player_, *enemies_, audio_, particles_);
    checkBulletBossCollision(*bullets_, *boss_, *player_, audio_);
    checkPowerUpPickup(*player_, *powerUps_, audio_);

    if (player_->score > hiScore_) hiScore_ = player_->score;

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
        renderMenu(renderer_, *assets_, font_, *menu_, hiScore_);
        SDL_RenderPresent(renderer_);
        return;
    }
    if (state_ == GameState::HIGH_SCORE) {
        renderHighScore(renderer_, font_, hiScore_);
        SDL_RenderPresent(renderer_);
        return;
    }
    if (state_ == GameState::SHIP_SELECT) {
        renderShipSelect(renderer_, *assets_, font_, *shipSelect_);
        SDL_RenderPresent(renderer_);
        return;
    }

    // Apply screen shake offset when bomb is active
    if (screenShakeTimer_ > 0.0f) {
        const float intensity = 10.0f * (screenShakeTimer_ / 0.35f);
        const int ox = static_cast<int>((std::rand() % static_cast<int>(intensity * 2 + 1)) - intensity);
        const int oy = static_cast<int>((std::rand() % static_cast<int>(intensity * 2 + 1)) - intensity);
        SDL_Rect viewport{ox, oy, SCREEN_W, SCREEN_H};
        SDL_RenderSetViewport(renderer_, &viewport);
    }

    renderEnemies(renderer_, *assets_, *enemies_);
    renderBoss(renderer_, *assets_, *boss_);
    renderPowerUps(renderer_, *assets_, *powerUps_);
    renderPlayer(renderer_, *assets_, *player_);
    renderParticles(renderer_, *particles_);
    renderBullets(renderer_, *assets_, *bullets_);

    // Reset viewport after shake
    if (screenShakeTimer_ > 0.0f) {
        SDL_RenderSetViewport(renderer_, nullptr);
    }

    // Bomb flash overlay
    if (player_->bombActive) {
        renderBombFlash(renderer_, player_->bombTimer);
    }

    if (state_ == GameState::PLAYING) {
        renderHUD(renderer_, *assets_, font_, *player_, stageNum_, hiScore_, displayScore_);
        if (boss_->active) {
            renderBossHP(renderer_, font_, boss_->hp, boss_->maxHp, boss_->phase);
        }
        if (stage_ && stage_->bossWarningActive) {
            renderBossWarning(renderer_, font_, stage_->bossWarningTimer);
        }
        renderFloatingTexts(renderer_, font_, floatingTexts_, MAX_FLOATING_TEXTS);
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
    prevBossWarningActive_ = false;
    displayScore_ = player_->score;
    for (auto& ft : floatingTexts_) ft.active = false;

    initStage(*stage_, num);
    if (audio_) audio_->playBGM(stageBgmKey(num));
}

void Game::onStageClear() {
    stageNum_++;
    if (stageNum_ > 5) {
        state_ = GameState::VICTORY;
        saveHiScore();
        if (audio_) audio_->playBGM(BGM_VICTORY);
    } else {
        state_ = GameState::STAGE_CLEAR;
    }
}

void Game::onGameOver() {
    state_ = GameState::GAMEOVER;
    saveHiScore();
    if (audio_) audio_->playBGM(BGM_GAMEOVER);
}

} // namespace galaxy
