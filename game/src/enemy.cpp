#include "enemy.h"

#include "bullet.h"
#include "sprites.h"

#include <cmath>

namespace galaxy {

namespace {
constexpr float PI = 3.14159265f;

void setupEnemyStats(Enemy& e, EnemyType type) {
    e.type = type;
    e.moveTimer = 0.0f;
    e.fireTimer = 0.7f;
    e.active = true;
    e.vel = {0.0f, 0.0f};

    switch (type) {
        case EnemyType::SMALL:
            e.bounds = {0.0f, 0.0f, 16.0f, 16.0f};
            e.hp = 1;
            e.pointValue = 100;
            e.vel.y = 140.0f;
            break;
        case EnemyType::MEDIUM:
            e.bounds = {0.0f, 0.0f, 24.0f, 24.0f};
            e.hp = 3;
            e.pointValue = 300;
            e.vel.y = 110.0f;
            break;
        case EnemyType::LARGE:
            e.bounds = {0.0f, 0.0f, 32.0f, 28.0f};
            e.hp = 5;
            e.pointValue = 500;
            e.vel.y = 70.0f;
            break;
    }
}
}

void spawnEnemy(EnemyPool& ep, EnemyType type, Vec2 pos) {
    for (Enemy& e : ep.pool) {
        if (e.active) continue;
        setupEnemyStats(e, type);
        e.pos = pos;
        return;
    }
}

void updateEnemies(EnemyPool& ep, float dt, BulletPool& bullets, Vec2 playerPos) {
    for (Enemy& e : ep.pool) {
        if (!e.active) continue;

        e.moveTimer += dt;
        e.fireTimer -= dt;

        switch (e.type) {
            case EnemyType::SMALL:
                e.vel.x = 0.0f;
                e.vel.y = 160.0f;
                break;
            case EnemyType::MEDIUM:
                e.vel.y = 120.0f;
                e.vel.x = std::sin(e.moveTimer * 4.5f) * 110.0f;
                break;
            case EnemyType::LARGE: {
                e.vel.y = 80.0f;
                float targetX = playerPos.x - e.pos.x;
                if (targetX > 8.0f) targetX = 8.0f;
                if (targetX < -8.0f) targetX = -8.0f;
                e.vel.x = targetX * 14.0f;
                break;
            }
        }

        e.pos += e.vel * dt;

        if (e.fireTimer <= 0.0f) {
            Vec2 origin = {e.pos.x + e.bounds.w * 0.5f, e.pos.y + e.bounds.h};
            Vec2 toPlayer = {playerPos.x - origin.x, playerPos.y - origin.y};
            float len = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
            if (len < 0.001f) len = 1.0f;
            Vec2 dir = {toPlayer.x / len, toPlayer.y / len};

            switch (e.type) {
                case EnemyType::SMALL:
                    fireBullet(bullets, origin, {dir.x * 180.0f, dir.y * 180.0f}, BulletOwner::ENEMY, 1);
                    e.fireTimer = 1.5f;
                    break;
                case EnemyType::MEDIUM:
                    fireBullet(bullets, origin, {dir.x * 200.0f - 50.0f, dir.y * 200.0f}, BulletOwner::ENEMY, 1);
                    fireBullet(bullets, origin, {dir.x * 200.0f + 50.0f, dir.y * 200.0f}, BulletOwner::ENEMY, 1);
                    e.fireTimer = 1.2f;
                    break;
                case EnemyType::LARGE:
                    fireBullet(bullets, origin, {dir.x * 220.0f, dir.y * 220.0f}, BulletOwner::ENEMY, 1);
                    fireBullet(bullets, origin, {std::cos(PI * 0.35f) * -220.0f, 220.0f}, BulletOwner::ENEMY, 1);
                    fireBullet(bullets, origin, {std::cos(PI * 0.35f) * 220.0f, 220.0f}, BulletOwner::ENEMY, 1);
                    e.fireTimer = 0.95f;
                    break;
            }
        }

        if (e.pos.y > SCREEN_H + 40.0f || e.pos.x < -80.0f || e.pos.x > SCREEN_W + 80.0f) {
            e.active = false;
        }
    }
}

void renderEnemies(SDL_Renderer* renderer, const EnemyPool& ep) {
    for (const Enemy& e : ep.pool) {
        if (!e.active) continue;
        renderEnemySprite(renderer, static_cast<int>(e.pos.x), static_cast<int>(e.pos.y), e.type);
    }
}

bool allEnemiesDefeated(const EnemyPool& ep) {
    for (const Enemy& e : ep.pool) {
        if (e.active) return false;
    }
    return true;
}

} // namespace galaxy
