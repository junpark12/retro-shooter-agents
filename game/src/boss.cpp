#include "boss.h"

#include "bullet.h"
#include "bullet_pattern.h"
#include "sprites.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace galaxy {

namespace {
constexpr float PI = 3.14159265f;
constexpr float ATTACK_WARNING_DURATION = 0.4f;  // pre-attack warning flash duration
constexpr float ENTRANCE_DURATION = 1.2f;        // boss slide-in duration
}

void initBoss(Boss& b, int stageNum) {
    b.stageNum = stageNum;
    b.phase = 1;
    b.attackTimer = 0.6f;
    b.moveTimer = 0.0f;
    b.active = true;
    b.pos = {SCREEN_W * 0.5f - 80.0f, 48.0f};
    b.vel = {0.0f, 0.0f};

    switch (stageNum) {
        case 1:
            b.maxHp = 300;
            b.bounds = {0.0f, 0.0f, 160.0f, 120.0f};
            break;
        case 2:
            b.maxHp = 500;
            b.bounds = {0.0f, 0.0f, 180.0f, 140.0f};
            break;
        case 3:
            b.maxHp = 800;
            b.bounds = {0.0f, 0.0f, 200.0f, 160.0f};
            break;
        case 4:
            b.maxHp = 1200;
            b.bounds = {0.0f, 0.0f, 210.0f, 170.0f};
            break;
        default:
            b.maxHp = 1800;
            b.bounds = {0.0f, 0.0f, 220.0f, 180.0f};
            break;
    }
    b.hp = b.maxHp;
    b.patternAngle = 0.0f;
    b.burstCount = 0;
    b.burstTimer = 0.0f;
    b.enraged = false;
    b.targetPos = {b.pos.x, b.pos.y};
    b.moveCooldown = 1.5f;
    b.lockedOn = false;
    b.attackWarningTimer = 0.0f;
    b.attackWarning = false;
    b.displayHp = static_cast<float>(b.maxHp);
    b.entranceTimer = ENTRANCE_DURATION;
    b.entranceDone = false;
    // Boss starts above screen and slides down during entrance
    b.pos.y = -static_cast<float>(b.bounds.h) - 10.0f;
}

