#include "enemy.h"

#include "bullet.h"
#include "bullet_pattern.h"
#include "sprites.h"

#include <cmath>
#include <cstdlib>

namespace galaxy {

namespace {
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
        case EnemyType::FAST:
            e.bounds = {0.0f, 0.0f, 18.0f, 18.0f};
            e.hp = 2;
            e.pointValue = 200;
            e.vel = {(std::rand() % 2 == 0) ? -130.0f : 130.0f, 180.0f};
            break;
        case EnemyType::ARMORED:
            e.bounds = {0.0f, 0.0f, 36.0f, 32.0f};
            e.hp = 10;
            e.pointValue = 700;
            e.vel = {0.0f, 75.0f};
            break;
    }

    switch (type) {
        case EnemyType::SMALL:   e.firePattern = BulletPattern::SINGLE; break;
        case EnemyType::MEDIUM:  e.firePattern = BulletPattern::SPREAD_3; break;
        case EnemyType::LARGE:   e.firePattern = BulletPattern::AIMED_SPREAD; break;
        case EnemyType::FAST:    e.firePattern = BulletPattern::AIMED; break;
        case EnemyType::ARMORED: e.firePattern = BulletPattern::CURTAIN; break;
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
        e.patternTimer += dt;

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
            case EnemyType::FAST:
                if (e.pos.x < 10.0f || e.pos.x > SCREEN_W - 30.0f) e.vel.x *= -1.0f;
                break;
            case EnemyType::ARMORED:
                e.vel.x = std::sin(e.moveTimer * 1.2f) * 45.0f;
                break;
        }

        e.pos += e.vel * dt;

        if (e.fireTimer <= 0.0f) {
            Vec2 origin = {e.pos.x + e.bounds.w * 0.5f, e.pos.y + e.bounds.h};
            float speed = 200.0f;
            float angle = e.patternTimer;
            int dmg = 1;

            switch (e.type) {
                case EnemyType::SMALL:   speed = 190.0f; e.fireTimer = 1.5f; break;
                case EnemyType::MEDIUM:  speed = 220.0f; e.fireTimer = 1.2f; break;
                case EnemyType::LARGE:   speed = 230.0f; e.fireTimer = 0.95f; break;
                case EnemyType::FAST:    speed = 280.0f; e.fireTimer = 0.75f; break;
                case EnemyType::ARMORED: speed = 210.0f; e.fireTimer = 1.35f; dmg = 2; break;
            }

            firePattern(bullets, e.firePattern, origin, playerPos, angle, speed, dmg, BulletOwner::ENEMY);
        }

        if (e.pos.y > SCREEN_H + 40.0f || e.pos.x < -80.0f || e.pos.x > SCREEN_W + 80.0f) {
            e.active = false;
        }
    }
}

void renderEnemies(SDL_Renderer* renderer, const EnemyPool& ep) {
    for (const Enemy& e : ep.pool) {
        if (!e.active) continue;
        renderEnemyPrimitive(renderer, static_cast<int>(e.pos.x), static_cast<int>(e.pos.y), e.type);
    }
}

bool allEnemiesDefeated(const EnemyPool& ep) {
    for (const Enemy& e : ep.pool) {
        if (e.active) return false;
    }
    return true;
}

} // namespace galaxy
