// Galaxy Storm — Bullet object pool implementation

#include "bullet.h"
#include "sprites.h"

namespace galaxy {

void fireBullet(BulletPool& bp, Vec2 pos, Vec2 vel, BulletOwner owner, int damage) {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet& b = bp.pool[i];
        if (!b.active) {
            b.pos    = pos;
            b.vel    = vel;
            b.owner  = owner;
            b.damage = damage;
            b.active = true;
            // Set collision bounds based on owner
            if (owner == BulletOwner::PLAYER) {
                b.bounds = { 0.0f, 0.0f, 4.0f, 12.0f };
            } else {
                b.bounds = { 0.0f, 0.0f, 6.0f, 6.0f };
            }
            return;
        }
    }
    // Pool is full — silently ignore
}

void updateBullets(BulletPool& bp, float dt) {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet& b = bp.pool[i];
        if (!b.active) continue;

        b.pos += b.vel * dt;

        // Deactivate bullets that leave the screen
        if (b.pos.y + b.bounds.h < 0.0f ||
            b.pos.y > static_cast<float>(SCREEN_H) ||
            b.pos.x + b.bounds.w < 0.0f ||
            b.pos.x > static_cast<float>(SCREEN_W)) {
            b.active = false;
        }
    }
}

void renderBullets(SDL_Renderer* renderer, const BulletPool& bp) {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        const Bullet& b = bp.pool[i];
        if (!b.active) continue;
        renderBulletSprite(renderer,
                           static_cast<int>(b.pos.x),
                           static_cast<int>(b.pos.y),
                           b.owner);
    }
}

} // namespace galaxy
