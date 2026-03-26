#pragma once
#include "entity.h"

namespace galaxy {

// ─── Bullet ───────────────────────────────────────────────────────────────────
struct Bullet : Entity {
    BulletOwner owner  = BulletOwner::PLAYER;
    int         damage = 1;
};

// Fixed-size pool — avoids dynamic allocation at runtime.
struct BulletPool {
    Bullet pool[MAX_BULLETS];
};

// Activate the first inactive bullet in the pool with the given parameters.
// If the pool is full, the call is silently ignored.
void fireBullet(BulletPool& bp, Vec2 pos, Vec2 vel, BulletOwner owner, int damage = 1);

// Advance all active bullets by dt seconds; deactivate out-of-screen ones.
void updateBullets(BulletPool& bp, float dt);

// Render all active bullets.
void renderBullets(SDL_Renderer* renderer, const BulletPool& bp);

} // namespace galaxy
