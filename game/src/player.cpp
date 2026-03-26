#include "player.h"

#include "bullet.h"
#include "sprites.h"

#include <algorithm>

namespace galaxy {

namespace {
constexpr float PLAYER_SPEED = 260.0f;
constexpr float BASE_FIRE_COOLDOWN = 0.18f;
}

void initPlayer(Player& p) {
    p.active = true;
    p.pos = {SCREEN_W * 0.5f - 14.0f, SCREEN_H - 90.0f};
    p.vel = {0.0f, 0.0f};
    p.bounds = {4.0f, 4.0f, 20.0f, 28.0f};
    p.hp = 3;
    p.lives = 3;
    p.score = 0;
    p.powerType = PowerUpType::SPREAD;
    p.hasPowerUp = false;
    p.shieldTimer = 0.0f;
    p.fireTimer = 0.0f;
    p.invincibleTimer = 0.0f;
    p.powerUpCount = 0;
}

void updatePlayer(Player& p, float dt, BulletPool& bullets) {
    if (!p.active) return;

    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    float dx = 0.0f;
    float dy = 0.0f;
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])  dx -= 1.0f;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) dx += 1.0f;
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])    dy -= 1.0f;
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])  dy += 1.0f;

    if (dx != 0.0f && dy != 0.0f) {
        dx *= 0.7071067f;
        dy *= 0.7071067f;
    }

    p.vel = {dx * PLAYER_SPEED, dy * PLAYER_SPEED};
    p.pos += p.vel * dt;

    const float maxX = static_cast<float>(SCREEN_W) - 28.0f;
    const float maxY = static_cast<float>(SCREEN_H) - 36.0f;
    p.pos.x = std::clamp(p.pos.x, 0.0f, maxX);
    p.pos.y = std::clamp(p.pos.y, 0.0f, maxY);

    p.fireTimer = std::max(0.0f, p.fireTimer - dt);
    p.invincibleTimer = std::max(0.0f, p.invincibleTimer - dt);
    p.shieldTimer = std::max(0.0f, p.shieldTimer - dt);

    if (p.shieldTimer <= 0.0f && p.powerType == PowerUpType::SHIELD && p.hasPowerUp) {
        p.hasPowerUp = false;
        p.powerType = PowerUpType::SPREAD;
    }

    if (!keys[SDL_SCANCODE_SPACE] || p.fireTimer > 0.0f) return;

    const Vec2 muzzle = {p.pos.x + 12.0f, p.pos.y - 8.0f};
    switch (p.powerType) {
        case PowerUpType::SPREAD:
            fireBullet(bullets, muzzle, {-320.0f, -460.0f}, BulletOwner::PLAYER, 1);
            fireBullet(bullets, muzzle, {-160.0f, -520.0f}, BulletOwner::PLAYER, 1);
            fireBullet(bullets, muzzle, {0.0f, -580.0f}, BulletOwner::PLAYER, 1);
            fireBullet(bullets, muzzle, {160.0f, -520.0f}, BulletOwner::PLAYER, 1);
            fireBullet(bullets, muzzle, {320.0f, -460.0f}, BulletOwner::PLAYER, 1);
            p.fireTimer = 0.20f;
            break;
        case PowerUpType::LASER:
            fireBullet(bullets, muzzle, {0.0f, -900.0f}, BulletOwner::PLAYER, 2);
            p.fireTimer = 0.10f;
            break;
        case PowerUpType::MISSILE:
            fireBullet(bullets, muzzle, {-120.0f, -420.0f}, BulletOwner::PLAYER, 2);
            fireBullet(bullets, muzzle, {120.0f, -420.0f}, BulletOwner::PLAYER, 2);
            p.fireTimer = 0.22f;
            break;
        case PowerUpType::SHIELD:
            // Shield is defensive: no projectile fired.
            p.fireTimer = 0.12f;
            break;
    }

    if (p.hasPowerUp && p.powerType != PowerUpType::SHIELD) {
        p.powerUpCount = std::max(0, p.powerUpCount - 1);
        if (p.powerUpCount == 0) {
            p.hasPowerUp = false;
            p.powerType = PowerUpType::SPREAD;
        }
    }

    if (!p.hasPowerUp && p.powerType == PowerUpType::SPREAD) {
        p.fireTimer = std::max(p.fireTimer, BASE_FIRE_COOLDOWN);
    }
}

void renderPlayer(SDL_Renderer* renderer, const Player& p) {
    if (!p.active) return;

    // Blink while invincible.
    if (p.invincibleTimer > 0.0f) {
        const int phase = static_cast<int>(p.invincibleTimer * 24.0f);
        if ((phase & 1) == 0) return;
    }

    renderPlayerSprite(renderer, static_cast<int>(p.pos.x), static_cast<int>(p.pos.y));
}

} // namespace galaxy
