#include "player.h"

#include "bullet.h"
#include "enemy.h"
#include "sprites.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace galaxy {

namespace {
constexpr float PI = 3.14159265f;
constexpr float MOVE_SPEED = 280.0f;
constexpr float CHARGE_TIME = 1.5f;
constexpr float LOCK_ACQUIRE_INTERVAL = 0.08f;
constexpr float BOMB_DURATION = 2.0f;

void resetLockTargets(Player& p) {
    p.lockTargetCount = 0;
    for (auto& lt : p.lockTargets) {
        lt.active = false;
        lt.enemyIdx = -2;
        lt.pos = {};
    }
}

bool alreadyLocked(const Player& p, int idx) {
    for (int i = 0; i < p.lockTargetCount; ++i) {
        if (p.lockTargets[i].active && p.lockTargets[i].enemyIdx == idx) {
            return true;
        }
    }
    return false;
}

void collectNearestTargets(Player& p, EnemyPool& enemies) {
    std::array<std::pair<float, int>, MAX_ENEMIES> candidates{};
    int count = 0;
    const Vec2 pc = p.center();
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy& e = enemies.pool[i];
        if (!e.active || alreadyLocked(p, i)) continue;
        const Vec2 ec = e.center();
        const float dx = ec.x - pc.x;
        const float dy = ec.y - pc.y;
        candidates[count++] = {dx * dx + dy * dy, i};
    }

    std::sort(candidates.begin(), candidates.begin() + count,
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (int i = 0; i < count && p.lockTargetCount < MAX_LOCK_TARGETS; ++i) {
        const int idx = candidates[i].second;
        Enemy& e = enemies.pool[idx];
        e.lockedOn = true;
        LockTarget& lt = p.lockTargets[p.lockTargetCount++];
        lt.active = true;
        lt.enemyIdx = idx;
        lt.pos = e.center();
    }
}

void firePrimary(Player& p, BulletPool& bullets) {
    const Vec2 muzzle{p.pos.x + 14.0f, p.pos.y};

    switch (p.shipType) {
        case ShipType::BAGON:
            fireBullet(bullets, muzzle, {0.0f, -760.0f}, BulletOwner::PLAYER, 2);
            p.fireTimer = 0.08f;
            break;
        case ShipType::DAMUL:
            for (int i = -2; i <= 2; ++i) {
                const float rad = (i * 20.0f) * PI / 180.0f;
                fireBullet(bullets, muzzle, {std::sin(rad) * 580.0f, -std::cos(rad) * 580.0f},
                           BulletOwner::PLAYER, 1);
            }
            p.fireTimer = 0.14f;
            break;
        case ShipType::GUNEX:
            for (int i = -1; i <= 1; ++i) {
                const float rad = (i * 18.0f) * PI / 180.0f;
                fireBullet(bullets, muzzle, {std::sin(rad) * 500.0f, -std::cos(rad) * 500.0f},
                           BulletOwner::PLAYER, 2);
            }
            fireHomingBullet(bullets, {muzzle.x - 6.0f, muzzle.y + 6.0f}, {-120.0f, -360.0f},
                             BulletOwner::PLAYER, {muzzle.x - 80.0f, muzzle.y - 180.0f}, 2);
            fireHomingBullet(bullets, {muzzle.x + 6.0f, muzzle.y + 6.0f}, {120.0f, -360.0f},
                             BulletOwner::PLAYER, {muzzle.x + 80.0f, muzzle.y - 180.0f}, 2);
            p.fireTimer = 0.20f;
            break;
    }
}

void fireCharge(Player& p, BulletPool& bullets) {
    const Vec2 c = p.center();
    switch (p.shipType) {
        case ShipType::BAGON:
            fireBullet(bullets, {c.x - 2.0f, c.y - 14.0f}, {0.0f, -1000.0f}, BulletOwner::PLAYER, 8);
            break;
        case ShipType::DAMUL:
            for (int i = -3; i <= 3; ++i) {
                const float rad = (i * 12.0f) * PI / 180.0f;
                fireBullet(bullets, c, {std::sin(rad) * 720.0f, -std::cos(rad) * 720.0f},
                           BulletOwner::PLAYER, 4);
            }
            break;
        case ShipType::GUNEX:
            for (int i = 0; i < 16; ++i) {
                const float rad = (i * 22.5f) * PI / 180.0f;
                fireBullet(bullets, c, {std::cos(rad) * 500.0f, std::sin(rad) * 500.0f},
                           BulletOwner::PLAYER, 5);
            }
            break;
    }
}

} // namespace