void updateBoss(Boss& b, float dt, BulletPool& bullets, Vec2 playerPos) {
    if (!b.active) return;

    // Smoothly animate displayed HP for HUD bar.
    const float lerpRate = 8.0f * dt;
    b.displayHp += (static_cast<float>(b.hp) - b.displayHp) * lerpRate;
    b.displayHp = std::max(b.displayHp, static_cast<float>(b.hp));

    // Entrance animation: slide from above screen to target Y.
    if (!b.entranceDone) {
        b.entranceTimer -= dt;
        const float t = std::clamp(1.0f - (b.entranceTimer / ENTRANCE_DURATION), 0.0f, 1.0f);
        const float startY = -static_cast<float>(b.bounds.h) - 10.0f;
        const float targetY = 48.0f;
        b.pos.y = startY + (targetY - startY) * t;
        if (b.entranceTimer <= 0.0f) {
            b.entranceDone = true;
            b.entranceTimer = 0.0f;
            b.pos.y = targetY;
        }
        return;
    }

    b.moveTimer += dt;
    b.attackTimer -= dt;

    if (b.hp <= b.maxHp / 3) b.phase = 3;
    else if (b.hp <= b.maxHp * 2 / 3) b.phase = 2;
    else b.phase = 1;
    b.enraged = (b.phase == 3);

    const float speedMul = b.enraged ? 2.0f : 1.0f;
    b.patternAngle += dt * 1.4f;

    // Horizontal patrol + occasional reposition.
    b.pos.x = SCREEN_W * 0.5f - b.bounds.w * 0.5f + std::sin(b.moveTimer * (1.1f + b.stageNum * 0.2f)) * 130.0f;
    b.moveCooldown -= dt;
    if (b.moveCooldown <= 0.0f) {
        b.targetPos = {static_cast<float>(20 + (std::rand() % (SCREEN_W - static_cast<int>(b.bounds.w) - 40))), 35.0f + static_cast<float>(std::rand() % 80)};
        b.moveCooldown = 2.5f;
    }
    Vec2 toTarget = b.targetPos - b.pos;
    if (toTarget.length() > 1.0f) {
        b.pos += toTarget.normalized() * (90.0f * speedMul * dt);
    }
    b.pos.x = std::clamp(b.pos.x, 0.0f, static_cast<float>(SCREEN_W) - b.bounds.w);
    b.pos.y = std::clamp(b.pos.y, 20.0f, 140.0f);

    Vec2 origin = {b.pos.x + b.bounds.w * 0.5f, b.pos.y + b.bounds.h * 0.65f};

    if (!b.attackWarning && b.attackTimer <= 0.0f) {
        b.attackWarning = true;
        b.attackWarningTimer = ATTACK_WARNING_DURATION;
    }
    if (b.attackWarning) {
        b.attackWarningTimer -= dt;
        if (b.attackWarningTimer > 0.0f) {
            return;
        }
        b.attackWarning = false;
        b.attackWarningTimer = 0.0f;
    } else {
        return;
    }

    if (b.stageNum == 1) {
        if (b.phase == 1) {
            firePattern(bullets, BulletPattern::CIRCLE_8, origin, playerPos, b.patternAngle, 180.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SINGLE, origin, playerPos, 0.0f, 200.0f, 1, BulletOwner::BOSS);
        } else if (b.phase == 2) {
            firePattern(bullets, BulletPattern::AIMED_SPREAD, origin, playerPos, 0.0f, 200.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPREAD_3, origin, playerPos, -PI * 0.5f, 210.0f, 1, BulletOwner::BOSS);
        } else {
            firePattern(bullets, BulletPattern::SPIRAL_CW, origin, playerPos, b.patternAngle, 220.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CIRCLE_16, origin, playerPos, b.patternAngle, 180.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::AIMED_SPREAD, origin, playerPos, 0.0f, 210.0f, 1, BulletOwner::BOSS);
        }
        b.attackTimer = (b.phase == 1 ? 1.0f : b.phase == 2 ? 0.8f : 0.6f);
    } else if (b.stageNum == 2) {
        if (b.phase == 1) {
            firePattern(bullets, BulletPattern::MISSILE, origin, playerPos, 0.0f, 250.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPREAD_3, origin, playerPos, -PI * 0.5f, 200.0f, 1, BulletOwner::BOSS);
        } else if (b.phase == 2) {
            firePattern(bullets, BulletPattern::MISSILE, origin, playerPos, 0.0f, 260.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::AIMED_BURST, origin, playerPos, 0.0f, 230.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CURTAIN, origin, playerPos, 0.0f, 200.0f, 1, BulletOwner::BOSS);
        } else {
            firePattern(bullets, BulletPattern::MISSILE, origin, playerPos, 0.0f, 260.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 240.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CIRCLE_8, origin, playerPos, b.patternAngle, 210.0f, 1, BulletOwner::BOSS);
        }
        b.attackTimer = (b.phase == 1 ? 0.95f : b.phase == 2 ? 0.75f : 0.5f);
    } else if (b.stageNum == 3) {
        if (b.phase == 1) {
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 260.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPIRAL_CW, origin, playerPos, b.patternAngle, 220.0f, 1, BulletOwner::BOSS);
        } else if (b.phase == 2) {
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 270.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPREAD_LASER, origin, playerPos, -PI * 0.5f, 230.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CURTAIN, origin, playerPos, 0.0f, 220.0f, 1, BulletOwner::BOSS);
        } else {
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 280.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::MISSILE, origin, playerPos, 0.0f, 250.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CIRCLE_16, origin, playerPos, b.patternAngle, 220.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPIRAL_CCW, origin, playerPos, b.patternAngle, 240.0f, 1, BulletOwner::BOSS);
        }
        b.attackTimer = (b.phase == 1 ? 0.85f : b.phase == 2 ? 0.65f : 0.45f);
    } else if (b.stageNum == 4) {
        if (b.phase == 1) {
            firePattern(bullets, BulletPattern::AIMED_BURST, origin, playerPos, 0.0f, 300.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::MISSILE, origin, playerPos, 0.0f, 290.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPIRAL_CW, origin, playerPos, b.patternAngle, 240.0f, 1, BulletOwner::BOSS);
        } else if (b.phase == 2) {
            firePattern(bullets, BulletPattern::SPREAD_LASER, origin, playerPos, -PI * 0.5f, 250.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 290.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CURTAIN, origin, playerPos, 0.0f, 240.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::AIMED_SPREAD, origin, playerPos, 0.0f, 280.0f, 1, BulletOwner::BOSS);
        } else {
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 310.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::MISSILE, origin, playerPos, 0.0f, 300.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CIRCLE_16, origin, playerPos, b.patternAngle, 245.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPIRAL_CW, origin, playerPos, b.patternAngle, 270.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::AIMED_BURST, origin, playerPos, 0.0f, 300.0f, 1, BulletOwner::BOSS);
        }
        b.attackTimer = (b.phase == 1 ? 0.75f : b.phase == 2 ? 0.55f : 0.38f);
    } else {
        if (b.phase == 1) {
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 300.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPIRAL_CW, origin, playerPos, b.patternAngle, 280.0f, 1, BulletOwner::BOSS);
        } else if (b.phase == 2) {
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 320.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CURTAIN, origin, playerPos, 0.0f, 270.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::AIMED_BURST, origin, playerPos, 0.0f, 310.0f, 1, BulletOwner::BOSS);
        } else {
            firePattern(bullets, BulletPattern::HOMING_LASER, origin, playerPos, 0.0f, 320.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CIRCLE_16, origin, playerPos, b.patternAngle, 280.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::SPIRAL_CW, origin, playerPos, b.patternAngle, 310.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::AIMED_SPREAD, origin, playerPos, 0.0f, 320.0f, 1, BulletOwner::BOSS);
            firePattern(bullets, BulletPattern::CURTAIN, origin, playerPos, 0.0f, 280.0f, 1, BulletOwner::BOSS);
        }
        b.attackTimer = (b.phase == 1 ? 0.75f : b.phase == 2 ? 0.55f : 0.38f);
    }
}

void renderBoss(SDL_Renderer* renderer, const AssetManager& assets, const Boss& b) {
    if (!b.active) return;
    renderBossSprite(renderer, assets, static_cast<int>(b.pos.x), static_cast<int>(b.pos.y), b.stageNum, b.lockedOn, b.phase);
}

} // namespace galaxy
