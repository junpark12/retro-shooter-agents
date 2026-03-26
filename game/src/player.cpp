// Galaxy Storm — Player implementation

#include "player.h"
#include "bullet.h"
#include "sprites.h"
#include <SDL.h>

namespace galaxy {

// Player sprite dimensions
static constexpr float PLAYER_W = 28.0f;
static constexpr float PLAYER_H = 36.0f;

// Movement speed in pixels per second
static constexpr float PLAYER_SPEED = 220.0f;

// Base fire cooldown in seconds
static constexpr float FIRE_COOLDOWN_BASE = 0.18f;

// Bullet speeds
static constexpr float BULLET_SPEED_UP      = -480.0f;
static constexpr float BULLET_SPREAD_ANGLE  = 200.0f; // horizontal component

void initPlayer(Player& p) {
    p.pos             = { static_cast<float>(SCREEN_W) * 0.5f - PLAYER_W * 0.5f,
                          static_cast<float>(SCREEN_H) - 80.0f };
    p.vel             = { 0.0f, 0.0f };
    p.bounds          = { 2.0f, 2.0f, PLAYER_W - 4.0f, PLAYER_H - 4.0f };
    p.active          = true;
    p.hp              = 3;
    p.lives           = 3;
    p.score           = 0;
    p.hasPowerUp      = false;
    p.powerType       = PowerUpType::SPREAD;
    p.shieldTimer     = 0.0f;
    p.fireTimer       = 0.0f;
    p.invincibleTimer = 0.0f;
    p.powerUpCount    = 0;
}

void updatePlayer(Player& p, float dt, BulletPool& bullets) {
    if (!p.active) return;

    // Tick timers
    if (p.shieldTimer     > 0.0f) p.shieldTimer     -= dt;
    if (p.invincibleTimer > 0.0f) p.invincibleTimer -= dt;
    if (p.fireTimer       > 0.0f) p.fireTimer       -= dt;

    // 8-direction movement via keyboard state
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    float vx = 0.0f;
    float vy = 0.0f;

    if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) vx -= PLAYER_SPEED;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) vx += PLAYER_SPEED;
    if (keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W]) vy -= PLAYER_SPEED;
    if (keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S]) vy += PLAYER_SPEED;

    // Normalise diagonal movement
    if (vx != 0.0f && vy != 0.0f) {
        vx *= 0.7071f; // 1/sqrt(2)
        vy *= 0.7071f;
    }

    p.pos.x += vx * dt;
    p.pos.y += vy * dt;

    // Screen boundary clamping
    if (p.pos.x < 0.0f) p.pos.x = 0.0f;
    if (p.pos.y < 0.0f) p.pos.y = 0.0f;
    if (p.pos.x > static_cast<float>(SCREEN_W) - PLAYER_W)
        p.pos.x = static_cast<float>(SCREEN_W) - PLAYER_W;
    if (p.pos.y > static_cast<float>(SCREEN_H) - PLAYER_H)
        p.pos.y = static_cast<float>(SCREEN_H) - PLAYER_H;

    // Firing
    if (keys[SDL_SCANCODE_SPACE] && p.fireTimer <= 0.0f) {
        float cx = p.pos.x + PLAYER_W * 0.5f - 2.0f; // bullet centre x
        float cy = p.pos.y;

        if (p.hasPowerUp && p.powerUpCount > 0) {
            switch (p.powerType) {
            case PowerUpType::SPREAD:
                // 5-way spread shot
                fireBullet(bullets, { cx, cy }, { 0.0f, BULLET_SPEED_UP }, BulletOwner::PLAYER, 1);
                fireBullet(bullets, { cx, cy }, { -BULLET_SPREAD_ANGLE * 0.5f, BULLET_SPEED_UP }, BulletOwner::PLAYER, 1);
                fireBullet(bullets, { cx, cy }, {  BULLET_SPREAD_ANGLE * 0.5f, BULLET_SPEED_UP }, BulletOwner::PLAYER, 1);
                fireBullet(bullets, { cx, cy }, { -BULLET_SPREAD_ANGLE, BULLET_SPEED_UP * 0.85f }, BulletOwner::PLAYER, 1);
                fireBullet(bullets, { cx, cy }, {  BULLET_SPREAD_ANGLE, BULLET_SPEED_UP * 0.85f }, BulletOwner::PLAYER, 1);
                break;

            case PowerUpType::LASER:
                // Single piercing beam (high damage, single bullet that travels full height)
                fireBullet(bullets, { cx, cy }, { 0.0f, BULLET_SPEED_UP * 1.5f }, BulletOwner::PLAYER, 3);
                break;

            case PowerUpType::MISSILE:
                // Triple homing-style angled missiles
                fireBullet(bullets, { cx - 12.0f, cy }, { -40.0f, BULLET_SPEED_UP }, BulletOwner::PLAYER, 2);
                fireBullet(bullets, { cx,          cy }, { 0.0f,   BULLET_SPEED_UP }, BulletOwner::PLAYER, 2);
                fireBullet(bullets, { cx + 12.0f, cy }, {  40.0f, BULLET_SPEED_UP }, BulletOwner::PLAYER, 2);
                break;

            case PowerUpType::SHIELD:
                // Shield power-up is handled in collision; fire normally while shielded
                fireBullet(bullets, { cx, cy }, { 0.0f, BULLET_SPEED_UP }, BulletOwner::PLAYER, 1);
                break;
            }
            --p.powerUpCount;
            if (p.powerUpCount <= 0) {
                p.hasPowerUp = false;
            }
        } else {
            // Normal single shot
            fireBullet(bullets, { cx, cy }, { 0.0f, BULLET_SPEED_UP }, BulletOwner::PLAYER, 1);
        }

        p.fireTimer = FIRE_COOLDOWN_BASE;
    }
}

void renderPlayer(SDL_Renderer* renderer, const Player& p) {
    if (!p.active) return;

    // Flash the player sprite while invincible (blink every 0.1 s)
    if (p.invincibleTimer > 0.0f) {
        // Use integer modulo on millisecond time to create a blink effect
        Uint32 ticks = SDL_GetTicks();
        if ((ticks / 100) % 2 == 0) return; // skip every other 100 ms
    }

    renderPlayerSprite(renderer,
                       static_cast<int>(p.pos.x),
                       static_cast<int>(p.pos.y));
}

} // namespace galaxy