void initPlayer(Player& p, ShipType ship) {
    p = {};
    p.active = true;
    p.shipType = ship;
    p.pos = {SCREEN_W * 0.5f - 14.0f, SCREEN_H - 90.0f};
    p.bounds = {4.0f, 6.0f, 20.0f, 24.0f};
    p.hp = 3;
    p.lives = 3;
    p.score = 0;
    p.nextLifeScore = SCORE_PER_EXTRA_LIFE;
    p.powerLevel = 1;
    p.bombStock = 3;
    p.powerType = PowerUpType::SPREAD;
    p.hasPowerUp = false;
    p.fireTimer = 0.0f;
    p.invincibleTimer = 0.0f;
    p.shieldTimer = 0.0f;
    p.chargeTimer = 0.0f;
    p.chargeReady = false;
    p.bombActive = false;
    p.bombTimer = 0.0f;
    p.lockOnActive = false;
    p.lockOnTimer = 0.0f;
    p.animFrame = 0;
    p.animTimer = 0.0f;
    resetLockTargets(p);

    switch (ship) {
        case ShipType::BAGON:
            p.hitRadius = 3.0f;
            break;
        case ShipType::DAMUL:
            p.hitRadius = 4.0f;
            break;
        case ShipType::GUNEX:
            p.hitRadius = 5.0f;
            break;
    }
}

void updatePlayer(Player& p, float dt, BulletPool& bullets, EnemyPool& enemies) {
    if (!p.active) return;

    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    const bool lockHeld = keys[SDL_SCANCODE_Z] != 0;
    const bool fireHeld = keys[SDL_SCANCODE_X] != 0 || keys[SDL_SCANCODE_SPACE] != 0;
    const bool bombHeld = keys[SDL_SCANCODE_C] != 0;

    // Use Player member variables instead of statics so initPlayer() resets them.
    bool& prevLockHeld = p.prevLockHeld;
    bool& prevFireHeld = p.prevFireHeld;
    bool& prevBombHeld = p.prevBombHeld;

    float mx = 0.0f;
    float my = 0.0f;
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])  mx -= 1.0f;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) mx += 1.0f;
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])    my -= 1.0f;
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])  my += 1.0f;
    if (mx != 0.0f && my != 0.0f) {
        mx *= 0.7071067f;
        my *= 0.7071067f;
    }
    p.vel = {mx * MOVE_SPEED, my * MOVE_SPEED};
    p.pos += p.vel * dt;
    p.pos.x = std::clamp(p.pos.x, 0.0f, static_cast<float>(SCREEN_W) - 28.0f);
    p.pos.y = std::clamp(p.pos.y, 0.0f, static_cast<float>(SCREEN_H) - 36.0f);

    p.fireTimer = std::max(0.0f, p.fireTimer - dt);
    p.invincibleTimer = std::max(0.0f, p.invincibleTimer - dt);
    p.shieldTimer = std::max(0.0f, p.shieldTimer - dt);
    p.animTimer += dt;
    if (p.animTimer >= 0.1f) {
        p.animTimer = 0.0f;
        p.animFrame = (p.animFrame + 1) & 3;
    }

    if (p.bombActive) {
        p.bombTimer -= dt;
        if (p.bombTimer <= 0.0f) {
            p.bombActive = false;
            p.bombTimer = 0.0f;
        }
    }

    if (lockHeld) {
        p.lockOnActive = true;
        p.lockOnTimer -= dt;
        if (p.lockOnTimer <= 0.0f && p.lockTargetCount < MAX_LOCK_TARGETS) {
            collectNearestTargets(p, enemies);
            p.lockOnTimer = LOCK_ACQUIRE_INTERVAL;
        }
    }

    if (!lockHeld && prevLockHeld) {
        releaseLockOn(p, bullets);
    }

    if (fireHeld) {
        p.chargeTimer += dt;
        if (p.chargeTimer >= CHARGE_TIME) {
            p.chargeReady = true;
        }

        if (p.fireTimer <= 0.0f) {
            firePrimary(p, bullets);
        }
    }

    if (!fireHeld && prevFireHeld) {
        if (p.chargeReady) {
            fireCharge(p, bullets);
        }
        p.chargeTimer = 0.0f;
        p.chargeReady = false;
    }

    if (bombHeld && !prevBombHeld) {
        activateBomb(p, bullets);
    }

    prevLockHeld = lockHeld;
    prevFireHeld = fireHeld;
    prevBombHeld = bombHeld;
}

void releaseLockOn(Player& p, BulletPool& bullets) {
    const Vec2 c = p.center();
    for (int i = 0; i < p.lockTargetCount; ++i) {
        if (!p.lockTargets[i].active) continue;
        const Vec2 tgt = p.lockTargets[i].pos;
        const Vec2 dir = (tgt - c).normalized();
        fireHomingBullet(bullets, c, dir * 420.0f, BulletOwner::PLAYER, tgt, 2);
    }
    p.lockOnActive = false;
    p.lockOnTimer = 0.0f;
    resetLockTargets(p);
}

void activateBomb(Player& p, BulletPool& bullets) {
    if (p.bombStock <= 0) return;
    p.bombStock--;
    p.bombActive = true;
    p.bombTimer = BOMB_DURATION;
    p.invincibleTimer = std::max(p.invincibleTimer, BOMB_DURATION);
    clearEnemyBullets(bullets);
}

void renderPlayer(SDL_Renderer* renderer, const Player& p) {
    if (!p.active) return;
    renderPlayerPrimitive(renderer, static_cast<int>(p.pos.x), static_cast<int>(p.pos.y), p.shipType);
}

} // namespace galaxy
