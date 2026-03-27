#include "boss.h"

#include "bullet.h"
#include "sprites.h"

#include <cmath>

namespace galaxy {

namespace {
constexpr float PI = 3.14159265f;

void fireAimed(BulletPool& bullets, Vec2 origin, Vec2 target, float speed, int damage = 1) {
    Vec2 d = {target.x - origin.x, target.y - origin.y};
    float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 0.001f) len = 1.0f;
    Vec2 dir = {d.x / len, d.y / len};
    fireBullet(bullets, origin, {dir.x * speed, dir.y * speed}, BulletOwner::ENEMY, damage);
}
}

void initBoss(Boss& b, int stageNum) {
    b.stageNum = stageNum;
    b.phase = 1;
    b.attackTimer = 0.6f;
    b.moveTimer = 0.0f;
    b.active = true;
    b.pos = {SCREEN_W * 0.5f - 36.0f, 48.0f};
    b.vel = {0.0f, 0.0f};

    switch (stageNum) {
        case 1:
            b.maxHp = 30;
            b.bounds = {0.0f, 0.0f, 64.0f, 48.0f};
            break;
        case 2:
            b.maxHp = 50;
            b.bounds = {0.0f, 0.0f, 72.0f, 56.0f};
            break;
        default:
            b.maxHp = 80;
            b.bounds = {0.0f, 0.0f, 80.0f, 64.0f};
            break;
    }
    b.hp = b.maxHp;
}

void updateBoss(Boss& b, float dt, BulletPool& bullets, Vec2 playerPos) {
    if (!b.active) return;

    b.moveTimer += dt;
    b.attackTimer -= dt;

    if (b.hp <= b.maxHp / 3) b.phase = 3;
    else if (b.hp <= b.maxHp / 2) b.phase = 2;
    else b.phase = 1;

    const float width = b.bounds.w;
    const float maxX = static_cast<float>(SCREEN_W) - width;

    if (b.stageNum == 1) {
        b.pos.x = SCREEN_W * 0.5f - width * 0.5f + std::sin(b.moveTimer * 1.6f) * 140.0f;
    } else if (b.stageNum == 2) {
        b.pos.x = SCREEN_W * 0.5f - width * 0.5f + std::sin(b.moveTimer * (1.8f + b.phase * 0.3f)) * 120.0f;
        b.pos.y = 52.0f + std::sin(b.moveTimer * 2.1f) * 26.0f;
    } else {
        // Stage 3: laser sweep feel with broad horizontal movement.
        b.pos.x = SCREEN_W * 0.5f - width * 0.5f + std::sin(b.moveTimer * (1.2f + b.phase * 0.25f)) * 165.0f;
        b.pos.y = 44.0f + std::sin(b.moveTimer * 1.0f) * 18.0f;
    }

    if (b.pos.x < 0.0f) b.pos.x = 0.0f;
    if (b.pos.x > maxX) b.pos.x = maxX;

    Vec2 origin = {b.pos.x + b.bounds.w * 0.5f, b.pos.y + b.bounds.h * 0.65f};

    if (b.attackTimer <= 0.0f) {
        if (b.stageNum == 1) {
            // 3-way burst.
            fireBullet(bullets, origin, {-140.0f, 220.0f}, BulletOwner::ENEMY, 1);
            fireBullet(bullets, origin, {0.0f, 260.0f}, BulletOwner::ENEMY, 1);
            fireBullet(bullets, origin, {140.0f, 220.0f}, BulletOwner::ENEMY, 1);
            b.attackTimer = (b.phase == 1) ? 1.0f : (b.phase == 2 ? 0.8f : 0.6f);
        } else if (b.stageNum == 2) {
            // 8-way circular burst + occasional aimed shot (dash pressure).
            constexpr Vec2 dirs[8] = {
                { 0.0f,  1.0f}, { 0.707f,  0.707f}, { 1.0f,  0.0f}, { 0.707f, -0.707f},
                { 0.0f, -1.0f}, {-0.707f, -0.707f}, {-1.0f,  0.0f}, {-0.707f,  0.707f}
            };
            float spd = 190.0f + b.phase * 20.0f;
            for (const Vec2& d : dirs) {
                fireBullet(bullets, origin, {d.x * spd, d.y * spd}, BulletOwner::ENEMY, 1);
            }
            fireAimed(bullets, origin, playerPos, 240.0f + b.phase * 30.0f, 1);
            b.attackTimer = (b.phase == 1) ? 1.1f : (b.phase == 2 ? 0.85f : 0.65f);
        } else {
            // Stage 3 phase A/B
            if (b.phase < 3) {
                // Laser sweep approximation: dense downward lanes.
                for (int i = -2; i <= 2; ++i) {
                    fireBullet(bullets, {origin.x + i * 16.0f, origin.y}, {i * 25.0f, 320.0f},
                               BulletOwner::ENEMY, 1);
                }
            } else {
                // Phase B: add guided-ish missiles.
                for (int i = -2; i <= 2; ++i) {
                    fireBullet(bullets, {origin.x + i * 18.0f, origin.y}, {i * 35.0f, 340.0f},
                               BulletOwner::ENEMY, 1);
                }
                fireAimed(bullets, origin, playerPos, 280.0f, 2);
                fireAimed(bullets, {origin.x - 24.0f, origin.y}, playerPos, 250.0f, 1);
                fireAimed(bullets, {origin.x + 24.0f, origin.y}, playerPos, 250.0f, 1);
            }
            b.attackTimer = (b.phase == 1) ? 0.95f : (b.phase == 2 ? 0.70f : 0.5f);
        }
    }
}

void renderBoss(SDL_Renderer* renderer, const Boss& b) {
    if (!b.active) return;
    renderBossSprite(renderer, static_cast<int>(b.pos.x), static_cast<int>(b.pos.y), b.stageNum);
}

} // namespace galaxy
