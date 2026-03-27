#include "bullet.h"
#include "sprites.h"

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

        if (owner == BulletOwner::PLAYER) {
            b.bounds = {0.0f, 0.0f, 4.0f, 12.0f};
        } else {
            b.bounds = {0.0f, 0.0f, 6.0f, 6.0f};
        }
        return;
    }
}

void updateBullets(BulletPool& bp, float dt) {
    for (Bullet& b : bp.pool) {
        if (!b.active) continue;

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
        renderBulletSprite(renderer, static_cast<int>(b.pos.x), static_cast<int>(b.pos.y), b.owner);
    }
}

} // namespace galaxy
