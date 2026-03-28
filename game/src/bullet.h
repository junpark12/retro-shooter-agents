#pragma once
#include "entity.h"

namespace galaxy {

// ─── Bullet ───────────────────────────────────────────────────────────────────
struct Bullet : Entity {
    BulletOwner owner    = BulletOwner::PLAYER;
    int         damage   = 1;
    bool        homing   = false;    // if true, slowly curves toward target
    Vec2        homingTarget = {};   // world pos to home toward
    float       angle    = 0.0f;    // rotation angle for rendering (radians)
    float       angularVel = 0.0f;  // for spiral patterns
    int         colorIdx = 0;       // sprite color variant (0–3)
};

// Fixed-size pool — avoids dynamic allocation at runtime.
struct BulletPool {
    Bullet pool[MAX_BULLETS];
};

// Activate the first inactive bullet in the pool with the given parameters.
// If the pool is full, the call is silently ignored.
void fireBullet(BulletPool& bp, Vec2 pos, Vec2 vel, BulletOwner owner, int damage = 1);

// Fire a homing bullet that curves toward a target position.
void fireHomingBullet(BulletPool& bp, Vec2 pos, Vec2 vel, BulletOwner owner,
                      Vec2 target, int damage = 1);

// Advance all active bullets by dt seconds; deactivate out-of-screen ones.
// Homing bullets gradually adjust velocity toward their target.
void updateBullets(BulletPool& bp, float dt);

// Render all active bullets.
void renderBullets(SDL_Renderer* renderer, const BulletPool& bp);

// Deactivate all enemy bullets (used when bomb is activated).
void clearEnemyBullets(BulletPool& bp);

} // namespace galaxy
