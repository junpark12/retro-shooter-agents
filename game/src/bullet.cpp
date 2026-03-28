#include "bullet.h"
#include "sprites.h"

#include <cmath>

namespace galaxy {

void fireBullet(BulletPool& bp, Vec2 pos, Vec2 vel, BulletOwner owner, int damage) {
    for (Bullet& b : bp.pool) {
        if (b.active) continue;

        b.active = true;
        b.pos = pos;
        b.vel = vel;
        b.owner = owner;
        b.damage = damage;
        b.hp = 1;
        b.homing = false;
        b.homingTarget = {};
        b.angle = std::atan2(vel.y, vel.x);
        b.angularVel = 0.0f;
        b.colorIdx = (owner == BulletOwner::PLAYER) ? 0 : 1;

        if (owner == BulletOwner::PLAYER) {
            b.bounds = {0.0f, 0.0f, 4.0f, 12.0f};
            b.hitRadius = 2.0f;
        } else {
            b.bounds = {0.0f, 0.0f, 6.0f, 6.0f};
            b.hitRadius = 3.0f;
        }
        return;
    }
}

void fireHomingBullet(BulletPool& bp, Vec2 pos, Vec2 vel, BulletOwner owner,
                      Vec2 target, int damage) {
    for (Bullet& b : bp.pool) {
        if (b.active) continue;
        b.active = true;
        b.pos = pos;
        b.vel = vel;
        b.owner = owner;
        b.damage = damage;
        b.hp = 1;
        b.homing = true;
        b.homingTarget = target;
        b.angle = std::atan2(vel.y, vel.x);
        b.angularVel = 0.0f;
        b.colorIdx = (owner == BulletOwner::PLAYER) ? 2 : 3;
        b.bounds = {0.0f, 0.0f, 6.0f, 6.0f};
        b.hitRadius = 3.0f;
        return;
    }
}

void updateBullets(BulletPool& bp, float dt) {
    for (Bullet& b : bp.pool) {
        if (!b.active) continue;

        if (b.homing) {
            Vec2 toTarget = b.homingTarget - b.pos;
            Vec2 desired = toTarget.normalized();
            Vec2 cur = b.vel.normalized();
            const float turnRate = 3.5f;
            Vec2 blended = (cur * (1.0f - turnRate * dt)) + (desired * (turnRate * dt));
            const float speed = b.vel.length();
            b.vel = blended.normalized() * speed;
        }

        if (b.angularVel != 0.0f) {
            b.angle += b.angularVel * dt;
        } else {
            b.angle = std::atan2(b.vel.y, b.vel.x);
        }

        b.pos += b.vel * dt;

        if (b.pos.x < -24.0f || b.pos.x > static_cast<float>(SCREEN_W + 24) ||
            b.pos.y < -40.0f || b.pos.y > static_cast<float>(SCREEN_H + 40)) {
            b.active = false;
        }
    }
}

void renderBullets(SDL_Renderer* renderer, const BulletPool& bp) {
    for (const Bullet& b : bp.pool) {
        if (!b.active) continue;
        renderBulletPrimitive(renderer, static_cast<int>(b.pos.x), static_cast<int>(b.pos.y),
                              b.owner, b.colorIdx);
    }
}

void clearEnemyBullets(BulletPool& bp) {
    for (Bullet& b : bp.pool) {
        if (b.active && b.owner == BulletOwner::ENEMY) {
            b.active = false;
        }
    }
}

} // namespace galaxy
