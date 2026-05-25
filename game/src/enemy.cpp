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
    e.colorVariant = std::rand() % 5;
    e.vel = {0.0f, 0.0f};

    switch (type) {
        case EnemyType::SMALL:
            e.bounds = {0.0f, 0.0f, 16.0f, 16.0f};
            e.hp = 1;
            e.maxHp = 1;
            e.pointValue = 100;
            e.vel.y = 140.0f;
            break;
        case EnemyType::MEDIUM:
            e.bounds = {0.0f, 0.0f, 24.0f, 24.0f};
            e.hp = 3;
            e.maxHp = 3;
            e.pointValue = 300;
            e.vel.y = 110.0f;
            break;
        case EnemyType::LARGE:
            e.bounds = {0.0f, 0.0f, 32.0f, 28.0f};
            e.hp = 5;
            e.maxHp = 5;
            e.pointValue = 500;
            e.vel.y = 70.0f;
            break;
        case EnemyType::FAST:
            e.bounds = {0.0f, 0.0f, 18.0f, 18.0f};
            e.hp = 2;
            e.maxHp = 2;
            e.pointValue = 200;
            e.vel = {(std::rand() % 2 == 0) ? -130.0f : 130.0f, 180.0f};
            break;
        case EnemyType::ARMORED:
            e.bounds = {0.0f, 0.0f, 36.0f, 32.0f};
            e.hp = 10;
            e.maxHp = 10;
            e.pointValue = 700;
            e.vel = {0.0f, 75.0f};
            break;
        case EnemyType::TURRET:
            e.bounds = {0.0f, 0.0f, 32.0f, 32.0f};
            e.hp = 8;
            e.maxHp = 8;
            e.pointValue = 800;
            e.vel = {0.0f, 0.0f};   // stationary — no movement
            e.angle = 180.0f;        // initially pointing down
            break;
    }

    switch (type) {
        case EnemyType::SMALL:   e.firePattern = BulletPattern::SINGLE; break;
        case EnemyType::MEDIUM:  e.firePattern = BulletPattern::SPREAD_3; break;
        case EnemyType::LARGE:   e.firePattern = BulletPattern::AIMED_SPREAD; break;
        case EnemyType::FAST:    e.firePattern = BulletPattern::AIMED; break;
        case EnemyType::ARMORED: e.firePattern = BulletPattern::CURTAIN; break;
        case EnemyType::TURRET: e.firePattern = BulletPattern::AIMED; break;
    }
}
}

Enemy* spawnEnemy(EnemyPool& ep, EnemyType type, Vec2 pos) {
    for (Enemy& e : ep.pool) {
        if (e.active) continue;
        setupEnemyStats(e, type);
        e.pos = pos;
        return &e;
    }
    return nullptr;
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
            case EnemyType::TURRET: {
                // Rotate barrel to track player
                float dx = playerPos.x - (e.pos.x + e.bounds.w * 0.5f);
                float dy = playerPos.y - (e.pos.y + e.bounds.h * 0.5f);
                // Convert atan2 angle (radians, CCW from +X) to SDL degrees (CW from up = -Y axis)
                e.angle = std::atan2(dy, dx) * 180.0f / 3.14159265f + 90.0f;
                // vel stays zero — turret does not move
                break;
            }
        }

        e.pos += e.vel * dt;

        if (e.fireTimer <= 0.0f) {
            Vec2 origin = {e.pos.x + e.bounds.w * 0.5f, e.pos.y + e.bounds.h};
            // For turrets, fire from muzzle point (end of rotating barrel)
            if (e.type == EnemyType::TURRET) {
                const float barrelLen = 18.0f;
                // Convert angle back to radians for muzzle offset
                float rad = (e.angle - 90.0f) * 3.14159265f / 180.0f;
                origin.x = e.pos.x + e.bounds.w * 0.5f + std::cos(rad) * barrelLen;
                origin.y = e.pos.y + e.bounds.h * 0.5f + std::sin(rad) * barrelLen;
            }
            float speed = 200.0f;
            float angle = e.patternTimer;
            int dmg = 1;

            switch (e.type) {
                case EnemyType::SMALL:   speed = 190.0f; e.fireTimer = 1.5f; break;
                case EnemyType::MEDIUM:  speed = 220.0f; e.fireTimer = 1.2f; break;
                case EnemyType::LARGE:   speed = 230.0f; e.fireTimer = 0.95f; break;
                case EnemyType::FAST:    speed = 280.0f; e.fireTimer = 0.75f; break;
                case EnemyType::ARMORED: speed = 210.0f; e.fireTimer = 1.35f; dmg = 2; break;
                case EnemyType::TURRET:  speed = 240.0f; e.fireTimer = 1.0f; break;
            }

            firePattern(bullets, e.firePattern, origin, playerPos, angle, speed, dmg, BulletOwner::ENEMY);
        }

        if (e.type != EnemyType::TURRET) {
            if (e.pos.y > SCREEN_H + 40.0f || e.pos.x < -80.0f || e.pos.x > SCREEN_W + 80.0f) {
                e.active = false;
            }
        }
    }
}

void renderEnemies(SDL_Renderer* renderer, const AssetManager& assets, const EnemyPool& ep) {
    for (const Enemy& e : ep.pool) {
        if (!e.active) continue;
        if (e.type == EnemyType::TURRET) {
            renderTurretSprite(renderer, assets, static_cast<int>(e.pos.x), static_cast<int>(e.pos.y), e.angle, e.lockedOn);
        } else {
            renderEnemySprite(renderer, assets, static_cast<int>(e.pos.x), static_cast<int>(e.pos.y), e.type, e.lockedOn, e.colorVariant);
        }
        if ((e.type == EnemyType::ARMORED || e.type == EnemyType::TURRET) && e.hp < e.maxHp) {
            renderEnemyHPBar(renderer, static_cast<int>(e.pos.x), static_cast<int>(e.pos.y),
                             static_cast<int>(e.bounds.w), e.hp, e.maxHp);
        }
    }
}

bool allEnemiesDefeated(const EnemyPool& ep) {
    for (const Enemy& e : ep.pool) {
        if (e.active) return false;
    }
    return true;
}

} // namespace galaxy
