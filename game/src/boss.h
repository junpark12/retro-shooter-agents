#pragma once
#include "entity.h"

namespace galaxy {

struct BulletPool; // defined in bullet.h

// ─── Boss ─────────────────────────────────────────────────────────────────────
struct Boss : Entity {
    int   stageNum    = 1;      // which stage this boss belongs to (1–3)
    int   phase       = 1;      // current attack phase (1–3, increases as HP drops)
    float attackTimer = 0.0f;   // time until next attack
    float moveTimer   = 0.0f;   // used for movement pattern oscillation
    int   maxHp       = 100;    // set by initBoss; used to compute phase thresholds
};

// Initialise the boss for the given stage (sets HP, speed, size, bounds).
void initBoss(Boss& b, int stageNum);

// Update boss movement and attack patterns; fires bullets into pool.
// Player position is used for aimed attacks.
void updateBoss(Boss& b, float dt, BulletPool& bullets, Vec2 playerPos);

// Render the boss sprite (delegates to sprites module).
void renderBoss(SDL_Renderer* renderer, const Boss& b);

} // namespace galaxy
