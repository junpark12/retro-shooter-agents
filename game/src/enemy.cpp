// Galaxy Storm — Enemy system implementation

#include "enemy.h"
#include "bullet.h"
#include "sprites.h"
#include <cmath>
#include <cstdlib>

namespace galaxy {

// Pixels-per-second speeds per enemy type
static constexpr float SPEED_SMALL  = 140.0f;
static constexpr float SPEED_MEDIUM = 100.0f;
static constexpr float SPEED_LARGE  = 70.0f;

// Sine-wave amplitude for MEDIUM enemies
static constexpr float SINE_AMP   = 60.0f;
static constexpr float SINE_FREQ  = 2.0f; // radians per second

// Fire intervals per enemy type (seconds)
static constexpr float FIRE_INTERVAL_MEDIUM = 2.5f;
static constexpr float FIRE_INTERVAL_LARGE  = 1.8f;

void spawnEnemy(EnemyPool& ep, EnemyType type, Vec2 pos) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy& e = ep.pool[i];
        if (!e.active) {
            e.pos       = pos;
            e.type      = type;
            e.active    = true;
            e.moveTimer = 0.0f;
            e.fireTimer = 0.0f;

            switch (type) {
            case EnemyType::SMALL:
                e.hp         = 1;
                e.pointValue = 100;
                e.vel        = { 0.0f, SPEED_SMALL };
                e.bounds     = { 0.0f, 0.0f, 16.0f, 16.0f };
                break;
            case EnemyType::MEDIUM:
                e.hp         = 3;
                e.pointValue = 250;
                e.vel        = { 0.0f, SPEED_MEDIUM };
                e.bounds     = { 0.0f, 0.0f, 24.0f, 24.0f };
                e.fireTimer  = FIRE_INTERVAL_MEDIUM;
                break;
            case EnemyType::LARGE:
                e.hp         = 8;
                e.pointValue = 500;
                e.vel        = { 0.0f, SPEED_LARGE };
                e.bounds     = { 0.0f, 0.0f, 32.0f, 28.0f };
                e.fireTimer  = FIRE_INTERVAL_LARGE;
                break;
            }
            return;
        }
    }
}

void updateEnemies(EnemyPool& ep, float dt, BulletPool& bullets, Vec2 playerPos) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy& e = ep.pool[i];
        if (!e.active) continue;

        e.moveTimer += dt;

        switch (e.type) {
        case EnemyType::SMALL:
            // Straight downward movement
            e.pos.y += SPEED_SMALL * dt;
            break;

        case EnemyType::MEDIUM:
            // Sine-wave horizontal oscillation while moving down
            e.pos.y += SPEED_MEDIUM * dt;
            e.pos.x += SINE_AMP * std::cos(e.moveTimer * SINE_FREQ) * dt;
            // Clamp to screen horizontally
            if (e.pos.x < 0.0f) e.pos.x = 0.0f;
            if (e.pos.x > static_cast<float>(SCREEN_W) - e.bounds.w)
                e.pos.x = static_cast<float>(SCREEN_W) - e.bounds.w;

            // Fire downward
            e.fireTimer -= dt;
            if (e.fireTimer <= 0.0f) {
                fireBullet(bullets,
                           { e.pos.x + e.bounds.w * 0.5f - 3.0f, e.pos.y + e.bounds.h },
                           { 0.0f, 220.0f },
                           BulletOwner::ENEMY, 1);
                e.fireTimer = FIRE_INTERVAL_MEDIUM;
            }
            break;

        case EnemyType::LARGE:
            {
                // Track toward player horizontally, move slowly downward
                float dx = (playerPos.x + 14.0f) - (e.pos.x + e.bounds.w * 0.5f);
                float dy = (playerPos.y) - (e.pos.y + e.bounds.h);
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 1.0f) {
                    e.pos.x += (dx / len) * SPEED_LARGE * dt;
                    e.pos.y += (dy / len) * SPEED_LARGE * dt * 0.5f;
                }

                // Also drift downward so it doesn't get stuck if player is above
                e.pos.y += 20.0f * dt;

                // Aimed shot toward player
                e.fireTimer -= dt;
                if (e.fireTimer <= 0.0f) {
                    float bx = e.pos.x + e.bounds.w * 0.5f - 3.0f;
                    float by = e.pos.y + e.bounds.h;
                    float adx = playerPos.x - bx;
                    float ady = playerPos.y - by;
                    float alen = std::sqrt(adx * adx + ady * ady);
                    if (alen > 1.0f) {
                        float speed = 180.0f;
                        fireBullet(bullets,
                                   { bx, by },
                                   { (adx / alen) * speed, (ady / alen) * speed },
                                   BulletOwner::ENEMY, 2);
                    }
                    e.fireTimer = FIRE_INTERVAL_LARGE;
                }
            }
            break;
        }

        // Deactivate if enemy leaves the bottom of the screen
        if (e.pos.y > static_cast<float>(SCREEN_H)) {
            e.active = false;
        }
    }
}

void renderEnemies(SDL_Renderer* renderer, const EnemyPool& ep) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        const Enemy& e = ep.pool[i];
        if (!e.active) continue;
        renderEnemySprite(renderer,
                          static_cast<int>(e.pos.x),
                          static_cast<int>(e.pos.y),
                          e.type);
    }
}

bool allEnemiesDefeated(const EnemyPool& ep) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (ep.pool[i].active) return false;
    }
    return true;
}

} // namespace galaxy
