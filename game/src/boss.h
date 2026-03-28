#pragma once
#include "entity.h"

namespace galaxy {

struct BulletPool; // defined in bullet.h

// ─── Boss attack phase descriptor ─────────────────────────────────────────────
struct BossPhase {
    int          hpThreshold;  // enter this phase when HP drops below this value
    BulletPattern pattern;     // primary attack pattern for this phase
    float        attackInterval; // seconds between attacks
    float        moveSpeed;    // movement speed during this phase
};

// ─── Boss ─────────────────────────────────────────────────────────────────────
struct Boss : Entity {
    int   stageNum       = 1;      // which stage this boss belongs to (1–3)
    int   phase          = 1;      // current attack phase (1–3, increases as HP drops)
    float attackTimer    = 0.0f;   // time until next attack
    float moveTimer      = 0.0f;   // used for movement pattern oscillation
    int   maxHp          = 100;    // set by initBoss; used to compute phase thresholds
    float patternAngle   = 0.0f;   // rotating angle for spiral/circle patterns
    int   burstCount     = 0;      // shots fired in current burst
    float burstTimer     = 0.0f;   // time between burst shots
    bool  enraged        = false;  // true when below 20% HP (final phase)
    Vec2  targetPos      = {};     // movement target
    float moveCooldown   = 0.0f;   // time until next repositioning
    bool  lockedOn       = false;  // player has locked onto this boss
};

// Initialise the boss for the given stage (sets HP, speed, size, bounds).
void initBoss(Boss& b, int stageNum);

// Update boss movement and attack patterns; fires bullets into pool.
// Player position is used for aimed attacks.
void updateBoss(Boss& b, float dt, BulletPool& bullets, Vec2 playerPos);

// Render the boss sprite (delegates to sprites module).
void renderBoss(SDL_Renderer* renderer, const Boss& b);

} // namespace galaxy
