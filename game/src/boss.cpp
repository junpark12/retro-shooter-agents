// Galaxy Storm — Boss system implementation

#include "boss.h"
#include "bullet.h"
#include "sprites.h"
#include <cmath>
#include <cstdlib>

namespace galaxy {

// Boss HP and size per stage
static constexpr int   BOSS_HP[3]     = { 80, 150, 250 };
static constexpr float BOSS_W[3]      = { 64.0f, 72.0f, 80.0f };
static constexpr float BOSS_H[3]      = { 48.0f, 56.0f, 64.0f };
static constexpr float BOSS_SPEED[3]  = { 80.0f, 100.0f, 120.0f };

// Y position where boss settles
static constexpr float BOSS_TARGET_Y  = 60.0f;
// Time between attacks per phase (shorter = harder)
static const float ATTACK_INTERVALS[3][3] = {
    { 2.0f, 1.6f, 1.2f }, // Stage 1 boss phases 1-3
    { 1.8f, 1.4f, 1.0f }, // Stage 2
    { 1.5f, 1.1f, 0.8f }, // Stage 3
};

void initBoss(Boss& b, int stageNum) {
    int idx      = stageNum - 1;
    b.stageNum   = stageNum;
    b.phase      = 1;
    b.maxHp      = BOSS_HP[idx];
    b.hp         = b.maxHp;
    b.active     = false; // activated by stage manager
    b.attackTimer = ATTACK_INTERVALS[idx][0];
    b.moveTimer   = 0.0f;

    float w = BOSS_W[idx];
    float h = BOSS_H[idx];
    b.pos    = { static_cast<float>(SCREEN_W) * 0.5f - w * 0.5f, -h };
    b.vel    = { 0.0f, BOSS_SPEED[idx] };
    b.bounds = { 4.0f, 4.0f, w - 8.0f, h - 8.0f };
}

// Update the phase based on remaining HP
static void updatePhase(Boss& b) {
    int idx = b.stageNum - 1;
    float ratio = static_cast<float>(b.hp) / static_cast<float>(b.maxHp);
    int newPhase;
    if      (ratio > 0.66f) newPhase = 1;
    else if (ratio > 0.33f) newPhase = 2;
    else                    newPhase = 3;

    if (newPhase != b.phase) {
        b.phase = newPhase;
        b.attackTimer = ATTACK_INTERVALS[idx][b.phase - 1];
    }
}

// Fire a radial burst of bullets centered from boss
static void fireBurst(Boss& b, BulletPool& bullets, int count, float speed) {
    float cx = b.pos.x + BOSS_W[b.stageNum - 1] * 0.5f;
    float cy = b.pos.y + BOSS_H[b.stageNum - 1];
    for (int i = 0; i < count; ++i) {
        float angle = static_cast<float>(i) / static_cast<float>(count) * 2.0f * 3.14159f;
        float vx = std::cos(angle) * speed;
        float vy = std::sin(angle) * speed;
        fireBullet(bullets, { cx - 3.0f, cy }, { vx, vy }, BulletOwner::ENEMY, 1);
    }
}

// Fire a spread aimed downward
static void fireAimed(Boss& b, BulletPool& bullets, Vec2 playerPos, int count) {
    float cx = b.pos.x + BOSS_W[b.stageNum - 1] * 0.5f;
    float cy = b.pos.y + BOSS_H[b.stageNum - 1];
    float dx = playerPos.x - cx;
    float dy = playerPos.y - cy;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.0f) return;

    float speed = 200.0f;
    float nx = dx / len;
    float ny = dy / len;

    for (int i = 0; i < count; ++i) {
        float offset = static_cast<float>(i - count / 2) * 0.2f; // slight spread
        float vx = (nx + offset) * speed;
        float vy = ny * speed;
        fireBullet(bullets, { cx - 3.0f, cy }, { vx, vy }, BulletOwner::ENEMY, 1);
    }
}

void updateBoss(Boss& b, float dt, BulletPool& bullets, Vec2 playerPos) {
    if (!b.active) return;

    b.moveTimer  += dt;
    b.attackTimer -= dt;

    int idx = b.stageNum - 1;

    // Entry: boss enters from top, slows to BOSS_TARGET_Y
    if (b.pos.y < BOSS_TARGET_Y) {
        b.pos.y += BOSS_SPEED[idx] * dt;
        if (b.pos.y > BOSS_TARGET_Y) b.pos.y = BOSS_TARGET_Y;
        return; // Don't attack while entering
    }

    // Horizontal oscillation
    float oscillation = BOSS_SPEED[idx] * std::sin(b.moveTimer * 0.8f);
    b.pos.x += oscillation * dt;

    // Clamp boss to screen
    if (b.pos.x < 0.0f) b.pos.x = 0.0f;
    if (b.pos.x > static_cast<float>(SCREEN_W) - BOSS_W[idx])
        b.pos.x = static_cast<float>(SCREEN_W) - BOSS_W[idx];

    // Update phase based on HP
    updatePhase(b);

    // Attack based on current phase
    if (b.attackTimer <= 0.0f) {
        float interval = ATTACK_INTERVALS[idx][b.phase - 1];

        switch (b.phase) {
        case 1:
            // Simple aimed burst
            fireAimed(b, bullets, playerPos, 3);
            break;
        case 2:
            // Aimed + side shots
            fireAimed(b, bullets, playerPos, 3);
            fireBullet(bullets,
                       { b.pos.x, b.pos.y + BOSS_H[idx] * 0.5f },
                       { -160.0f, 100.0f }, BulletOwner::ENEMY, 1);
            fireBullet(bullets,
                       { b.pos.x + BOSS_W[idx], b.pos.y + BOSS_H[idx] * 0.5f },
                       {  160.0f, 100.0f }, BulletOwner::ENEMY, 1);
            break;
        case 3:
            // Radial burst + aimed shots
            fireBurst(b, bullets, 8 + b.stageNum * 2, 180.0f);
            fireAimed(b, bullets, playerPos, 5);
            break;
        }

        b.attackTimer = interval;
    }
}

void renderBoss(SDL_Renderer* renderer, const Boss& b) {
    if (!b.active) return;
    renderBossSprite(renderer,
                     static_cast<int>(b.pos.x),
                     static_cast<int>(b.pos.y),
                     b.stageNum);
}

} // namespace galaxy
